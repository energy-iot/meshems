/**
 * @file modbus_dds238.cpp
 * @brief Implementation of DDS238 Single Phase Energy Meter Modbus Interface
 * 
 * This file implements the Modbus RTU communication protocol for the DDS238
 * single-phase energy meter. It provides functions to read electrical parameters
 * including voltage, current, power, energy consumption, and power quality metrics.
 */

#include <modbus_dds238.h>
#include <TimeLib.h>
#include <data_model.h>

// Configuration Constants
#define PAUSE_ON_RAMP_LEVELS 30000  ///< Delay for power ramping (unused)

/**
 * @brief Constructor for DDS238 Modbus interface
 * 
 * Initializes the DDS238 Modbus interface with default values.
 * The Modbus address must be set separately using set_modbus_address().
 */
Modbus_DDS238::Modbus_DDS238() {
    modbus_address = 1;  // Default address
    timestamp_last_report = 0;
    timestamp_last_failure = 0;
}

/**
 * @brief Get the current Modbus slave address
 * @return Current Modbus address (1-247)
 */
uint8_t Modbus_DDS238::get_modbus_address() {
    return modbus_address;
}

/**
 * @brief Set the Modbus slave address for the DDS238 meter
 * @param addr Modbus address (1-247)
 */
void Modbus_DDS238::set_modbus_address(uint8_t addr) {
    modbus_address = addr;
}

/**
 * @brief Read a single 16-bit register value from the DDS238 meter
 * 
 * Reads one holding register and returns the raw value. This function
 * includes error handling and will throw an exception if communication fails.
 * 
 * @param registerAddress Modbus register address to read
 * @return Raw register value as float
 * @throws std::runtime_error if Modbus communication fails
 */
float Modbus_DDS238::read_modbus_value(uint16_t registerAddress) {
    uint8_t result = readHoldingRegisters(registerAddress, 1);
    if (result != ku8MBSuccess) {
        Serial.printf("DDS238: Failed to read register 0x%04X (error code: %d)\n", 
                     registerAddress, result);
        timestamp_last_failure = millis();
        throw std::runtime_error("Modbus read error");
    }
    return static_cast<float>(getResponseBuffer(0));
}

/**
 * @brief Read a 32-bit value from two consecutive registers
 * 
 * Reads two consecutive holding registers and combines them into a 32-bit value.
 * The first register contains the high word, the second contains the low word.
 * 
 * @param registerAddress Starting Modbus register address
 * @return Combined 32-bit value as float
 * @throws std::runtime_error if Modbus communication fails
 */
float Modbus_DDS238::read_modbus_extended_value(uint16_t registerAddress) {
    uint8_t result = readHoldingRegisters(registerAddress, 2);
    if (result != ku8MBSuccess) {
        Serial.printf("DDS238: Failed to read extended register 0x%04X (error code: %d)\n", 
                     registerAddress, result);
        timestamp_last_failure = millis();
        throw std::runtime_error("Modbus read error");
    }
    
    // Combine high and low words into 32-bit value
    uint32_t combined = (static_cast<uint32_t>(getResponseBuffer(0)) << 16) + 
                       getResponseBuffer(1);
    return static_cast<float>(combined);
}

/**
 * @brief Poll all electrical parameters from the DDS238 meter
 * 
 * Reads all available electrical parameters from the DDS238 energy meter
 * and returns them in a structured PowerData format. This function handles
 * scaling factors to convert raw register values to engineering units.
 * 
 * Scaling factors applied:
 * - Energy values: divided by 100 (register units: 1/100 kWh)
 * - Voltage: divided by 10 (register units: 1/10 V)
 * - Current: divided by 100 (register units: 1/100 A)
 * - Power: divided by 1000 (register units: 1 W, converted to kW)
 * - Power factor: divided by 1000 (register units: 1/1000)
 * - Frequency: divided by 100 (register units: 1/100 Hz)
 * 
 * @return PowerData structure containing all measured values with proper scaling
 */
PowerData Modbus_DDS238::poll() {
    PowerData current_reading;
    
    try {
        // Read energy values (32-bit registers)
        current_reading.total_energy = read_modbus_extended_value(rTOTAL_ENERGY) / 100.0f;
        current_reading.export_energy = read_modbus_extended_value(rEXPORT_ENERGY_LOW) / 100.0f;
        current_reading.import_energy = read_modbus_extended_value(rIMPORT_ENERGY_LOW) / 100.0f;
        
        // Read instantaneous electrical parameters (16-bit registers)
        current_reading.voltage = read_modbus_value(rVOLTAGE) / 10.0f;
        current_reading.current = read_modbus_value(rCURRENT) / 100.0f;
        current_reading.active_power = read_modbus_value(rACTIVE_POWER) / 1000.0f;  // Convert W to kW
        current_reading.reactive_power = read_modbus_value(rREACTIVE_POWER) / 1000.0f;  // Convert VAr to kVAr
        current_reading.power_factor = read_modbus_value(rPOWER_FACTOR) / 1000.0f;
        current_reading.frequency = read_modbus_value(rFREQUENCY) / 100.0f;
        
        // Read metadata and timestamp
        current_reading.metadata = static_cast<uint16_t>(read_modbus_value(rMETADATA));
        current_reading.timestamp_last_report = now();
        
        // Update cached reading
        last_reading = current_reading;
        timestamp_last_report = millis();
        
        // Log successful reading
        Serial.printf("DDS238: Poll successful - V:%.1fV I:%.2fA P:%.3fkW E:%.2fkWh\n",
                     current_reading.voltage, current_reading.current, 
                     current_reading.active_power, current_reading.total_energy);
        
    } catch (const std::runtime_error& e) {
        Serial.printf("DDS238: Poll failed - %s\n", e.what());
        // Return last known good reading on error
        current_reading = last_reading;
    }
    
    return current_reading;
}

// Individual Parameter Getter Functions
// These functions return the last successfully read values

/**
 * @brief Get total energy consumption from last successful reading
 * @return Total energy in kWh
 */
float Modbus_DDS238::getTotalEnergy() {
    return last_reading.total_energy;
}

/**
 * @brief Get exported energy from last successful reading
 * @return Exported energy in kWh
 */
float Modbus_DDS238::getExportEnergy() {
    return last_reading.export_energy;
}

/**
 * @brief Get imported energy from last successful reading
 * @return Imported energy in kWh
 */
float Modbus_DDS238::getImportEnergy() {
    return last_reading.import_energy;
}

/**
 * @brief Get line voltage from last successful reading
 * @return Voltage in Volts
 */
float Modbus_DDS238::getVoltage() {
    return last_reading.voltage;
}

/**
 * @brief Get line current from last successful reading
 * @return Current in Amperes
 */
float Modbus_DDS238::getCurrent() {
    return last_reading.current;
}

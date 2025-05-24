#pragma once

/**
 * @file modbus_dds238.h
 * @brief DDS238 Single Phase Energy Meter Modbus Interface
 * 
 * This class provides a Modbus RTU interface for the DDS238 single-phase
 * energy meter. It handles reading electrical parameters including voltage,
 * current, power, energy consumption, and power quality metrics.
 * 
 * The DDS238 is a single-phase energy meter that communicates via Modbus RTU
 * and provides real-time electrical measurements and accumulated energy data.
 */

#include <modbus_master.h>
#include "data_model.h"

/**
 * @class Modbus_DDS238
 * @brief Modbus interface for DDS238 single-phase energy meter
 * 
 * Inherits from ModbusMaster to provide specific functionality for
 * communicating with DDS238 energy meters via Modbus RTU protocol.
 */
class Modbus_DDS238 : public ModbusMaster {
public:
    /**
     * @brief Constructor for DDS238 Modbus interface
     */
    Modbus_DDS238();
    
    /**
     * @brief Destructor
     */
    ~Modbus_DDS238() {};

    // Address Management
    /**
     * @brief Get the current Modbus slave address
     * @return Current Modbus address (1-247)
     */
    uint8_t get_modbus_address();
    
    /**
     * @brief Set the Modbus slave address
     * @param addr Modbus address (1-247)
     */
    void set_modbus_address(uint8_t addr);

    // Register Reading Functions
    /**
     * @brief Read a single 16-bit register value
     * @param registerAddress Modbus register address
     * @return Raw register value as float
     * @throws std::runtime_error if Modbus communication fails
     */
    float read_modbus_value(uint16_t registerAddress);
    
    /**
     * @brief Read a 32-bit value from two consecutive registers
     * @param registerAddress Starting Modbus register address
     * @return Combined 32-bit value as float
     * @throws std::runtime_error if Modbus communication fails
     */
    float read_modbus_extended_value(uint16_t registerAddress);

    /**
     * @brief DDS238 Modbus Register Map
     * 
     * Defines the register addresses and their scaling factors for the DDS238
     * energy meter. All values are read as holding registers.
     */
    enum MB_Reg {
        rTOTAL_ENERGY = 0x00,        ///< Total energy consumption (1/100 kWh)
        rEXPORT_ENERGY_LOW = 0x08,   ///< Export energy low word (1/100 kWh)
        rEXPORT_ENERGY_HIGH = 0x09,  ///< Export energy high word (1/100 kWh)
        rIMPORT_ENERGY_LOW = 0x0A,   ///< Import energy low word (1/100 kWh)
        rIMPORT_ENERGY_HIGH = 0x0B,  ///< Import energy high word (1/100 kWh)
        rVOLTAGE = 0x0C,             ///< Line voltage (1/10 V)
        rCURRENT = 0x0D,             ///< Line current (1/100 A)
        rACTIVE_POWER = 0x0E,        ///< Active power (1 W)
        rREACTIVE_POWER = 0x0F,      ///< Reactive power (1 VAr)
        rPOWER_FACTOR = 0x10,        ///< Power factor (1/1000)
        rFREQUENCY = 0x11,           ///< Line frequency (1/100 Hz)
        rMETADATA = 0x15             ///< Device metadata (address: high byte, version: low byte)
    };

    // Data Polling and Access
    /**
     * @brief Poll all electrical parameters from the DDS238 meter
     * @return PowerData structure containing all measured values
     * 
     * Reads all available electrical parameters from the meter and returns
     * them in a structured format. Handles scaling and unit conversion.
     */
    PowerData poll();

    // Individual Parameter Getters
    /**
     * @brief Get total energy consumption
     * @return Total energy in kWh
     */
    float getTotalEnergy();
    
    /**
     * @brief Get exported energy
     * @return Exported energy in kWh
     */
    float getExportEnergy();
    
    /**
     * @brief Get imported energy
     * @return Imported energy in kWh
     */
    float getImportEnergy();
    
    /**
     * @brief Get line voltage
     * @return Voltage in Volts
     */
    float getVoltage();
    
    /**
     * @brief Get line current
     * @return Current in Amperes
     */
    float getCurrent();

private:
    uint8_t modbus_address;              ///< Modbus slave address (1-247)
    unsigned long timestamp_last_report; ///< Timestamp of last successful reading
    unsigned long timestamp_last_failure;///< Timestamp of last communication failure
    PowerData last_reading;              ///< Cache of last successful reading
};

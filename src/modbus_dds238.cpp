#include <modbus_dds238.h>
#include <TimeLib.h>
#include <data_model.h>
//#include <DTMPowerCache.h>
#define PAUSE_ON_RAMP_LEVELS 30000

/*
DDS328 is a single phase 240VAC Class 1 meter used per tenant in the streetpoleEMS subpanel
we read data in a loop every n secs/mins then cache this data including totalizers and stats on working and failed reads
*/

Modbus_DDS238::Modbus_DDS238() {
}

uint8_t Modbus_DDS238::get_modbus_address() {
    return modbus_address;
}

void Modbus_DDS238::set_modbus_address(uint8_t addr) {
    modbus_address = addr;
}

// Function to read a Modbus value and throw an exception if it fails
float Modbus_DDS238::read_modbus_value(uint16_t registerAddress) {
    uint8_t result = readHoldingRegisters(registerAddress, 1);
    if (result != ku8MBSuccess) {
        Serial.printf("MODBUS DDS238: Error reading register %d\n", registerAddress);
        throw std::runtime_error("Modbus read error");
    }
    return getResponseBuffer(0);
}

float Modbus_DDS238::read_modbus_extended_value(uint16_t registerAddress) {
    uint8_t result = readHoldingRegisters(registerAddress, 2);
    if (result != ku8MBSuccess) {
        Serial.printf("MODBUS DDS238: Error reading register %d\n", registerAddress);
        throw std::runtime_error("Modbus read error");
    }
    return (float)(getResponseBuffer(0) << 16 + getResponseBuffer(1));
}

void Modbus_DDS238::poll() {
    // Create a PowerData struct and populate it by making getResponseBuffer() calls
    try {
        last_reading.total_energy = read_modbus_extended_value(rTOTAL_ENERGY)/100;
        last_reading.export_energy = read_modbus_extended_value(rEXPORT_ENERGY_LOW)/100;
        last_reading.import_energy = read_modbus_extended_value(rIMPORT_ENERGY_LOW)/100;
        last_reading.voltage = read_modbus_value(rVOLTAGE)/10;
        last_reading.current = read_modbus_value(rCURRENT)/100;
        last_reading.active_power = read_modbus_value(rACTIVE_POWER)/1000;
        last_reading.reactive_power = read_modbus_value(rREACTIVE_POWER)/1000;
        last_reading.power_factor = read_modbus_value(rPOWER_FACTOR)/1000;
        last_reading.frequency = read_modbus_value(rFREQUENCY)/100;
        last_reading.timestamp_last_report = now();
        last_reading.metadata = read_modbus_value(rMETADATA);

        Serial.printf("MODBUS DDS238: Total Energy: %.2f kWh\n", last_reading.total_energy);
        Serial.printf("MODBUS DDS238: Export Energy: %.2f kWh\n", last_reading.export_energy);
        Serial.printf("MODBUS DDS238: Import Energy: %.2f kWh\n", last_reading.import_energy);
        Serial.printf("MODBUS DDS238: Voltage: %.2f V\n", last_reading.voltage);
        Serial.printf("MODBUS DDS238: Current: %.2f A\n", last_reading.current);
        Serial.printf("MODBUS DDS238: Active Power: %.3f kW\n", last_reading.active_power);
        Serial.printf("MODBUS DDS238: Reactive Power: %.3f kVAr\n", last_reading.reactive_power);
        Serial.printf("MODBUS DDS238: Power Factor: %.3f\n", last_reading.power_factor);
        Serial.printf("MODBUS DDS238: Frequency: %.2f Hz\n", last_reading.frequency);
        Serial.printf("MODBUS DDS238: Metadata: %d\n", last_reading.metadata);
    } catch (std::runtime_error& e) {
        Serial.println("MODBUS DDS238: Error reading registers");
    }
}

 float Modbus_DDS238::getTotalEnergy() {
    return last_reading.total_energy;
 }

float Modbus_DDS238::getExportEnergy() {
    return last_reading.export_energy;
}
float Modbus_DDS238::getImportEnergy() {
    return last_reading.import_energy;
}

float Modbus_DDS238::getVoltage() {
    return last_reading.voltage;
}

float Modbus_DDS238::getCurrent() {
    return last_reading.current;
}

float Modbus_DDS238::getActivePower() {
    return last_reading.active_power;
}

float Modbus_DDS238::getPowerFactor() {
    return last_reading.power_factor;
}

float Modbus_DDS238::getFrequency() {
    return last_reading.frequency;
}
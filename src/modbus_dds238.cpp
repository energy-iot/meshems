#include <modbus_dds238.h>
#include <TimeLib.h>

#define PAUSE_ON_RAMP_LEVELS 30000

Modbus_DDS238::Modbus_DDS238() {
}

uint8_t Modbus_DDS238::get_modbus_address() {
    return modbus_address;
}

void Modbus_DDS238::set_modbus_address(uint8_t addr) {
    modbus_address = addr;
}

// Function to read a Modbus value and throw an exception if it fails
uint16_t Modbus_DDS238::read_modbus_value(int registerAddress) {
    uint8_t result = readHoldingRegisters(registerAddress, 1);
    if (result != ku8MBSuccess) {
        Serial.printf("MODBUS DDS238: Error reading register %d\n", registerAddress);
        throw std::runtime_error("Modbus read error");
    }
    return getResponseBuffer(0);
}

void Modbus_DDS238::poll() {
    // Create a PowerData struct and populate it by making getResponseBuffer() calls
    try {
        last_reading.total_energy = getResponseBuffer(rTOTAL_ENERGY);
        last_reading.export_energy = getResponseBuffer(rEXPORT_ENERGY_LOW) + (getResponseBuffer(rEXPORT_ENERGY_HIGH) << 16);
        last_reading.import_energy = getResponseBuffer(rIMPORT_ENERGY_LOW) + (getResponseBuffer(rIMPORT_ENERGY_HIGH) << 16);
        last_reading.voltage = getResponseBuffer(rVOLTAGE);
        last_reading.current = getResponseBuffer(rCURRENT);
        last_reading.active_power = getResponseBuffer(rACTIVE_POWER);
        last_reading.reactive_power = getResponseBuffer(rREACTIVE_POWER);
        last_reading.power_factor = getResponseBuffer(rPOWER_FACTOR);
        last_reading.frequency = getResponseBuffer(rFREQUENCY);
        last_reading.timestamp_last_report = now();
    } catch (std::runtime_error& e) {
        Serial.println("MODBUS DDS238: Error reading registers");
    }
}
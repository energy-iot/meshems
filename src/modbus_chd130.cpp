#include <modbus_chd130.h>
#include <TimeLib.h>
#include <data_model.h>
#include <DTMPowerCache.h>

Modbus_CHD130::Modbus_CHD130() {}
/*

TODO populat and support apparent_power or reactive_power

Next Step Suggestions
Scaling Factors – Once you share the CHD130 Modbus register spec, I’ll patch in actual scaling factors (e.g., /1000, /10, or signed 32-bit handling).

Multiple Instances – You can now define Modbus_CHD130 chd130_1;, chd130_2;, etc. in your .ino or firmware file.

*/
uint8_t Modbus_CHD130::get_modbus_address() {
    return modbus_address;
}

void Modbus_CHD130::set_modbus_address(uint8_t addr) {
    modbus_address = addr;
}

float Modbus_CHD130::read_modbus_value(uint16_t registerAddress) {
    uint8_t result = readHoldingRegisters(registerAddress, 1);
    if (result != ku8MBSuccess) {
        Serial.printf("MODBUS CHD130: Error reading register 0x%04X\n", registerAddress);
        throw std::runtime_error("Modbus read error");
    }
    return getResponseBuffer(0);
}

float Modbus_CHD130::read_modbus_extended_value(uint16_t registerAddress) {
    uint8_t result = readHoldingRegisters(registerAddress, 2);
    if (result != ku8MBSuccess) {
        Serial.printf("MODBUS CHD130: Error reading register 0x%04X\n", registerAddress);
        throw std::runtime_error("Modbus read error");
    }
    return (float)((getResponseBuffer(0) << 16) | getResponseBuffer(1));
}

void Modbus_CHD130::poll() {
    try {
        last_reading.total_energy   = read_modbus_extended_value(rTOTAL_ENERGY);       // Scaling TBD
        last_reading.import_energy  = read_modbus_extended_value(rIMPORT_ENERGY);      // Scaling TBD
        last_reading.export_energy  = read_modbus_extended_value(rEXPORT_ENERGY);      // Scaling TBD
        last_reading.voltage        = read_modbus_value(rVOLTAGE);                     // e.g. /10
        last_reading.current        = read_modbus_value(rCURRENT);                     // e.g. /100
        last_reading.active_power   = read_modbus_value(rACTIVE_POWER);                // e.g. /1000
       // last_reading.apparent_power = read_modbus_value(rAPPARENT_POWER);              // e.g. /1000
        last_reading.reactive_power = read_modbus_value(rREACTIVE_POWER);              // e.g. /1000
        last_reading.power_factor   = read_modbus_value(rPOWER_FACTOR);                // e.g. /1000
        last_reading.frequency      = read_modbus_value(rFREQUENCY);                   // e.g. /100
        last_reading.timestamp_last_report = now();
        last_reading.metadata       = read_modbus_value(rMETADATA);

        Serial.printf("MODBUS CHD130: Total Energy: %.2f\n", last_reading.total_energy);
        Serial.printf("MODBUS CHD130: Import Energy: %.2f\n", last_reading.import_energy);
        Serial.printf("MODBUS CHD130: Export Energy: %.2f\n", last_reading.export_energy);
        Serial.printf("MODBUS CHD130: Voltage: %.2f V\n", last_reading.voltage);
        Serial.printf("MODBUS CHD130: Current: %.2f A\n", last_reading.current);
        Serial.printf("MODBUS CHD130: Active Power: %.2f W\n", last_reading.active_power);
        //Serial.printf("MODBUS CHD130: Apparent Power: %.2f VA\n", last_reading.apparent_power);
        Serial.printf("MODBUS CHD130: Reactive Power: %.2f VAr\n", last_reading.reactive_power);
        Serial.printf("MODBUS CHD130: Power Factor: %.3f\n", last_reading.power_factor);
        Serial.printf("MODBUS CHD130: Frequency: %.2f Hz\n", last_reading.frequency);
        Serial.printf("MODBUS CHD130: Metadata: %d\n", last_reading.metadata);

    } catch (std::runtime_error& e) {
        Serial.println("MODBUS CHD130: Error reading registers");
    }
}

// Getters
float Modbus_CHD130::getTotalEnergy()    { return last_reading.total_energy; }
float Modbus_CHD130::getImportEnergy()   { return last_reading.import_energy; }
float Modbus_CHD130::getExportEnergy()   { return last_reading.export_energy; }
float Modbus_CHD130::getVoltage()        { return last_reading.voltage; }
float Modbus_CHD130::getCurrent()        { return last_reading.current; }
float Modbus_CHD130::getActivePower()    { return last_reading.active_power; }
//float Modbus_CHD130::getApparentPower()  { return last_reading.apparent_power; }
float Modbus_CHD130::getReactivePower()  { return last_reading.reactive_power; }
float Modbus_CHD130::getPowerFactor()    { return last_reading.power_factor; }
float Modbus_CHD130::getFrequency()      { return last_reading.frequency; }

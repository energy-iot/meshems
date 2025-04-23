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

void Modbus_DDS238::route_poll_response(uint16_t reg, uint16_t response) {
    switch (reg) {
        case rVOLTAGE :
            Serial.printf("MODBUS DDS238: Voltage: %2.1f\n", response * 10);
            voltage = response;
            break;
        case rPOWER_FACTOR :
            Serial.printf("MODBUS DDS238: Power Factor %2.1f%\n", response * 1000);
            power_factor = response;
            break;
        default:break;
    }
}

float Modbus_DDS238::getVoltage() {
    return voltage;
}
float Modbus_DDS238::getCurrent() {
    return current;
}

uint8_t Modbus_DDS238::poll() {
    //read two registers: 1=temp, 2=humidity. Divide responses by 10.
    uint8_t result = readHoldingRegisters(rCURRENT, 1);
    if (result == ku8MBSuccess) {
#ifdef ENABLE_DEBUG_MODBUS
        Serial.printf("MODBUS SHT20: addr:%d reg:0x001 value:%d temperature:%2.1fC\n", modbus_address, getResponseBuffer(0), getResponseBuffer(0)/10.0); 
#endif
        timestamp_last_report = now();
        current = getResponseBuffer(0);
        //route_poll_response(reg, getResponseBuffer(0); //TODO parse multi-byte response in route_poll_response instead of here
        //Serial.printf("MODBUS DDS238: Current: %2.1f\n", current*100);

    } else {
        timestamp_last_failure = now();
        Serial.println("MODBUS DDS238 POLL FAIL");
    }
    
    return result;
}

uint8_t Modbus_DDS238::query_register(uint16_t reg) {

    uint8_t result = readInputRegisters(reg, 1);
#ifdef ENABLE_DEBUG_MODBUS
    Serial.printf("MODBUS SHT20 %d: readInputRegisters( %d ): %s\n", modbus_address, reg, (result == ku8MBSuccess) ? "OK" : "FAIL");
#endif
    if (result == ku8MBSuccess) {
        route_poll_response(reg, getResponseBuffer(0));
    } else {
        timestamp_last_failure = now();
    }
    
    return result;
}
#include <modbus_sht20.h>
#include <TimeLib.h>

#define PAUSE_ON_RAMP_LEVELS 30000

Modbus_SHT20::Modbus_SHT20() {
    temperature = -1;
    humidity = -1;
}

uint8_t Modbus_SHT20::get_modbus_address() {
    return modbus_address;
}

void Modbus_SHT20::set_modbus_address(uint8_t addr) {
    modbus_address = addr;
}
float Modbus_SHT20::getTemperature() { return temperature/10.0; }
float Modbus_SHT20::getHumidity() { return humidity/10.0; }
float Modbus_SHT20::getRawTemperature() { return temperature; }
float Modbus_SHT20::getRawHumidity() { return humidity; }

void Modbus_SHT20::route_poll_response(uint16_t reg, uint16_t response) {
    switch (reg) {
        case rTEMPERATURE :
            Serial.printf("MODBUS SHT20: Temperature: %2.1fC\n", response/10.0);
            temperature = response;
            break;
        case rHUMIDITY :
            Serial.printf("MODBUS SHT20: Humidity %2.1f%%\n", response/10.0);
            humidity = response;
            break;
        default:break;
    }
}

uint8_t Modbus_SHT20::poll() {
    //read two registers: 1=temp, 2=humidity. Divide responses by 10.
    uint8_t result = readInputRegisters(rTEMPERATURE, 2);
    if (result == ku8MBSuccess) {
#ifdef ENABLE_DEBUG_MODBUS
        Serial.printf("MODBUS SHT20: addr:%d reg:0x001 value:%d temperature:%2.1fC\n", modbus_address, getResponseBuffer(0), getResponseBuffer(0)/10.0); 
#endif
        timestamp_last_report = now();
        temperature = getResponseBuffer(0);
        humidity = getResponseBuffer(1);
        //route_poll_response(reg, getResponseBuffer(0); //TODO parse multi-byte response in route_poll_response instead of here
        Serial.printf("MODBUS SHT20: Temperature: %2.1fC  Humidity:%2.1f%%\n", temperature/10.0, humidity/10.0);

    } else {
        timestamp_last_failure = now();
        Serial.println("MODBUS SHT20 POLL FAIL");
    }
    
    return result;
}

uint8_t Modbus_SHT20::query_register(uint16_t reg) {

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
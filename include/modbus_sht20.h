#pragma once

#include <modbus_master.h>

class Modbus_SHT20 : public ModbusMaster {
    public:
        Modbus_SHT20();
        ~Modbus_SHT20() {};

        uint8_t poll(); //return num polled
        void route_poll_response(uint16_t reg, uint16_t response);
        uint8_t get_modbus_address();
        void set_modbus_address(uint8_t addr);
        uint8_t query_register(uint16_t reg);

        enum MB_Reg {
          rTEMPERATURE = 1,
          rHUMIDITY
        };

        float getTemperature();
        float getHumidity();

    private:
        uint8_t modbus_address;
        unsigned long timestamp_last_report;
        unsigned long timestamp_last_failure;
        uint16_t temperature;
        uint16_t humidity;
};
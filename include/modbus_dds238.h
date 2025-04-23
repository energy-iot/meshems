#pragma once

#include <modbus_master.h>

class Modbus_DDS238 : public ModbusMaster {
    public:
        Modbus_DDS238();
        ~Modbus_DDS238() {};

        //void route_poll_response(uint16_t reg, uint16_t response);
        uint8_t get_modbus_address();
        void set_modbus_address(uint8_t addr);
        //uint8_t query_register(uint16_t reg);
        uint16_t read_modbus_value(int registerAddress);

        enum MB_Reg {
            rTOTAL_ENERGY = 0,          // 1/100 kWh
            rEXPORT_ENERGY_LOW = 8,     
            rEXPORT_ENERGY_HIGH = 9,
            rIMPORT_ENERGY_LOW = 0xA,   // 1/100 kWh
            rIMPORT_ENERGY_HIGH = 0xB,
            rVOLTAGE = 0xC,             // 1/10 V
            rCURRENT = 0xD,             // 1/100 A
            rACTIVE_POWER = 0xE,        // 1W
            rREACTIVE_POWER = 0xF,      // 1VAr
            rPOWER_FACTOR = 0x10,       // 1/1000
            rFREQUENCY = 0x11           // 1/100 Hz
        };

        // Struct for current, voltage, and power factor
        struct PowerData {
            unsigned long timestamp_last_report;
            unsigned int total_energy;  // Wh
            unsigned int export_energy; // Wh
            unsigned int import_energy; // Wh
            unsigned int voltage;       // mV
            unsigned int current;       // mA
            unsigned int active_power;  // W
            unsigned int reactive_power; // VAr
            unsigned int power_factor;  // 0.001
            unsigned int frequency;     // 0.01 Hz
        };

        void poll(); //return num polled
        PowerData last_reading;

        // float getTotalEnergy();
        // float getExportEnergy();
        // float getImportEnergy();
        // float getVoltage();
        // float getCurrent();
    private:
        uint8_t modbus_address;
        unsigned long timestamp_last_report;
        unsigned long timestamp_last_failure;

        // float voltage;
        // float current;
        // float power_factor;
        // float total_enery;
        // float export_energy;
};
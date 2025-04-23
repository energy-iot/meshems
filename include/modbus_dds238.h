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
        float read_modbus_value(uint16_t registerAddress);

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
            rFREQUENCY = 0x11,           // 1/100 Hz
            rMETADATA = 0x15          // 1-247 (high byte), 1-16 (low byte)
        };

        // Struct for current, voltage, and power factor
        struct PowerData {
            unsigned long timestamp_last_report = 0;
            float total_energy = 0;  // Wh
            float export_energy = 0; // Wh
            float import_energy = 0; // Wh
            float voltage = 0;       // mV
            float current = 0;       // mA
            float active_power = 0;  // W
            float reactive_power = 0; // VAr
            float power_factor = 0;  // 0.001
            float frequency = 0;     // 0.01 Hz
            float metadata = 0;      // 1-247 (high byte), 1-16 (low byte)
        };

        void poll(); //return num polled
        PowerData last_reading;

        float getTotalEnergy();
        float getExportEnergy();
        float getImportEnergy();
        float getVoltage();
        float getCurrent();
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
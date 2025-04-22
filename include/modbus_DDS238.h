#pragma once

#include <modbus_master.h>

class Modbus_DDS238 : public ModbusMaster {
    public:
        Modbus_DDS238();
        ~Modbus_DDS238() {};

        uint8_t poll(); //return num polled
        void route_poll_response(uint16_t reg, uint16_t response);
        uint8_t get_modbus_address();
        void set_modbus_address(uint8_t addr);
        uint8_t query_register(uint16_t reg);

        enum MB_Reg {
            rTOTAL_ENERGY = 0,
            rEXPORT_ENERGY_LOW = 8,
            rEXPORT_ENERGY_HIGH,
            rIMPORT_ENERGY_LOW = 10,
            rIMPORT_ENERGY_HIGH,
            rVOLTAGE = 12,
            rCURRENT,
            rACTIVE_POWER,
            rREACTIVE_POWER,
            rPOWER_FACTOR,
            rFREQUENCY
        };
        float getTotalEnergy();
        float getExportEnergy();
        float getImportEnergy();
        float getVoltage();
        float getCurrent();
    private:
        uint8_t modbus_address;
        unsigned long timestamp_last_report;
        unsigned long timestamp_last_failure;

        float voltage;
        float current;
        float power_factor;
        float total_enery;
        float export_energy;
};
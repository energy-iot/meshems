#pragma once

#include <modbus/modbus_master.h>
#include <data_model.h>

#define NESLDTM_REG_START  1000
#define NESLDTM_REG_END    1068
#define NESLDTM_REG_COUNT  (NESLDTM_REG_END - NESLDTM_REG_START + 1)  // 69 registers

// reg_cache[] offsets — from dtm-ems modbus_slave.cpp
#define NESLDTM_IDX_BOARD_STATUS   0   // bit0=meter OK, bit2=net up
#define NESLDTM_IDX_SYS_STATUS0    1
#define NESLDTM_IDX_SYS_STATUS1    2
#define NESLDTM_IDX_METER_STATUS0  3
#define NESLDTM_IDX_METER_STATUS1  4
#define NESLDTM_IDX_VOLTAGE        5   // ×10  → divide by 10 for V
#define NESLDTM_IDX_FREQUENCY      6   // ×100 → divide by 100 for Hz

class Modbus_NESLDTM : public ModbusMaster {
    public:
        Modbus_NESLDTM();
        ~Modbus_NESLDTM() {};

        uint8_t get_modbus_address();
        void set_modbus_address(uint8_t addr);
        void poll();
        DTMData last_reading;

    private:
        uint8_t modbus_address;
        uint16_t reg_cache[NESLDTM_REG_COUNT];
};

extern Modbus_NESLDTM nesldtm_1;

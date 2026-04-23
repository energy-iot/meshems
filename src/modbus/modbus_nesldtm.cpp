#include <modbus/modbus_nesldtm.h>
#include <TimeLib.h>

Modbus_NESLDTM::Modbus_NESLDTM() {
}

uint8_t Modbus_NESLDTM::get_modbus_address() {
    return modbus_address;
}

void Modbus_NESLDTM::set_modbus_address(uint8_t addr) {
    modbus_address = addr;
}

static inline float u16(uint16_t v, float scale) { return v / scale; }
static inline float i16(uint16_t v, float scale)  { return (int16_t)v / scale; }
static inline float u32(uint16_t hi, uint16_t lo, float scale) {
    return ((uint32_t)hi << 16 | lo) / scale;
}

void Modbus_NESLDTM::poll() {
    uint8_t result = readHoldingRegisters(NESLDTM_REG_START, NESLDTM_REG_COUNT);
    if (result != ku8MBSuccess) {
        Serial.printf("NESLDTM[%d]: Error reading registers %d-%d (err=0x%02X)\n",
                      modbus_address, NESLDTM_REG_START, NESLDTM_REG_END, result);
        return;
    }

    for (int i = 0; i < NESLDTM_REG_COUNT; i++)
        reg_cache[i] = getResponseBuffer(i);

    // Group 0: Line info (regs 1000-1008)
    last_reading.board_status  = reg_cache[0];
    last_reading.sys_status0   = reg_cache[1];
    last_reading.sys_status1   = reg_cache[2];
    last_reading.meter_status0 = reg_cache[3];
    last_reading.meter_status1 = reg_cache[4];
    last_reading.voltage       = u16(reg_cache[5],  10.0f);
    last_reading.frequency     = u16(reg_cache[6], 100.0f);
    last_reading.temp          = i16(reg_cache[7],  10.0f);
    last_reading.voltage_diff  = i16(reg_cache[8],  10.0f);

    // Group 1: CT1 (regs 1010-1016)
    last_reading.ct1_voltage     = u16(reg_cache[10],   10.0f);
    last_reading.ct1_current     = i16(reg_cache[11],  100.0f);
    last_reading.ct1_real_power  = i16(reg_cache[12],    1.0f);
    last_reading.ct1_react_power = i16(reg_cache[13],    1.0f);
    last_reading.ct1_va_power    = u16(reg_cache[14],    1.0f);
    last_reading.ct1_pf          = i16(reg_cache[15], 1000.0f);
    last_reading.ct1_phase       = i16(reg_cache[16],   10.0f);

    // Group 2: CT2 (regs 1020-1026)
    last_reading.ct2_voltage     = u16(reg_cache[20],   10.0f);
    last_reading.ct2_current     = i16(reg_cache[21],  100.0f);
    last_reading.ct2_real_power  = i16(reg_cache[22],    1.0f);
    last_reading.ct2_react_power = i16(reg_cache[23],    1.0f);
    last_reading.ct2_va_power    = u16(reg_cache[24],    1.0f);
    last_reading.ct2_pf          = i16(reg_cache[25], 1000.0f);
    last_reading.ct2_phase       = i16(reg_cache[26],   10.0f);

    // Group 3: CT3 (regs 1030-1036)
    last_reading.ct3_voltage     = u16(reg_cache[30],   10.0f);
    last_reading.ct3_current     = i16(reg_cache[31],  100.0f);
    last_reading.ct3_real_power  = i16(reg_cache[32],    1.0f);
    last_reading.ct3_react_power = i16(reg_cache[33],    1.0f);
    last_reading.ct3_va_power    = u16(reg_cache[34],    1.0f);
    last_reading.ct3_pf          = i16(reg_cache[35], 1000.0f);
    last_reading.ct3_phase       = i16(reg_cache[36],   10.0f);

    // Group 4: Neutral (reg 1040)
    last_reading.current_n = i16(reg_cache[40], 1000.0f);

    // Group 5: Totals (regs 1050-1055)
    last_reading.total_active_power   = i16(reg_cache[50],    1.0f);
    last_reading.total_reactive_power = i16(reg_cache[51],    1.0f);
    last_reading.total_apparent_power = u16(reg_cache[52],    1.0f);
    last_reading.total_pf             = i16(reg_cache[53], 1000.0f);
    last_reading.total_fund_power     = i16(reg_cache[54],    1.0f);
    last_reading.total_har_power      = i16(reg_cache[55],    1.0f);

    // Group 6: Energy accumulators (regs 1060-1068, 32-bit HI/LO ÷10)
    last_reading.import_energy          = u32(reg_cache[60], reg_cache[61], 10.0f);
    last_reading.export_energy          = u32(reg_cache[62], reg_cache[63], 10.0f);
    last_reading.import_reactive_energy = u32(reg_cache[64], reg_cache[65], 10.0f);
    last_reading.export_reactive_energy = u32(reg_cache[66], reg_cache[67], 10.0f);
    last_reading.import_apparent_energy = ((uint32_t)reg_cache[68] << 16) / 10.0f; // LO at 1069 out of scan range

    last_reading.timestamp_last_report = now();

    Serial.printf("NESLDTM[%d]: V=%.1f Hz=%.2f T=%.1fC | board=0x%04X(meterOK=%d netUp=%d)\n",
                  modbus_address,
                  last_reading.voltage, last_reading.frequency, last_reading.temp,
                  last_reading.board_status,
                  (last_reading.board_status & 0x0001) ? 1 : 0,
                  (last_reading.board_status & 0x0004) ? 1 : 0);
    Serial.printf("  CT1: V=%.1f A=%.2f W=%.0f VAR=%.0f PF=%.3f\n",
                  last_reading.ct1_voltage, last_reading.ct1_current,
                  last_reading.ct1_real_power, last_reading.ct1_react_power, last_reading.ct1_pf);
    Serial.printf("  CT2: V=%.1f A=%.2f W=%.0f VAR=%.0f PF=%.3f\n",
                  last_reading.ct2_voltage, last_reading.ct2_current,
                  last_reading.ct2_real_power, last_reading.ct2_react_power, last_reading.ct2_pf);
    Serial.printf("  CT3: V=%.1f A=%.2f W=%.0f VAR=%.0f PF=%.3f\n",
                  last_reading.ct3_voltage, last_reading.ct3_current,
                  last_reading.ct3_real_power, last_reading.ct3_react_power, last_reading.ct3_pf);
    Serial.printf("  Tot: W=%.0f VAR=%.0f VA=%.0f PF=%.3f | ImpE=%.1f ExpE=%.1f kWh\n",
                  last_reading.total_active_power, last_reading.total_reactive_power,
                  last_reading.total_apparent_power, last_reading.total_pf,
                  last_reading.import_energy, last_reading.export_energy);
}

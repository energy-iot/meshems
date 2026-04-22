#include <modbus/modbus_nesldtm.h>

Modbus_NESLDTM::Modbus_NESLDTM() {
}

uint8_t Modbus_NESLDTM::get_modbus_address() {
    return modbus_address;
}

void Modbus_NESLDTM::set_modbus_address(uint8_t addr) {
    modbus_address = addr;
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

    uint16_t board_st  = reg_cache[NESLDTM_IDX_BOARD_STATUS];
    uint16_t sys_st0   = reg_cache[NESLDTM_IDX_SYS_STATUS0];
    uint16_t sys_st1   = reg_cache[NESLDTM_IDX_SYS_STATUS1];
    uint16_t meter_st0 = reg_cache[NESLDTM_IDX_METER_STATUS0];
    uint16_t meter_st1 = reg_cache[NESLDTM_IDX_METER_STATUS1];
    float    voltage   = reg_cache[NESLDTM_IDX_VOLTAGE]   / 10.0f;
    float    freq      = reg_cache[NESLDTM_IDX_FREQUENCY] / 100.0f;

    Serial.printf("NESLDTM[%d]: V=%.1f Hz=%.2f | board=0x%04X(meterOK=%d netUp=%d) sys0=0x%04X sys1=0x%04X meter0=0x%04X meter1=0x%04X\n",
                  modbus_address,
                  voltage, freq,
                  board_st, (board_st & 0x0001) ? 1 : 0, (board_st & 0x0004) ? 1 : 0,
                  sys_st0, sys_st1, meter_st0, meter_st1);
}

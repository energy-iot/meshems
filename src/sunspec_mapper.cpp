#include <sunspec_models.h>
#include <data_model.h>
#include <modbus_solark.h>

// External reference to the Sol-Ark instance
extern Modbus_SolArkLV solark;

// WORKING ONE
// void set_sunspec_string(uint16_t* registers, const char* str, uint8_t max_len) {
//     uint8_t i;
//     for (i = 0; i < max_len && str[i] != '\0'; i += 2) {
//         uint16_t val = str[i];
//         if (i + 1 < max_len && str[i + 1] != '\0') {
//             val |= (str[i + 1] << 8);
//         }
//         registers[i / 2] = val;
//     }
// }

// Helper function to set a string in the register map
void set_sunspec_string(uint16_t* registers, const char* str, uint8_t max_len) {
    uint8_t i;
    for (i = 0; i < max_len && str[i] != '\0'; i += 2) {
        uint16_t val = str[i] << 8;  // First character in high byte
        if (i + 1 < max_len && str[i + 1] != '\0') {
            val |= str[i + 1];  // Second character in low byte
        }
        registers[i / 2] = val;
    }
    
    // Pad with zeros
    for (; i < max_len; i += 2) {
        registers[i / 2] = 0;
    }
}

// Initialize the SunSpec models in the Modbus register map
void setup_sunspec_models() {
    // Initialize all registers to 0
    for (uint16_t i = 0; i < MODUBS_NUM_HOLDING_REGISTERS; i++) {
        holdingRegisters[i] = 0;
    }
    
    // Set SunSpec identifier "SunS"
    holdingRegisters[0] = SUNSPEC_ID_MSW;
    holdingRegisters[1] = SUNSPEC_ID_LSW;
    
    // Set Common Model (1) header
    uint16_t common_offset = 2;
    holdingRegisters[common_offset + COMMON_MODEL_ID] = SUNSPEC_MODEL_COMMON;
    holdingRegisters[common_offset + COMMON_MODEL_LENGTH] = 66;  // Length of model block in 16-bit registers
    
    // Set Common Model data
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_MANUFACTURER], "SolArk", 32);
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_MODEL], "SOLARK-LV-MODBUS", 32);
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_OPTIONS], "NA", 16);
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_VERSION], "130", 16);
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_SERIAL], "123456", 32);
    holdingRegisters[common_offset + COMMON_DEVICE_ADDR] = 1;
    
    // Set Inverter Model (701) header
    uint16_t inverter_offset = common_offset + 66 + 2;  // Common model + 2 for next model header
    holdingRegisters[inverter_offset + INV_MODEL_ID] = SUNSPEC_MODEL_INVERTER;
    holdingRegisters[inverter_offset + INV_MODEL_LENGTH] = 153;  // Length of model block in 16-bit registers
    
    // Initialize all inverter model values to "not implemented"
    for (uint16_t i = inverter_offset + 2; i < inverter_offset + 153; i++) {
        holdingRegisters[i] = SUNSPEC_NOT_IMPLEMENTED;
    }
    
    // Set scale factors
    holdingRegisters[inverter_offset + INV_SF_CURRENT] = SCALE_FACTOR_0_01;      // Current scale factor: -2 (0.01)
    holdingRegisters[inverter_offset + INV_SF_VOLTAGE] = SCALE_FACTOR_0_1;       // Voltage scale factor: -1 (0.1)
    holdingRegisters[inverter_offset + INV_SF_FREQUENCY] = SCALE_FACTOR_0_01;    // Frequency scale factor: -2 (0.01)
    holdingRegisters[inverter_offset + INV_SF_POWER] = SCALE_FACTOR_1;           // Power scale factor: 0 (1)
    holdingRegisters[inverter_offset + INV_SF_PF] = SCALE_FACTOR_0_01;           // Power factor scale factor: -2 (0.01)
    holdingRegisters[inverter_offset + INV_SF_VA] = SCALE_FACTOR_1;              // Apparent power scale factor: 0 (1)
    holdingRegisters[inverter_offset + INV_SF_VAR] = SCALE_FACTOR_1;             // Reactive power scale factor: 0 (1)
    holdingRegisters[inverter_offset + INV_SF_ENERGY] = SCALE_FACTOR_0_001;      // Energy scale factor: -3 (0.001)
    holdingRegisters[inverter_offset + INV_SF_REACTIVE_ENERGY] = SCALE_FACTOR_0_001; // Reactive energy scale factor: -3 (0.001)
    holdingRegisters[inverter_offset + INV_SF_TEMP] = SCALE_FACTOR_0_1;          // Temperature scale factor: -1 (0.1)
}

// Update SunSpec registers with Sol-Ark data
void update_sunspec_from_solark() {
    uint16_t common_offset = 2;
    uint16_t inverter_offset = common_offset + 66 + 2;  // Common model + 2 for next model header
    
    // Set AC wiring type (split phase for typical residential)
    holdingRegisters[inverter_offset + INV_AC_TYPE] = 1;  // Split phase
    
    // Set operating state
    holdingRegisters[inverter_offset + INV_OPERATING_STATE] = solark.getInverterPower() > 0 ? 1 : 0;  // 0=Off, 1=On
    
    // Set inverter state
    uint16_t invState = 0;  // OFF
    if (solark.getInverterPower() > 0) {
        if (solark.getInverterPower() < 100) {
            invState = 7;  // STANDBY
        } else {
            invState = 3;  // RUNNING
        }
    }
    holdingRegisters[inverter_offset + INV_STATUS] = invState;
    
    // Set grid connection state
    holdingRegisters[inverter_offset + INV_GRID_CONNECTION] = solark.isGridConnected() ? 1 : 0;
    
    // Set alarm bitfield (not implemented in this example)
    holdingRegisters[inverter_offset + INV_ALARM] = 0;
    holdingRegisters[inverter_offset + INV_ALARM + 1] = 0;
    
    // Set DER operational characteristics
    uint32_t derMode = 0;
    derMode |= 0x0001;  // Grid following
    holdingRegisters[inverter_offset + INV_DER_MODE] = (derMode >> 16) & 0xFFFF;
    holdingRegisters[inverter_offset + INV_DER_MODE + 1] = derMode & 0xFFFF;
    
    // Set power measurements
    holdingRegisters[inverter_offset + INV_AC_POWER] = solark.getInverterPower();
    holdingRegisters[inverter_offset + INV_AC_VA] = solark.getInverterPower();  // Approximation
    holdingRegisters[inverter_offset + INV_AC_VAR] = 0;  // Not available
    holdingRegisters[inverter_offset + INV_AC_PF] = 100;  // Assuming power factor of 1.0 (scaled by 100)
    
    // Set current and voltage measurements
    float ac_current_total = (solark.getInverterCurrentL1() + solark.getInverterCurrentL2()) / 2.0f;
    holdingRegisters[inverter_offset + INV_AC_CURRENT] = ac_current_total * 100;  // Scale by 100 for 0.01 scale factor
    holdingRegisters[inverter_offset + INV_AC_VOLTAGE_LL] = solark.getInverterVoltage() * 10;  // Scale by 10 for 0.1 scale factor
    holdingRegisters[inverter_offset + INV_AC_VOLTAGE_LN] = (solark.getInverterVoltage() / 1.732) * 10;  // Approximation for LN voltage
    
    // Set frequency (2 registers for uint32)
    uint32_t frequency = solark.getInverterFrequency() * 100;  // Scale by 100 for 0.01 scale factor
    holdingRegisters[inverter_offset + INV_AC_FREQUENCY] = (frequency >> 16) & 0xFFFF;
    holdingRegisters[inverter_offset + INV_AC_FREQUENCY + 1] = frequency & 0xFFFF;
    
    // Set energy measurements (4 registers each for uint64)
    uint64_t energy_wh = solark.getLoadEnergy() * 1000;  // Convert kWh to Wh
    holdingRegisters[inverter_offset + INV_ENERGY_INJECTED] = (energy_wh >> 48) & 0xFFFF;
    holdingRegisters[inverter_offset + INV_ENERGY_INJECTED + 1] = (energy_wh >> 32) & 0xFFFF;
    holdingRegisters[inverter_offset + INV_ENERGY_INJECTED + 2] = (energy_wh >> 16) & 0xFFFF;
    holdingRegisters[inverter_offset + INV_ENERGY_INJECTED + 3] = energy_wh & 0xFFFF;
    
    // Set temperature measurements
    holdingRegisters[inverter_offset + INV_TEMP_CABINET] = solark.getBatteryTemperature() * 10;  // Scale by 10 for 0.1 scale factor
    
    // Set phase L1 measurements
    holdingRegisters[inverter_offset + INV_AC_POWER_L1] = solark.getLoadPowerL1();
    holdingRegisters[inverter_offset + INV_AC_CURRENT_L1] = solark.getInverterCurrentL1() * 100;
    holdingRegisters[inverter_offset + INV_AC_VOLTAGE_L1L2] = solark.getInverterVoltage() * 10;
    
    // Set phase L2 measurements
    holdingRegisters[inverter_offset + INV_AC_POWER_L2] = solark.getLoadPowerL2();
    holdingRegisters[inverter_offset + INV_AC_CURRENT_L2] = solark.getInverterCurrentL2() * 100;
    
    // Vendor-specific status information in the alarm info field
    char alarmInfo[64] = {0};
    sprintf(alarmInfo, "Grid:%s Batt:%s", 
            solark.isGridConnected() ? "Connected" : "Disconnected",
            solark.isBatteryCharging() ? "Charging" : (solark.isBatteryDischarging() ? "Discharging" : "Idle"));
    
    set_sunspec_string(&holdingRegisters[inverter_offset + INV_ALARM_INFO], alarmInfo, 64);
    
    //SunSpec Ending Block
    uint16_t end_offset = inverter_offset + 153 + 2;  // After Inverter Model + 2 for next model header
    holdingRegisters[end_offset] = 0xFFFF;
    holdingRegisters[end_offset + 1] = 0x0000;
}

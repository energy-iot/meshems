#include <sunspec_models.h>
#include <data_model.h>
#include <modbus_solark.h>

// External reference to the Sol-Ark instance
extern Modbus_SolArkLV solark;

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
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_MANUFACTURER], "Sol-Ark", 32);
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_MODEL], "Sol-Ark-12K-P", 32);
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_OPTIONS], "None", 16);
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
    
    // Set AC wiring type based on Sol-Ark register 286 (Grid Type)
    // Sol-Ark Grid Type: 0x00=Single-phase, 0x01=Split-phase, 0x02=Three-phase Wye
    // SunSpec INV_AC_TYPE (enum ACType): 0=Unknown, 1=Split Phase, 2=Single Phase, 3=Three Phase Wye
    uint8_t solark_grid_type = solark.getGridType();
    uint16_t sunspec_ac_type = 0; // Default to Unknown
    switch (solark_grid_type) {
        case 0x00: // Single-phase
            sunspec_ac_type = 2; // Single Phase
            break;
        case 0x01: // Split-phase
            sunspec_ac_type = 1; // Split Phase
            break;
        case 0x02: // Three-phase Wye
            sunspec_ac_type = 3; // Three Phase Wye
            break;
        default:
            sunspec_ac_type = 0; // Unknown
            break;
    }
    holdingRegisters[inverter_offset + INV_AC_TYPE] = sunspec_ac_type;
    
    // Set operating state based on Sol-Ark inverter status (register 59)
    uint8_t solarkStatus = solark.getInverterStatus();
    holdingRegisters[inverter_offset + INV_OPERATING_STATE] = (solarkStatus == 2) ? 1 : 0;  // 0=Off, 1=On (only "Normal" is considered "On")
    
    // Set inverter state based on Sol-Ark status mapping
    // Sol-Ark States: 1=Self-test, 2=Normal, 3=Alarm, 4=Fault
    // SunSpec States: 0=OFF, 1=SLEEPING, 2=STARTING, 3=RUNNING, 4=THROTTLED, 5=SHUTTING_DOWN, 6=FAULT, 7=STANDBY
    uint16_t invState = 0;  // Default to OFF
    
    switch (solarkStatus) {
        case 1:  // Self-test
            invState = 2;  // STARTING
            break;
        case 2:  // Normal
            if (solark.getInverterPower() > 100) {
                invState = 3;  // RUNNING
            } else {
                invState = 7;  // STANDBY
            }
            break;
        case 3:  // Alarm
            invState = 4;  // THROTTLED (closest match for alarm state)
            break;
        case 4:  // Fault
            invState = 6;  // FAULT
            break;
        default:
            invState = 0;  // OFF
            break;
    }
    
    holdingRegisters[inverter_offset + INV_STATUS] = invState;
    
    // Set grid connection state
    // Set grid connection state based on Sol-Ark register 194 (Grid side relay status)
    // Sol-Ark register 194: 1 = Open (Disconnected), 2 = Closed (Connected)
    // SunSpec INV_GRID_CONNECTION (enum Conn): 0 = Disconnected, 1 = Connected
    uint8_t raw_grid_relay_status = solark.getGridRelayStatus();
    holdingRegisters[inverter_offset + INV_GRID_CONNECTION] = (raw_grid_relay_status == 2) ? 1 : 0;
    
    // Set alarm bitfield (not implemented in this example)
    holdingRegisters[inverter_offset + INV_ALARM] = 0;
    holdingRegisters[inverter_offset + INV_ALARM + 1] = 0;
    
    // Set DER operational characteristics
    // INV_DER_MODE (DERCtl bitmask): Bit 0 = GridFollowing (0x0001), Bit 1 = GridForming (0x0002)
    uint32_t derMode = 0;
    if (raw_grid_relay_status == 2) { // Sol-Ark relay closed (Connected to grid)
        derMode |= 0x0001; // Set Grid Following
    } else { // Sol-Ark relay open (Disconnected from grid)
        derMode |= 0x0002; // Set Grid Forming
    }
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
    holdingRegisters[inverter_offset + INV_TEMP_TRANSFORMER] = solark.getDCDCTemp() * 10;  // Scale by 10 for 0.1 scale factor
    holdingRegisters[inverter_offset + INV_TEMP_IGBT] = solark.getIGBTTemp() * 10;  // Scale by 10 for 0.1 scale factor
    
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
    
    // Set DER Storage Capacity Model (713) header
    uint16_t storage_offset = inverter_offset + 153 + 2;  // After Inverter model + 2 for next model header
    holdingRegisters[storage_offset + STORAGE_MODEL_ID] = SUNSPEC_MODEL_DER_STORAGE;
    holdingRegisters[storage_offset + STORAGE_MODEL_LENGTH] = 7;  // Length of model block in 16-bit registers
    
    // Initialize all storage model values to "not implemented"
    for (uint16_t i = storage_offset + 2; i < storage_offset + 9; i++) {
        holdingRegisters[i] = SUNSPEC_NOT_IMPLEMENTED;
    }
    
    // Set scale factors for storage model
    holdingRegisters[storage_offset + STORAGE_SF_ENERGY] = SCALE_FACTOR_0_001;  // Energy scale factor: -3 (0.001)
    holdingRegisters[storage_offset + STORAGE_SF_PERCENT] = SCALE_FACTOR_0_1;   // Percentage scale factor: -1 (0.1)
    
    // Update storage model with Sol-Ark battery data using actual registers
    // Calculate battery energy rating from capacity (Ah) and nominal voltage
    float batteryCapacityAh = solark.getBatteryCapacity();
    float nominalVoltage = 51.2f;  // Typical 48V LFP system, could be made configurable

    uint16_t batteryEnergyRating = (batteryCapacityAh * nominalVoltage);  // Wh = Ah * V
    holdingRegisters[storage_offset + STORAGE_ENERGY_RATING] = batteryEnergyRating;
    
    // Energy available (WHAvail = WHRtg * SoC * SoH)
    // Using actual SOC from Sol-Ark and assuming 100% SoH if not available from BMS
    float soc = solark.getBatterySOC() / 100.0f;
    float soh = 1.0f;  // Default to 100% if BMS data not available
    
    // Try to get BMS SOC if available (for lithium batteries)
    float bmsSOC = solark.getBMSRealTimeSOC();
    if (bmsSOC > 0 && bmsSOC <= 100) {
        soc = bmsSOC / 100.0f;
    }
    
    uint16_t batteryEnergyAvailable = batteryEnergyRating * soc * soh;
    holdingRegisters[storage_offset + STORAGE_ENERGY_AVAILABLE] = batteryEnergyAvailable;
    
    // State of charge (%) - use BMS SOC if available, otherwise use Sol-Ark SOC
    float socPercent = solark.getBatterySOC();
    if (bmsSOC > 0 && bmsSOC <= 100) {
        socPercent = bmsSOC;
    }
    holdingRegisters[storage_offset + STORAGE_SOC] = socPercent * 10;  // Scale by 10 for 0.1 scale factor
    
    // State of health (%) - default to 100% since Sol-Ark doesn't provide this directly
    // In a real implementation, this could be calculated from battery degradation over time
    //holdingRegisters[storage_offset + STORAGE_SOH] = 1000;  // 100.0% (scaled by 10)
    
    // Storage status - determine from BMS warnings/faults and battery state
    uint16_t storageStatus = 0;  // 0 = OK
    uint16_t bmsWarning = solark.getBMSWarning();
    uint16_t bmsFault = solark.getBMSFault();
    
    if (bmsFault > 0) {
        storageStatus = 2;  // Error
    } else if (bmsWarning > 0) {
        storageStatus = 1;  // Warning
    }
    
    holdingRegisters[storage_offset + STORAGE_STATUS] = storageStatus;
    
    //SunSpec Ending Block
    uint16_t end_offset = storage_offset + 9;  // After Storage Model
    holdingRegisters[end_offset] = 0xFFFF;
    holdingRegisters[end_offset + 1] = 0x0000;
}

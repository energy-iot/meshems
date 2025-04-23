#include <sunspec_models.h>
#include <data_model.h>
#include <modbus_solark.h>

// External reference to the Sol-Ark instance
extern Modbus_SolArkLV solark;

// Helper function to set a string in the register map
void set_sunspec_string(uint16_t* registers, const char* str, uint8_t max_len) {
    uint8_t i;
    for (i = 0; i < max_len && str[i] != '\0'; i += 2) {
        uint16_t val = str[i];
        if (i + 1 < max_len && str[i + 1] != '\0') {
            val |= (str[i + 1] << 8);
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
    holdingRegisters[common_offset + COMMON_MODEL_LENGTH] = 65;  // Length of model block in 16-bit registers
    
    // Set Common Model data
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_MANUFACTURER], "Sol-Ark", 16);
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_MODEL], "SolArk-LV", 16);
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_OPTIONS], "EMS-Dev", 16);
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_VERSION], "1.0", 16);
    set_sunspec_string(&holdingRegisters[common_offset + COMMON_SERIAL], "DEMO-1", 16);
    
    // Set Inverter Model (701) header
    uint16_t inverter_offset = common_offset + 65 + 2;  // Common model + 2 for next model header
    holdingRegisters[inverter_offset + INV_MODEL_ID] = SUNSPEC_MODEL_INVERTER;
    holdingRegisters[inverter_offset + INV_MODEL_LENGTH] = 54;  // Length of model block in 16-bit registers
    
    // Initialize all inverter model values to "not implemented"
    for (uint16_t i = inverter_offset + 4; i < inverter_offset + 54; i++) {
        holdingRegisters[i] = SUNSPEC_NOT_IMPLEMENTED;
    }
    
    // Set scale factors (these are typically negative powers of 10)
    holdingRegisters[inverter_offset + INV_AC_CURRENT + 1] = SCALE_FACTOR_0_01;     // AC current scale factor: -2 (0.01)
    holdingRegisters[inverter_offset + INV_AC_VOLTAGE_AB + 1] = SCALE_FACTOR_0_1;   // AC voltage scale factor: -1 (0.1)
    holdingRegisters[inverter_offset + INV_AC_POWER + 1] = SCALE_FACTOR_1;          // AC power scale factor: 0 (1)
    holdingRegisters[inverter_offset + INV_AC_FREQUENCY + 1] = SCALE_FACTOR_0_01;   // AC frequency scale factor: -2 (0.01)
    holdingRegisters[inverter_offset + INV_AC_ENERGY_WH + 1] = SCALE_FACTOR_0_001;  // AC energy scale factor: -3 (0.001)
    holdingRegisters[inverter_offset + INV_DC_CURRENT + 1] = SCALE_FACTOR_0_01;     // DC current scale factor: -2 (0.01)
    holdingRegisters[inverter_offset + INV_DC_VOLTAGE + 1] = SCALE_FACTOR_0_1;      // DC voltage scale factor: -1 (0.1)
    holdingRegisters[inverter_offset + INV_DC_POWER + 1] = SCALE_FACTOR_1;          // DC power scale factor: 0 (1)
    holdingRegisters[inverter_offset + INV_TEMP_CABINET + 1] = SCALE_FACTOR_0_1;    // Temperature scale factor: -1 (0.1)
}

// Update SunSpec registers with Sol-Ark data
void update_sunspec_from_solark() {
    uint16_t common_offset = 2;
    uint16_t inverter_offset = common_offset + 65 + 2;  // Common model + 2 for next model header
    
    // Update AC measurements
    // Total AC current - average of L1 and L2
    float ac_current_total = (solark.getInverterCurrentL1() + solark.getInverterCurrentL2()) / 2.0f;
    holdingRegisters[inverter_offset + INV_AC_CURRENT] = ac_current_total * 100;  // Scale by 100 for 0.01 scale factor
    
    // Phase currents
    holdingRegisters[inverter_offset + INV_AC_CURRENT_A] = solark.getInverterCurrentL1() * 100;
    holdingRegisters[inverter_offset + INV_AC_CURRENT_B] = solark.getInverterCurrentL2() * 100;
    holdingRegisters[inverter_offset + INV_AC_CURRENT_C] = SUNSPEC_NOT_IMPLEMENTED;  // Not used in this system
    
    // AC voltage
    holdingRegisters[inverter_offset + INV_AC_VOLTAGE_AB] = solark.getInverterVoltage() * 10;  // Scale by 10 for 0.1 scale factor
    holdingRegisters[inverter_offset + INV_AC_VOLTAGE_BC] = SUNSPEC_NOT_IMPLEMENTED;
    holdingRegisters[inverter_offset + INV_AC_VOLTAGE_CA] = SUNSPEC_NOT_IMPLEMENTED;
    
    // Phase-to-neutral voltages (not available in this system)
    holdingRegisters[inverter_offset + INV_AC_VOLTAGE_AN] = SUNSPEC_NOT_IMPLEMENTED;
    holdingRegisters[inverter_offset + INV_AC_VOLTAGE_BN] = SUNSPEC_NOT_IMPLEMENTED;
    holdingRegisters[inverter_offset + INV_AC_VOLTAGE_CN] = SUNSPEC_NOT_IMPLEMENTED;
    
    // AC power
    holdingRegisters[inverter_offset + INV_AC_POWER] = solark.getInverterPower();
    
    // AC frequency
    holdingRegisters[inverter_offset + INV_AC_FREQUENCY] = solark.getInverterFrequency() * 100;  // Scale by 100 for 0.01 scale factor
    
    // AC energy
    // Convert kWh to Wh and scale by 1000 for 0.001 scale factor
    float load_energy_wh = solark.getLoadEnergy() * 1000.0f;
    holdingRegisters[inverter_offset + INV_AC_ENERGY_WH] = load_energy_wh;
    
    // DC measurements
    holdingRegisters[inverter_offset + INV_DC_CURRENT] = solark.getBatteryCurrent() * 100;  // Scale by 100 for 0.01 scale factor
    holdingRegisters[inverter_offset + INV_DC_VOLTAGE] = solark.getBatteryVoltage() * 10;   // Scale by 10 for 0.1 scale factor
    holdingRegisters[inverter_offset + INV_DC_POWER] = solark.getBatteryPower();
    
    // Temperature
    holdingRegisters[inverter_offset + INV_TEMP_CABINET] = solark.getBatteryTemperature() * 10;  // Scale by 10 for 0.1 scale factor
    
    // Status
    uint16_t status = 0;
    
    // Determine inverter status based on available data
    if (solark.getInverterPower() == 0) {
        status |= STAT_OFF;
    } else if (solark.getInverterPower() < 100) {  // Arbitrary low power threshold
        status |= STAT_STANDBY;
    } else {
        // Normal operation - could be refined with more detailed status information
        status |= STAT_MPPT;
    }
    
    holdingRegisters[inverter_offset + INV_STATUS] = status;
    
    // Vendor-specific status information
    uint16_t vendor_status = 0;
    if (solark.isGridConnected()) {
        vendor_status |= 0x0001;  // Bit 0: Grid connected
    }
    if (solark.isGeneratorConnected()) {
        vendor_status |= 0x0002;  // Bit 1: Generator connected
    }
    if (solark.isBatteryCharging()) {
        vendor_status |= 0x0004;  // Bit 2: Battery charging
    }
    if (solark.isBatteryDischarging()) {
        vendor_status |= 0x0008;  // Bit 3: Battery discharging
    }
    if (solark.isSellingToGrid()) {
        vendor_status |= 0x0010;  // Bit 4: Selling to grid
    }
    if (solark.isBuyingFromGrid()) {
        vendor_status |= 0x0020;  // Bit 5: Buying from grid
    }
    
    holdingRegisters[inverter_offset + INV_STATUS_VENDOR] = vendor_status;
}

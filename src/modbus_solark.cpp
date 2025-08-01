#include <modbus_solark.h>
#include <TimeLib.h>

#define SOLARK_POLL_INTERVAL 5000 // 5 seconds

// Define the blocks of registers to be read
// Max 20 registers per block as requested
const ModbusReadBlock solark_read_blocks[] = {
    {SolArkBlockType::ENERGY, SolArkRegisterMap::BATTERY_CHARGE_ENERGY, 15, "Energy Data (70-84)"}, // 70-84 (15 regs)
    {SolArkBlockType::PV_ENERGY, SolArkRegisterMap::PV_ENERGY, 1, "PV Energy (108)"}, // 108 (1 reg)
    {SolArkBlockType::INVERTER_STATUS, SolArkRegisterMap::INVERTER_STATUS, 1, "Inverter Status (59)"}, // 59 (1 reg)
    {SolArkBlockType::TEMPERATURES, SolArkRegisterMap::DCDC_XFRMR_TEMP, 2, "Temperatures (90-91)"}, // 90-91 (2 regs)
    // Split 150-169 (20 regs) into two blocks if needed, but it's exactly 20.
    {SolArkBlockType::GRID_INVERTER_150, 150, 20, "Grid/Inverter Data (150-169)"}, // 150-169 (20 regs)
    // Split 170-189 (20 regs) into two blocks if needed, but it's exactly 20.
    {SolArkBlockType::POWER_BATTERY_170, 170, 20, "Power/Battery Data (170-189)"}, // 170-189 (20 regs)
    {SolArkBlockType::BATTERY_STATUS_190, SolArkRegisterMap::BATTERY_POWER, 10, "Battery Status (190-199)"}, // 190-199 (10 regs)
    {SolArkBlockType::BATTERY_CAPACITY_204, SolArkRegisterMap::BATTERY_CAPACITY, 1, "Battery Capacity (204)"}, // 204 (1 reg)
    {SolArkBlockType::CORRECTED_BATTERY_CAPACITY_107, SolArkRegisterMap::CORRECTED_BATTERY_CAPACITY, 1, "Corrected Battery Capacity (107)"}, // 107 (1 reg)
    {SolArkBlockType::BATTERY_EMPTY_VOLTAGE_205, SolArkRegisterMap::BATTERY_EMPTY_VOLTAGE, 1, "Battery Empty Voltage (205)"}, // 205 (1 reg)
    {SolArkBlockType::BATTERY_VOLTAGE_THRESHOLDS_220, SolArkRegisterMap::BATTERY_SHUTDOWN_VOLTAGE, 3, "Battery Voltage Thresholds (220-222)"}, // 220-222 (3 regs)
    {SolArkBlockType::BATTERY_PERCENT_THRESHOLDS_217, SolArkRegisterMap::BATTERY_SHUTDOWN_PERCENT, 3, "Battery Percent Thresholds (217-219)"}, // 217-219 (3 regs)
    {SolArkBlockType::BMS_DATA_312, SolArkRegisterMap::BMS_CHARGING_VOLTAGE, 12, "BMS Data (312-323)"}, // 312-323 (12 regs)
    {SolArkBlockType::GRID_TYPE_286, SolArkRegisterMap::GRID_TYPE, 1, "Grid Type (286)"}, // 286 (1 reg)
    {SolArkBlockType::DIAGNOSTICS, SolArkRegisterMap::COMM_VERSION, 6, "Diagnostics (2-7)"} // COMM_VERSION (2) + 5 SN_BYTES (3-7) = 6 regs
};
const size_t solark_read_blocks_count = sizeof(solark_read_blocks) / sizeof(solark_read_blocks[0]);


Modbus_SolArkLV::Modbus_SolArkLV() {
    // Initialize class variables
    
    // Diagnostic variables
    comm_version_val = 0;
    for (int i = 0; i < 5; ++i) {
        serial_number_parts[i] = 0;
    }
    igbt_temp = 0;
    dcdc_xfrmr_temp = 0;

    // Status variables
    battery_power = 0;
    battery_current = 0;
    load_frequency = 0;
    inverter_frequency = 0;
    grid_relay_status = 0;
    generator_relay_status = 0;
    
    // Energy variables
    battery_charge_energy = 0;
    battery_discharge_energy = 0;
    grid_buy_energy = 0;
    grid_sell_energy = 0;
    load_energy = 0;
    pv_energy = 0;
    
    // Power variables
    grid_power = 0;
    inverter_output_power = 0;
    load_power_l1 = 0;
    load_power_l2 = 0;
    load_power_total = 0;
    pv1_power = 0;
    pv2_power = 0;
    pv_power_total = 0;
    smart_load_power = 0;
    
    // Battery variables
    battery_temperature = 0;
    battery_voltage = 0;
    battery_soc = 0;
    
    // Battery configuration variables (for SunSpec model 713)
    battery_capacity = 0;
    corrected_battery_capacity = 0;
    battery_empty_voltage = 0;
    battery_shutdown_voltage = 0;
    battery_restart_voltage = 0;
    battery_low_voltage = 0;
    battery_shutdown_percent = 0;
    battery_restart_percent = 0;
    battery_low_percent = 0;
    
    // BMS variables (for lithium batteries)
    bms_charging_voltage = 0;
    bms_discharge_voltage = 0;
    bms_charging_current_limit = 0;
    bms_discharge_current_limit = 0;
    bms_real_time_soc = 0;
    bms_real_time_voltage = 0;
    bms_real_time_current = 0;
    bms_real_time_temp = 0;
    bms_warning = 0;
    bms_fault = 0;
    
    // Grid variables
    grid_voltage = 0;
    grid_current_l1 = 0;
    grid_current_l2 = 0;
    grid_frequency = 0;
    
    // Inverter variables
    inverter_voltage = 0;
    inverter_voltage_ln = 0;
    inverter_voltage_l2n = 0;
    inverter_power_l1 = 0;
    inverter_power_l2 = 0;
    inverter_current_l1 = 0;
    inverter_current_l2 = 0;
    inverter_status = 0;
    
    // Load variables
    load_current_l1 = 0;
    load_current_l2 = 0;
    grid_type = 0; // Initialize grid_type
}

uint8_t Modbus_SolArkLV::get_modbus_address() {
    return modbus_address;
}

void Modbus_SolArkLV::set_modbus_address(uint8_t addr) {
    modbus_address = addr;
}

uint8_t Modbus_SolArkLV::poll() {
    uint8_t last_result = ku8MBSuccess;
    bool any_success = false;

    for (size_t i = 0; i < solark_read_blocks_count; ++i) {
        const ModbusReadBlock& block = solark_read_blocks[i];
        
        // Make the Modbus call
        last_result = readHoldingRegisters(block.start_register, block.num_registers);
        
        if (last_result == ku8MBSuccess) {
            processBlock(block); // Process the data for this specific block
            Serial.printf("INFO - SolArk: Poll block '%s' (Reg %d, Count %d) success\n", block.description, block.start_register, block.num_registers);
            any_success = true;
        } else {
            Serial.printf("ERROR - SolArk: Poll block '%s' (Reg %d, Count %d) FAIL. Error: %d\n", block.description, block.start_register, block.num_registers, last_result);
            timestamp_last_failure = now();
            // Optionally, break or decide if one failure means total failure for the poll cycle
        }
        // Add a small delay here if necessary for the device, e.g., delay(10);
    }
    
    if (any_success) {
        timestamp_last_report = now();
        // If all blocks must succeed for an overall success, adjust this logic
        return ku8MBSuccess;
    }
    return last_result; // Return the result of the last failed operation or ku8MBSuccess if all were fine
}

void Modbus_SolArkLV::processBlock(const ModbusReadBlock& block) {
    // getResponseBuffer(offset) gets the (offset)th register from the block just read.
    switch (block.type) {
        case SolArkBlockType::ENERGY: // Registers 70-84 (15 regs)
            // Offsets are from block.start_register (70)
            battery_charge_energy = getResponseBuffer(SolArkRegisterMap::BATTERY_CHARGE_ENERGY - block.start_register) / SolArkScalingFactors::ENERGY;
            battery_discharge_energy = getResponseBuffer(SolArkRegisterMap::BATTERY_DISCHARGE_ENERGY - block.start_register) / SolArkScalingFactors::ENERGY;
            grid_buy_energy = getResponseBuffer(SolArkRegisterMap::GRID_BUY_ENERGY - block.start_register) / SolArkScalingFactors::ENERGY;
            grid_sell_energy = getResponseBuffer(SolArkRegisterMap::GRID_SELL_ENERGY - block.start_register) / SolArkScalingFactors::ENERGY;
            grid_frequency = getResponseBuffer(SolArkRegisterMap::GRID_FREQUENCY - block.start_register) / SolArkScalingFactors::FREQUENCY;
            load_energy = getResponseBuffer(SolArkRegisterMap::LOAD_ENERGY - block.start_register) / SolArkScalingFactors::ENERGY;
            break;
        case SolArkBlockType::PV_ENERGY: // Register 108 (1 reg)
            pv_energy = getResponseBuffer(0) / SolArkScalingFactors::ENERGY;
            break;
        case SolArkBlockType::INVERTER_STATUS: // Register 59 (1 reg)
            inverter_status = getResponseBuffer(0);
            break;
        case SolArkBlockType::TEMPERATURES: // Registers 90-91 (2 regs)
            dcdc_xfrmr_temp = (getResponseBuffer(SolArkRegisterMap::DCDC_XFRMR_TEMP - block.start_register) - SolArkScalingFactors::TEMPERATURE_OFFSET) / SolArkScalingFactors::TEMPERATURE_SCALE;
            igbt_temp = (getResponseBuffer(SolArkRegisterMap::IGBT_HEATSINK_TEMP - block.start_register) - SolArkScalingFactors::TEMPERATURE_OFFSET) / SolArkScalingFactors::TEMPERATURE_SCALE;
            break;
        case SolArkBlockType::GRID_INVERTER_150: // Registers 150-169 (20 regs)
            grid_voltage = getResponseBuffer(SolArkRegisterMap::GRID_VOLTAGE - block.start_register) / SolArkScalingFactors::VOLTAGE;
            inverter_voltage_ln = getResponseBuffer(SolArkRegisterMap::INVERTER_VOLTAGE_LN - block.start_register) / SolArkScalingFactors::VOLTAGE;
            inverter_voltage_l2n = getResponseBuffer(SolArkRegisterMap::INVERTER_VOLTAGE_L2N - block.start_register) / SolArkScalingFactors::VOLTAGE;
            inverter_voltage = getResponseBuffer(SolArkRegisterMap::INVERTER_VOLTAGE - block.start_register) / SolArkScalingFactors::VOLTAGE;
            grid_current_l1 = getResponseBuffer(SolArkRegisterMap::GRID_CURRENT_L1 - block.start_register) / SolArkScalingFactors::CURRENT;
            grid_current_l2 = getResponseBuffer(SolArkRegisterMap::GRID_CURRENT_L2 - block.start_register) / SolArkScalingFactors::CURRENT;
            grid_CT_current_l1 = getResponseBuffer(SolArkRegisterMap::GRID_CT_CURRENT_L1 - block.start_register) / SolArkScalingFactors::CURRENT;
            grid_CT_current_l2 = getResponseBuffer(SolArkRegisterMap::GRID_CT_CURRENT_L2 - block.start_register) / SolArkScalingFactors::CURRENT;
            inverter_current_l1 = getResponseBuffer(SolArkRegisterMap::INVERTER_CURRENT_L1 - block.start_register) / SolArkScalingFactors::CURRENT;
            inverter_current_l2 = getResponseBuffer(SolArkRegisterMap::INVERTER_CURRENT_L2 - block.start_register) / SolArkScalingFactors::CURRENT;
            smart_load_power = getResponseBuffer(SolArkRegisterMap::SMART_LOAD_POWER - block.start_register);
            grid_power = correctSignedValue(getResponseBuffer(SolArkRegisterMap::GRID_POWER - block.start_register));
            break;
        case SolArkBlockType::POWER_BATTERY_170: // Registers 170-189 (20 regs)
            inverter_output_power = correctSignedValue(getResponseBuffer(SolArkRegisterMap::INVERTER_OUTPUT_POWER - block.start_register));
            inverter_power_l1 = getResponseBuffer(SolArkRegisterMap::INVERTER_POWER_L1 - block.start_register);
            inverter_power_l2 = getResponseBuffer(SolArkRegisterMap::INVERTER_POWER_L2 - block.start_register);
            load_power_l1 = getResponseBuffer(SolArkRegisterMap::LOAD_POWER_L1 - block.start_register);
            load_power_l2 = getResponseBuffer(SolArkRegisterMap::LOAD_POWER_L2 - block.start_register);
            load_power_total = getResponseBuffer(SolArkRegisterMap::LOAD_POWER_TOTAL - block.start_register);
            load_current_l1 = getResponseBuffer(SolArkRegisterMap::LOAD_CURRENT_L1 - block.start_register) / SolArkScalingFactors::CURRENT;
            load_current_l2 = getResponseBuffer(SolArkRegisterMap::LOAD_CURRENT_L2 - block.start_register) / SolArkScalingFactors::CURRENT;
            battery_temperature = (getResponseBuffer(SolArkRegisterMap::BATTERY_TEMPERATURE - block.start_register) - SolArkScalingFactors::TEMPERATURE_OFFSET) / SolArkScalingFactors::TEMPERATURE_SCALE;
            battery_voltage = getResponseBuffer(SolArkRegisterMap::BATTERY_VOLTAGE - block.start_register) / SolArkScalingFactors::CURRENT; // Reverted to original CURRENT scaling
            battery_soc = getResponseBuffer(SolArkRegisterMap::BATTERY_SOC - block.start_register);
            pv1_power = getResponseBuffer(SolArkRegisterMap::PV1_POWER - block.start_register);
            pv2_power = getResponseBuffer(SolArkRegisterMap::PV2_POWER - block.start_register);
            pv_power_total = (pv1_power + pv2_power) / 1000.0f; // Re-added /1000.0f as in original
            break;
        case SolArkBlockType::BATTERY_STATUS_190: // Registers 190-199 (10 regs)
            battery_power = correctSignedValue(getResponseBuffer(SolArkRegisterMap::BATTERY_POWER - block.start_register));
            battery_current = correctSignedValue(getResponseBuffer(SolArkRegisterMap::BATTERY_CURRENT - block.start_register)) / SolArkScalingFactors::CURRENT;
            load_frequency = getResponseBuffer(SolArkRegisterMap::LOAD_FREQUENCY - block.start_register) / SolArkScalingFactors::FREQUENCY;
            inverter_frequency = getResponseBuffer(SolArkRegisterMap::INVERTER_FREQUENCY - block.start_register) / SolArkScalingFactors::FREQUENCY;
            grid_relay_status = getResponseBuffer(SolArkRegisterMap::GRID_RELAY_STATUS - block.start_register);
            generator_relay_status = getResponseBuffer(SolArkRegisterMap::GENERATOR_RELAY_STATUS - block.start_register);
            break;
        case SolArkBlockType::BATTERY_CAPACITY_204: // Register 204
            battery_capacity = getResponseBuffer(0);
            break;
        case SolArkBlockType::CORRECTED_BATTERY_CAPACITY_107: // Register 107
            corrected_battery_capacity = getResponseBuffer(0);
            break;
        case SolArkBlockType::BATTERY_EMPTY_VOLTAGE_205: // Register 205
            battery_empty_voltage = getResponseBuffer(0) / SolArkScalingFactors::CURRENT; // Reverted to original CURRENT scaling
            break;
        case SolArkBlockType::BATTERY_VOLTAGE_THRESHOLDS_220: // Registers 220-222 (3 regs)
            battery_shutdown_voltage = getResponseBuffer(SolArkRegisterMap::BATTERY_SHUTDOWN_VOLTAGE - block.start_register) / SolArkScalingFactors::CURRENT; // Reverted to original CURRENT scaling
            battery_restart_voltage = getResponseBuffer(SolArkRegisterMap::BATTERY_RESTART_VOLTAGE - block.start_register) / SolArkScalingFactors::CURRENT;   // Reverted to original CURRENT scaling
            battery_low_voltage = getResponseBuffer(SolArkRegisterMap::BATTERY_LOW_VOLTAGE - block.start_register) / SolArkScalingFactors::CURRENT;       // Reverted to original CURRENT scaling
            break;
        case SolArkBlockType::BATTERY_PERCENT_THRESHOLDS_217: // Registers 217-219 (3 regs)
            battery_shutdown_percent = getResponseBuffer(SolArkRegisterMap::BATTERY_SHUTDOWN_PERCENT - block.start_register);
            battery_restart_percent = getResponseBuffer(SolArkRegisterMap::BATTERY_RESTART_PERCENT - block.start_register);
            battery_low_percent = getResponseBuffer(SolArkRegisterMap::BATTERY_LOW_PERCENT - block.start_register);
            break;
        case SolArkBlockType::BMS_DATA_312: // Registers 312-323 (12 regs)
            bms_charging_voltage = getResponseBuffer(SolArkRegisterMap::BMS_CHARGING_VOLTAGE - block.start_register) / SolArkScalingFactors::CURRENT; // Reverted to original CURRENT scaling
            bms_discharge_voltage = getResponseBuffer(SolArkRegisterMap::BMS_DISCHARGE_VOLTAGE - block.start_register) / SolArkScalingFactors::CURRENT; // Reverted to original CURRENT scaling
            bms_charging_current_limit = getResponseBuffer(SolArkRegisterMap::BMS_CHARGING_CURRENT_LIMIT - block.start_register);
            bms_discharge_current_limit = getResponseBuffer(SolArkRegisterMap::BMS_DISCHARGE_CURRENT_LIMIT - block.start_register);
            bms_real_time_soc = getResponseBuffer(SolArkRegisterMap::BMS_REAL_TIME_SOC - block.start_register);
            bms_real_time_voltage = getResponseBuffer(SolArkRegisterMap::BMS_REAL_TIME_VOLTAGE - block.start_register) / SolArkScalingFactors::CURRENT; // Reverted to original CURRENT scaling
            bms_real_time_current = getResponseBuffer(SolArkRegisterMap::BMS_REAL_TIME_CURRENT - block.start_register); // Assuming raw, needs scaling? Check map.
            bms_real_time_temp = (getResponseBuffer(SolArkRegisterMap::BMS_REAL_TIME_TEMP - block.start_register) - SolArkScalingFactors::TEMPERATURE_OFFSET) / SolArkScalingFactors::TEMPERATURE_SCALE;
            bms_warning = getResponseBuffer(SolArkRegisterMap::BMS_WARNING - block.start_register);
            bms_fault = getResponseBuffer(SolArkRegisterMap::BMS_FAULT - block.start_register);
            break;
        case SolArkBlockType::GRID_TYPE_286: // Register 286
            grid_type = getResponseBuffer(0);
            break;
        case SolArkBlockType::DIAGNOSTICS: // Registers 2-7
            // Offset from block.start_register (SolArkRegisterMap::COMM_VERSION which is 2)
            comm_version_val = getResponseBuffer(SolArkRegisterMap::COMM_VERSION - block.start_register);
            serial_number_parts[0] = getResponseBuffer(SolArkRegisterMap::SN_BYTE_01 - block.start_register);
            serial_number_parts[1] = getResponseBuffer(SolArkRegisterMap::SN_BYTE_02 - block.start_register);
            serial_number_parts[2] = getResponseBuffer(SolArkRegisterMap::SN_BYTE_03 - block.start_register);
            serial_number_parts[3] = getResponseBuffer(SolArkRegisterMap::SN_BYTE_04 - block.start_register);
            serial_number_parts[4] = getResponseBuffer(SolArkRegisterMap::SN_BYTE_05 - block.start_register);
            break;
        default:
            Serial.printf("WARNING - SolArk: Unknown block type %d in processBlock\n", static_cast<int>(block.type));
            break;
    }
}

// Diagnostic Status Getters
uint16_t Modbus_SolArkLV::getCommVersion() {
    return comm_version_val;
}

uint16_t Modbus_SolArkLV::getSerialNumberPart(uint8_t index) {
    if (index < 5) {
        return serial_number_parts[index];
    }
    return 0; // Or some error indicator
}

float Modbus_SolArkLV::getIGBTTemp() {
    return igbt_temp;
}

float Modbus_SolArkLV::getDCDCTemp() {
    return dcdc_xfrmr_temp;
}

// Generator Status Getters
uint8_t Modbus_SolArkLV::getGeneratorRelayStatus() {
    return generator_relay_status;
}

uint8_t Modbus_SolArkLV::getGridType() {
    return grid_type;
}

// Battery Status Getters
float Modbus_SolArkLV::getBatteryPower() {
    return battery_power;
}

float Modbus_SolArkLV::getBatteryCurrent() {
    return battery_current;
}

float Modbus_SolArkLV::getBatteryVoltage() {
    return battery_voltage;
}

float Modbus_SolArkLV::getBatterySOC() {
    return battery_soc;
}

float Modbus_SolArkLV::getBatteryTemperature() {
    return battery_temperature;
}

float Modbus_SolArkLV::getBatteryTemperatureF() {
    return (battery_temperature * 9.0f / 5.0f) + 32.0f;
}

// Energy Getters
float Modbus_SolArkLV::getBatteryChargeEnergy() {
    return battery_charge_energy;
}

float Modbus_SolArkLV::getBatteryDischargeEnergy() {
    return battery_discharge_energy;
}

float Modbus_SolArkLV::getGridBuyEnergy() {
    return grid_buy_energy;
}

float Modbus_SolArkLV::getGridSellEnergy() {
    return grid_sell_energy;
}

float Modbus_SolArkLV::getLoadEnergy() {
    return load_energy;
}

float Modbus_SolArkLV::getPVEnergy() {
    return pv_energy;
}

// Power Getters
float Modbus_SolArkLV::getGridPower() {
    return grid_power;
}

float Modbus_SolArkLV::getInverterPower() {
    return inverter_output_power;
}

float Modbus_SolArkLV::getLoadPowerL1() {
    return load_power_l1;
}

float Modbus_SolArkLV::getLoadPowerL2() {
    return load_power_l2;
}

float Modbus_SolArkLV::getLoadPowerTotal() {
    return load_power_total;
}

float Modbus_SolArkLV::getPV1Power() {
    return pv1_power;
}

float Modbus_SolArkLV::getPV2Power() {
    return pv2_power;
}

float Modbus_SolArkLV::getPVPowerTotal() {
    return pv_power_total;
}

float Modbus_SolArkLV::getSmartLoadPower() {
    return smart_load_power;
}

// Grid Status Getters
float Modbus_SolArkLV::getGridVoltage() {
    return grid_voltage;
}

float Modbus_SolArkLV::getGridCurrentL1() {
    return grid_current_l1;
}

float Modbus_SolArkLV::getGridCurrentL2() {
    return grid_current_l2;
}



float Modbus_SolArkLV::getGridFrequency() {
    return grid_frequency;
}

uint8_t Modbus_SolArkLV::getGridRelayStatus() {
    return grid_relay_status;
}

// Inverter Status Getters
float Modbus_SolArkLV::getInverterVoltage() {
    return inverter_voltage;
}

float Modbus_SolArkLV::getInverterVoltageLN() {
    return inverter_voltage_ln;
}

float Modbus_SolArkLV::getInverterVoltageL2N() {
    return inverter_voltage_l2n;
}

float Modbus_SolArkLV::getInverterPowerL1() {
    return inverter_power_l1;
}

float Modbus_SolArkLV::getInverterPowerL2() {
    return inverter_power_l2;
}

float Modbus_SolArkLV::getInverterCurrentL1() {
    return inverter_current_l1;
}

float Modbus_SolArkLV::getInverterCurrentL2() {
    return inverter_current_l2;
}

float Modbus_SolArkLV::getInverterFrequency() {
    return inverter_frequency;
}

uint8_t Modbus_SolArkLV::getInverterStatus() {
    return inverter_status;
}

// Load Status Getters
float Modbus_SolArkLV::getLoadCurrentL1() {
    return load_current_l1;
}

float Modbus_SolArkLV::getLoadCurrentL2() {
    return load_current_l2;
}

float Modbus_SolArkLV::getLoadFrequency() {
    return load_frequency;
}

// Convenience Methods
bool Modbus_SolArkLV::isGridConnected() {
    return grid_relay_status > 0;
}

bool Modbus_SolArkLV::isGeneratorConnected() {
    return generator_relay_status > 0;
}

bool Modbus_SolArkLV::isBatteryCharging() {
    return battery_power < 0;
}

bool Modbus_SolArkLV::isBatteryDischarging() {
    return battery_power > 0;
}

bool Modbus_SolArkLV::isSellingToGrid() {
    return grid_power < 0;
}

bool Modbus_SolArkLV::isBuyingFromGrid() {
    return grid_power > 0;
}

// Battery Configuration Getters (for SunSpec model 713)
float Modbus_SolArkLV::getBatteryCapacity() {
    return battery_capacity;
}

float Modbus_SolArkLV::getCorrectedBatteryCapacity() {
    return corrected_battery_capacity;
}

float Modbus_SolArkLV::getBatteryEmptyVoltage() {
    return battery_empty_voltage;
}

float Modbus_SolArkLV::getBatteryShutdownVoltage() {
    return battery_shutdown_voltage;
}

float Modbus_SolArkLV::getBatteryRestartVoltage() {
    return battery_restart_voltage;
}

float Modbus_SolArkLV::getBatteryLowVoltage() {
    return battery_low_voltage;
}

uint8_t Modbus_SolArkLV::getBatteryShutdownPercent() {
    return battery_shutdown_percent;
}

uint8_t Modbus_SolArkLV::getBatteryRestartPercent() {
    return battery_restart_percent;
}

uint8_t Modbus_SolArkLV::getBatteryLowPercent() {
    return battery_low_percent;
}

// BMS Getters (for lithium batteries)
float Modbus_SolArkLV::getBMSChargingVoltage() {
    return bms_charging_voltage;
}

float Modbus_SolArkLV::getBMSDischargeVoltage() {
    return bms_discharge_voltage;
}

float Modbus_SolArkLV::getBMSChargingCurrentLimit() {
    return bms_charging_current_limit;
}

float Modbus_SolArkLV::getBMSDischargeCurrentLimit() {
    return bms_discharge_current_limit;
}

float Modbus_SolArkLV::getBMSRealTimeSOC() {
    return bms_real_time_soc;
}

float Modbus_SolArkLV::getBMSRealTimeVoltage() {
    return bms_real_time_voltage;
}

float Modbus_SolArkLV::getBMSRealTimeCurrent() {
    return bms_real_time_current;
}

float Modbus_SolArkLV::getBMSRealTimeTemp() {
    return bms_real_time_temp;
}

uint16_t Modbus_SolArkLV::getBMSWarning() {
    return bms_warning;
}

uint16_t Modbus_SolArkLV::getBMSFault() {
    return bms_fault;
}

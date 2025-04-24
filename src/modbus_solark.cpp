#include <modbus_solark.h>
#include <TimeLib.h>

#define SOLARK_POLL_INTERVAL 5000 // 5 seconds

Modbus_SolArkLV::Modbus_SolArkLV() {
    // Initialize class variables
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
    
    // Grid variables
    grid_voltage = 0;
    grid_current_l1 = 0;
    grid_current_l2 = 0;
    grid_frequency = 0;
    
    // Inverter variables
    inverter_voltage = 0;
    inverter_current_l1 = 0;
    inverter_current_l2 = 0;
    
    // Load variables
    load_current_l1 = 0;
    load_current_l2 = 0;
}

uint8_t Modbus_SolArkLV::get_modbus_address() {
    return modbus_address;
}

void Modbus_SolArkLV::set_modbus_address(uint8_t addr) {
    modbus_address = addr;
}

uint8_t Modbus_SolArkLV::poll() {
    uint8_t result = ku8MBSuccess;
    
    // Poll energy data - registers 70-85
    result = readHoldingRegisters(SolArkRegisterMap::BATTERY_CHARGE_ENERGY, 15);
    if (result == ku8MBSuccess) {
        // Process energy data
        battery_charge_energy = getResponseBuffer(0) / SolArkScalingFactors::ENERGY; // Reg 70
        battery_discharge_energy = getResponseBuffer(1) / SolArkScalingFactors::ENERGY; // Reg 71
        grid_buy_energy = getResponseBuffer(6) / SolArkScalingFactors::ENERGY; // Reg 76
        grid_sell_energy = getResponseBuffer(7) / SolArkScalingFactors::ENERGY; // Reg 77
        grid_frequency = getResponseBuffer(9) / SolArkScalingFactors::FREQUENCY; // Reg 79
        load_energy = getResponseBuffer(14) / SolArkScalingFactors::ENERGY; // Reg 84
        
        Serial.println("INFO - SolArk: Energy data poll success");
    } else {
        Serial.println("INFO - SolArk: Energy data poll FAIL");
        timestamp_last_failure = now();
    }
    
    // Poll PV energy - register 108
    result = readHoldingRegisters(SolArkRegisterMap::PV_ENERGY, 1);
    if (result == ku8MBSuccess) {
        pv_energy = getResponseBuffer(0) / SolArkScalingFactors::ENERGY; // Reg 108
        Serial.println("INFO - SolArk: PV energy poll success");
    } else {
        Serial.println("INFO - SolArk: PV energy poll FAIL");
    }
    
    // Poll grid and inverter data - registers 150-170
    result = readHoldingRegisters(150, 20);
    if (result == ku8MBSuccess) {
        // Process grid and inverter data
        grid_voltage = getResponseBuffer(SolArkRegisterMap::GRID_VOLTAGE - 150) / SolArkScalingFactors::VOLTAGE;
        inverter_voltage = getResponseBuffer(SolArkRegisterMap::INVERTER_VOLTAGE - 150) / SolArkScalingFactors::VOLTAGE;
        
        grid_current_l1 = getResponseBuffer(SolArkRegisterMap::GRID_CURRENT_L1 - 150) / SolArkScalingFactors::CURRENT;
        grid_current_l2 = getResponseBuffer(SolArkRegisterMap::GRID_CURRENT_L2 - 150) / SolArkScalingFactors::CURRENT;
        
        grid_CT_current_l1 = getResponseBuffer(SolArkRegisterMap::GRID_CT_CURRENT_L1 - 150) / SolArkScalingFactors::CURRENT;
        grid_CT_current_l2 = getResponseBuffer(SolArkRegisterMap::GRID_CT_CURRENT_L2 - 150) / SolArkScalingFactors::CURRENT;

        inverter_current_l1 = getResponseBuffer(SolArkRegisterMap::INVERTER_CURRENT_L1 - 150) / SolArkScalingFactors::CURRENT;
        inverter_current_l2 = getResponseBuffer(SolArkRegisterMap::INVERTER_CURRENT_L2 - 150) / SolArkScalingFactors::CURRENT;
        
        smart_load_power = getResponseBuffer(SolArkRegisterMap::SMART_LOAD_POWER - 150);
        grid_power = getResponseBuffer(SolArkRegisterMap::GRID_POWER - 150);
        
        // Apply sign correction to grid power
        grid_power = correctSignedValue(grid_power);
        
        Serial.println("INFO - SolArk: Grid/Inverter data poll success");
    } else {
        Serial.println("INFO - SolArk: Grid/Inverter data poll FAIL");
    }
    
    // Poll power and battery data - registers 170-190
    result = readHoldingRegisters(170, 20);
    if (result == ku8MBSuccess) {
        // Process power and battery data
        inverter_output_power = getResponseBuffer(SolArkRegisterMap::INVERTER_OUTPUT_POWER - 170); 
        load_power_l1 = getResponseBuffer(SolArkRegisterMap::LOAD_POWER_L1 - 170); 
        load_power_l2 = getResponseBuffer(SolArkRegisterMap::LOAD_POWER_L2 - 170); 
        load_power_total = getResponseBuffer(SolArkRegisterMap::LOAD_POWER_TOTAL - 170); 
        load_current_l1 = getResponseBuffer(SolArkRegisterMap::LOAD_CURRENT_L1 - 170) / SolArkScalingFactors::CURRENT;
        load_current_l2 = getResponseBuffer(SolArkRegisterMap::LOAD_CURRENT_L2 - 170) / SolArkScalingFactors::CURRENT;
        
        battery_temperature = (getResponseBuffer(SolArkRegisterMap::BATTERY_TEMPERATURE - 170) - 
                              SolArkScalingFactors::TEMPERATURE_OFFSET) / SolArkScalingFactors::TEMPERATURE_SCALE;
        battery_voltage = getResponseBuffer(SolArkRegisterMap::BATTERY_VOLTAGE - 170) / SolArkScalingFactors::CURRENT;
        battery_soc = getResponseBuffer(SolArkRegisterMap::BATTERY_SOC - 170);
        
        pv1_power = getResponseBuffer(SolArkRegisterMap::PV1_POWER - 170);
        pv2_power = getResponseBuffer(SolArkRegisterMap::PV2_POWER - 170);
        pv_power_total = (pv1_power + pv2_power) / 1000.0f; // Total in kW
        
        // Apply sign correction to inverter power
        inverter_output_power = correctSignedValue(inverter_output_power);
        
        Serial.println("INFO - SolArk: Power/Battery data poll success");
    } else {
        Serial.println("INFO - SolArk: Power/Battery data poll FAIL");
    }
    
    // Poll battery status - registers 190-199
    result = readHoldingRegisters(SolArkRegisterMap::BATTERY_POWER, 10);
    if (result == ku8MBSuccess) {
        // Process battery status
        battery_power = correctSignedValue(getResponseBuffer(0));
        
        // Handle battery current with sign correction and scaling
        uint16_t battCurrentRaw = getResponseBuffer(SolArkRegisterMap::BATTERY_CURRENT - SolArkRegisterMap::BATTERY_POWER);
        battery_current = correctSignedValue(battCurrentRaw) / SolArkScalingFactors::CURRENT;
        
        load_frequency = getResponseBuffer(SolArkRegisterMap::LOAD_FREQUENCY - SolArkRegisterMap::BATTERY_POWER) / 
                         SolArkScalingFactors::FREQUENCY;
        
        inverter_frequency = getResponseBuffer(SolArkRegisterMap::INVERTER_FREQUENCY - SolArkRegisterMap::BATTERY_POWER) / 
                            SolArkScalingFactors::FREQUENCY;
        
        grid_relay_status = getResponseBuffer(SolArkRegisterMap::GRID_RELAY_STATUS - SolArkRegisterMap::BATTERY_POWER);
        generator_relay_status = getResponseBuffer(SolArkRegisterMap::GENERATOR_RELAY_STATUS - SolArkRegisterMap::BATTERY_POWER);
        
        Serial.println("INFO - SolArk: Battery status poll success");
    } else {
        Serial.println("INFO - SolArk: Battery status poll FAIL");
    }
    
    // Report timestamp of successful poll
    if (result == ku8MBSuccess) {
        timestamp_last_report = now();
    }
    
    return result;
}

uint8_t Modbus_SolArkLV::query_register(uint16_t reg) {
    uint8_t result = readHoldingRegisters(reg, 1);
    
    if (result == ku8MBSuccess) {
        route_poll_response(reg, getResponseBuffer(0));
    } else {
        timestamp_last_failure = now();
        Serial.printf("ERROR - SolArk: Query register 0x%04X FAIL\n", reg);
    }
    
    return result;
}

void Modbus_SolArkLV::route_poll_response(uint16_t reg, uint16_t response) {
    // Route specific register responses based on register address
    switch (reg) {
        // Energy registers (70-108)
        case SolArkRegisterMap::BATTERY_CHARGE_ENERGY:
            battery_charge_energy = response / SolArkScalingFactors::ENERGY;
            Serial.printf("SolArk: Battery charge energy: %.1f kWh\n", battery_charge_energy);
            break;
        case SolArkRegisterMap::BATTERY_DISCHARGE_ENERGY:
            battery_discharge_energy = response / SolArkScalingFactors::ENERGY;
            Serial.printf("SolArk: Battery discharge energy: %.1f kWh\n", battery_discharge_energy);
            break;
        case SolArkRegisterMap::GRID_BUY_ENERGY:
            grid_buy_energy = response / SolArkScalingFactors::ENERGY;
            Serial.printf("SolArk: Grid buy energy: %.1f kWh\n", grid_buy_energy);
            break;
        case SolArkRegisterMap::GRID_SELL_ENERGY:
            grid_sell_energy = response / SolArkScalingFactors::ENERGY;
            Serial.printf("SolArk: Grid sell energy: %.1f kWh\n", grid_sell_energy);
            break;
        case SolArkRegisterMap::GRID_FREQUENCY:
            grid_frequency = response / SolArkScalingFactors::FREQUENCY;
            Serial.printf("SolArk: Grid frequency: %.2f Hz\n", grid_frequency);
            break;
        case SolArkRegisterMap::LOAD_ENERGY:
            load_energy = response / SolArkScalingFactors::ENERGY;
            Serial.printf("SolArk: Load energy: %.1f kWh\n", load_energy);
            break;
        case SolArkRegisterMap::PV_ENERGY:
            pv_energy = response / SolArkScalingFactors::ENERGY;
            Serial.printf("SolArk: PV energy: %.1f kWh\n", pv_energy);
            break;
            
        // Voltage registers (150-156)
        case SolArkRegisterMap::GRID_VOLTAGE:
            grid_voltage = response / SolArkScalingFactors::VOLTAGE;
            Serial.printf("SolArk: Grid voltage: %.1f V\n", grid_voltage);
            break;
        case SolArkRegisterMap::INVERTER_VOLTAGE:
            inverter_voltage = response / SolArkScalingFactors::VOLTAGE;
            Serial.printf("SolArk: Inverter voltage: %.1f V\n", inverter_voltage);
            break;
            
        // Current registers (160-165, 179-180)
        case SolArkRegisterMap::GRID_CURRENT_L1:
            grid_current_l1 = response / SolArkScalingFactors::CURRENT;
            Serial.printf("SolArk: Grid current L1: %.2f A\n", grid_current_l1);
            break;
        case SolArkRegisterMap::GRID_CURRENT_L2:
            grid_current_l2 = response / SolArkScalingFactors::CURRENT;
            Serial.printf("SolArk: Grid current L2: %.2f A\n", grid_current_l2);
            break;

        case SolArkRegisterMap::GRID_CT_CURRENT_L1:
            grid_CT_current_l1 = response / SolArkScalingFactors::CURRENT;
            Serial.printf("SolArk: Grid CT current L1: %.2f A\n", grid_CT_current_l1);
            break;
        case SolArkRegisterMap::GRID_CT_CURRENT_L2:
            grid_CT_current_l2 = response / SolArkScalingFactors::CURRENT;
            Serial.printf("SolArk: Grid CT current L2: %.2f A\n", grid_CT_current_l2);
            break;

        case SolArkRegisterMap::INVERTER_CURRENT_L1:
            inverter_current_l1 = response / SolArkScalingFactors::CURRENT;
            Serial.printf("SolArk: Inverter current L1: %.2f A\n", inverter_current_l1);
            break;
        case SolArkRegisterMap::INVERTER_CURRENT_L2:
            inverter_current_l2 = response / SolArkScalingFactors::CURRENT;
            Serial.printf("SolArk: Inverter current L2: %.2f A\n", inverter_current_l2);
            break;
        case SolArkRegisterMap::LOAD_CURRENT_L1:
            load_current_l1 = response / SolArkScalingFactors::CURRENT;
            Serial.printf("SolArk: Load current L1: %.2f A\n", load_current_l1);
            break;
        case SolArkRegisterMap::LOAD_CURRENT_L2:
            load_current_l2 = response / SolArkScalingFactors::CURRENT;
            Serial.printf("SolArk: Load current L2: %.2f A\n", load_current_l2);
            break;
            
        // Power registers (166-169, 175-178, 186-187, 190)
        case SolArkRegisterMap::SMART_LOAD_POWER:
            smart_load_power = response;
            Serial.printf("SolArk: Smart load power: %d W\n", smart_load_power);
            break;
        case SolArkRegisterMap::GRID_POWER:
            grid_power = correctSignedValue(response);
            Serial.printf("SolArk: Grid power: %d W\n", grid_power);
            break;
        case SolArkRegisterMap::INVERTER_OUTPUT_POWER:
            inverter_output_power = correctSignedValue(response);
            Serial.printf("SolArk: Inverter output power: %d W\n", inverter_output_power);
            break;
        case SolArkRegisterMap::LOAD_POWER_L1:
            load_power_l1 = response;
            Serial.printf("SolArk: Load power L1: %d W\n", load_power_l1);
            break;
        case SolArkRegisterMap::LOAD_POWER_L2:
            load_power_l2 = response;
            Serial.printf("SolArk: Load power L2: %d W\n", load_power_l2);
            break;
        case SolArkRegisterMap::LOAD_POWER_TOTAL:
            load_power_total = response;
            Serial.printf("SolArk: Load power total: %d W\n", load_power_total);
            break;
        case SolArkRegisterMap::PV1_POWER:
            pv1_power = response;
            Serial.printf("SolArk: PV1 power: %d W\n", pv1_power);
            pv_power_total = (pv1_power + pv2_power) / 1000.0f;
            break;
        case SolArkRegisterMap::PV2_POWER:
            pv2_power = response;
            Serial.printf("SolArk: PV2 power: %d W\n", pv2_power);
            pv_power_total = (pv1_power + pv2_power) / 1000.0f;
            break;
        case SolArkRegisterMap::BATTERY_POWER:
            battery_power = correctSignedValue(response);
            Serial.printf("SolArk: Battery power: %d W\n", battery_power);
            break;
        
        // Battery data registers (182-184, 191)
        case SolArkRegisterMap::BATTERY_TEMPERATURE:
            battery_temperature = (response - SolArkScalingFactors::TEMPERATURE_OFFSET) / SolArkScalingFactors::TEMPERATURE_SCALE;
            Serial.printf("SolArk: Battery temperature: %.1f C\n", battery_temperature);
            break;
        case SolArkRegisterMap::BATTERY_VOLTAGE:
            battery_voltage = response / SolArkScalingFactors::CURRENT;
            Serial.printf("SolArk: Battery voltage: %.2f V\n", battery_voltage);
            break;
        case SolArkRegisterMap::BATTERY_SOC:
            battery_soc = response;
            Serial.printf("SolArk: Battery SOC: %d%%\n", battery_soc);
            break;
        case SolArkRegisterMap::BATTERY_CURRENT:
            battery_current = correctSignedValue(response) / SolArkScalingFactors::CURRENT;
            Serial.printf("SolArk: Battery current: %.2f A\n", battery_current);
            break;
            
        // Other status registers (192-195)
        case SolArkRegisterMap::LOAD_FREQUENCY:
            load_frequency = response / SolArkScalingFactors::FREQUENCY;
            Serial.printf("SolArk: Load frequency: %.2f Hz\n", load_frequency);
            break;
        case SolArkRegisterMap::INVERTER_FREQUENCY:
            inverter_frequency = response / SolArkScalingFactors::FREQUENCY;
            Serial.printf("SolArk: Inverter output frequency: %.2f Hz\n", inverter_frequency);
            break;
        case SolArkRegisterMap::GRID_RELAY_STATUS:
            grid_relay_status = response;
            Serial.printf("SolArk: Grid relay status: %d\n", grid_relay_status);
            break;
        case SolArkRegisterMap::GENERATOR_RELAY_STATUS:
            generator_relay_status = response;
            Serial.printf("SolArk: Generator relay status: %d\n", generator_relay_status);
            break;
            
        default:
            Serial.printf("SolArk: Unknown register: 0x%04X, value: 0x%04X\n", reg, response);
            break;
    }
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

float Modbus_SolArkLV::getInverterCurrentL1() {
    return inverter_current_l1;
}

float Modbus_SolArkLV::getInverterCurrentL2() {
    return inverter_current_l2;
}

float Modbus_SolArkLV::getInverterFrequency() {
    return inverter_frequency;
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

// Generator Status Getters
uint8_t Modbus_SolArkLV::getGeneratorRelayStatus() {
    return generator_relay_status;
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

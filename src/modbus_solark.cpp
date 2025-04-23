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
    result = readHoldingRegisters(70, 15);
    if (result == ku8MBSuccess) {
        // Process energy data
        battery_charge_energy = getResponseBuffer(0) / 10.0f; // Reg 70
        battery_discharge_energy = getResponseBuffer(1) / 10.0f; // Reg 71
        grid_buy_energy = getResponseBuffer(6) / 10.0f; // Reg 76
        grid_sell_energy = getResponseBuffer(7) / 10.0f; // Reg 77
        grid_frequency = getResponseBuffer(9) / 100.0f; // Reg 79
        load_energy = getResponseBuffer(14) / 10.0f; // Reg 84
        
        Serial.println("INFO - SolArk: Energy data poll success");
    } else {
        Serial.println("INFO - SolArk: Energy data poll FAIL");
        timestamp_last_failure = now();
    }
    
    // Poll PV energy - register 108
    result = readHoldingRegisters(108, 1);
    if (result == ku8MBSuccess) {
        pv_energy = getResponseBuffer(0) / 10.0f; // Reg 108
        Serial.println("INFO - SolArk: PV energy poll success");
    } else {
        Serial.println("INFO - SolArk: PV energy poll FAIL");
    }
    
    // Poll grid and inverter data - registers 150-170
    result = readHoldingRegisters(150, 20);
    if (result == ku8MBSuccess) {
        // Process grid and inverter data
        grid_voltage = getResponseBuffer(2) / 10.0f; // Reg 152 (Grid voltage L1-L2)
        inverter_voltage = getResponseBuffer(6) / 10.0f; // Reg 156 (Inverter voltage L1-L2)
        grid_current_l1 = getResponseBuffer(10) / 100.0f; // Reg 160
        grid_current_l2 = getResponseBuffer(11) / 100.0f; // Reg 161
        
        grid_CT_current_l1 = getResponseBuffer(12) / 100.0f; // Reg 162
        grid_CT_current_l2 = getResponseBuffer(13) / 100.0f; // Reg 163

        inverter_current_l1 = getResponseBuffer(14) / 100.0f; // Reg 164
        inverter_current_l2 = getResponseBuffer(15) / 100.0f; // Reg 165
        smart_load_power = getResponseBuffer(16); // Reg 166
        grid_power = getResponseBuffer(19); // Reg 169
        
        // Check if grid power needs sign correction
        if (grid_power > 32767) {
            grid_power = (grid_power - 65535);
        }
        
        Serial.println("INFO - SolArk: Grid/Inverter data poll success");
    } else {
        Serial.println("INFO - SolArk: Grid/Inverter data poll FAIL");
    }
    
    // Poll power and battery data - registers 170-190
    result = readHoldingRegisters(170, 20);
    if (result == ku8MBSuccess) {
        // Process power and battery data
        inverter_output_power = getResponseBuffer(5); // Reg 175
        load_power_l1 = getResponseBuffer(6); // Reg 176
        load_power_l2 = getResponseBuffer(7); // Reg 177
        load_power_total = getResponseBuffer(8); // Reg 178
        load_current_l1 = getResponseBuffer(9) / 100.0f; // Reg 179
        load_current_l2 = getResponseBuffer(10) / 100.0f; // Reg 180
        battery_temperature = (getResponseBuffer(12) - 1000) / 10.0f; // Reg 182
        battery_voltage = getResponseBuffer(13) / 100.0f; // Reg 183
        battery_soc = getResponseBuffer(14); // Reg 184
        pv1_power = getResponseBuffer(16); // Reg 186
        pv2_power = getResponseBuffer(17); // Reg 187
        pv_power_total = (pv1_power + pv2_power) / 1000.0f; // Total in kW
        
        // Check if inverter power needs sign correction
        if (inverter_output_power > 32767) {
            inverter_output_power = (inverter_output_power - 65535);
        }
        
        Serial.println("INFO - SolArk: Power/Battery data poll success");
    } else {
        Serial.println("INFO - SolArk: Power/Battery data poll FAIL");
    }
    
    // Poll battery status - registers 190-199
    result = readHoldingRegisters(190, 10);
    if (result == ku8MBSuccess) {
        // Process battery status
        battery_power = getResponseBuffer(0); // Reg 190
        if (battery_power > 32767) {
            battery_power = (battery_power - 65535);
        }
        
        battery_current = getResponseBuffer(1); // Reg 191
        if (battery_current > 32767) {
            battery_current = (battery_current - 65535) / 100.0f;
        } else {
            battery_current = battery_current / 100.0f;
        }
        
        load_frequency = getResponseBuffer(2) / 100.0f; // Reg 192
        inverter_frequency = getResponseBuffer(3) / 100.0f; // Reg 193
        grid_relay_status = getResponseBuffer(4); // Reg 194
        generator_relay_status = getResponseBuffer(5); // Reg 195
        
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
        case 70:
            battery_charge_energy = response / 10.0f;
            Serial.printf("SolArk: Battery charge energy: %.1f kWh\n", battery_charge_energy);
            break;
        case 71:
            battery_discharge_energy = response / 10.0f;
            Serial.printf("SolArk: Battery discharge energy: %.1f kWh\n", battery_discharge_energy);
            break;
        case 76:
            grid_buy_energy = response / 10.0f;
            Serial.printf("SolArk: Grid buy energy: %.1f kWh\n", grid_buy_energy);
            break;
        case 77:
            grid_sell_energy = response / 10.0f;
            Serial.printf("SolArk: Grid sell energy: %.1f kWh\n", grid_sell_energy);
            break;
        case 79:
            grid_frequency = response / 100.0f;
            Serial.printf("SolArk: Grid frequency: %d\n", grid_frequency);
            break;
        case 84:
            load_energy = response / 10.0f;
            Serial.printf("SolArk: Load energy: %.1f kWh\n", load_energy);
            break;
        case 108:
            pv_energy = response / 10.0f;
            Serial.printf("SolArk: PV energy: %.1f kWh\n", pv_energy);
            break;
            
        // Voltage registers (150-156)
        case 152:
            grid_voltage = response / 10.0f;
            Serial.printf("SolArk: Grid voltage: %.1f V\n", grid_voltage);
            break;
        case 156:
            inverter_voltage = response / 10.0f;
            Serial.printf("SolArk: Inverter voltage: %.1f V\n", inverter_voltage);
            break;
            
        // Current registers (160-165, 179-180)
        case 160:
            grid_current_l1 = response / 100.0f;
            Serial.printf("SolArk: Grid current L1: %.2f A\n", grid_current_l1);
            break;
        case 161:
            grid_current_l2 = response / 100.0f;
            Serial.printf("SolArk: Grid current L2: %.2f A\n", grid_current_l2);
            break;

        case 162:
            grid_CT_current_l1 = response / 100.0f;
            Serial.printf("SolArk: Grid CT current L1: %.2f A\n", grid_current_l1);
            break;
        case 163:
            grid_CT_current_l2 = response / 100.0f;
            Serial.printf("SolArk: Grid CT current L2: %.2f A\n", grid_current_l2);
            break;

        case 164:
            inverter_current_l1 = response / 100.0f;
            Serial.printf("SolArk: Inverter current L1: %.2f A\n", inverter_current_l1);
            break;
        case 165:
            inverter_current_l2 = response / 100.0f;
            Serial.printf("SolArk: Inverter current L2: %.2f A\n", inverter_current_l2);
            break;
        case 179:
            load_current_l1 = response / 100.0f;
            Serial.printf("SolArk: Load current L1: %.2f A\n", load_current_l1);
            break;
        case 180:
            load_current_l2 = response / 100.0f;
            Serial.printf("SolArk: Load current L2: %.2f A\n", load_current_l2);
            break;
            
        // Power registers (166-169, 175-178, 186-187, 190)
        case 166:
            smart_load_power = response;
            Serial.printf("SolArk: Smart load power: %d W\n", smart_load_power);
            break;
        case 169: {
            int16_t signedValue = response;
            if (response > 32767) {
                signedValue = response - 65535;
            }
            grid_power = signedValue;
            Serial.printf("SolArk: Grid power: %d W\n", grid_power);
            break;
        }
        case 175: {
            int16_t signedValue = response;
            if (response > 32767) {
                signedValue = response - 65535;
            }
            inverter_output_power = signedValue;
            Serial.printf("SolArk: Inverter output power: %d W\n", inverter_output_power);
            break;
        }
        case 176:
            load_power_l1 = response;
            Serial.printf("SolArk: Load power L1: %d W\n", load_power_l1);
            break;
        case 177:
            load_power_l2 = response;
            Serial.printf("SolArk: Load power L2: %d W\n", load_power_l2);
            break;
        case 178:
            load_power_total = response;
            Serial.printf("SolArk: Load power total: %d W\n", load_power_total);
            break;
        case 186:
            pv1_power = response;
            Serial.printf("SolArk: PV1 power: %d W\n", pv1_power);
            pv_power_total = (pv1_power + pv2_power) / 1000.0f;
            break;
        case 187:
            pv2_power = response;
            Serial.printf("SolArk: PV2 power: %d W\n", pv2_power);
            pv_power_total = (pv1_power + pv2_power) / 1000.0f;
            break;
        case 190: {
            int16_t signedValue = response;
            if (response > 32767) {
                signedValue = response - 65535;
            }
            battery_power = signedValue;
            Serial.printf("SolArk: Battery power: %d W\n", battery_power);
            break;
        }
        
        // Battery data registers (182-184, 191)
        case 182:
            battery_temperature = (response - 1000) / 10.0f;
            Serial.printf("SolArk: Battery temperature: %.1f C\n", battery_temperature);
            break;
        case 183:
            battery_voltage = response / 100.0f;
            Serial.printf("SolArk: Battery voltage: %.2f V\n", battery_voltage);
            break;
        case 184:
            battery_soc = response;
            Serial.printf("SolArk: Battery SOC: %d%%\n", battery_soc);
            break;
        case 191: {
            float current;
            if (response > 32767) {
                current = (response - 65535) / 100.0f;
            } else {
                current = response / 100.0f;
            }
            battery_current = current;
            Serial.printf("SolArk: Battery current: %.2f A\n", battery_current);
            break;
        }
            
        // Other status registers (192-195)
        case 192:
            load_frequency = response / 100.0f;
            Serial.printf("SolArk: Load frequency: %.2f Hz\n", load_frequency);
            break;
        case 193:
            inverter_frequency = response / 100.0f;
            Serial.printf("SolArk: Inverter output frequency: %.2f Hz\n", inverter_frequency);
            break;
        case 194:
            grid_relay_status = response;
            Serial.printf("SolArk: Grid relay status: %d\n", grid_relay_status);
            break;
        case 195:
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
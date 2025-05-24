#include <modbus_evse.h>
#include <TimeLib.h>

Modbus_EVSE::Modbus_EVSE() {
    // Initialize all register cache variables to 0
    // Configuration registers (R/W) - Register IDs 86-139
    over_voltage_setpoint = 0;
    under_voltage_setpoint = 0;
    overcurrent_percentage = 0;
    remote_start_stop = 0;
    ext_meter_a_voltage_addr = 0;
    ext_meter_b_voltage_addr = 0;
    ext_meter_c_voltage_addr = 0;
    ext_meter_current_addr = 0;
    ext_meter_power_addr = 0;
    ext_meter_kwh_addr = 0;
    ext_meter_dlb_current_addr = 0;
    modbus_device_address = 0;
    dlb_max_startup_current = 0;
    dlb_max_protection_current = 0;
    dlb_max_ct_current = 0;
    dlb_current_calibration = 0;
    max_output_pwm_duty = 0;
    rcmu_function_select = 0;
    rfid_function_select = 0;
    lock_function_select = 0;
    cable_function_select = 0;
    dlb_function_select = 0;
    dlb_pid_p = 0;
    dlb_pid_i = 0;
    dlb_pid_d = 0;
    controller_id = 0;
    temp_correction_h = 0;
    temp_correction_l = 0;
    release_temp_setpoint = 0;
    max_temp_setpoint = 0;
    frequency_correction = 0;
    duty_cycle_correction = 0;
    trademark_selection = 0;
    number_of_poles = 0;
    for (int i = 0; i < 6; i++) {
        dial_settings[i] = 0;
    }
    
    // Status registers (Read-only) - Register IDs 140-166
    software_version = 0;
    current_status = 0;
    cable_gauge_pwm = 0;
    rcmu_status = 0;
    rfid_status = 0;
    lock_status = 0;
    dlb_current_value = 0;
    charging_current = 0;
    charging_voltage = 0;
    charging_power = 0;
    ref_current_ad = 0;
    rotary_switch_pwm = 0;
    current_output_pwm = 0;
    cp_positive_voltage = 0;
    cp_negative_voltage = 0;
    overcurrent_count = 0;
    small_current_count = 0;
    current_temperature = 0;
    temperature_ad = 0;
    meter_a_voltage = 0;
    meter_b_voltage = 0;
    meter_c_voltage = 0;
    meter_current = 0;
    meter_total_power = 0;
    meter_total_kwh = 0;
    dlb_meter_current = 0;
}

uint8_t Modbus_EVSE::get_modbus_address() {
    return modbus_address;
}

void Modbus_EVSE::set_modbus_address(uint8_t addr) {
    modbus_address = addr;
}

void Modbus_EVSE::route_poll_response(uint16_t reg, uint16_t response) {
    // Update register cache based on register ID from CSV
    switch (reg) {
        // Status registers (140-166)
        case rSOFTWARE_VERSION:
            Serial.printf("MODBUS EVSE: Software Version: %d.%03d\n", response / 1000, response % 1000);
            software_version = response;
            break;
        case rCURRENT_STATUS:
            Serial.printf("MODBUS EVSE: Current Status: %d (%s)\n", response, getStatusString());
            current_status = response;
            break;
        case rCABLE_GAUGE_PWM:
            Serial.printf("MODBUS EVSE: Cable Gauge PWM: %d.%02d%%\n", response / 100, response % 100);
            cable_gauge_pwm = response;
            break;
        case rRCMU_STATUS:
            Serial.printf("MODBUS EVSE: RCMU Status: 0x%04X\n", response);
            rcmu_status = response;
            break;
        case rRFID_STATUS:
            Serial.printf("MODBUS EVSE: RFID Status: 0x%04X\n", response);
            rfid_status = response;
            break;
        case rLOCK_STATUS:
            Serial.printf("MODBUS EVSE: Lock Status: %d\n", response);
            lock_status = response;
            break;
        case rDLB_CURRENT_VALUE:
            Serial.printf("MODBUS EVSE: DLB Current Value: %d\n", response);
            dlb_current_value = response;
            break;
        case rCHARGING_CURRENT:
            Serial.printf("MODBUS EVSE: Charging Current: %d A\n", response);
            charging_current = response;
            break;
        case rCHARGING_VOLTAGE:
            Serial.printf("MODBUS EVSE: Charging Voltage: %d V\n", response);
            charging_voltage = response;
            break;
        case rCHARGING_POWER:
            Serial.printf("MODBUS EVSE: Charging Power: %d W\n", response);
            charging_power = response;
            break;
        case rREF_CURRENT_AD:
            Serial.printf("MODBUS EVSE: Reference Current AD: %d\n", response);
            ref_current_ad = response;
            break;
        case rROTARY_SWITCH_PWM:
            Serial.printf("MODBUS EVSE: Rotary Switch PWM: %d.%02d%%\n", response / 100, response % 100);
            rotary_switch_pwm = response;
            break;
        case rCURRENT_OUTPUT_PWM:
            Serial.printf("MODBUS EVSE: Current Output PWM: %d.%02d%%\n", response / 100, response % 100);
            current_output_pwm = response;
            break;
        case rCP_POSITIVE_VOLTAGE:
            Serial.printf("MODBUS EVSE: CP Positive Voltage: %d\n", response);
            cp_positive_voltage = response;
            break;
        case rCP_NEGATIVE_VOLTAGE:
            Serial.printf("MODBUS EVSE: CP Negative Voltage: %d\n", response);
            cp_negative_voltage = response;
            break;
        case rOVERCURRENT_COUNT:
            Serial.printf("MODBUS EVSE: Overcurrent Count: %d\n", response);
            overcurrent_count = response;
            break;
        case rSMALL_CURRENT_COUNT:
            Serial.printf("MODBUS EVSE: Small Current Count: %d\n", response);
            small_current_count = response;
            break;
        case rCURRENT_TEMPERATURE:
            Serial.printf("MODBUS EVSE: Current Temperature: %d°C\n", response);
            current_temperature = response;
            break;
        case rTEMPERATURE_AD:
            Serial.printf("MODBUS EVSE: Temperature AD: %d\n", response);
            temperature_ad = response;
            break;
        case rMETER_A_VOLTAGE:
            Serial.printf("MODBUS EVSE: Meter A Voltage: %d V\n", response);
            meter_a_voltage = response;
            break;
        case rMETER_B_VOLTAGE:
            Serial.printf("MODBUS EVSE: Meter B Voltage: %d V\n", response);
            meter_b_voltage = response;
            break;
        case rMETER_C_VOLTAGE:
            Serial.printf("MODBUS EVSE: Meter C Voltage: %d V\n", response);
            meter_c_voltage = response;
            break;
        case rMETER_CURRENT:
            Serial.printf("MODBUS EVSE: Meter Current: %d A\n", response);
            meter_current = response;
            break;
        case rMETER_TOTAL_POWER:
            Serial.printf("MODBUS EVSE: Meter Total Power: %d W\n", response);
            meter_total_power = response;
            break;
        case rMETER_TOTAL_KWH_HIGH:
            Serial.printf("MODBUS EVSE: Meter Total kWh High: %d\n", response);
            meter_total_kwh = (meter_total_kwh & 0xFFFF) | ((uint32_t)response << 16);
            break;
        case rMETER_TOTAL_KWH_LOW:
            Serial.printf("MODBUS EVSE: Meter Total kWh Low: %d\n", response);
            meter_total_kwh = (meter_total_kwh & 0xFFFF0000) | response;
            break;
        case rDLB_METER_CURRENT:
            Serial.printf("MODBUS EVSE: DLB Meter Current: %d A\n", response);
            dlb_meter_current = response;
            break;
            
        // Configuration registers (86-139)
        case rOVER_VOLTAGE_SETPOINT:
            Serial.printf("MODBUS EVSE: Over Voltage Setpoint: %d.%02d V\n", response / 100, response % 100);
            over_voltage_setpoint = response;
            break;
        case rUNDER_VOLTAGE_SETPOINT:
            Serial.printf("MODBUS EVSE: Under Voltage Setpoint: %d.%02d V\n", response / 100, response % 100);
            under_voltage_setpoint = response;
            break;
        case rOVERCURRENT_PERCENTAGE:
            Serial.printf("MODBUS EVSE: Overcurrent Percentage: %d%%\n", response);
            overcurrent_percentage = response;
            break;
        case rREMOTE_START_STOP:
            Serial.printf("MODBUS EVSE: Remote Start/Stop: %d\n", response);
            remote_start_stop = response;
            break;
        case rMAX_OUTPUT_PWM_DUTY:
            Serial.printf("MODBUS EVSE: Max Output PWM Duty: %d.%02d%%\n", response / 100, response % 100);
            max_output_pwm_duty = response;
            break;
        case rDIAL_SETTING_1:
            Serial.printf("MODBUS EVSE: Dial Setting 1: %d.%02d%%\n", response / 100, response % 100);
            dial_settings[0] = response;
            break;
        case rDIAL_SETTING_2:
            Serial.printf("MODBUS EVSE: Dial Setting 2: %d.%02d%%\n", response / 100, response % 100);
            dial_settings[1] = response;
            break;
        case rDIAL_SETTING_3:
            Serial.printf("MODBUS EVSE: Dial Setting 3: %d.%02d%%\n", response / 100, response % 100);
            dial_settings[2] = response;
            break;
        case rDIAL_SETTING_4:
            Serial.printf("MODBUS EVSE: Dial Setting 4: %d.%02d%%\n", response / 100, response % 100);
            dial_settings[3] = response;
            break;
        case rDIAL_SETTING_5:
            Serial.printf("MODBUS EVSE: Dial Setting 5: %d.%02d%%\n", response / 100, response % 100);
            dial_settings[4] = response;
            break;
        case rDIAL_SETTING_6:
            Serial.printf("MODBUS EVSE: Dial Setting 6: %d.%02d%%\n", response / 100, response % 100);
            dial_settings[5] = response;
            break;
        default:
            Serial.printf("MODBUS EVSE: Unknown register: 0x%04X, value: 0x%04X\n", reg, response);
            break;
    }
}

uint8_t Modbus_EVSE::poll() {
    // Enhanced polling using the new register map
    Serial.println("MODBUS EVSE: Enhanced polling started");
    
    // Read critical status registers first (current status, charging current, voltage, power)
    uint8_t result = readHoldingRegisters(rCURRENT_STATUS, 4);
    
    if (result == ku8MBSuccess) {
        timestamp_last_report = now();
        
        // Update critical status variables
        current_status = getResponseBuffer(0);
        cable_gauge_pwm = getResponseBuffer(1);
        rcmu_status = getResponseBuffer(2);
        rfid_status = getResponseBuffer(3);
        
        Serial.println("MODBUS EVSE: Critical status poll success");
        Serial.printf("MODBUS EVSE: Status: %d (%s)\n", current_status, getStatusString());
        Serial.printf("MODBUS EVSE: RCMU: %s, RFID: %s\n", getRCMUStatusString(), getRFIDStatusString());
    } else {
        timestamp_last_failure = now();
        Serial.println("MODBUS EVSE: Critical status poll FAIL");
        return result;
    }
    
    // Read charging data (current, voltage, power)
    result = readHoldingRegisters(rCHARGING_CURRENT, 3);
    
    if (result == ku8MBSuccess) {
        charging_current = getResponseBuffer(0);
        charging_voltage = getResponseBuffer(1);
        charging_power = getResponseBuffer(2);
        
        Serial.printf("MODBUS EVSE: Charging: %dA, %dV, %dW\n", charging_current, charging_voltage, charging_power);
    } else {
        Serial.println("MODBUS EVSE: Charging data poll FAIL");
    }
    
    // Read PWM and CP data
    result = readHoldingRegisters(rCURRENT_OUTPUT_PWM, 2);
    
    if (result == ku8MBSuccess) {
        current_output_pwm = getResponseBuffer(0);
        cp_positive_voltage = getResponseBuffer(1);
        
        Serial.printf("MODBUS EVSE: PWM: %d.%02d%%, CP+: %d\n", 
                      current_output_pwm / 100, current_output_pwm % 100, cp_positive_voltage);
    } else {
        Serial.println("MODBUS EVSE: PWM/CP data poll FAIL");
    }
    
    // Read software version and lock status
    result = readHoldingRegisters(rSOFTWARE_VERSION, 1);
    if (result == ku8MBSuccess) {
        software_version = getResponseBuffer(0);
        Serial.printf("MODBUS EVSE: Software Version: %d.%03d\n", 
                      software_version / 1000, software_version % 1000);
    }
    
    result = readHoldingRegisters(rLOCK_STATUS, 1);
    if (result == ku8MBSuccess) {
        lock_status = getResponseBuffer(0);
        Serial.printf("MODBUS EVSE: Lock Status: %d\n", lock_status);
    }
    
    // Read some key configuration registers
    result = readHoldingRegisters(rREMOTE_START_STOP, 1);
    if (result == ku8MBSuccess) {
        remote_start_stop = getResponseBuffer(0);
        Serial.printf("MODBUS EVSE: Remote Start/Stop: %d\n", remote_start_stop);
    }
    
    result = readHoldingRegisters(rMAX_OUTPUT_PWM_DUTY, 1);
    if (result == ku8MBSuccess) {
        max_output_pwm_duty = getResponseBuffer(0);
        Serial.printf("MODBUS EVSE: Max PWM Duty: %d.%02d%%\n", 
                      max_output_pwm_duty / 100, max_output_pwm_duty % 100);
    }
    
    result = readHoldingRegisters(rDIAL_SETTING_1, 1);
    if (result == ku8MBSuccess) {
        dial_settings[0] = getResponseBuffer(0);
        Serial.printf("MODBUS EVSE: Min Current (Dial 1): %d.%02d%%\n", 
                      dial_settings[0] / 100, dial_settings[0] % 100);
    }
    
    Serial.printf("MODBUS EVSE: Poll complete - Charging: %s, Connected: %s\n", 
                  isCharging() ? "Yes" : "No", isConnected() ? "Yes" : "No");
    
    return ku8MBSuccess;
}

uint8_t Modbus_EVSE::query_register(uint16_t reg) {
    uint8_t result;
    
    // Determine if it's a status or config register and use appropriate function
    if (reg >= 0x1000 && reg < 0x2000) {
        result = readInputRegisters(reg, 1);
    } else if (reg >= 0x2000 && reg < 0x3000) {
        result = readHoldingRegisters(reg, 1);
    } else {
        Serial.printf("MODBUS EVSE: Invalid register address: 0x%04X\n", reg);
        return ku8MBIllegalDataAddress;
    }
    
    if (result == ku8MBSuccess) {
        route_poll_response(reg, getResponseBuffer(0));
    } else {
        timestamp_last_failure = now();
        Serial.printf("MODBUS EVSE: Query register 0x%04X FAIL\n", reg);
    }
    
    return result;
}

uint8_t Modbus_EVSE::write_register(uint16_t reg, uint16_t value) {
    // Only allow writing to configuration registers (86-139)
    if (reg < rOVER_VOLTAGE_SETPOINT || reg > rSPARE_139) {
        Serial.printf("MODBUS EVSE: Cannot write to register %d, not a configuration register (valid range: %d-%d)\n", 
                      reg, rOVER_VOLTAGE_SETPOINT, rSPARE_139);
        return ku8MBIllegalDataAddress;
    }
    
    // Check if it's a read-only status register
    if (reg >= rSOFTWARE_VERSION && reg <= rDLB_METER_CURRENT) {
        Serial.printf("MODBUS EVSE: Cannot write to register %d, it's a read-only status register\n", reg);
        return ku8MBIllegalDataAddress;
    }
    
    uint8_t result = writeSingleRegister(reg, value);
    
    if (result == ku8MBSuccess) {
        Serial.printf("MODBUS EVSE: Write to register %d value 0x%04X success\n", reg, value);
        
        // Update our local copy of the value using route_poll_response
        route_poll_response(reg, value);
    } else {
        Serial.printf("MODBUS EVSE: Write to register %d FAIL\n", reg);
    }
    
    return result;
}

// Convenience functions
bool Modbus_EVSE::isCharging() {
    // Based on CSV status values: 5 = "Charging: CP connect to diode+2.7KΩ parallel connect 1.3KΩ"
    return (current_status == 5);
}

bool Modbus_EVSE::isConnected() {
    // Based on CSV status values: 3,4,5 indicate connected states
    return (current_status >= 3 && current_status <= 5);
}

const char* Modbus_EVSE::getStatusString() {
    // Decode status register to human-readable string based on CSV
    switch(current_status) {
      case 0: return "Fault: Power self-check failed! Please turn the power back on!";
      case 1: return "Ready: CP disconnection";
      case 2: return "RFID Waiting: Waiting for IC card";
      case 3: return "Connected: CP connect to diode+2.7KΩ";
      case 4: return "Connected: CP connect to diode+1.3KΩ";
      case 5: return "Charging: CP connect to diode+2.7KΩ parallel connect 1.3KΩ";
      case 6: return "Fault: Need Ventilation!";
      case 7: return "Fault: CP-PE short circuit! Please check the CP line";
      case 8: return "Fault: RCMU leakage or self-inspection failure";
      case 9: return "Fault: EV-Charging Socket Fault";
      case 10: return "Fault: SPLIT PP wire, Please check the PP line";
      case 11: return "Fault: Electronic Lock Disabled";
      default: return "Unknown Status";
    }
}

// Enhanced data reading functions
uint8_t Modbus_EVSE::readAllStatusRegisters() {
    Serial.println("MODBUS EVSE: Reading all status registers (140-166)");
    uint8_t result = readHoldingRegisters(rSOFTWARE_VERSION, 27); // Read 27 registers from 140-166
    
    if (result == ku8MBSuccess) {
        // Update all status register cache
        software_version = getResponseBuffer(0);
        current_status = getResponseBuffer(1);
        cable_gauge_pwm = getResponseBuffer(2);
        rcmu_status = getResponseBuffer(3);
        rfid_status = getResponseBuffer(4);
        lock_status = getResponseBuffer(5);
        dlb_current_value = getResponseBuffer(6);
        charging_current = getResponseBuffer(7);
        charging_voltage = getResponseBuffer(8);
        charging_power = getResponseBuffer(9);
        ref_current_ad = getResponseBuffer(10);
        rotary_switch_pwm = getResponseBuffer(11);
        current_output_pwm = getResponseBuffer(12);
        cp_positive_voltage = getResponseBuffer(13);
        cp_negative_voltage = getResponseBuffer(14);
        overcurrent_count = getResponseBuffer(15);
        small_current_count = getResponseBuffer(16);
        current_temperature = getResponseBuffer(17);
        temperature_ad = getResponseBuffer(18);
        meter_a_voltage = getResponseBuffer(19);
        meter_b_voltage = getResponseBuffer(20);
        meter_c_voltage = getResponseBuffer(21);
        meter_current = getResponseBuffer(22);
        meter_total_power = getResponseBuffer(23);
        meter_total_kwh = ((uint32_t)getResponseBuffer(24) << 16) | getResponseBuffer(25);
        dlb_meter_current = getResponseBuffer(26);
        
        Serial.println("MODBUS EVSE: All status registers read successfully");
    } else {
        Serial.println("MODBUS EVSE: Failed to read status registers");
    }
    
    return result;
}

uint8_t Modbus_EVSE::readAllConfigRegisters() {
    Serial.println("MODBUS EVSE: Reading all config registers (86-139)");
    uint8_t result = readHoldingRegisters(rOVER_VOLTAGE_SETPOINT, 54); // Read 54 registers from 86-139
    
    if (result == ku8MBSuccess) {
        // Update all config register cache
        over_voltage_setpoint = getResponseBuffer(0);
        under_voltage_setpoint = getResponseBuffer(1);
        overcurrent_percentage = getResponseBuffer(2);
        remote_start_stop = getResponseBuffer(3);
        ext_meter_a_voltage_addr = getResponseBuffer(4);
        ext_meter_b_voltage_addr = getResponseBuffer(5);
        ext_meter_c_voltage_addr = getResponseBuffer(6);
        ext_meter_current_addr = getResponseBuffer(7);
        ext_meter_power_addr = getResponseBuffer(8);
        ext_meter_kwh_addr = getResponseBuffer(9);
        ext_meter_dlb_current_addr = getResponseBuffer(10);
        // Skip spare registers 97-99 (indices 11-13)
        modbus_device_address = getResponseBuffer(14);
        dlb_max_startup_current = getResponseBuffer(15);
        dlb_max_protection_current = getResponseBuffer(16);
        dlb_max_ct_current = getResponseBuffer(17);
        dlb_current_calibration = getResponseBuffer(18);
        // Skip spare registers 105-108 (indices 19-22)
        max_output_pwm_duty = getResponseBuffer(23);
        rcmu_function_select = getResponseBuffer(24);
        rfid_function_select = getResponseBuffer(25);
        lock_function_select = getResponseBuffer(26);
        cable_function_select = getResponseBuffer(27);
        dlb_function_select = getResponseBuffer(28);
        dlb_pid_p = getResponseBuffer(29);
        dlb_pid_i = getResponseBuffer(30);
        dlb_pid_d = getResponseBuffer(31);
        controller_id = ((uint32_t)getResponseBuffer(32) << 16) | getResponseBuffer(33);
        temp_correction_h = getResponseBuffer(34);
        temp_correction_l = getResponseBuffer(35);
        release_temp_setpoint = getResponseBuffer(36);
        max_temp_setpoint = getResponseBuffer(37);
        frequency_correction = getResponseBuffer(38);
        duty_cycle_correction = getResponseBuffer(39);
        trademark_selection = getResponseBuffer(40);
        number_of_poles = getResponseBuffer(41);
        for (int i = 0; i < 6; i++) {
            dial_settings[i] = getResponseBuffer(42 + i);
        }
        
        Serial.println("MODBUS EVSE: All config registers read successfully");
    } else {
        Serial.println("MODBUS EVSE: Failed to read config registers");
    }
    
    return result;
}

uint8_t Modbus_EVSE::readMeterData() {
    Serial.println("MODBUS EVSE: Reading meter data (159-166)");
    uint8_t result = readHoldingRegisters(rMETER_A_VOLTAGE, 8); // Read 8 registers from 159-166
    
    if (result == ku8MBSuccess) {
        meter_a_voltage = getResponseBuffer(0);
        meter_b_voltage = getResponseBuffer(1);
        meter_c_voltage = getResponseBuffer(2);
        meter_current = getResponseBuffer(3);
        meter_total_power = getResponseBuffer(4);
        meter_total_kwh = ((uint32_t)getResponseBuffer(5) << 16) | getResponseBuffer(6);
        dlb_meter_current = getResponseBuffer(7);
        
        Serial.println("MODBUS EVSE: Meter data read successfully");
        Serial.printf("MODBUS EVSE: Voltages: A=%dV, B=%dV, C=%dV\n", meter_a_voltage, meter_b_voltage, meter_c_voltage);
        Serial.printf("MODBUS EVSE: Current: %dA, Power: %dW, Total kWh: %lu\n", meter_current, meter_total_power, meter_total_kwh);
    } else {
        Serial.println("MODBUS EVSE: Failed to read meter data");
    }
    
    return result;
}

uint8_t Modbus_EVSE::readTemperatureData() {
    Serial.println("MODBUS EVSE: Reading temperature data (157-158)");
    uint8_t result = readHoldingRegisters(rCURRENT_TEMPERATURE, 2); // Read 2 registers from 157-158
    
    if (result == ku8MBSuccess) {
        current_temperature = getResponseBuffer(0);
        temperature_ad = getResponseBuffer(1);
        
        Serial.printf("MODBUS EVSE: Temperature: %d°C, AD Value: %d\n", current_temperature, temperature_ad);
    } else {
        Serial.println("MODBUS EVSE: Failed to read temperature data");
    }
    
    return result;
}

// New status getters (based on CSV register map)
uint16_t Modbus_EVSE::getSoftwareVersion() { return software_version; }
uint16_t Modbus_EVSE::getCurrentStatus() { return current_status; }
uint16_t Modbus_EVSE::getCableGaugePWM() { return cable_gauge_pwm; }
uint16_t Modbus_EVSE::getRCMUStatus() { return rcmu_status; }
uint16_t Modbus_EVSE::getRFIDStatus() { return rfid_status; }
uint16_t Modbus_EVSE::getLockStatus() { return lock_status; }
uint16_t Modbus_EVSE::getDLBCurrentValue() { return dlb_current_value; }
uint16_t Modbus_EVSE::getChargingCurrent() { return charging_current; }
uint16_t Modbus_EVSE::getChargingVoltage() { return charging_voltage; }
uint16_t Modbus_EVSE::getChargingPower() { return charging_power; }
uint16_t Modbus_EVSE::getRefCurrentAD() { return ref_current_ad; }
uint16_t Modbus_EVSE::getRotarySwitchPWM() { return rotary_switch_pwm; }
uint16_t Modbus_EVSE::getCurrentOutputPWM() { return current_output_pwm; }
uint16_t Modbus_EVSE::getCPPositiveVoltage() { return cp_positive_voltage; }
uint16_t Modbus_EVSE::getCPNegativeVoltage() { return cp_negative_voltage; }
uint16_t Modbus_EVSE::getOvercurrentCount() { return overcurrent_count; }
uint16_t Modbus_EVSE::getSmallCurrentCount() { return small_current_count; }
uint16_t Modbus_EVSE::getCurrentTemperature() { return current_temperature; }
uint16_t Modbus_EVSE::getTemperatureAD() { return temperature_ad; }
uint16_t Modbus_EVSE::getMeterAVoltage() { return meter_a_voltage; }
uint16_t Modbus_EVSE::getMeterBVoltage() { return meter_b_voltage; }
uint16_t Modbus_EVSE::getMeterCVoltage() { return meter_c_voltage; }
uint16_t Modbus_EVSE::getMeterCurrent() { return meter_current; }
uint16_t Modbus_EVSE::getMeterTotalPower() { return meter_total_power; }
uint32_t Modbus_EVSE::getMeterTotalKWH() { return meter_total_kwh; }
uint16_t Modbus_EVSE::getDLBMeterCurrent() { return dlb_meter_current; }

// New configuration getters
uint16_t Modbus_EVSE::getOverVoltageSetpoint() { return over_voltage_setpoint; }
uint16_t Modbus_EVSE::getUnderVoltageSetpoint() { return under_voltage_setpoint; }
uint16_t Modbus_EVSE::getOvercurrentPercentage() { return overcurrent_percentage; }
uint16_t Modbus_EVSE::getRemoteStartStop() { return remote_start_stop; }
uint16_t Modbus_EVSE::getModbusDeviceAddress() { return modbus_device_address; }
uint16_t Modbus_EVSE::getDLBMaxStartupCurrent() { return dlb_max_startup_current; }
uint16_t Modbus_EVSE::getDLBMaxProtectionCurrent() { return dlb_max_protection_current; }
uint16_t Modbus_EVSE::getDLBMaxCTCurrent() { return dlb_max_ct_current; }
uint16_t Modbus_EVSE::getDLBCurrentCalibration() { return dlb_current_calibration; }
uint16_t Modbus_EVSE::getMaxOutputPWMDuty() { return max_output_pwm_duty; }
uint16_t Modbus_EVSE::getRCMUFunctionSelect() { return rcmu_function_select; }
uint16_t Modbus_EVSE::getRFIDFunctionSelect() { return rfid_function_select; }
uint16_t Modbus_EVSE::getLockFunctionSelect() { return lock_function_select; }
uint16_t Modbus_EVSE::getCableFunctionSelect() { return cable_function_select; }
uint16_t Modbus_EVSE::getDLBFunctionSelect() { return dlb_function_select; }
uint16_t Modbus_EVSE::getDLBPIDP() { return dlb_pid_p; }
uint16_t Modbus_EVSE::getDLBPIDI() { return dlb_pid_i; }
uint16_t Modbus_EVSE::getDLBPIDD() { return dlb_pid_d; }
uint32_t Modbus_EVSE::getControllerID() { return controller_id; }
uint16_t Modbus_EVSE::getTempCorrectionH() { return temp_correction_h; }
uint16_t Modbus_EVSE::getTempCorrectionL() { return temp_correction_l; }
uint16_t Modbus_EVSE::getReleaseTempSetpoint() { return release_temp_setpoint; }
uint16_t Modbus_EVSE::getMaxTempSetpoint() { return max_temp_setpoint; }
uint16_t Modbus_EVSE::getFrequencyCorrection() { return frequency_correction; }
uint16_t Modbus_EVSE::getDutyCycleCorrection() { return duty_cycle_correction; }
uint16_t Modbus_EVSE::getTrademarkSelection() { return trademark_selection; }
uint16_t Modbus_EVSE::getNumberOfPoles() { return number_of_poles; }

uint16_t Modbus_EVSE::getDialSetting(uint8_t setting_num) {
    if (setting_num >= 1 && setting_num <= 6) {
        return dial_settings[setting_num - 1];
    }
    return 0;
}

// New configuration setters
uint8_t Modbus_EVSE::setOverVoltageSetpoint(uint16_t voltage) {
    uint8_t result = write_register(rOVER_VOLTAGE_SETPOINT, voltage);
    if (result == ku8MBSuccess) over_voltage_setpoint = voltage;
    return result;
}

uint8_t Modbus_EVSE::setUnderVoltageSetpoint(uint16_t voltage) {
    uint8_t result = write_register(rUNDER_VOLTAGE_SETPOINT, voltage);
    if (result == ku8MBSuccess) under_voltage_setpoint = voltage;
    return result;
}

uint8_t Modbus_EVSE::setOvercurrentPercentage(uint16_t percentage) {
    uint8_t result = write_register(rOVERCURRENT_PERCENTAGE, percentage);
    if (result == ku8MBSuccess) overcurrent_percentage = percentage;
    return result;
}

uint8_t Modbus_EVSE::setRemoteStartStop(uint16_t command) {
    uint8_t result = write_register(rREMOTE_START_STOP, command);
    if (result == ku8MBSuccess) remote_start_stop = command;
    return result;
}

uint8_t Modbus_EVSE::setModbusDeviceAddress(uint16_t address) {
    uint8_t result = write_register(rMODBUS_DEVICE_ADDRESS, address);
    if (result == ku8MBSuccess) modbus_device_address = address;
    return result;
}

uint8_t Modbus_EVSE::setDLBMaxStartupCurrent(uint16_t current) {
    uint8_t result = write_register(rDLB_MAX_STARTUP_CURRENT, current);
    if (result == ku8MBSuccess) dlb_max_startup_current = current;
    return result;
}

uint8_t Modbus_EVSE::setDLBMaxProtectionCurrent(uint16_t current) {
    uint8_t result = write_register(rDLB_MAX_PROTECTION_CURRENT, current);
    if (result == ku8MBSuccess) dlb_max_protection_current = current;
    return result;
}

uint8_t Modbus_EVSE::setDLBMaxCTCurrent(uint16_t current) {
    uint8_t result = write_register(rDLB_MAX_CT_CURRENT, current);
    if (result == ku8MBSuccess) dlb_max_ct_current = current;
    return result;
}

uint8_t Modbus_EVSE::setDLBCurrentCalibration(uint16_t calibration) {
    uint8_t result = write_register(rDLB_CURRENT_CALIBRATION, calibration);
    if (result == ku8MBSuccess) dlb_current_calibration = calibration;
    return result;
}

uint8_t Modbus_EVSE::setMaxOutputPWMDuty(uint16_t duty) {
    uint8_t result = write_register(rMAX_OUTPUT_PWM_DUTY, duty);
    if (result == ku8MBSuccess) max_output_pwm_duty = duty;
    return result;
}

uint8_t Modbus_EVSE::setRCMUFunctionSelect(uint16_t selection) {
    uint8_t result = write_register(rRCMU_FUNCTION_SELECT, selection);
    if (result == ku8MBSuccess) rcmu_function_select = selection;
    return result;
}

uint8_t Modbus_EVSE::setRFIDFunctionSelect(uint16_t selection) {
    uint8_t result = write_register(rRFID_FUNCTION_SELECT, selection);
    if (result == ku8MBSuccess) rfid_function_select = selection;
    return result;
}

uint8_t Modbus_EVSE::setLockFunctionSelect(uint16_t selection) {
    uint8_t result = write_register(rLOCK_FUNCTION_SELECT, selection);
    if (result == ku8MBSuccess) lock_function_select = selection;
    return result;
}

uint8_t Modbus_EVSE::setCableFunctionSelect(uint16_t selection) {
    uint8_t result = write_register(rCABLE_FUNCTION_SELECT, selection);
    if (result == ku8MBSuccess) cable_function_select = selection;
    return result;
}

uint8_t Modbus_EVSE::setDLBFunctionSelect(uint16_t selection) {
    uint8_t result = write_register(rDLB_FUNCTION_SELECT, selection);
    if (result == ku8MBSuccess) dlb_function_select = selection;
    return result;
}

uint8_t Modbus_EVSE::setDLBPIDP(uint16_t p_value) {
    uint8_t result = write_register(rDLB_PID_P, p_value);
    if (result == ku8MBSuccess) dlb_pid_p = p_value;
    return result;
}

uint8_t Modbus_EVSE::setDLBPIDI(uint16_t i_value) {
    uint8_t result = write_register(rDLB_PID_I, i_value);
    if (result == ku8MBSuccess) dlb_pid_i = i_value;
    return result;
}

uint8_t Modbus_EVSE::setDLBPIDD(uint16_t d_value) {
    uint8_t result = write_register(rDLB_PID_D, d_value);
    if (result == ku8MBSuccess) dlb_pid_d = d_value;
    return result;
}

uint8_t Modbus_EVSE::setControllerID(uint32_t id) {
    // Write high word first
    uint8_t result = write_register(rCONTROLLER_ID_HIGH, (uint16_t)(id >> 16));
    if (result != ku8MBSuccess) return result;
    
    // Write low word
    result = write_register(rCONTROLLER_ID_LOW, (uint16_t)(id & 0xFFFF));
    if (result == ku8MBSuccess) controller_id = id;
    return result;
}

uint8_t Modbus_EVSE::setTempCorrectionH(uint16_t correction) {
    uint8_t result = write_register(rTEMP_CORRECTION_H, correction);
    if (result == ku8MBSuccess) temp_correction_h = correction;
    return result;
}

uint8_t Modbus_EVSE::setTempCorrectionL(uint16_t correction) {
    uint8_t result = write_register(rTEMP_CORRECTION_L, correction);
    if (result == ku8MBSuccess) temp_correction_l = correction;
    return result;
}

uint8_t Modbus_EVSE::setReleaseTempSetpoint(uint16_t temp) {
    uint8_t result = write_register(rRELEASE_TEMP_SETPOINT, temp);
    if (result == ku8MBSuccess) release_temp_setpoint = temp;
    return result;
}

uint8_t Modbus_EVSE::setMaxTempSetpoint(uint16_t temp) {
    uint8_t result = write_register(rMAX_TEMP_SETPOINT, temp);
    if (result == ku8MBSuccess) max_temp_setpoint = temp;
    return result;
}

uint8_t Modbus_EVSE::setFrequencyCorrection(uint16_t correction) {
    uint8_t result = write_register(rFREQUENCY_CORRECTION, correction);
    if (result == ku8MBSuccess) frequency_correction = correction;
    return result;
}

uint8_t Modbus_EVSE::setDutyCycleCorrection(uint16_t correction) {
    uint8_t result = write_register(rDUTY_CYCLE_CORRECTION, correction);
    if (result == ku8MBSuccess) duty_cycle_correction = correction;
    return result;
}

uint8_t Modbus_EVSE::setTrademarkSelection(uint16_t selection) {
    uint8_t result = write_register(rTRADEMARK_SELECTION, selection);
    if (result == ku8MBSuccess) trademark_selection = selection;
    return result;
}

uint8_t Modbus_EVSE::setNumberOfPoles(uint16_t poles) {
    uint8_t result = write_register(rNUMBER_OF_POLES, poles);
    if (result == ku8MBSuccess) number_of_poles = poles;
    return result;
}

uint8_t Modbus_EVSE::setDialSetting(uint8_t setting_num, uint16_t duty_cycle) {
    if (setting_num < 1 || setting_num > 6) {
        Serial.printf("MODBUS EVSE: Invalid dial setting number: %d (must be 1-6)\n", setting_num);
        return ku8MBIllegalDataValue;
    }
    
    uint16_t reg = rDIAL_SETTING_1 + (setting_num - 1);
    uint8_t result = write_register(reg, duty_cycle);
    if (result == ku8MBSuccess) {
        dial_settings[setting_num - 1] = duty_cycle;
    }
    return result;
}

// Additional convenience functions
const char* Modbus_EVSE::getRCMUStatusString() {
    switch(rcmu_status) {
        case 0: return "Not selected";
        case 1: return "Normal";
        case 2: return "Self-test failed";
        case 3: return "Leakage detected";
        default: return "Unknown RCMU status";
    }
}

const char* Modbus_EVSE::getRFIDStatusString() {
    switch(rfid_status) {
        case 0: return "Not selected";
        case 1: return "IC card not operating";
        case 2: return "Closed";
        case 3: return "Open";
        default: return "Unknown RFID status";
    }
}

// EVSE Control functions
uint8_t Modbus_EVSE::startCharging() {
    Serial.println("MODBUS EVSE: Starting charging session");
    return setRemoteStartStop(1);
}

uint8_t Modbus_EVSE::stopCharging() {
    Serial.println("MODBUS EVSE: Stopping charging session");
    return setRemoteStartStop(2);
}

uint8_t Modbus_EVSE::enableRCMU() {
    Serial.println("MODBUS EVSE: Enabling RCMU function");
    return setRCMUFunctionSelect(1);
}

uint8_t Modbus_EVSE::disableRCMU() {
    Serial.println("MODBUS EVSE: Disabling RCMU function");
    return setRCMUFunctionSelect(0);
}

uint8_t Modbus_EVSE::enableRFID() {
    Serial.println("MODBUS EVSE: Enabling RFID function");
    return setRFIDFunctionSelect(1);
}

uint8_t Modbus_EVSE::disableRFID() {
    Serial.println("MODBUS EVSE: Disabling RFID function");
    return setRFIDFunctionSelect(0);
}

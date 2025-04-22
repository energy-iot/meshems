#include <modbus_evse.h>
#include <TimeLib.h>

#define EVSE_POLL_INTERVAL 5000 // 5 seconds

Modbus_EVSE::Modbus_EVSE() {
    // Initialize variables
    status = 0;
    charge_current = 0;
    pilot_duty = 0;
    proximity = 0;
    pilot_state = 0;
    actual_current = 0;
    max_current = 0;
    min_current = 0;
    master_control = 0;
    cable_gauge = 0;
    firmware_version = 0;
}

uint8_t Modbus_EVSE::get_modbus_address() {
    return modbus_address;
}

void Modbus_EVSE::set_modbus_address(uint8_t addr) {
    modbus_address = addr;
}

void Modbus_EVSE::route_poll_response(uint16_t reg, uint16_t response) {
    switch (reg) {
        case rSTATUS:
            Serial.printf("MODBUS EVSE: Status: 0x%04X\n", response);
            status = response;
            break;
        case rCHARGE_CURRENT:
            Serial.printf("MODBUS EVSE: Charge Current: %d\n", response);
            charge_current = response;
            break;
        case rPILOT_DUTY:
            Serial.printf("MODBUS EVSE: Pilot Duty: %d%%\n", response);
            pilot_duty = response;
            break;
        case rPROXIMITY:
            Serial.printf("MODBUS EVSE: Proximity: %d\n", response);
            proximity = response;
            break;
        case rPILOT_STATE:
            Serial.printf("MODBUS EVSE: Pilot State: %d\n", response);
            pilot_state = response;
            break;
        case rACTUAL_CURRENT:
            Serial.printf("MODBUS EVSE: Actual Current: %d\n", response);
            actual_current = response;
            break;
        case rMAX_CURRENT:
            Serial.printf("MODBUS EVSE: Max Current: %d\n", response);
            max_current = response;
            break;
        case rMIN_CURRENT:
            Serial.printf("MODBUS EVSE: Min Current: %d\n", response);
            min_current = response;
            break;
        case rMASTER_CONTROL:
            Serial.printf("MODBUS EVSE: Master Control: 0x%04X\n", response);
            master_control = response;
            break;
        case rCABLE_GAUGE:
            Serial.printf("MODBUS EVSE: Cable Gauge PWM: 0x%04X\n", response);
            cable_gauge = response;
            break;
        case rVERSION:
            Serial.printf("MODBUS EVSE: Firmware Version: %d.%d\n", response >> 8, response & 0xFF);
            firmware_version = response;
            break;
        default:
            Serial.printf("MODBUS EVSE: Unknown register: 0x%04X, value: 0x%04X\n", reg, response);
            break;
    }
}

uint8_t Modbus_EVSE::poll() {
    // Read status registers (6 registers starting from rSTATUS)
    uint8_t result = readHoldingRegisters(rSTATUS, 6);

    uint8_t debug = readHoldingRegisters(0x56, 40);
    
    if (result == ku8MBSuccess) {
        timestamp_last_report = now();
        
        // Update status variables
        status = getResponseBuffer(0);
        charge_current = getResponseBuffer(1);
        pilot_duty = getResponseBuffer(2);
        proximity = getResponseBuffer(3);
        pilot_state = getResponseBuffer(4);
        actual_current = getResponseBuffer(5);
        
        Serial.println("MODBUS EVSE: Status poll success");
        Serial.printf("MODBUS EVSE: Status: 0x%04X, Charging: %s\n", status, isCharging() ? "Yes" : "No");
        Serial.printf("MODBUS EVSE: Current: %d A, Pilot Duty: %d%%\n", charge_current, pilot_duty);
    } else {
        timestamp_last_failure = now();
        Serial.println("MODBUS EVSE: Status poll FAIL");
    }
    
    // Read configuration registers (5 registers starting from rMAX_CURRENT)
    result = readHoldingRegisters(rMAX_CURRENT, 5);
    
    if (result == ku8MBSuccess) {
        // Update config variables
        max_current = getResponseBuffer(0);
        min_current = getResponseBuffer(1);
        master_control = getResponseBuffer(2);
        cable_gauge = getResponseBuffer(3);
        firmware_version = getResponseBuffer(4);
        
        Serial.println("MODBUS EVSE: Config poll success");
        Serial.printf("MODBUS EVSE: Max Current: %d A, Min Current: %d A\n", max_current, min_current);
        Serial.printf("MODBUS EVSE: Cable Gauge: 0x%04X, FW Version: %d.%d\n", 
                      cable_gauge, firmware_version >> 8, firmware_version & 0xFF);
    } else {
        Serial.println("MODBUS EVSE: Config poll FAIL");
    }
    
    return result;
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
    // Only allow writing to holding registers (configuration)
    if (reg < 0x2000 || reg >= 0x3000) {
        Serial.printf("MODBUS EVSE: Cannot write to register 0x%04X, not a holding register\n", reg);
        return ku8MBIllegalDataAddress;
    }
    
    uint8_t result = writeSingleRegister(reg, value);
    
    if (result == ku8MBSuccess) {
        Serial.printf("MODBUS EVSE: Write to register 0x%04X value 0x%04X success\n", reg, value);
        
        // Update our local copy of the value
        switch(reg) {
            case rMAX_CURRENT:
                max_current = value;
                break;
            case rMIN_CURRENT:
                min_current = value;
                break;
            case rMASTER_CONTROL:
                master_control = value;
                break;
            default:
                break;
        }
    } else {
        Serial.printf("MODBUS EVSE: Write to register 0x%04X FAIL\n", reg);
    }
    
    return result;
}

// Status getters
uint16_t Modbus_EVSE::getStatus() {
    return status;
}

uint16_t Modbus_EVSE::getChargeCurrent() {
    return charge_current;
}

uint16_t Modbus_EVSE::getPilotDuty() {
    return pilot_duty;
}

uint16_t Modbus_EVSE::getProximity() {
    return proximity;
}

uint16_t Modbus_EVSE::getPilotState() {
    return pilot_state;
}

uint16_t Modbus_EVSE::getActualCurrent() {
    return actual_current;
}

// Configuration getters
uint16_t Modbus_EVSE::getMaxCurrent() {
    return max_current;
}

uint16_t Modbus_EVSE::getMinCurrent() {
    return min_current;
}

uint16_t Modbus_EVSE::getMasterControl() {
    return master_control;
}

uint16_t Modbus_EVSE::getCableGauge() {
    return cable_gauge;
}

uint16_t Modbus_EVSE::getFirmwareVersion() {
    return firmware_version;
}

// Configuration setters
uint8_t Modbus_EVSE::setMaxCurrent(uint16_t current) {
    return write_register(rMAX_CURRENT, current);
}

uint8_t Modbus_EVSE::setMinCurrent(uint16_t current) {
    return write_register(rMIN_CURRENT, current);
}

uint8_t Modbus_EVSE::setMasterControl(uint16_t control) {
    return write_register(rMASTER_CONTROL, control);
}

// Convenience functions
bool Modbus_EVSE::isCharging() {
    // Assuming bit 0 of status register indicates charging
    return (status & 0x0001) > 0;
}

bool Modbus_EVSE::isConnected() {
    // Assuming proximity > 0 means connected
    return proximity > 0;
}

const char* Modbus_EVSE::getStatusString() {
    // Decode status register to human-readable string
    switch(status) {
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
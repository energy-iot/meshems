#pragma once

#include <modbus_master.h>

class Modbus_EVSE : public ModbusMaster {
    public:
        Modbus_EVSE();
        ~Modbus_EVSE() {};

        // Basic setup and polling functions
        uint8_t poll();
        void route_poll_response(uint16_t reg, uint16_t response);
        uint8_t get_modbus_address();
        void set_modbus_address(uint8_t addr);
        uint8_t query_register(uint16_t reg);
        uint8_t write_register(uint16_t reg, uint16_t value);

        // EVSE controller specific registers (based on modbus map)
        enum MB_Reg {
            // Status registers
            rSTATUS = 0x8D,           // EVSE current status
            rCHARGE_CURRENT = 0x92,   // Current charge current
            rPILOT_DUTY = 0x97,       // Current pilot duty cycle
            rPROXIMITY = 0x95,        // Proximity status
            rPILOT_STATE = 0x99,      // Pilot state
            rACTUAL_CURRENT = 0xA2,   // Actual current
            
            // Configuration registers
            rMAX_CURRENT = 0x66,      // Max current setting
            rMIN_CURRENT = 0x65,      // Min current setting
            rMASTER_CONTROL = 0x59,   // Master control (0 invalid, 1 start, 2 stop)
            rCABLE_GAUGE = 0x8E,      // PWM value for cable gauge
            rVERSION = 0x8C           // Firmware version
        };

        // Status getters
        uint16_t getStatus();
        uint16_t getChargeCurrent();
        uint16_t getPilotDuty();
        uint16_t getProximity();
        uint16_t getPilotState();
        uint16_t getActualCurrent();
        
        // Configuration getters
        uint16_t getMaxCurrent();
        uint16_t getMinCurrent();
        uint16_t getMasterControl();
        uint16_t getCableGauge();
        uint16_t getFirmwareVersion();
        
        // Configuration setters
        uint8_t setMaxCurrent(uint16_t current);
        uint8_t setMinCurrent(uint16_t current);
        uint8_t setMasterControl(uint16_t control);
        
        // Convenience functions
        bool isCharging();
        bool isConnected();
        const char* getStatusString();

    private:
        uint8_t modbus_address;
        unsigned long timestamp_last_report;
        unsigned long timestamp_last_failure;
        
        // Status variables
        uint16_t status;
        uint16_t charge_current;
        uint16_t pilot_duty;
        uint16_t proximity;
        uint16_t pilot_state;
        uint16_t actual_current;
        
        // Configuration variables
        uint16_t max_current;
        uint16_t min_current;
        uint16_t master_control;
        uint16_t cable_gauge;
        uint16_t firmware_version;
};
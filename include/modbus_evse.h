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

        // EVSE controller specific registers (based on CSV modbus map)
        enum MB_Reg {
            // Configuration registers (R/W) - Register IDs 86-139
            rOVER_VOLTAGE_SETPOINT = 86,        // Over-voltage protection setpoint (0.01V)
            rUNDER_VOLTAGE_SETPOINT = 87,       // Under-voltage protection setpoint (0.01V)
            rOVERCURRENT_PERCENTAGE = 88,       // Overcurrent protection percentage (1%)
            rREMOTE_START_STOP = 89,            // Remote start/stop command (0=invalid, 1=start, 2=stop)
            rEXT_METER_A_VOLTAGE_ADDR = 90,     // External meter A-phase voltage modbus address
            rEXT_METER_B_VOLTAGE_ADDR = 91,     // External meter B-phase voltage modbus address
            rEXT_METER_C_VOLTAGE_ADDR = 92,     // External meter C-phase voltage modbus address
            rEXT_METER_CURRENT_ADDR = 93,       // External meter total current address
            rEXT_METER_POWER_ADDR = 94,         // External meter total power address
            rEXT_METER_KWH_ADDR = 95,           // External meter total KWH address
            rEXT_METER_DLB_CURRENT_ADDR = 96,   // External meter DLB current address
            rSPARE_97 = 97,                     // Spare
            rSPARE_98 = 98,                     // Spare
            rSPARE_99 = 99,                     // Spare
            rMODBUS_DEVICE_ADDRESS = 100,       // Modbus device address
            rDLB_MAX_STARTUP_CURRENT = 101,     // DLB maximum startup current (0.01A)
            rDLB_MAX_PROTECTION_CURRENT = 102,  // DLB maximum protection current (0.01A)
            rDLB_MAX_CT_CURRENT = 103,          // Maximum current of DLB CT (0.01A)
            rDLB_CURRENT_CALIBRATION = 104,     // DLB current sampling calibration coefficient
            rSPARE_105 = 105,                   // Spare
            rSPARE_106 = 106,                   // Spare
            rSPARE_107 = 107,                   // Spare
            rSPARE_108 = 108,                   // Spare
            rMAX_OUTPUT_PWM_DUTY = 109,         // Max output current PWM duty cycle (0.01%)
            rRCMU_FUNCTION_SELECT = 110,        // RCMU function selection
            rRFID_FUNCTION_SELECT = 111,        // RFID function selection
            rLOCK_FUNCTION_SELECT = 112,        // Lock function selection
            rCABLE_FUNCTION_SELECT = 113,       // Cable function version selection
            rDLB_FUNCTION_SELECT = 114,         // DLB function selection
            rDLB_PID_P = 115,                   // PID control parameter P of DLB
            rDLB_PID_I = 116,                   // PID control parameter I of DLB
            rDLB_PID_D = 117,                   // PID control parameter D of DLB
            rCONTROLLER_ID_HIGH = 118,          // Controller ID number (high 32-bit)
            rCONTROLLER_ID_LOW = 119,           // Controller ID number (low 32-bit)
            rTEMP_CORRECTION_H = 120,           // Temperature correction (input difference) H
            rTEMP_CORRECTION_L = 121,           // Temperature correction (input difference) L
            rRELEASE_TEMP_SETPOINT = 122,       // Release temperature protection setpoint (0.1°C)
            rMAX_TEMP_SETPOINT = 123,           // Maximum temperature protection setpoint (0.1°C)
            rFREQUENCY_CORRECTION = 124,        // Frequency correction (input difference)
            rDUTY_CYCLE_CORRECTION = 125,       // Duty cycle correction (input difference)
            rTRADEMARK_SELECTION = 126,         // Trademark selection (0=none, 1=WATT, 2=VOLU)
            rNUMBER_OF_POLES = 127,             // Number of poles selection (1=1P, 3=3P)
            rDIAL_SETTING_1 = 128,              // Controller dial setting #1 duty cycle (0.01%)
            rDIAL_SETTING_2 = 129,              // Controller dial setting #2 duty cycle (0.01%)
            rDIAL_SETTING_3 = 130,              // Controller dial setting #3 duty cycle (0.01%)
            rDIAL_SETTING_4 = 131,              // Controller dial setting #4 duty cycle (0.01%)
            rDIAL_SETTING_5 = 132,              // Controller dial setting #5 duty cycle (0.01%)
            rDIAL_SETTING_6 = 133,              // Controller dial setting #6 duty cycle (0.01%)
            rSPARE_134 = 134,                   // Spare
            rSPARE_135 = 135,                   // Spare
            rSPARE_136 = 136,                   // Spare
            rSPARE_137 = 137,                   // Spare
            rSPARE_138 = 138,                   // Spare
            rSPARE_139 = 139,                   // Spare
            
            // Status registers (Read-only) - Register IDs 140-166
            rSOFTWARE_VERSION = 140,            // Controller software version (x0.001)
            rCURRENT_STATUS = 141,              // Current working status (0-11)
            rCABLE_GAUGE_PWM = 142,             // PWM value for cable gauge (0.01%)
            rRCMU_STATUS = 143,                 // RCMU status bitfield
            rRFID_STATUS = 144,                 // RFID status bitfield
            rLOCK_STATUS = 145,                 // Lock status
            rDLB_CURRENT_VALUE = 146,           // Current value of DLB function
            rCHARGING_CURRENT = 147,            // Current value of charging pile (1A)
            rCHARGING_VOLTAGE = 148,            // Current voltage value of charging pile (1V)
            rCHARGING_POWER = 149,              // Current power value of charging pile (1W)
            rREF_CURRENT_AD = 150,              // Calibration value AD of reference current
            rROTARY_SWITCH_PWM = 151,           // PWM duty cycle from rotary switch (0.01%)
            rCURRENT_OUTPUT_PWM = 152,          // Current output PWM duty cycle (0.01%)
            rCP_POSITIVE_VOLTAGE = 153,         // CP positive voltage
            rCP_NEGATIVE_VOLTAGE = 154,         // CP negative voltage
            rOVERCURRENT_COUNT = 155,           // Overcurrent count
            rSMALL_CURRENT_COUNT = 156,         // Small current count
            rCURRENT_TEMPERATURE = 157,         // Current temperature (°C)
            rTEMPERATURE_AD = 158,              // Temperature AD value
            rMETER_A_VOLTAGE = 159,             // 1# meter A phase voltage (V)
            rMETER_B_VOLTAGE = 160,             // 1# meter B phase voltage (V)
            rMETER_C_VOLTAGE = 161,             // 1# meter C phase voltage (V)
            rMETER_CURRENT = 162,               // 1# meter current (A)
            rMETER_TOTAL_POWER = 163,           // 1# total power of meter (W)
            rMETER_TOTAL_KWH_HIGH = 164,        // 1# total electricity high (kWh)
            rMETER_TOTAL_KWH_LOW = 165,         // 1# total electricity low (kWh)
            rDLB_METER_CURRENT = 166,           // 2# DLB meter current (A)
            
        };

        
        // Enhanced data reading functions
        uint8_t readAllStatusRegisters();
        uint8_t readAllConfigRegisters();
        uint8_t readMeterData();
        uint8_t readTemperatureData();
        
        // New status getters (based on CSV register map)
        uint16_t getSoftwareVersion();
        uint16_t getCurrentStatus();
        uint16_t getCableGaugePWM();
        uint16_t getRCMUStatus();
        uint16_t getRFIDStatus();
        uint16_t getLockStatus();
        uint16_t getDLBCurrentValue();
        uint16_t getChargingCurrent();
        uint16_t getChargingVoltage();
        uint16_t getChargingPower();
        uint16_t getRefCurrentAD();
        uint16_t getRotarySwitchPWM();
        uint16_t getCurrentOutputPWM();
        uint16_t getCPPositiveVoltage();
        uint16_t getCPNegativeVoltage();
        uint16_t getOvercurrentCount();
        uint16_t getSmallCurrentCount();
        uint16_t getCurrentTemperature();
        uint16_t getTemperatureAD();
        uint16_t getMeterAVoltage();
        uint16_t getMeterBVoltage();
        uint16_t getMeterCVoltage();
        uint16_t getMeterCurrent();
        uint16_t getMeterTotalPower();
        uint32_t getMeterTotalKWH();
        uint16_t getDLBMeterCurrent();
        
        // New configuration getters
        uint16_t getOverVoltageSetpoint();
        uint16_t getUnderVoltageSetpoint();
        uint16_t getOvercurrentPercentage();
        uint16_t getRemoteStartStop();
        uint16_t getModbusDeviceAddress();
        uint16_t getDLBMaxStartupCurrent();
        uint16_t getDLBMaxProtectionCurrent();
        uint16_t getDLBMaxCTCurrent();
        uint16_t getDLBCurrentCalibration();
        uint16_t getMaxOutputPWMDuty();
        uint16_t getRCMUFunctionSelect();
        uint16_t getRFIDFunctionSelect();
        uint16_t getLockFunctionSelect();
        uint16_t getCableFunctionSelect();
        uint16_t getDLBFunctionSelect();
        uint16_t getDLBPIDP();
        uint16_t getDLBPIDI();
        uint16_t getDLBPIDD();
        uint32_t getControllerID();
        uint16_t getTempCorrectionH();
        uint16_t getTempCorrectionL();
        uint16_t getReleaseTempSetpoint();
        uint16_t getMaxTempSetpoint();
        uint16_t getFrequencyCorrection();
        uint16_t getDutyCycleCorrection();
        uint16_t getTrademarkSelection();
        uint16_t getNumberOfPoles();
        uint16_t getDialSetting(uint8_t setting_num); // 1-6
        
        // New configuration setters
        uint8_t setOverVoltageSetpoint(uint16_t voltage);
        uint8_t setUnderVoltageSetpoint(uint16_t voltage);
        uint8_t setOvercurrentPercentage(uint16_t percentage);
        uint8_t setRemoteStartStop(uint16_t command);
        uint8_t setModbusDeviceAddress(uint16_t address);
        uint8_t setDLBMaxStartupCurrent(uint16_t current);
        uint8_t setDLBMaxProtectionCurrent(uint16_t current);
        uint8_t setDLBMaxCTCurrent(uint16_t current);
        uint8_t setDLBCurrentCalibration(uint16_t calibration);
        uint8_t setMaxOutputPWMDuty(uint16_t duty);
        uint8_t setRCMUFunctionSelect(uint16_t selection);
        uint8_t setRFIDFunctionSelect(uint16_t selection);
        uint8_t setLockFunctionSelect(uint16_t selection);
        uint8_t setCableFunctionSelect(uint16_t selection);
        uint8_t setDLBFunctionSelect(uint16_t selection);
        uint8_t setDLBPIDP(uint16_t p_value);
        uint8_t setDLBPIDI(uint16_t i_value);
        uint8_t setDLBPIDD(uint16_t d_value);
        uint8_t setControllerID(uint32_t id);
        uint8_t setTempCorrectionH(uint16_t correction);
        uint8_t setTempCorrectionL(uint16_t correction);
        uint8_t setReleaseTempSetpoint(uint16_t temp);
        uint8_t setMaxTempSetpoint(uint16_t temp);
        uint8_t setFrequencyCorrection(uint16_t correction);
        uint8_t setDutyCycleCorrection(uint16_t correction);
        uint8_t setTrademarkSelection(uint16_t selection);
        uint8_t setNumberOfPoles(uint16_t poles);
        uint8_t setDialSetting(uint8_t setting_num, uint16_t duty_cycle); // 1-6
        
        // Convenience functions
        bool isCharging();
        bool isConnected();
        const char* getStatusString();
        const char* getRCMUStatusString();
        const char* getRFIDStatusString();
        
        // EVSE Control functions
        uint8_t startCharging();
        uint8_t stopCharging();
        uint8_t enableRCMU();
        uint8_t disableRCMU();
        uint8_t enableRFID();
        uint8_t disableRFID();

    private:
        uint8_t modbus_address;
        unsigned long timestamp_last_report;
        unsigned long timestamp_last_failure;
        
        // Complete register cache (based on CSV register map)
        // Configuration registers (R/W) - Register IDs 86-139
        uint16_t over_voltage_setpoint;
        uint16_t under_voltage_setpoint;
        uint16_t overcurrent_percentage;
        uint16_t remote_start_stop;
        uint16_t ext_meter_a_voltage_addr;
        uint16_t ext_meter_b_voltage_addr;
        uint16_t ext_meter_c_voltage_addr;
        uint16_t ext_meter_current_addr;
        uint16_t ext_meter_power_addr;
        uint16_t ext_meter_kwh_addr;
        uint16_t ext_meter_dlb_current_addr;
        uint16_t modbus_device_address;
        uint16_t dlb_max_startup_current;
        uint16_t dlb_max_protection_current;
        uint16_t dlb_max_ct_current;
        uint16_t dlb_current_calibration;
        uint16_t max_output_pwm_duty;
        uint16_t rcmu_function_select;
        uint16_t rfid_function_select;
        uint16_t lock_function_select;
        uint16_t cable_function_select;
        uint16_t dlb_function_select;
        uint16_t dlb_pid_p;
        uint16_t dlb_pid_i;
        uint16_t dlb_pid_d;
        uint32_t controller_id;
        uint16_t temp_correction_h;
        uint16_t temp_correction_l;
        uint16_t release_temp_setpoint;
        uint16_t max_temp_setpoint;
        uint16_t frequency_correction;
        uint16_t duty_cycle_correction;
        uint16_t trademark_selection;
        uint16_t number_of_poles;
        uint16_t dial_settings[6]; // Dial settings 1-6
        
        // Status registers (Read-only) - Register IDs 140-166
        uint16_t software_version;
        uint16_t current_status;
        uint16_t cable_gauge_pwm;
        uint16_t rcmu_status;
        uint16_t rfid_status;
        uint16_t lock_status;
        uint16_t dlb_current_value;
        uint16_t charging_current;
        uint16_t charging_voltage;
        uint16_t charging_power;
        uint16_t ref_current_ad;
        uint16_t rotary_switch_pwm;
        uint16_t current_output_pwm;
        uint16_t cp_positive_voltage;
        uint16_t cp_negative_voltage;
        uint16_t overcurrent_count;
        uint16_t small_current_count;
        uint16_t current_temperature;
        uint16_t temperature_ad;
        uint16_t meter_a_voltage;
        uint16_t meter_b_voltage;
        uint16_t meter_c_voltage;
        uint16_t meter_current;
        uint16_t meter_total_power;
        uint32_t meter_total_kwh;
        uint16_t dlb_meter_current;
};

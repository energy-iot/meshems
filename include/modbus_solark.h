#pragma once

#include <modbus_master.h>

// Register mapping structure for Sol-Ark
struct SolArkRegisterMap {
    // Diagnostic registers
    static const uint16_t COMM_VERSION = 2; //NOT FULLY IMPLEMENTED

    static const uint16_t SN_BYTE_01 = 3; //NOT FULLY IMPLEMENTED
    static const uint16_t SN_BYTE_02 = 4; //NOT FULLY IMPLEMENTED
    static const uint16_t SN_BYTE_03 = 5; //NOT FULLY IMPLEMENTED
    static const uint16_t SN_BYTE_04 = 6; //NOT FULLY IMPLEMENTED
    static const uint16_t SN_BYTE_05 = 7; //NOT FULLY IMPLEMENTED

    // Energy registers
    static const uint16_t BATTERY_CHARGE_ENERGY = 70;
    static const uint16_t BATTERY_DISCHARGE_ENERGY = 71;
    static const uint16_t GRID_BUY_ENERGY = 76;
    static const uint16_t GRID_SELL_ENERGY = 77;
    static const uint16_t GRID_FREQUENCY = 79;
    static const uint16_t LOAD_ENERGY = 84;
    static const uint16_t PV_ENERGY = 108;
    
    // Grid and inverter registers
    static const uint16_t GRID_VOLTAGE = 152;
    static const uint16_t INVERTER_VOLTAGE = 156;
    static const uint16_t GRID_CURRENT_L1 = 160;
    static const uint16_t GRID_CURRENT_L2 = 161;
    static const uint16_t GRID_CT_CURRENT_L1 = 162;
    static const uint16_t GRID_CT_CURRENT_L2 = 163;
    static const uint16_t INVERTER_CURRENT_L1 = 164;
    static const uint16_t INVERTER_CURRENT_L2 = 165;
    static const uint16_t SMART_LOAD_POWER = 166;
    static const uint16_t GRID_POWER = 169;

    // Inverter status register
    static const uint16_t INVERTER_STATUS = 59;             // Inverter Status: 1=Self-test, 2=Normal, 3=Alarm, 4=Fault
    
    // Inverter temperature registers
    static const uint16_t DCDC_XFRMR_TEMP = 90;
    static const uint16_t IGBT_HEATSINK_TEMP = 91;
    
    // Power and battery registers
    static const uint16_t INVERTER_OUTPUT_POWER = 175;
    static const uint16_t LOAD_POWER_L1 = 176;
    static const uint16_t LOAD_POWER_L2 = 177;
    static const uint16_t LOAD_POWER_TOTAL = 178;
    static const uint16_t LOAD_CURRENT_L1 = 179;
    static const uint16_t LOAD_CURRENT_L2 = 180;
    static const uint16_t BATTERY_TEMPERATURE = 182;
    static const uint16_t BATTERY_VOLTAGE = 183;
    static const uint16_t BATTERY_SOC = 184;
    static const uint16_t PV1_POWER = 186;
    static const uint16_t PV2_POWER = 187;
    
    // Battery status registers
    static const uint16_t BATTERY_POWER = 190;
    static const uint16_t BATTERY_CURRENT = 191;
    static const uint16_t LOAD_FREQUENCY = 192;
    static const uint16_t INVERTER_FREQUENCY = 193;
    static const uint16_t GRID_RELAY_STATUS = 194;
    static const uint16_t GENERATOR_RELAY_STATUS = 195;
    
    // Battery configuration and capacity registers (for SunSpec model 713)
    static const uint16_t BATTERY_CAPACITY = 204;           // Battery capacity in Ah
    static const uint16_t CORRECTED_BATTERY_CAPACITY = 107; // Corrected battery capacity in Ah
    static const uint16_t BATTERY_EMPTY_VOLTAGE = 205;      // Battery empty voltage (0% SOC)
    static const uint16_t BATTERY_SHUTDOWN_VOLTAGE = 220;   // Battery shutdown voltage
    static const uint16_t BATTERY_RESTART_VOLTAGE = 221;    // Battery restart voltage
    static const uint16_t BATTERY_LOW_VOLTAGE = 222;        // Battery low voltage
    static const uint16_t BATTERY_SHUTDOWN_PERCENT = 217;   // Battery shutdown percentage
    static const uint16_t BATTERY_RESTART_PERCENT = 218;    // Battery restart percentage
    static const uint16_t BATTERY_LOW_PERCENT = 219;        // Battery low percentage
    
    // Battery BMS registers (for lithium batteries)
    static const uint16_t BMS_CHARGING_VOLTAGE = 312;       // BMS charging voltage
    static const uint16_t BMS_DISCHARGE_VOLTAGE = 313;      // BMS discharge voltage
    static const uint16_t BMS_CHARGING_CURRENT_LIMIT = 314; // BMS charging current limit
    static const uint16_t BMS_DISCHARGE_CURRENT_LIMIT = 315; // BMS discharge current limit
    static const uint16_t BMS_REAL_TIME_SOC = 316;          // BMS real-time SOC
    static const uint16_t BMS_REAL_TIME_VOLTAGE = 317;      // BMS real-time voltage
    static const uint16_t BMS_REAL_TIME_CURRENT = 318;      // BMS real-time current
    static const uint16_t BMS_REAL_TIME_TEMP = 319;         // BMS real-time temperature
    static const uint16_t BMS_WARNING = 322;                // BMS lithium battery warning
    static const uint16_t BMS_FAULT = 323;                  // BMS lithium battery fault
    static const uint16_t GRID_TYPE = 286;                  // Grid Type (0=Single, 1=Split, 2=Three-phase)
};

// Scaling factors for Sol-Ark values
struct SolArkScalingFactors {
    static constexpr float VOLTAGE = 10.0f;
    static constexpr float CURRENT = 100.0f;
    static constexpr float ENERGY = 10.0f;
    static constexpr float FREQUENCY = 100.0f;
    static constexpr float TEMPERATURE_OFFSET = 1000.0f;
    static constexpr float TEMPERATURE_SCALE = 10.0f;
};

class Modbus_SolArkLV : public ModbusMaster {
    public:
        Modbus_SolArkLV();
        ~Modbus_SolArkLV() {};

        // Basic setup and polling functions
        uint8_t poll();
        void route_poll_response(uint16_t reg, uint16_t response);
        uint8_t get_modbus_address();
        void set_modbus_address(uint8_t addr);
        uint8_t query_register(uint16_t reg);

        // Diagnostic Status Getters
        float getIGBTTemp();
        float getDCDCTemp();

        // Battery Status Getters
        float getBatteryPower();
        float getBatteryCurrent();
        float getBatteryVoltage();
        float getBatterySOC();
        float getBatteryTemperature();
        float getBatteryTemperatureF();
        
        // Energy Getters
        float getBatteryChargeEnergy();
        float getBatteryDischargeEnergy();
        float getGridBuyEnergy();
        float getGridSellEnergy();
        float getLoadEnergy();
        float getPVEnergy();
        
        // Power Getters
        float getGridPower();
        float getInverterPower();
        float getLoadPowerL1();
        float getLoadPowerL2();
        float getLoadPowerTotal();
        float getPV1Power();
        float getPV2Power();
        float getPVPowerTotal();
        float getSmartLoadPower();
        
        // Grid Status Getters
        float getGridVoltage();
        float getGridCurrentL1();
        float getGridCurrentL2();
        float getGridFrequency();
        uint8_t getGridRelayStatus();
        
        // Inverter Status Getters
        float getInverterVoltage();
        float getInverterCurrentL1();
        float getInverterCurrentL2();
        float getInverterFrequency();
        uint8_t getInverterStatus();
        
        // Load Status Getters
        float getLoadCurrentL1();
        float getLoadCurrentL2();
        float getLoadFrequency();
        
        // Generator Status Getters
        uint8_t getGeneratorRelayStatus();
        uint8_t getGridType(); // Getter for Grid Type (Reg 286)
        
        // Battery Configuration Getters (for SunSpec model 713)
        float getBatteryCapacity();
        float getCorrectedBatteryCapacity();
        float getBatteryEmptyVoltage();
        float getBatteryShutdownVoltage();
        float getBatteryRestartVoltage();
        float getBatteryLowVoltage();
        uint8_t getBatteryShutdownPercent();
        uint8_t getBatteryRestartPercent();
        uint8_t getBatteryLowPercent();
        
        // BMS Getters (for lithium batteries)
        float getBMSChargingVoltage();
        float getBMSDischargeVoltage();
        float getBMSChargingCurrentLimit();
        float getBMSDischargeCurrentLimit();
        float getBMSRealTimeSOC();
        float getBMSRealTimeVoltage();
        float getBMSRealTimeCurrent();
        float getBMSRealTimeTemp();
        uint16_t getBMSWarning();
        uint16_t getBMSFault();
        
        // Convenience Methods
        bool isGridConnected();
        bool isGeneratorConnected();
        bool isBatteryCharging();
        bool isBatteryDischarging();
        bool isSellingToGrid();
        bool isBuyingFromGrid();

    private:
        // Helper method for sign correction
        int16_t correctSignedValue(uint16_t value) {
            if (value > 32767) {
                return value - 65535;
            }
            return value;
        }
        
        uint8_t modbus_address;
        unsigned long timestamp_last_report;
        unsigned long timestamp_last_failure;
        
        // Diagnostic variables
        float igbt_temp = 0;            // Register 91: IGBT Heat Sink temperature
        float dcdc_xfrmr_temp = 0;      // Register 90: DC/DC Transformer temperature

        // Battery variables
        float battery_power;            // Register 190: Battery output power
        float battery_current;          // Register 191: Battery output current
        float battery_voltage;          // Register 183: Battery voltage
        float battery_soc;              // Register 184: Battery SOC
        float battery_temperature;      // Register 182: Battery temperature
        
        // Energy counters
        float battery_charge_energy;    // Register 70: Hybrid Day Batt Charge Power
        float battery_discharge_energy; // Register 71: Hybrid Day Batt Discharge Power
        float grid_buy_energy;          // Register 76: Hybrid Day Grid Buy Power
        float grid_sell_energy;         // Register 77: Hybrid Day Grid Sell Power
        float load_energy;              // Register 84: Hybrid SG: Day Load Power
        float pv_energy;                // Register 108: Daily PV Power (Wh)
        
        // Power variables
        float grid_power;               // Register 169: Total power of grid side L1L2
        float inverter_output_power;    // Register 175: Inverter output Total power
        float load_power_l1;            // Register 176: Load side L1 power
        float load_power_l2;            // Register 177: Load side L2 power
        float load_power_total;         // Register 178: Load side Total power
        float pv1_power;                // Register 186: PV1 input power
        float pv2_power;                // Register 187: PV2 input power
        float pv_power_total;           // Calculated: Total PV input power (PV1 + PV2)
        float smart_load_power;         // Register 166: Gen or AC Coupled power P
        
        // Grid variables
        float grid_voltage;             // Register 152: Grid side voltage L1-L2
        float grid_current_l1;          // Register 160: Grid side current L1
        float grid_current_l2;          // Register 161: Grid side current L2
        
        float grid_CT_current_l1;       // Register 162: Grid external Limiter current L1
        float grid_CT_current_l2;       // Register 163: Grid external Limiter current L2

        float grid_frequency;           // Register 79: Grid frequency
        uint8_t grid_relay_status;      // Register 194: Grid side relay status
        
        // Inverter variables
        float inverter_voltage;         // Register 156: Inverter output voltage L1-L2
        float inverter_current_l1;      // Register 164: Inverter output current L1
        float inverter_current_l2;      // Register 165: Inverter output current L2
        float inverter_frequency;       // Register 193: Inverter output frequency
        uint8_t inverter_status;        // Register 59: Inverter Status
        
        // Load variables
        float load_current_l1;          // Register 179: Load current L1
        float load_current_l2;          // Register 180: Load current L2
        float load_frequency;           // Register 192: Load frequency
        
        // Generator variables
        uint8_t generator_relay_status; // Register 195: Generator side relay status
        uint8_t grid_type;              // Register 286: Grid Type
        
        // Battery configuration variables (for SunSpec model 713)
        float battery_capacity;           // Register 204: Batt Capacity
        float corrected_battery_capacity; // Register 107: Corrected Batt Capacity
        float battery_empty_voltage;      // Register 205: Batt Empty V
        float battery_shutdown_voltage;   // Register 220: Battery Shut Down V
        float battery_restart_voltage;    // Register 221: Battery Restart V
        float battery_low_voltage;        // Register 222: Battery Low Batt V
        uint8_t battery_shutdown_percent; // Register 217: Battery Shut Down %
        uint8_t battery_restart_percent;  // Register 218: Battery Restart %
        uint8_t battery_low_percent;      // Register 219: Battery Low Batt %
        
        // BMS variables (for lithium batteries)
        float bms_charging_voltage;       // Register 312: Charging voltage
        float bms_discharge_voltage;      // Register 313: Discharge voltage
        float bms_charging_current_limit; // Register 314: Charging current limit
        float bms_discharge_current_limit; // Register 315: Discharge current limit
        float bms_real_time_soc;          // Register 316: Real time SOC
        float bms_real_time_voltage;      // Register 317: Real time voltage
        float bms_real_time_current;      // Register 318: Real time current
        float bms_real_time_temp;         // Register 319: Real time temp
        uint16_t bms_warning;             // Register 322: BMS Lithium battery warning
        uint16_t bms_fault;               // Register 323: BMS Lithium battery fault
};

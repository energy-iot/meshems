#pragma once

#include <modbus_master.h>

// Register mapping structure for Sol-Ark
struct SolArkRegisterMap {
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
        
        // Load Status Getters
        float getLoadCurrentL1();
        float getLoadCurrentL2();
        float getLoadFrequency();
        
        // Generator Status Getters
        uint8_t getGeneratorRelayStatus();
        
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
        
        // Battery variables
        float battery_power;       // Register 190
        float battery_current;     // Register 191
        float battery_voltage;     // Register 183
        float battery_soc;         // Register 184
        float battery_temperature; // Register 182
        
        // Energy counters
        float battery_charge_energy;    // Register 70
        float battery_discharge_energy; // Register 71
        float grid_buy_energy;          // Register 76
        float grid_sell_energy;         // Register 77
        float load_energy;              // Register 84
        float pv_energy;                // Register 108
        
        // Power variables
        float grid_power;           // Register 169
        float inverter_output_power; // Register 175
        float load_power_l1;         // Register 176
        float load_power_l2;         // Register 177
        float load_power_total;      // Register 178
        float pv1_power;             // Register 186
        float pv2_power;             // Register 187
        float pv_power_total;        // Calculated
        float smart_load_power;      // Register 166
        
        // Grid variables
        float grid_voltage;     // Register 152
        float grid_current_l1;  // Register 160
        float grid_current_l2;  // Register 161
        
        float grid_CT_current_l1; // 162
        float grid_CT_current_l2; // 163

        float grid_frequency;   // Register 79
        uint8_t grid_relay_status; // Register 194
        
        // Inverter variables
        float inverter_voltage;     // Register 156
        float inverter_current_l1;  // Register 164
        float inverter_current_l2;  // Register 165
        float inverter_frequency;   // Register 193
        
        // Load variables
        float load_current_l1;   // Register 179
        float load_current_l2;   // Register 180
        float load_frequency;    // Register 192
        
        // Generator variables
        uint8_t generator_relay_status; // Register 195
};

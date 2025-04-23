#pragma once

#include <modbus_master.h>

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
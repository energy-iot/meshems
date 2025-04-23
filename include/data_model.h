#pragma once

#define MODBUS_NUM_COILS                2
#define MODBUS_NUM_DISCRETE_INPUTS      2
#define MODUBS_NUM_HOLDING_REGISTERS    2
#define MODBUS_NUM_INPUT_REGISTERS      4

extern bool coils[MODBUS_NUM_COILS];
extern bool discreteInputs[MODBUS_NUM_DISCRETE_INPUTS];
extern uint16_t holdingRegisters[MODUBS_NUM_HOLDING_REGISTERS];
extern uint16_t inputRegisters[MODBUS_NUM_INPUT_REGISTERS];

// Struct for current, voltage, and power factor
struct PowerData {
    unsigned long timestamp_last_report = 0;
    float total_energy = 0;  // kWh
    float export_energy = 0; // kWh
    float import_energy = 0; // kWh
    float voltage = 0;       // V
    float current = 0;       // A
    float active_power = 0;  // kW
    float reactive_power = 0; // kVAr
    float power_factor = 0;  // 0-1
    float frequency = 0;     // Hz
    float metadata = 0;      // 1-247 (high byte), 1-16 (low byte)
};

extern PowerData last_reading;
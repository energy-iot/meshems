#pragma once

// Expanded register space to accommodate SunSpec models
#define MODBUS_NUM_COILS                2
#define MODBUS_NUM_DISCRETE_INPUTS      2
#define MODUBS_NUM_HOLDING_REGISTERS    300  // Expanded for SunSpec models
#define MODBUS_NUM_INPUT_REGISTERS      4

extern bool coils[MODBUS_NUM_COILS];
extern bool discreteInputs[MODBUS_NUM_DISCRETE_INPUTS];
extern uint16_t holdingRegisters[MODUBS_NUM_HOLDING_REGISTERS];
extern uint16_t inputRegisters[MODBUS_NUM_INPUT_REGISTERS];

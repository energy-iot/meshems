#include <Arduino.h>
#include <data_model.h>

bool coils[MODBUS_NUM_COILS];
bool discreteInputs[MODBUS_NUM_DISCRETE_INPUTS];
uint16_t holdingRegisters[MODUBS_NUM_HOLDING_REGISTERS];
uint16_t inputRegisters[MODBUS_NUM_INPUT_REGISTERS];
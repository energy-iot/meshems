#pragma once

#define MODBUS_NUM_COILS                2
#define MODBUS_NUM_DISCRETE_INPUTS      2
#define MODUBS_NUM_HOLDING_REGISTERS    2
#define MODBUS_NUM_INPUT_REGISTERS      4
#define CURRENT_HISTORY_SIZE            128  // Store 128 historical readings

extern bool coils[MODBUS_NUM_COILS];
extern bool discreteInputs[MODBUS_NUM_DISCRETE_INPUTS];
extern uint16_t holdingRegisters[MODUBS_NUM_HOLDING_REGISTERS];
extern uint16_t inputRegisters[MODBUS_NUM_INPUT_REGISTERS];

// Current history data structure for timeline graph
typedef struct {
    float values[CURRENT_HISTORY_SIZE];  // Circular buffer for current values
    int currentIndex;                    // Current position in the buffer
    int count;                           // Number of readings stored (up to CURRENT_HISTORY_SIZE)
    float minValue;                      // Minimum value in the buffer (for auto-scaling)
    float maxValue;                      // Maximum value in the buffer (for auto-scaling)
} CurrentHistory;

extern CurrentHistory currentHistory;

// Function to add a new current reading to the history buffer
void addCurrentReading(float value);
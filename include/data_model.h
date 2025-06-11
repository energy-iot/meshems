#pragma once

//#define MODBUS_NUM_METERS 1
#define MODBUS_NUM_METERS 3 //TODO 
#define MODBUS_NUM_THERMOSTATS 1

#define MODBUS_NUM_COILS                2
#define MODBUS_NUM_DISCRETE_INPUTS      2
#define MODUBS_NUM_HOLDING_REGISTERS    2
#define MODBUS_NUM_INPUT_REGISTERS      4
#define CURRENT_HISTORY_SIZE            128  // Store 128 historical readings

extern bool coils[MODBUS_NUM_COILS];
extern bool discreteInputs[MODBUS_NUM_DISCRETE_INPUTS];
extern uint16_t holdingRegisters[MODUBS_NUM_HOLDING_REGISTERS];
extern uint16_t inputRegisters[MODBUS_NUM_INPUT_REGISTERS];

 // Struct for current, voltage, and power factor
//EMS as a 3 phase subpanel with N  meters and x meters per phase SO has multiple dimensions of powerdata to totalize and publish
// 1. all 3 phases totalized powerdata usage per StreetPoleSubpanel
// 2. each Phase powerdata summary per subpanel
// 3. each singlephase meter powerdata including which phase - TODO o\is qr code all subpanel networked items
// powerdata has leakage and harmonic transients that are key to track for periodic engineering Operations and maintenance and rebalancing alerts
// for a future staging/installer app, all networking device shall have meaningful QR codes
// Staging or install or at maintenance time  scan the subpanel and all active networking parts installed to the subpanel
// are auto provisoned in a backend Db addressable to the MS subpanel globally unique QR code
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

// Current history data structure for timeline graph
typedef struct {
    float values[CURRENT_HISTORY_SIZE];  // Circular buffer for current values
    int currentIndex;                    // Current position in the buffer
    int count;                           // Number of readings stored (up to CURRENT_HISTORY_SIZE)
    float minValue;                      // Minimum value in the buffer (for auto-scaling)
    float maxValue;                      // Maximum value in the buffer (for auto-scaling)
} CurrentHistory;

extern CurrentHistory currentHistory;
extern PowerData readings[]; // Array to hold readings for each meter

// Function to add a new current reading to the history buffer
void addCurrentReading(float value);

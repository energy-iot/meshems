#pragma once

//#define MODBUS_NUM_METERS 1
#define MODBUS_NUM_METERS 1 //TODO 
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

struct PowerData {  // single phase per meter data, equivalent to per tenant
    unsigned long timestamp_last_report = 0;
    float total_energy = 0;  // kWh
    float export_energy = 0; // kWh
    float import_energy = 0; // kWh
    float stored_energy =0;  // kWh
    float transform_energy = 0; // tracks total energy transformed AC-DC inverted and converted dc-dc or ac-ac
    float voltage = 0;       // V
    float current = 0;       // A
    float active_power = 0;  // kW
    float reactive_power = 0; // kVAr
    float power_factor = 0;  // 0-1
    float frequency = 0;     // Hz
    float phase  = 0;        // a,b,c if 3 phase EMS subpanel, or 0 if single phase subpanel
    float meterid = 0;       // use modbus node number 
    float metadata = 0;      // 1-247 (high byte), 1-16 (low byte)
};

struct Power1PhData {  // single phase per meter data, equivalent to per tenant
    unsigned long timestamp_last_report = 0;
    float total_energy = 0;  // kWh
    float export_energy = 0; // kWh
    float import_energy = 0; // kWh
    float stored_energy =0;  // kWh
    float transform_energy = 0; // tracks total energy transformed AC-DC inverted and converted dc-dc or ac-ac
    float voltage = 0;       // V
    float current = 0;       // A
    float active_power = 0;  // kW
    float reactive_power = 0; // kVAr
    float power_factor = 0;  // 0-1
    float frequency = 0;     // Hz
    float phase  = 0;        // a,b,c if 3 phase EMS subpanel, or 0 if single phase subpanel
    float meterid = 0;       // use modbus node number 
    float metadata = 0;      // 1-247 (high byte), 1-16 (low byte)
};

struct Power3PhData {  // each EMS subpanel has 3phase multiple tenants  per pahse multiple per phase totals
    unsigned long timestamp_last_report = 0;
    //TODO add 3PhasePowerData cached data items here
    float metadata = 0;      // 1-247 (high byte), 1-16 (low byte)
};

struct DTMData {
    unsigned long timestamp_last_report = 0;

    // Group 0: Line info — regs 1000-1008
    uint16_t board_status  = 0;   // reg 1000: bit0=meterOK bit2=netUp
    uint16_t sys_status0   = 0;   // reg 1001
    uint16_t sys_status1   = 0;   // reg 1002
    uint16_t meter_status0 = 0;   // reg 1003
    uint16_t meter_status1 = 0;   // reg 1004
    float    voltage       = 0;   // reg 1005 ÷10   V
    float    frequency     = 0;   // reg 1006 ÷100  Hz
    float    temp          = 0;   // reg 1007 ÷10   °C (signed)
    float    voltage_diff  = 0;   // reg 1008 ÷10   V  (signed)

    // Group 1: CT1 — regs 1010-1016
    float ct1_voltage     = 0;   // reg 1010 ÷10    V
    float ct1_current     = 0;   // reg 1011 ÷100   A  (signed)
    float ct1_real_power  = 0;   // reg 1012 ÷1     W  (signed)
    float ct1_react_power = 0;   // reg 1013 ÷1     VAR (signed)
    float ct1_va_power    = 0;   // reg 1014 ÷1     VA
    float ct1_pf          = 0;   // reg 1015 ÷1000  (signed)
    float ct1_phase       = 0;   // reg 1016 ÷10    ° (signed)

    // Group 2: CT2 — regs 1020-1026
    float ct2_voltage     = 0;
    float ct2_current     = 0;
    float ct2_real_power  = 0;
    float ct2_react_power = 0;
    float ct2_va_power    = 0;
    float ct2_pf          = 0;
    float ct2_phase       = 0;

    // Group 3: CT3 — regs 1030-1036
    float ct3_voltage     = 0;
    float ct3_current     = 0;
    float ct3_real_power  = 0;
    float ct3_react_power = 0;
    float ct3_va_power    = 0;
    float ct3_pf          = 0;
    float ct3_phase       = 0;

    // Group 4: Neutral — reg 1040
    float current_n = 0;          // reg 1040 ÷1000  A (signed)

    // Group 5: Totals — regs 1050-1055
    float total_active_power   = 0;  // reg 1050 ÷1  W   (signed)
    float total_reactive_power = 0;  // reg 1051 ÷1  VAR (signed)
    float total_apparent_power = 0;  // reg 1052 ÷1  VA
    float total_pf             = 0;  // reg 1053 ÷1000 (signed)
    float total_fund_power     = 0;  // reg 1054 ÷1  W   (signed)
    float total_har_power      = 0;  // reg 1055 ÷1  W   (signed)

    // Group 6: Energy accumulators — regs 1060-1068 (32-bit HI/LO pairs, ÷10)
    float import_energy          = 0;  // regs 1060-1061  kWh
    float export_energy          = 0;  // regs 1062-1063  kWh
    float import_reactive_energy = 0;  // regs 1064-1065  kVARh
    float export_reactive_energy = 0;  // regs 1066-1067  kVARh
    float import_apparent_energy = 0;  // reg  1068 HI only (1069 LO beyond scan range)  kVAh
};

struct LeakageData {  // TODO expand this struct for all powerdata OR have different structs
    unsigned long timestamp_last_report = 0;
    //TODO add LeakageData cached data items here
    float metadata = 0;      // 1-247 (high byte), 1-16 (low byte)
};
struct HarmonicsData {  // TODO expand this struct for all powerdata OR have different structs
    unsigned long timestamp_last_report = 0;
    //TODO add Harmonics cached data items here
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

extern PowerData last_reading; // older cache data to allow easier iterative design testing and debugging
extern Power3PhData last_EMS_power_reading;  // 3 phase streetpole EMS ( include EMS subpanel scoped energy data totalized per phase)
extern Power1PhData last_power_reading;         // TODO make per meter/tenant per phase modbus node num unique to EMS only
extern HarmonicsData last_harmonics_reading;    // TODO breakout Harmonics data Current and Voltage per phase as its own 
extern LeakageData last_leakage_reading;        // Leakage data is measured reported and actionable  perm streetPoleEMS per phase
                                                // TODO single phase meter can have per phase leakage either as mRCM or RCD
                                                // RCD leakage will set fault and cause tenant contactor to open circuit 
                                                // RCM leakage will report per tenant leakage measurements and autonoomous trigger of tenant contactor to open if hits life threatening levels as per Type B
                                                // if per phase leakage is measured then may not have to per tenant leakage - downside is the\\at all tenants on a phase get opencircuited if phase leakage RCM hits life threatening levels


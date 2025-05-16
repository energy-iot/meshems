#pragma once

#include <Arduino.h>

// SunSpec ID marker "SunS" (0x53756e53)
#define SUNSPEC_ID_MSW 0x5375
#define SUNSPEC_ID_LSW 0x6e53

// SunSpec model IDs
#define SUNSPEC_MODEL_COMMON 1
#define SUNSPEC_MODEL_INVERTER 701
#define SUNSPEC_MODEL_DER_STORAGE 713

// Register map base addresses
#define SUNSPEC_BASE_ADDR 40000
#define SUNSPEC_COMMON_START (SUNSPEC_BASE_ADDR + 4)
#define SUNSPEC_INVERTER_START (SUNSPEC_COMMON_START + 66)

// TEST Common Model (1) register offsets
#define COMMON_MODEL_ID 0
#define COMMON_MODEL_LENGTH 1
#define COMMON_MANUFACTURER 2
#define COMMON_MODEL 18
#define COMMON_OPTIONS 34
#define COMMON_VERSION 42
#define COMMON_SERIAL 50
#define COMMON_DEVICE_ADDR 66

// Inverter Model (701) register offsets
#define INV_MODEL_ID 0
#define INV_MODEL_LENGTH 1
#define INV_AC_TYPE 2          // AC Wiring Type
#define INV_OPERATING_STATE 3  // Operating State
#define INV_STATUS 4           // Inverter State
#define INV_GRID_CONNECTION 5  // Grid Connection State
#define INV_ALARM 6            // Alarm Bitfield (2 registers)
#define INV_DER_MODE 8         // DER Operational Characteristics (2 registers)
#define INV_AC_POWER 10        // Active Power
#define INV_AC_VA 11           // Apparent Power
#define INV_AC_VAR 12          // Reactive Power
#define INV_AC_PF 13           // Power Factor
#define INV_AC_CURRENT 14      // Total AC Current
#define INV_AC_VOLTAGE_LL 15   // Voltage LL
#define INV_AC_VOLTAGE_LN 16   // Voltage LN
#define INV_AC_FREQUENCY 17    // Frequency (2 registers)

// Energy registers
#define INV_ENERGY_INJECTED 19    // Total Energy Injected (4 registers)
#define INV_ENERGY_ABSORBED 23    // Total Energy Absorbed (4 registers)
#define INV_REACTIVE_INJECTED 27  // Total Reactive Energy Injected (4 registers)
#define INV_REACTIVE_ABSORBED 31  // Total Reactive Energy Absorbed (4 registers)

// Temperature registers
#define INV_TEMP_AMBIENT 35      // Ambient Temperature
#define INV_TEMP_CABINET 36      // Cabinet Temperature
#define INV_TEMP_SINK 37         // Heat Sink Temperature
#define INV_TEMP_TRANSFORMER 38  // Transformer Temperature
#define INV_TEMP_IGBT 39         // IGBT/MOSFET Temperature
#define INV_TEMP_OTHER 40        // Other Temperature

// Phase L1 registers
#define INV_AC_POWER_L1 41       // Active Power L1
#define INV_AC_VA_L1 42          // Apparent Power L1
#define INV_AC_VAR_L1 43         // Reactive Power L1
#define INV_AC_PF_L1 44          // Power Factor L1
#define INV_AC_CURRENT_L1 45     // Current L1
#define INV_AC_VOLTAGE_L1L2 46   // Phase Voltage L1-L2
#define INV_AC_VOLTAGE_L1N 47    // Phase Voltage L1-N

// Phase L2 registers
#define INV_AC_POWER_L2 64       // Active Power L2
#define INV_AC_VA_L2 65          // Apparent Power L2
#define INV_AC_VAR_L2 66         // Reactive Power L2
#define INV_AC_PF_L2 67          // Power Factor L2
#define INV_AC_CURRENT_L2 68     // Current L2
#define INV_AC_VOLTAGE_L2L3 69   // Phase Voltage L2-L3
#define INV_AC_VOLTAGE_L2N 70    // Phase Voltage L2-N

// Phase L3 registers
#define INV_AC_POWER_L3 87       // Active Power L3
#define INV_AC_VA_L3 88          // Apparent Power L3
#define INV_AC_VAR_L3 89         // Reactive Power L3
#define INV_AC_PF_L3 90          // Power Factor L3
#define INV_AC_CURRENT_L3 91     // Current L3
#define INV_AC_VOLTAGE_L3L1 92   // Phase Voltage L3-L1
#define INV_AC_VOLTAGE_L3N 93    // Phase Voltage L3-N

// Throttling registers
#define INV_THROTTLE_PCT 110     // Throttling In Pct
#define INV_THROTTLE_SRC 111     // Throttle Source Information (2 registers)

// Scale factors
#define INV_SF_CURRENT 113       // Current Scale Factor
#define INV_SF_VOLTAGE 114       // Voltage Scale Factor
#define INV_SF_FREQUENCY 115     // Frequency Scale Factor
#define INV_SF_POWER 116         // Active Power Scale Factor
#define INV_SF_PF 117            // Power Factor Scale Factor
#define INV_SF_VA 118            // Apparent Power Scale Factor
#define INV_SF_VAR 119           // Reactive Power Scale Factor
#define INV_SF_ENERGY 120        // Active Energy Scale Factor
#define INV_SF_REACTIVE_ENERGY 121 // Reactive Energy Scale Factor
#define INV_SF_TEMP 122          // Temperature Scale Factor

// Vendor information
#define INV_ALARM_INFO 123       // Manufacturer Alarm Info (32 registers)

// DER Storage Capacity Model (713) register offsets
#define STORAGE_MODEL_ID 0
#define STORAGE_MODEL_LENGTH 1
#define STORAGE_ENERGY_RATING 2      // Energy rating of the DER storage
#define STORAGE_ENERGY_AVAILABLE 3   // Energy available of the DER storage
#define STORAGE_SOC 4                // State of charge of the DER storage
#define STORAGE_SOH 5                // State of health of the DER storage
#define STORAGE_STATUS 6             // Storage status
#define STORAGE_SF_ENERGY 7          // Scale factor for energy capacity
#define STORAGE_SF_PERCENT 8         // Scale factor for percentage

// Status flags for INV_STATUS
#define STAT_OFF 0x0001
#define STAT_SLEEPING 0x0002
#define STAT_STARTING 0x0004
#define STAT_MPPT 0x0008
#define STAT_THROTTLED 0x0010
#define STAT_SHUTTING_DOWN 0x0020
#define STAT_FAULT 0x0040
#define STAT_STANDBY 0x0080

// Scale factors
#define SCALE_FACTOR_1 0
#define SCALE_FACTOR_0_1 -1
#define SCALE_FACTOR_0_01 -2
#define SCALE_FACTOR_0_001 -3
#define SCALE_FACTOR_10 1
#define SCALE_FACTOR_100 2
#define SCALE_FACTOR_1000 3

// SunSpec special values
#define SUNSPEC_NOT_IMPLEMENTED 0xFFFF
#define SUNSPEC_NOT_SUPPORTED 0x8000

// Function to initialize SunSpec models in the Modbus register map
void setup_sunspec_models();

// Function to update SunSpec registers with Sol-Ark data
void update_sunspec_from_solark();

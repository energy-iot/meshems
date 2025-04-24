#pragma once

#include <Arduino.h>

// SunSpec ID marker "SunS" (0x53756e53)
#define SUNSPEC_ID_MSW 0x5375
#define SUNSPEC_ID_LSW 0x6e53

// SunSpec model IDs
#define SUNSPEC_MODEL_COMMON 1
#define SUNSPEC_MODEL_INVERTER 701

// Register map base addresses
#define SUNSPEC_BASE_ADDR 40000
#define SUNSPEC_COMMON_START (SUNSPEC_BASE_ADDR + 4)
#define SUNSPEC_INVERTER_START (SUNSPEC_COMMON_START + 66)

// Common Model (1) register offsets
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
#define INV_STATUS 4 // 4 Inverter State

#define INV_AC_CURRENT 14
#define INV_AC_CURRENT_A 45
#define INV_AC_CURRENT_B 68
#define INV_AC_CURRENT_C 91
#define INV_AC_VOLTAGE_AB 46
#define INV_AC_VOLTAGE_BC 69
#define INV_AC_VOLTAGE_CA 92
#define INV_AC_VOLTAGE_AN 47
#define INV_AC_VOLTAGE_BN 70
#define INV_AC_VOLTAGE_CN 93
#define INV_AC_POWER 10
#define INV_AC_FREQUENCY 17
#define INV_AC_VA 11
#define INV_AC_VAR 12
#define INV_AC_PF 13

#define INV_AC_ENERGY_WH 34
#define INV_DC_CURRENT 36
#define INV_DC_VOLTAGE 38
#define INV_DC_POWER 40
#define INV_TEMP_CABINET 42
#define INV_TEMP_SINK 44
#define INV_TEMP_TRANSFORMER 46
#define INV_TEMP_OTHER 48

#define INV_STATUS_VENDOR 123

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

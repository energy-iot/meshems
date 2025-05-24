#pragma once

/**
 * @file data_model.h
 * @brief Data structures and Modbus register definitions for EMS device
 * 
 * This file defines the data structures used throughout the EMS system
 * including Modbus register arrays and power measurement data structures.
 */

// Modbus Register Array Sizes
#define MODBUS_NUM_COILS                2
#define MODBUS_NUM_DISCRETE_INPUTS      2
#define MODBUS_NUM_HOLDING_REGISTERS    2
#define MODBUS_NUM_INPUT_REGISTERS      4

/**
 * @struct PowerData
 * @brief Structure to hold electrical power measurements
 * 
 * Contains all electrical parameters typically measured by energy meters
 * including voltage, current, power, energy, and power quality metrics.
 */
struct PowerData {
    float voltage;                    ///< Line voltage (V)
    float current;                    ///< Line current (A)
    float active_power;               ///< Active power (kW)
    float reactive_power;             ///< Reactive power (kVAr)
    float power_factor;               ///< Power factor (dimensionless, 0-1)
    float frequency;                  ///< Line frequency (Hz)
    float total_energy;               ///< Total energy consumption (kWh)
    float export_energy;              ///< Energy exported to grid (kWh)
    float import_energy;              ///< Energy imported from grid (kWh)
    unsigned long timestamp_last_report; ///< Timestamp of last measurement
    uint16_t metadata;                ///< Device-specific metadata
};

// Modbus Register Arrays
extern bool coils[MODBUS_NUM_COILS];
extern bool discreteInputs[MODBUS_NUM_DISCRETE_INPUTS];
extern uint16_t holdingRegisters[MODBUS_NUM_HOLDING_REGISTERS];
extern uint16_t inputRegisters[MODBUS_NUM_INPUT_REGISTERS];

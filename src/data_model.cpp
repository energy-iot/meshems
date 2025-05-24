/**
 * @file data_model.cpp
 * @brief Implementation of the Modbus data model
 * 
 * This file defines the data structures that represent the four standard Modbus
 * data types: coils, discrete inputs, holding registers, and input registers.
 * These arrays store the values that can be read from or written to by Modbus
 * master/client devices.
 */

 #include <Arduino.h>
 #include <data_model.h>
 
 /**
  * Coils (read-write digital outputs)
  * 
  * This array stores the state of all coils in the system.
  * Coils are binary (ON/OFF) values that can be read and written by Modbus masters.
  * They typically represent control outputs such as relays or digital outputs.
  */
 bool coils[MODBUS_NUM_COILS];
 
 /**
  * Discrete Inputs (read-only digital inputs)
  * 
  * This array stores the state of all discrete inputs in the system.
  * Discrete inputs are binary (ON/OFF) values that can only be read by Modbus masters.
  * They typically represent the state of digital inputs like switches or sensors.
  */
 bool discreteInputs[MODBUS_NUM_DISCRETE_INPUTS];
 
 /**
  * Holding Registers (read-write 16-bit registers)
  * 
  * This array stores the values of all holding registers in the system.
  * Holding registers are 16-bit values that can be read and written by Modbus masters.
  * They typically store configuration parameters or setpoints for the device.
  */
 uint16_t holdingRegisters[MODBUS_NUM_HOLDING_REGISTERS];
 
 /**
  * Input Registers (read-only 16-bit registers)
  * 
  * This array stores the values of all input registers in the system.
  * Input registers are 16-bit values that can only be read by Modbus masters.
  * They typically store measured values from sensors or status information.
  */
 uint16_t inputRegisters[MODBUS_NUM_INPUT_REGISTERS];

/*
  ModbusRTUSlaveExample

  This example demonstrates how to setup and use the ModbusRTUSlave library (https://github.com/CMB27/ModbusRTUSlave).
  It is intended to be used with a second board running ModbusRTUMasterExample from the ModbusRTUMaster library (https://github.com/CMB27/ModbusRTUMaster).
  
  Created: 2023-07-22
  By: C. M. Bulliner
  Last Modified: 2024-01-27
  By: C. M. Bulliner
  
  Modified for EMS ModCan Hub by: doug mendonca
  
  SunSpec compliance added: 2025-04-23
*/
#include <modbus.h>
#include <SoftwareSerial.h>
#include <pins.h>
#include <data_model.h>
#include <sunspec_models.h>

SoftwareSerial _modbus2(RS485_RX_2, RS485_TX_2); //(rx, tx) corresponds with HW519 rxd txd pins
ModbusRTUSlave modbus_client(_modbus2);

// Flag to track if SunSpec registers have been updated at least once
bool sunspec_initialized = false;

void setup_modbus_client() {
  //#if defined ESP32
  //  analogReadResolution(10);
  //#endif

  modbus_client.configureCoils(coils, MODBUS_NUM_COILS);                       // bool array of coil values, number of coils
  modbus_client.configureDiscreteInputs(discreteInputs, MODBUS_NUM_DISCRETE_INPUTS);     // bool array of discrete input values, number of discrete inputs
  modbus_client.configureHoldingRegisters(holdingRegisters, MODUBS_NUM_HOLDING_REGISTERS); // unsigned 16 bit integer array of holding register values, number of holding registers
  modbus_client.configureInputRegisters(inputRegisters, MODBUS_NUM_INPUT_REGISTERS);     // unsigned 16 bit integer array of input register values, number of input registers

  // Initialize SunSpec models in the Modbus register map
  setup_sunspec_models();
  
  Serial.println("INFO - Modbus Client: SunSpec models initialized");
  Serial.println("INFO - Modbus Client: SunSpec Common (1) and Inverter (701) models available");

  _modbus2.begin(9600);
  modbus_client.begin(1, 9600);
  
  Serial.println("INFO - Modbus Client: Started as SunSpec-compliant server on address 1");
}

void loop_modbus_client() {
  // Update SunSpec registers with latest Sol-Ark data
  update_sunspec_from_solark();
  
  // Poll for Modbus requests
  modbus_client.poll();
  
  // Log SunSpec initialization once
  if (!sunspec_initialized) {
    Serial.println("INFO - Modbus Client: SunSpec registers updated with initial Sol-Ark data");
    sunspec_initialized = true;
  }
}

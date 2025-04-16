/*
  ModbusRTUSlaveExample

  This example demonstrates how to setup and use the ModbusRTUSlave library (https://github.com/CMB27/ModbusRTUSlave).
  It is intended to be used with a second board running ModbusRTUMasterExample from the ModbusRTUMaster library (https://github.com/CMB27/ModbusRTUMaster).
  
  Created: 2023-07-22
  By: C. M. Bulliner
  Last Modified: 2024-01-27
  By: C. M. Bulliner
  
  Modified for EMS ModCan Hub by: doug mendonca
*/
#include <modbus.h>
#include <SoftwareSerial.h>
#include <pins.h>
#include <data_model.h>

SoftwareSerial _modbus2(RS485_RX_2, RS485_TX_2); //(rx, tx) corresponds with HW519 rxd txd pins
ModbusRTUSlave modbus_client(_modbus2);

void setup_modbus_client() {
  //#if defined ESP32
  //  analogReadResolution(10);
  //#endif

  modbus_client.configureCoils(coils, 2);                       // bool array of coil values, number of coils
  modbus_client.configureDiscreteInputs(discreteInputs, 2);     // bool array of discrete input values, number of discrete inputs
  modbus_client.configureHoldingRegisters(holdingRegisters, 2); // unsigned 16 bit integer array of holding register values, number of holding registers
  modbus_client.configureInputRegisters(inputRegisters, 4);     // unsigned 16 bit integer array of input register values, number of input registers

  _modbus2.begin(9600);
  modbus_client.begin(1, 9600);
}

void loop_modbus_client() {
  modbus_client.poll();
}

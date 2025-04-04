/*
  ModbusRTUSlaveExample

  This example demonstrates how to setup and use the ModbusRTUSlave library (https://github.com/CMB27/ModbusRTUSlave).
  It is intended to be used with a second board running ModbusRTUMasterExample from the ModbusRTUMaster library (https://github.com/CMB27/ModbusRTUMaster).

  This program has been succsessfully tested with the following boards:
  - Arduino Leonardo
  - Arduino Make Your UNO (USB had to be unplugged to work with HardwareSerial)
  - Arduino Mega 2560
  - Arduino Nano
  - Arduino Nano 33 BLE
  - Arduino Nano 33 IoT
  - Arduino Nano ESP32
  - Arduino Nano Every
  - Arduino Nano RP2040 Connect - Using Earle F. Philhower's arduino-pico core
  - Arduino UNO R3 SMD
  - Arduino UNO R4

  Problems were encountered with the following board:
  - Arduino Nano RP2040 Connect - Using Arduino's ArduinoCore-mbed (Reliable communication could not be established with the master/client board)

  !WARNING
  When connecting boards using UART, as described in the circuit below, the logic level voltages must match (5V or 3.3V).
  If they do not, use a logic level converter, otherwise your 3.3V board could be damaged.
  
  Circuit:
  - The center pin of a potentiometer to pin A0, and the outside pins of the potentiometer to your board's logic level voltage (5V or 3.3V) and GND
  - The center pin of a potentiometer to pin A1, and the outside pins of the potentiometer to your board's logic level voltage (5V or 3.3V) and GND
  - A pushbutton switch from pin 2 to GND
  - A pushbutton switch from pin 3 to GND
  - A LED from pin 5 to GND with a 1K ohm series resistor
  - A LED from pin 6 to GND with a 1K ohm series resistor
  - A LED from pin 7 to GND with a 1K ohm series resistor
  - A LED from pin 8 to GND with a 1K ohm series resistor
  - RX pin (typically pin 0 or pin 10 if using SoftwareSerial) to TX pin of the master/client board
  - TX pin (typically pin 1 or pin 11 if using SoftwareSerial) to RX pin of the master/client board
  - GND to GND of the master/client board
  - Pin 13 is set up as the driver enable pin. This pin will be HIGH whenever the board is transmitting.

  !NOTE
  Both boards will also need power.
  
  Created: 2023-07-22
  By: C. M. Bulliner
  Last Modified: 2024-01-27
  By: C. M. Bulliner
  
  modified for EMS ModCan Hub by: doug mendonca
*/
#include <modbus.h>
#include <SoftwareSerial.h>
#include <pins.h>

SoftwareSerial _modbus2(RS485_RX_2, RS485_TX_2); //(rx, tx) corresponds with HW519 rxd txd pins
ModbusRTUSlave modbus_client(_modbus2);

bool coils[2];
bool discreteInputs[2];
uint16_t holdingRegisters[2];
uint16_t inputRegisters[4];

void setup_modbus_client() {
  //#if defined ESP32
  //  analogReadResolution(10);
  //#endif
  inputRegisters[0] = 33;//map(analogRead(potPins[0]), 0, 1023, 0, 255);
  inputRegisters[1] = 44;// map(analogRead(potPins[1]), 0, 1023, 0, 255);
  inputRegisters[2] = 55;// map(analogRead(potPins[1]), 0, 1023, 0, 255);
  inputRegisters[3] = 66;// map(analogRead(potPins[1]), 0, 1023, 0, 255);
  discreteInputs[0] = 8;//!digitalRead(buttonPins[0]);
  discreteInputs[1] = 9;//!digitalRead(buttonPins[1]);
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

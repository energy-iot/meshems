/*
  ModbusTCP SunSpec Server

  This implementation provides a SunSpec-compliant Modbus TCP server using the modbus-esp8266 library.
  It exposes Sol-Ark inverter data in a standardized SunSpec format over TCP/IP.
  
  Created: 2025-05-14
  Based on previous RTU implementation by: C. M. Bulliner, doug mendonca
  
  SunSpec compliance added: 2025-04-23
  TCP/IP implementation added: 2025-05-14
*/
#include <modbus.h>
#include <WiFi.h>
#include <pins.h>
#include <data_model.h>
#include <sunspec_models.h>
#include <modbus_client.h>

// Create ModbusIP instance
ModbusIP mb;

// Flag to track if SunSpec registers have been updated at least once
bool sunspec_initialized = false;

void setup_modbus_client() {
  // Initialize SunSpec models in the Modbus register map
  setup_sunspec_models();
  
  Serial.println("INFO - Modbus Client: SunSpec models initialized");
  Serial.println("INFO - Modbus Client: SunSpec Common (1) and Inverter (701) models available");

  // Start WiFi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Wait for WiFi connection
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());
  
  // Configure Modbus registers
  // For ModbusIP, we need to add each register individually
  // Add coils
  for (uint16_t i = 0; i < MODBUS_NUM_COILS; i++) {
    mb.addCoil(i, coils[i]);
  }
  
  // Add discrete inputs
  for (uint16_t i = 0; i < MODBUS_NUM_DISCRETE_INPUTS; i++) {
    mb.addIsts(i, discreteInputs[i]);
  }
  
  // Add holding registers
  for (uint16_t i = 0; i < MODUBS_NUM_HOLDING_REGISTERS; i++) {
    mb.addHreg(i, holdingRegisters[i]);
  }
  
  // Add input registers
  for (uint16_t i = 0; i < MODBUS_NUM_INPUT_REGISTERS; i++) {
    mb.addIreg(i, inputRegisters[i]);
  }
  
  // Start Modbus TCP server on port 8502
  mb.server(8502);
  
  Serial.println("INFO - Modbus Client: Started as SunSpec-compliant TCP server on port 8502");
}

void loop_modbus_client() {
  // Update SunSpec registers with latest Sol-Ark data
  update_sunspec_from_solark();
  
  // Update the Modbus registers with the latest values
  for (uint16_t i = 0; i < MODUBS_NUM_HOLDING_REGISTERS; i++) {
    mb.Hreg(i, holdingRegisters[i]);
  }
  
  // Process Modbus TCP requests
  mb.task();
  
  // Log SunSpec initialization once
  if (!sunspec_initialized) {
    Serial.println("INFO - Modbus Client: SunSpec registers updated with initial Sol-Ark data");
    sunspec_initialized = true;
  }
}

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>

// Define the success code if not already defined
#ifndef ku8MBSuccess
#define ku8MBSuccess 0x00
#endif

// This function scans for Modbus devices on the given serial port
void scanModbusDevices(SoftwareSerial &serialPort) {
    ModbusMaster scanner;
    
    Serial.println("Starting Modbus scan...");
    
    // Scan through possible addresses (1-247)
    for (uint8_t address = 1; address <= 247; address++) {
        // Initialize ModbusMaster with current address
        scanner.begin(address, serialPort);
        
        // Try to read a single register - many devices have register 0 or 1 implemented
        // Try multiple registers to increase chances of finding a device
        uint8_t result = scanner.readHoldingRegisters(0x64, 2);
        
        if (result == ku8MBSuccess) {
            Serial.printf("Device found at address: %d\n", address);
            
            // Try to read a few more registers to get more info
            result = scanner.readHoldingRegisters(0x2000, 4); // Try config registers
            if (result == ku8MBSuccess) {
                Serial.println("Successfully read configuration registers");
                for (int i = 0; i < 4; i++) {
                    uint16_t value = scanner.getResponseBuffer(i);
                    Serial.printf("Register %d: 0x%04X (%d)\n", i, value, value);
                }
            }
            
            result = scanner.readInputRegisters(0x1000, 4); // Try status registers
            if (result == ku8MBSuccess) {
                Serial.println("Successfully read status registers");
                for (int i = 0; i < 4; i++) {
                    uint16_t value = scanner.getResponseBuffer(i);
                    Serial.printf("Register %d: 0x%04X (%d)\n", i, value, value);
                }
            }
        }
        
        // Add a small delay between queries to prevent overwhelming the bus
        delay(50);
    }
    
    Serial.println("Modbus scan complete");
}
/**
 * @file modbus_master.cpp
 */

 #include <SoftwareSerial.h>
 #include <modbus.h>
 #include <pins.h>
 #include <data_model.h>
 #include <console.h>

 // Declare the scanner function
void scanModbusDevices(SoftwareSerial &serialPort);

 // ModbusMaster success/failure constants if not defined
#ifndef ku8MBSuccess
#define ku8MBSuccess 0x00
#define ku8MBIllegalFunction 0x01
#define ku8MBIllegalDataAddress 0x02
#define ku8MBIllegalDataValue 0x03
#define ku8MBSlaveDeviceFailure 0x04
#define ku8MBTimeout 0xE0
#define ku8MBInvalidCRC 0xE1
#define ku8MBInvalidSlaveID 0xE2
#endif
 
 // Poll every x seconds
 #define POLL_INTERVAL 5000 
 #define EVSE_POLL_INTERVAL 5000 // 5 seconds

// ==================== Modbus Device Addresses ====================
 // #define THERMOSTAT_1_ADDR 0x00
 #define EVSE_ADDR 0x07
 #define DDS238_ADDR 0x01
 
 // ==================== Serial Interface Setup ====================
 // RS485 serial connections
 SoftwareSerial _modbus1(RS485_RX_1, RS485_TX_1); // HW519 module pinout
 //SoftwareSerial *modbus2(RS485_RX_2, RS485_TX_2); // Client in EMS ModCan

 //the EVSE controller
Modbus_EVSE evse;
 
 // Timing variables
 unsigned long lastMillis, lastEVSEMillis, lastEVSEChargingMillis = 0;

/**
 * Initialize EVSE Controller
 */
 void setup_evse() {
    Serial.printf("SETUP: MODBUS: EVSE: address:%d\n", EVSE_ADDR);
    evse.set_modbus_address(EVSE_ADDR);
    evse.begin(EVSE_ADDR, _modbus1);
    
    // Initial poll to get current state
    uint8_t result = evse.poll();
    if (result == ku8MBSuccess) {
      Serial.printf("INFO - EVSE initial state: %s\n", evse.getStatusString());
      
      // Could set initial parameters here if needed
      // For example:
      // evse.setMaxCurrent(32); // Set max current to 32A
    } else {
      Serial.println("ERROR - EVSE initialization failed!");
    }
  }
 
 /**
  * Initialize all Modbus clients
  */
 void setup_modbus_clients() {
     setup_evse();           // Initialize EKEPC2 EVSE
 }
 
 /**
  * Initialize Modbus master interface
  */
 void setup_modbus_master() {
     // Reset GPIO pins for RS485
     gpio_reset_pin(RS485_RX_1);
     gpio_reset_pin(RS485_TX_1);
     gpio_reset_pin(RS485_RX_2);
     gpio_reset_pin(RS485_TX_2);
 
     // Initialize serial at 9600 baud
     _modbus1.begin(9600);
     
     // Setup connected devices
     setup_modbus_clients();

     // Scan for devices - output only to Serial, not console
    // Serial.println("Scanning for devices...");
    // Serial.println("Port 1:");
    // scanModbusDevices(_modbus1);
 }
 
 
 void loop_evse() {
    if (millis() - lastEVSEMillis > EVSE_POLL_INTERVAL) {
      Serial.println("INFO - Polling EVSE controller");
      uint8_t result = evse.poll();
      
      if (result == ku8MBSuccess) {
        Serial.println("INFO - EVSE Status: " + String(evse.getStatusString()));
        
        if (evse.isCharging()) {
            Serial.println("INFO - EVSE Charging at " + String(evse.getChargingCurrent()) + "A");
        } else if (evse.isConnected()) {
          Serial.println("EV connected, not charging");
        }
      }
      
      lastEVSEMillis = millis();
    }
  }

 /**
  * Main polling loop for Modbus communication
  */
 void loop_modbus_master() {
     if (millis() - lastMillis > POLL_INTERVAL) {
         Serial.println("INFO - Poll Devices");
         Serial.println("INFO - No other devices to poll at this time");
         lastMillis = millis();
     }
     
     // Poll EVSE at its own interval
     loop_evse();
 }
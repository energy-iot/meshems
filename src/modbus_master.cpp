/**
 * @file modbus_master.cpp
 * @brief Modbus master implementation for SHT20 temperature/humidity sensors
 */

 #include <SoftwareSerial.h>
 #include <modbus.h>
 #include <pins.h>
 #include <data_model.h>
 
 // Poll every 10 seconds (300000ms = 5 mins for production)
 #define POLL_INTERVAL 10000 

// ==================== Modbus Device Addresses ====================
 #define THERMOSTAT_1_ADDR 0x01
 
 // ==================== Serial Interface Setup ====================
 // RS485 serial connections
 SoftwareSerial _modbus1(RS485_RX_1, RS485_TX_1); // HW519 module pinout
 //SoftwareSerial *modbus2(RS485_RX_2, RS485_TX_2); // Client in EMS ModCan
 
 // Temperature/humidity sensor
 Modbus_SHT20 sht20;
 
 // Timing variables
 unsigned long lastMillis, lastEVSEMillis, lastEVSEChargingMillis = 0;
 
 /**
  * Initialize SHT20 temperature/humidity sensor
  */
 void setup_sht20() {
     Serial.printf("SETUP: MODBUS: SHT20 #1: address:%d\n", THERMOSTAT_1_ADDR);
     sht20.set_modbus_address(THERMOSTAT_1_ADDR);
     sht20.begin(THERMOSTAT_1_ADDR, _modbus1);
 }
 
 /**
  * Initialize all Modbus clients
  */
 void setup_modbus_clients() {
     //setup_thermostats();  // Future expansion
     //setup_dtm();          // Future expansion
     setup_sht20();          // Initialize SHT20
     //setup_evse();         // Future expansion
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
 }
 
 /**
  * Update data model with current sensor readings
  */
 void update() {
     inputRegisters[0] = sht20.getTemperature();
     inputRegisters[1] = sht20.getHumidity();
 }
 
 /**
  * Main polling loop for Modbus communication
  */
 void loop_modbus_master() {
     if (millis() - lastMillis > POLL_INTERVAL) {
         Serial.println("poll thermostat");
         sht20.poll();        // Get new readings
         update();            // Update data model
         lastMillis = millis();
     }
 }
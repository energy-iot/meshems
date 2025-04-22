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
#define THERMOSTAT_1_ADDR 0x04
#define DDS238_1_ADDR 0x01
#define DDS238_2_ADDR 0x02
#define DDS238_3_ADDR 0x03
 
 // ==================== Serial Interface Setup ====================
 // RS485 serial connections
 SoftwareSerial _modbus1(RS485_RX_1, RS485_TX_1); // HW519 module pinout
 //SoftwareSerial *modbus2(RS485_RX_2, RS485_TX_2); // Client in EMS ModCan
 
 // Temperature/humidity sensor
 Modbus_SHT20 sht20;

 //DDS238
 Modbus_DDS238 dds238_1;
 Modbus_DDS238 dds238_2;
 Modbus_DDS238 dds238_3;
 
 // Timing variables
 unsigned long lastMillis, lastEVSEMillis, lastEVSEChargingMillis = 0;
 
 /**
  * Initialize SHT20 temperature/humidity sensor
  */
 void setup_sht20() {
     Serial.printf("SETUP: MODBUS: SHT20 #1: address:%d\n", THERMOSTAT_1_ADDR);
     //sht20.set_modbus_address(THERMOSTAT_1_ADDR);
     //sht20.begin(THERMOSTAT_1_ADDR, _modbus1);
 }

 void setup_dds238() {
    Serial.printf("SETUP: MODBUS: DDS238 #1: address:%d\n", THERMOSTAT_1_ADDR);
    dds238_1.set_modbus_address(2);
    dds238_2.set_modbus_address(DDS238_2_ADDR);
    dds238_3.set_modbus_address(DDS238_3_ADDR);
    dds238_1.begin(2, _modbus1);
    //dds238_2.begin(DDS238_2_ADDR, _modbus1);
    //dds238_3.begin(DDS238_3_ADDR, _modbus1);
 }
 
 /**
  * Initialize all Modbus clients
  */
 void setup_modbus_clients() {
     //setup_thermostats();  // Future expansion
     //setup_dtm();          // Future expansion
     setup_sht20();          // Initialize SHT20
     //setup_evse();         // Future expansion
     setup_dds238();
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
     Serial.printf("MODBUS METER current: %2.1f\n", dds238_1.getCurrent());
 }
 
 /**
  * Main polling loop for Modbus communication
  */
 void loop_modbus_master() {
     if (millis() - lastMillis > POLL_INTERVAL) {
         Serial.println("poll thermostat");
         //sht20.poll();        // Get new readings
         dds238_1.poll();     // Get new readings
         //dds238_2.poll();     // Get new readings
         //dds238_3.poll();     // Get new readings

         update();            // Update data model
         lastMillis = millis();
     }
 }
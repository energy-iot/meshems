/**
 * @file modbus_master.cpp
 * @brief Modbus master implementation for SHT20 temperature/humidity sensors
 */

 #include <SoftwareSerial.h>
 #include <modbus.h>
 #include <pins.h>
 #include <data_model.h>
 #include <console.h>
 #include <mqtt_client.h>

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
 
 // Poll every 5 seconds for SHT20 sensor
 #define SHT20_POLL_INTERVAL 5000 
 #define EVSE_POLL_INTERVAL 5000 // 5 seconds

// ==================== Modbus Device Addresses ====================
 #define THERMOSTAT_1_ADDR 0x01
 #define EVSE_ADDR 0x06
 
 // ==================== Serial Interface Setup ====================
 // RS485 serial connections
 SoftwareSerial _modbus1(RS485_RX_1, RS485_TX_1); // HW519 module pinout
 //SoftwareSerial _modbus2(RS485_RX_2, RS485_TX_2); // Client in EMS ModCan
 
 // Temperature/humidity sensor
 Modbus_SHT20 sht20;
 
 // Timing variables
 unsigned long lastMillis, lastSHT20Millis, lastEVSEMillis, lastEVSEChargingMillis = 0;
 
 // Since _console is defined in main.cpp, we need to declare it as external here
extern Console _console;

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
     
     // Initialize SHT20 sensor
     setup_sht20();
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

 /**
  * Main polling loop for Modbus communication
  */
 void loop_modbus_master() {
     unsigned long currentMillis = millis();
     
     // Poll SHT20 sensor every 5 seconds
     if (currentMillis - lastSHT20Millis >= SHT20_POLL_INTERVAL) {
         lastSHT20Millis = currentMillis;
         
         // Poll the SHT20 sensor
         uint8_t result = sht20.poll();
         
         if (result == ku8MBSuccess) {
             // Get temperature and humidity values
             float temperature = sht20.getTemperature();
             float humidity = sht20.getHumidity();

             Serial.println(temperature);
             Serial.println(humidity);
             
             // Publish to MQTT (if enabled)
             mqtt_publish_temperature(temperature);
             mqtt_publish_humidity(humidity);
             
             // Add to console display
             char buffer[50];
             snprintf(buffer, sizeof(buffer), "Temp: %.1fC  Humidity: %.1f%%", temperature, humidity);
             _console.addLine(buffer);
         } else {
             Serial.println("Failed to poll SHT20 sensor");
         }
     }
 }

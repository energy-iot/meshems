/**
 * @file modbus_master.cpp
 * @brief Modbus master implementation for SHT20 temperature/humidity sensors
 */

#include <SoftwareSerial.h>
#include <modbus.h>
#include <pins.h>
#include <data_model.h>
#include <math.h>  // For sin function in test data

// Poll every 10 seconds (300000ms = 5 mins for production)
// Changed to 200ms for more frequent updates
// Make the polling interval adjustable and accessible from other files
unsigned int POLL_INTERVAL = 200;  // Initial value of 200ms

 // ==================== Modbus Device Addresses ====================
 #define THERMOSTAT_1_ADDR 0x01
 #define DDS238_1_ADDR 0x01
 #define DDS238_2_ADDR 0x02
 #define DDS238_3_ADDR 0x03
 
 // ==================== Serial Interface Setup ====================
 SoftwareSerial _modbus1(RS485_RX_1, RS485_TX_1); // HW519 module pinout
 //SoftwareSerial *modbus2(RS485_RX_2, RS485_TX_2); // Client in EMS ModCan
 
 // Temperature/humidity sensor
 Modbus_SHT20 sht20;

//DDS238
Modbus_DDS238 dds238_1;

//UNcomment for the 3-meter boxes
//Modbus_DDS238 dds238_2;
//Modbus_DDS238 dds238_3;

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
   dds238_1.set_modbus_address(DDS238_1_ADDR);
   //dds238_2.set_modbus_address(DDS238_2_ADDR);
   //dds238_3.set_modbus_address(DDS238_3_ADDR);
   dds238_1.begin(DDS238_1_ADDR, _modbus1);
   //dds238_2.begin(DDS238_2_ADDR, _modbus1);
   //dds238_3.begin(DDS238_3_ADDR, _modbus1);
}

/**
 * Initialize all Modbus clients
 */
void setup_modbus_clients() {
    //setup_thermostats();  // Future expansion
    //setup_dtm();          // Future expansion
    //setup_sht20();          // Initialize SHT20
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
    
    // Get current and add to history buffer
    float current = dds238_1.getCurrent();
    
    // If current readings are zero or invalid, generate test data
    if (current <= 0.001) {
        // Simulate a sine wave pattern for testing
        current = 2.5 + 2.0 * sin(millis() / 2000.0);
        Serial.printf("MODBUS METER using simulated current: %2.3f\n", current);
    } else {
        Serial.printf("MODBUS METER real current: %2.3f\n", current);
    }
    
    // Add the reading to our history buffer for timeline plotting
    addCurrentReading(current);
    
    // Send CSV formatted data over USB for plotting on computer
    // Format: timestamp,current - using a clear marker DATA for easier parsing
    Serial.printf("DATA,%lu,%.3f\n", millis(), current);
}

/**
 * Main polling loop for Modbus communication
 */
void loop_modbus_master() {
    if (millis() - lastMillis > POLL_INTERVAL) {
        Serial.println("Starting poll cycle...");
        //sht20.poll();        // Get new readings
        dds238_1.poll();     // Get new readings
        //dds238_2.poll();     // Get new readings
        //dds238_3.poll();     // Get new readings

        update();            // Update data model
        lastMillis = millis();
    }
}
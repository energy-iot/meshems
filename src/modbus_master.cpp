/**
 * @file modbus_master.cpp
 * @brief Modbus master implementation for SHT20 temperature/humidity sensors
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
 
 #define POLL_INTERVAL 10000 
 
 #define SOLARK_POLL_INTERVAL 1000  // Poll every 5 seconds

// ==================== Modbus Device Addresses ====================
 #define SOLARK_ADDR 0x01  // Adjust this to match your SolArk device address
 
 // ==================== Serial Interface Setup ====================
 // RS485 serial connections
 SoftwareSerial _modbus1(RS485_RX_1, RS485_TX_1); // HW519 module pinout
 //SoftwareSerial *modbus2(RS485_RX_2, RS485_TX_2); // Client in EMS ModCan

 //the Sol-Ark Low Voltage Inverter
Modbus_SolArkLV solark;
 
 // Timing variables
 unsigned long lastMillis, lastSolArkMillis = 0;
 
 // Since _console is defined in main.cpp, we need to declare it as external here
//extern Console _console;

// Add this function to initialize the SolArk device
void setup_solark() {
  Serial.printf("SETUP: MODBUS: SolArk #1: address:%d\n", SOLARK_ADDR);
  solark.set_modbus_address(SOLARK_ADDR);
  solark.begin(SOLARK_ADDR, _modbus1);
}
 
 /**
  * Initialize all Modbus clients
  */
 void setup_modbus_clients() {
     setup_solark();           // Initialize Sol-Ark LV device
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

 void printBatteryStatus() {
    Serial.println("BATTERY STATUS:");
    Serial.printf("  Power:       %.1f W\n", solark.getBatteryPower());
    Serial.printf("  Current:     %.2f A\n", solark.getBatteryCurrent());
    Serial.printf("  Voltage:     %.2f V\n", solark.getBatteryVoltage());
    Serial.printf("  SOC:         %.0f%%\n", solark.getBatterySOC());
    Serial.printf("  Temperature: %.1f°C (%.1f°F)\n", 
                  solark.getBatteryTemperature(), 
                  solark.getBatteryTemperatureF());
    
    // Show charging/discharging status
    Serial.print("  Status:      ");
    if (solark.isBatteryCharging()) {
      Serial.println("CHARGING");
    } else if (solark.isBatteryDischarging()) {
      Serial.println("DISCHARGING");
    } else {
      Serial.println("IDLE");
    }
  }
  
  void printGridStatus() {
    Serial.println("GRID STATUS:");
    Serial.printf("  Power:       %.1f W\n", solark.getGridPower());
    Serial.printf("  Voltage:     %.1f V\n", solark.getGridVoltage());
    Serial.printf("  Current L1:  %.2f A\n", solark.getGridCurrentL1());
    Serial.printf("  Current L2:  %.2f A\n", solark.getGridCurrentL2());
    Serial.printf("  Grid CT Current L1:  %.2f A\n", solark.getGridCurrentL1());
    Serial.printf("  Grid CT Current L2:  %.2f A\n", solark.getGridCurrentL2());
    Serial.printf("  Frequency:   %.2f Hz\n", solark.getGridFrequency());
    
    // Show grid connection status
    Serial.print("  Connection:  ");
    if (solark.isGridConnected()) {
      Serial.println("CONNECTED");
      
      // Show buying/selling status
      Serial.print("  Flow:        ");
      if (solark.isSellingToGrid()) {
        Serial.println("SELLING TO GRID");
      } else if (solark.isBuyingFromGrid()) {
        Serial.println("BUYING FROM GRID");
      } else {
        Serial.println("NO POWER FLOW");
      }
    } else {
      Serial.println("DISCONNECTED");
    }
  }
  
  void printPVStatus() {
    Serial.println("SOLAR PV STATUS:");
    Serial.printf("  PV1 Power:   %.1f W\n", solark.getPV1Power());
    Serial.printf("  PV2 Power:   %.1f W\n", solark.getPV2Power());
    Serial.printf("  Total Power: %.1f W\n", solark.getPV1Power() + solark.getPV2Power());
    Serial.printf("  Total Power: %.3f kW\n", solark.getPVPowerTotal());
  }
  
  void printLoadStatus() {
    Serial.println("LOAD STATUS:");
    Serial.printf("  Load L1:     %.1f W\n", solark.getLoadPowerL1());
    Serial.printf("  Load L2:     %.1f W\n", solark.getLoadPowerL2());
    Serial.printf("  Total Load:  %.1f W\n", solark.getLoadPowerTotal());
    Serial.printf("  Smart Load:  %.1f W\n", solark.getSmartLoadPower());
    Serial.printf("  Frequency:   %.2f Hz\n", solark.getLoadFrequency());
  }
  
  void printEnergyMeters() {
    Serial.println("ENERGY METERS (kWh):");
    Serial.printf("  Battery Charge:    %.1f kWh\n", solark.getBatteryChargeEnergy());
    Serial.printf("  Battery Discharge: %.1f kWh\n", solark.getBatteryDischargeEnergy());
    Serial.printf("  Grid Buy:          %.1f kWh\n", solark.getGridBuyEnergy());
    Serial.printf("  Grid Sell:         %.1f kWh\n", solark.getGridSellEnergy());
    Serial.printf("  Load:              %.1f kWh\n", solark.getLoadEnergy());
    Serial.printf("  PV Generation:     %.1f kWh\n", solark.getPVEnergy());
  }

 void loop_solark() {
    if (millis() - lastSolArkMillis > SOLARK_POLL_INTERVAL) {
        Serial.println("Poll SolArk inverter");
        uint8_t result = solark.poll();
        if (result == 0) { // 0 = ku8MBSuccess
            // Display the decoded values
            printBatteryStatus();
            printGridStatus();
            printPVStatus();
            printLoadStatus();
            printEnergyMeters();
          } else {
            Serial.println("Error polling SolArk inverter");
          }
          
        Serial.println("-------------------------------------");
        lastSolArkMillis = millis();
    }
}

 /**
  * Main polling loop for Modbus communication
  */
 void loop_modbus_master() {
     // Poll Sol-Ark at its own interval
     loop_solark();
 }
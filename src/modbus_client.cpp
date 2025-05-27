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
  
 #define SOLARK_POLL_INTERVAL 1000  // Poll every 1 seconds

// ==================== Modbus Device Setup ====================
 #define SOLARK_ADDR 0x01  // Adjust this to match your SolArk device address
 #define DEVICE_BAUD_RATE 9600 // Default, Sol-Ark only supports 9600 baud
 
 // ==================== Serial Interface Setup ====================
 // RS485 serial connections
 SoftwareSerial _modbus1(RS485_RX_1, RS485_TX_1); // Client - HW519 module
 
 // Not Used for the SunSpec TCP/IP Modbus Bridge, could be used for SunSpec Modbus RTU
 //SoftwareSerial *modbus2(RS485_RX_2, RS485_TX_2); // Server - HW519 module

 //the Sol-Ark Low Voltage Inverter
Modbus_SolArkLV solark;
 
 // Timing variables
 unsigned long lastMillis, lastSolArkMillis = 0;

// Add this function to initialize the SolArk device
void setup_solark() {
  Serial.printf("SETUP: MODBUS: SolArk #1: address:%d\n", SOLARK_ADDR);
  solark.begin(SOLARK_ADDR, _modbus1); // Initialize a new instance of the Sol-Ark class on the CLIENT RS485 interface
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
 void setup_modbus_client_interface() {
     // Reset GPIO pins for RS485
     gpio_reset_pin(RS485_RX_1);
     gpio_reset_pin(RS485_TX_1);
     gpio_reset_pin(RS485_RX_2);
     gpio_reset_pin(RS485_TX_2);
 
     _modbus1.begin(DEVICE_BAUD_RATE);    // Initialize serial at 9600 baud
     
     setup_modbus_clients();              // Setup connected devices

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
   Serial.printf("  Capacity:    %.1f Ah\n", solark.getBatteryCapacity());
   Serial.printf("  BMS SOC:     %.1f%%\n", solark.getBMSRealTimeSOC());
   Serial.printf("  BMS Warning: 0x%04X\n", solark.getBMSWarning());
   Serial.printf("  BMS Fault:   0x%04X\n", solark.getBMSFault());
   
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

  void printInverterDetails() {
    Serial.println("INVERTER DETAILS:");
    Serial.printf("  Comm Version: %u\n", solark.getCommVersion());
    
    char serial_str[33]; // Max 32 chars for serial + null terminator
    char* p_serial = serial_str;
    for (int i = 0; i < 5; ++i) {
        uint16_t sn_part = solark.getSerialNumberPart(i);
        if (sn_part == 0) break;
        char char1 = (sn_part >> 8) & 0xFF;
        char char2 = sn_part & 0xFF;
        if (char1 != 0 && p_serial < serial_str + sizeof(serial_str) -1) *p_serial++ = char1; else if (char1 == 0) break;
        if (char2 != 0 && p_serial < serial_str + sizeof(serial_str) -1) *p_serial++ = char2; else if (char2 == 0) break;
    }
    *p_serial = '\0';
    Serial.printf("  Serial No:   %s\n", serial_str);

    Serial.printf("  Grid Type:   %u (0:Single, 1:Split, 2:Three-Phase Wye)\n", solark.getGridType());
    Serial.printf("  Inv Status:  %u (1:Self-test, 2:Normal, 3:Alarm, 4:Fault)\n", solark.getInverterStatus());
    Serial.printf("  DCDC Temp:   %.1f°C\n", solark.getDCDCTemp());
    Serial.printf("  IGBT Temp:   %.1f°C\n", solark.getIGBTTemp());
  }

 void loop_solark() {
    if (millis() - lastSolArkMillis > SOLARK_POLL_INTERVAL) {
        Serial.println("Poll SolArk inverter");
        uint8_t result = solark.poll();
        if (result == 0) { // 0 = ku8MBSuccess
            // Display the decoded values
            printInverterDetails();
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
 void loop_modbus_client() {
     // Poll Sol-Ark at its own interval
     loop_solark();
 }
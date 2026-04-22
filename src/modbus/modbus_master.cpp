/**
 * @file modbus_master.cpp
 * @brief Modbus master implementation for SHT20 temperature/humidity sensors
 */

#include <SoftwareSerial.h>
#include <modbus/modbus.h>
#include <pins.h>
#include <data_model.h>
#include <config.h>
#include <math.h>  // For sin function in test data

// Poll every 10 seconds (300000ms = 5 mins for production)
// Changed to 500ms for more stable operation
// Make the polling interval adjustable and accessible from other files
//unsigned int POLL_INTERVAL = 100;  // moved to config.h 

// ==================== Modbus Device Addresses ====================
// during staging of subpanel must stage each modbus meter with its assigned node number - in future could use qr code sticker per meter for faster staging of a subpanel
//#define THERMOSTAT_1_ADDR 0x01    // when no meters shared on same rs485 modbus RTU line
#define THERMOSTAT_1_ADDR 0x99  // at staging of subpanel set temp/humid sensor as modbus node num 99 so never conflicts

// 3 meter subpanel , either 1 tenant meter per phase or all 3 meters on same phase 
#define DDS238_1_ADDR 0x01
#define DDS238_2_ADDR 0x02
#define DDS238_3_ADDR 0x03
// uncomment for 6 meter subpanel , either n tenant meters per phase, or all  meters on same phase 
//#define DDS238_1_ADDR 0x04
//#define DDS238_2_ADDR 0x05
//#define DDS238_3_ADDR 0x06
// uncomment for 9 meter subpanel , either n tenant meters per phase, or all  meters on same phase 
//#define DDS238_1_ADDR 0x07
//#define DDS238_2_ADDR 0x08
//#define DDS238_3_ADDR 0x09
// NOTE 3/6/9 multitenant subpanels can be cookie cutter options - 3 phase so x tenants/meters per phase typical, assume subanels in multiples of 3


// ==================== Serial Interface Setup ====================
SoftwareSerial _modbus1(RS485_RX_A, RS485_TX_A); // RS485 modbus HW519 module pinout - all meters on a rs485 daisy chain (and door thermostat  and tamper door alarm)
//SoftwareSerial *modbus2(RS485_RX_B, RS485_TX_B); // uncomment when 2 modbus masters are to be used on the EMS PCB
// TODO merge Canbus support and Modbus Client in other EMS branch to here ModCan

// Temperature/humidity sensor
Modbus_SHT20 sht20;

// NESL DTM meter (registers 1000-1068, address 1) — diagnostic register scan
Modbus_NESLDTM nesldtm_1;

// Energy Meter (HIKING DDS238) — commented out, replaced by NESL DTM
//Modbus_DDS238 dds238_1;
//Modbus_DDS238 dds238_2;
//Modbus_DDS238 dds238_3;
//Modbus_DDS238 dds238_4;
//Modbus_DDS238 dds238_5;
//Modbus_DDS238 dds238_6;
//Modbus_DDS238 dds238_7;
//Modbus_DDS238 dds238_8;
//Modbus_DDS238 dds238_9;

//Modbus_DDS238* dds238_meters[MODBUS_NUM_METERS] = {&dds238_1, &dds238_2, &dds238_3};
//ModbusMaster* meters[MODBUS_NUM_METERS] = {&dds238_1, &dds238_2, &dds238_3};
// Timing variables
unsigned long lastEVSEMillis, lastEVSEChargingMillis = 0;

/**
 * Initialize SHT20 temperature/humidity sensor
 */
void setup_sht20() {
    Serial.printf("SETUP: MODBUS: SHT20 #1: address:%d\n", THERMOSTAT_1_ADDR);
    sht20.set_modbus_address(THERMOSTAT_1_ADDR);
    sht20.begin(THERMOSTAT_1_ADDR, _modbus1);    // node number 99 so doesnt conflict with meters at 1-n
}

void setup_nesldtm() {
    Serial.printf("SETUP: MODBUS: NESLDTM #1: address:%d regs:%d-%d\n",
                  DDS238_1_ADDR, NESLDTM_REG_START, NESLDTM_REG_END);
    nesldtm_1.set_modbus_address(DDS238_1_ADDR);
    nesldtm_1.begin(DDS238_1_ADDR, _modbus1);
}

/*
void setup_dds238() {
   Serial.printf("SETUP: MODBUS: DDS238 #1: address:%d\n", DDS238_1_ADDR);
   dds238_1.set_modbus_address(DDS238_1_ADDR);
   dds238_2.set_modbus_address(DDS238_2_ADDR);
   dds238_3.set_modbus_address(DDS238_3_ADDR);
   dds238_1.begin(DDS238_1_ADDR, _modbus1);
   dds238_2.begin(DDS238_2_ADDR, _modbus1);
   dds238_3.begin(DDS238_3_ADDR, _modbus1);
}
*/

/**
 * Initialize all Modbus clients
 */
void setup_modbus_clients() {
    //setup_thermostats();  // Future expansion for modbus building thermostat
    //setup_dtm();          // Future expansion Building mains
    //TODO support thermostat and mete on same modbus master daisy chain
    setup_sht20();          // Initialize SHT20 temp/humid modbus sensor
    //setup_evse();         // Future expansion multitenant EV charging
    setup_nesldtm();        // Initialize NESL DTM meter register scan (regs 1000-1068)
    //setup_dds238();       // commented out: replaced by NESL DTM scan
}

/**
 * Initialize Modbus master interface
 */
void setup_modbus_master() {
    // Reset GPIO pins for 2 ports of  RS485
    gpio_reset_pin(RS485_RX_A);
    gpio_reset_pin(RS485_TX_A);
    gpio_reset_pin(RS485_RX_B);
    gpio_reset_pin(RS485_TX_B);

    // Initialize serial at 9600 baud for now, good for less than 20 modbus nodes of scanning
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

    // DDS238 meter update — commented out while using NESL DTM scan
    /*
    for(int i=0;i<MODBUS_NUM_METERS;i++) {
        readings[i].current = dds238_meters[i]->getCurrent();
        readings[i].voltage = dds238_meters[i]->getVoltage();
        readings[i].active_power = dds238_meters[i]->getActivePower();
        readings[i].power_factor = dds238_meters[i]->getPowerFactor();
        readings[i].frequency = dds238_meters[i]->getFrequency();
        readings[i].total_energy = dds238_meters[i]->getTotalEnergy();
        readings[i].export_energy = dds238_meters[i]->getExportEnergy();
        readings[i].import_energy = dds238_meters[i]->getImportEnergy();
    }
    addCurrentReading(readings[0].current);
    Serial.printf("DATA,%lu,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                 millis(), readings[0].current, readings[0].voltage, readings[0].active_power, readings[0].power_factor, readings[0].frequency);
    */
}

void poll_energy_meters() {
    // Poll NESL DTM meter — dumps registers 1000-1068 to serial
    nesldtm_1.poll();
    // update(); // skipped: no data model mapping yet for NESL DTM
}

void poll_thermostats() {
    // Poll cabinet temp/humid sensor
    for(int i = 0; i < MODBUS_NUM_THERMOSTATS; i++) {
        //sht20_thermostats[i]->poll();  // TODO future option for multiple temp sensing in cabinet and nearby 
    }
    // Update data model cache with latest readings
    update();
}
//sht20.poll();        // Get new readings

/**
 * Main polling loop for Modbus communication
 */
void loop_modbus_master() {
    Serial.println("Starting poll cycle...");
    poll_thermostats();   // cabinet temp/humid
    poll_energy_meters(); // NESL DTM register scan
}
#include <SoftwareSerial.h>
#include <modbus.h>
#include <pins.h>
#include <data_model.h>

#define POLL_INTERVAL 10000 //300000 //5 mins
//MODBUS slave devices
#define THERMOSTAT_1_ADDR 0x01

SoftwareSerial _modbus1(RS485_RX_1, RS485_TX_1); //(rx, tx) corresponds with HW519 rxd txd pins
//mb2 is a client in EMS ModCan 
//SoftwareSerial _modbus2(RS485_RX_2, RS485_TX_2); //(rx, tx) corresponds with HW519 rxd txd pins

//the temp/humidity sensor (SHT20)
Modbus_SHT20 sht20;

unsigned long lastMillis, lastEVSEMillis, lastEVSEChargingMillis = 0;

void setup_sht20() {
  Serial.printf("SETUP: MODBUS: SHT20 #1: address:%d\n",THERMOSTAT_1_ADDR); 
  sht20.set_modbus_address(THERMOSTAT_1_ADDR);
  sht20.begin(THERMOSTAT_1_ADDR, _modbus1);
}


void setup_modbus_clients() {
  //setup_thermostats();
  //setup_dtm();
  setup_sht20();
  //setup_evse();
}

void setup_modbus_master() {
 
  gpio_reset_pin(RS485_RX_1);
  gpio_reset_pin(RS485_TX_1);
  gpio_reset_pin(RS485_RX_2);
  gpio_reset_pin(RS485_TX_2);

  _modbus1.begin(9600);

  setup_modbus_clients();

  //sht20.poll();
}

void update() {
  //harvest temp/humidity from sht20
  inputRegisters[1] = sht20.getRawTemperature();
  inputRegisters[2] = sht20.getRawHumidity();
}

void loop_modbus_master() {
  if (millis() - lastMillis > POLL_INTERVAL) {
    Serial.println("poll thermostat");
    sht20.poll();
    update();
    lastMillis = millis();
  }
}
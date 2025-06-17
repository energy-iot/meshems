#include <config.h>
#include <Arduino.h>

char device_id[MAX_DEVICE_ID_CHARS] = {0};

// this data can be downloaded from a linux java policy server aggregation node for every m streetpoleEMS nodes
int ModbusMaster_rate = 1000;
int MQTTPublish_rootrate = 30000;


void generatefullDeviceID() {// includes OUI vendorid of the ethernet MAC inside the ESP32S2 -prefer to not use  full MACid
  uint32_t low = ESP.getEfuseMac() & 0xFFFFFFFF;
  uint32_t high = (ESP.getEfuseMac() >> 32) & 0xFFFFFFFF;
  //uint64_t fullMAC = word(low, high);
  sprintf(device_id, "%s%X%X", DEVICE_ID_PREFIX, high, low);
}

void generateDeviceID() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  //Serial.printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  sprintf(device_id, "%s%02X%02X%02X", DEVICE_ID_PREFIX, mac[3], mac[4], mac[5]);
}

const char* getDeviceID() {
  return (const char*)device_id;
}
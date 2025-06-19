#include <config.h>
#include <Arduino.h>

char device_id[MAX_DEVICE_ID_CHARS] = {0};

// this data can be downloaded from a linux java policy server aggregation node for every m streetpoleEMS nodes
int ModbusMaster_pollrate = 100;  // in milliseconds
int MQTTPublish_rootrate = 300000; // in milliseconds
// TODO add different appropriate mqtt publish rates for environmental vs stats vs RCMleaks and harmonics vs real time billing energy usage 
// keep the BPS donw below 300kbps , ideally 100 kbps per multitenant subpanel - reduce string char lengths convert to binary compression etc
// this leaves room for villager-villager low bit rate side channel on the packet control plane shared with energy : PTT store-and-forward clear-voice/txt/imaging 


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
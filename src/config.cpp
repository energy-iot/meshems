#include <config.h>
#include <Arduino.h>
// moved to 
//String mqtt_server("test.mosquitto.org"); //("public.cloud.shiftr.io"); //note: shiftr requires user:public pw:public
//String mqtt_user;                         //("public");
//String mqtt_pass;                         //("public");

char device_id[MAX_DEVICE_ID_CHARS] = {0};

void generatefullDeviceID() {// includes OUI vendorid of the ethernet MAC inside the ESP32S2 -prefer to not use  full MACid
  uint32_t low = ESP.getEfuseMac() & 0xFFFFFFFF;
  uint32_t high = (ESP.getEfuseMac() >> 32) % 0xFFFFFFFF;
  //uint64_t fullMAC = word(low, high);
  sprintf(device_id, "%s%X%X", DEVICE_ID_PREFIX, high, low);
}

void generateDeviceID() {
  uint32_t uniquePart = (ESP.getEfuseMac() >> 32) % 0xFFFFFFFF;  // Keep only the lower 4 bytes
  sprintf(device_id, "%s%X", DEVICE_ID_PREFIX, uniquePart);
}

const char* getDeviceID() {
  return (const char*)device_id;
}
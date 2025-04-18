#include <config.h>
#include <Arduino.h>

String mqtt_server("test.mosquitto.org"); //("public.cloud.shiftr.io"); //note: shiftr requires user:public pw:public
String mqtt_user;                         //("public");
String mqtt_pass;                         //("public");

char device_id[MAX_DEVICE_ID_CHARS] = {0};

void generateDeviceID() {
  uint32_t low = ESP.getEfuseMac() & 0xFFFFFFFF;
  uint32_t high = (ESP.getEfuseMac() >> 32) % 0xFFFFFFFF;
  //uint64_t fullMAC = word(low, high);
  sprintf(device_id, "%s%X%X", DEVICE_ID_PREFIX, high, low);
}

const char* getDeviceID() {
  return (const char*)device_id;
}
#pragma once
#include <Arduino.h>

#define MAX_DEVICE_ID_CHARS   32
#define DEVICE_ID_PREFIX      "NESLEMS_"
#define DEVICE_ID_PREFIX      "StreetPoleEMS_"

#define MQTT_TOPIC              "nesl"
#define MQTT_PUBLISH_INTERVAL   60000
#define MQTT_SERVER             "public.cloud.shiftr.io" //"test.mosquitto.org"
#define MQTT_TOPIC              "openami" // "openami/StreetPoleEMS_<EMSid>"
//#define MQTT_TOPIC              "nesl"
#define MQTT_PUBLISH_INTERVAL   30000
#define MQTT_SERVER             "public.cloud.shiftr.io" 
//#define MQTT_SERVER             "test.mosquitto.org"
#define MQTT_USER               "public"
#define MQTT_PW                 "public"

void generateDeviceID();
const char* getDeviceID();
#pragma once
#include <Arduino.h>

#define MAX_DEVICE_ID_CHARS   32
#define DEVICE_ID_PREFIX      "NESLEMS_"

#define MQTT_TOPIC              "nesl"
#define MQTT_PUBLISH_INTERVAL   60000
#define MQTT_SERVER             "public.cloud.shiftr.io" //"test.mosquitto.org"
#define MQTT_USER               "public"
#define MQTT_PW                 "public"

void generateDeviceID();
const char* getDeviceID();
#pragma once
#include <Arduino.h>

#define MAX_DEVICE_ID_CHARS   32
#define DEVICE_ID_PREFIX      "NESL"

#define MQTT_TOPIC              "nesl"
#define MQTT_PUBLISH_INTERVAL   30000
#define MQTT_SERVER             "test.mosquitto.org"
#define MQTT_USER               ""
#define MQTT_PW                 ""

void generateDeviceID();
const char* getDeviceID();
#pragma once
#include <Arduino.h>

#define MAX_DEVICE_ID_CHARS   32
#define DEVICE_ID_PREFIX      "StreetPoleEMS_"

#define MQTT_TOPIC              "openami" // "openami/StreetPoleEMS_<EMSid>"
//#define MQTT_TOPIC              "nesl"
#define MQTT_PUBLISH_INTERVAL   30000
#define MQTT_SERVER             "public.cloud.shiftr.io" 
//#define MQTT_SERVER             "test.mosquitto.org"
#define MQTT_USER               "public"
#define MQTT_PW                 "public"

// DTM485 custom Ascii on serial on rsa485 -  Serial Pins and Baud Rate
#define DTM485_SERIAL Serial2
#define DTM485_DE_RE_PIN 23
#define DTM485_BAUDRATE 9600

// Poll and Publish Intervals
#define DTM485POLL_INTERVAL_MS 5000        // 5 seconds
#define DTM_MQTT_PUBLISH_INTERVAL_MS 300000   // 5 minutes

void generateDeviceID();
const char* getDeviceID();
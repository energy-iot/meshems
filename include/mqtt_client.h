#pragma once

// Include environment configuration
#include "env_config.h"

// Default values if not defined in env_config.h
#ifndef ENABLE_MQTT
#define ENABLE_MQTT 0  // Default to disabled if not defined in .env
#endif

#include <Arduino.h>
#if ENABLE_MQTT
#include <WiFi.h>
#include <PubSubClient.h>
#endif

// MQTT Configuration - default values if not defined in env_config.h
#ifndef MQTT_SERVER
#define MQTT_SERVER "localhost"
#endif

#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif

#ifndef MQTT_CLIENT_ID
#define MQTT_CLIENT_ID "EMS_DEV"
#endif

#ifndef MQTT_USERNAME
#define MQTT_USERNAME ""
#endif

#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD ""
#endif

// MQTT Topics - default values if not defined in env_config.h
#ifndef MQTT_TOPIC_TEMPERATURE
#define MQTT_TOPIC_TEMPERATURE "ems/sensor/sht20/temperature"
#endif

#ifndef MQTT_TOPIC_HUMIDITY
#define MQTT_TOPIC_HUMIDITY "ems/sensor/sht20/humidity"
#endif

// WiFi Configuration - default values if not defined in env_config.h
#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

// Function declarations
void setup_mqtt();
void loop_mqtt();
bool mqtt_publish_temperature(float temperature);
bool mqtt_publish_humidity(float humidity);
void mqtt_reconnect();

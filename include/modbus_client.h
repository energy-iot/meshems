#pragma once
#include <ModbusIP_ESP8266.h>

// WiFi credentials - these should be defined in a separate header file in practice
// #define WIFI_SSID "Eris"
// #define WIFI_PASSWORD "LimaOscarAlpha28!"

#define WIFI_SSID "Port Labs Members"
#define WIFI_PASSWORD "lesstalkmorewifi"

// #define WIFI_SSID "SSID"
// #define WIFI_PASSWORD "PASSWORD"

void setup_modbus_client();
void loop_modbus_client();

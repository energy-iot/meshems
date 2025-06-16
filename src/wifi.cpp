#include <Arduino.h>
#include <WiFiMulti.h>
#include <wifi.h>

//#define WIFI_PW "E@rthday2025"
//#define WIFI_SSID "EMSLab"
//#define WIFI_SSID "EMSWorkshop"
#define WIFI_SSID "energy-iot-2.4"
#define WIFI_PW "Pride1@Azure"
int CONNECT_ATTEMPTS = 6;
WiFiMulti wifiMulti;

bool wifi_client_connected() {
  return WiFi.isConnected() && (WIFI_STA == (WiFi.getMode() & WIFI_STA));
}

bool setup_wifi() {
  Serial.printf("wifi connecting: %s\n", WIFI_SSID);
  wifiMulti.addAP(WIFI_SSID, WIFI_PW);
  while (wifiMulti.run() != WL_CONNECTED && (CONNECT_ATTEMPTS-- > 0)) {
    delay(1000);
    Serial.printf("wifi failed to connect - retrying %s\n", WIFI_SSID);
  }
  Serial.printf("wifi: %s: %s\n", WIFI_SSID, wifi_client_connected() ?  WiFi.localIP().toString().c_str() : "FAILED");
  return wifi_client_connected();
}
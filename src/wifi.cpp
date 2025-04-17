#include <Arduino.h>
#include <WiFiMulti.h>
#include <wifi.h>

#define WIFI_SSID "Starlink"
#define WIFI_PW "tikka2013"
int CONNECT_ATTEMPTS = 10;

String SSID;
String PW;

WiFiMulti wifiMulti;

bool wifi_client_connected() {
  return WiFi.isConnected() && (WIFI_STA == (WiFi.getMode() & WIFI_STA));
}

bool setup_wifi() {
  return setup_wifi(SSID, PW);
}

bool setup_wifi(String SSID, String PW) {
  if (SSID.isEmpty()) {
    SSID = WIFI_SSID;
  }
  if (PW.isEmpty()) {
    PW = WIFI_PW;
  }
  Serial.println("wifi connecting...");
  Serial.println(SSID);
  wifiMulti.addAP(SSID.c_str(), PW.c_str());
  while (wifiMulti.run() != WL_CONNECTED && (CONNECT_ATTEMPTS-- > 0)) {
    delay(1000);
    Serial.printf("wifi failed to connect - retrying %s\n", SSID);
  }
  if (wifi_client_connected()) {
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.printf("wifi failed to connect to %s\n", SSID.c_str());
    return false;
  }
  return true;
}
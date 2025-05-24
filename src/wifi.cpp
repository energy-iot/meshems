/**
 * @file wifi.cpp
 * @brief WiFi connection management implementation
 * 
 * Handles WiFi connectivity for the EMS device including connection setup,
 * status monitoring, and connection attempts with retry logic.
 */

#include <Arduino.h>
#include <WiFiMulti.h>
#include <wifi.h>

// WiFi Configuration
#define WIFI_SSID "Port Labs Members"
#define WIFI_PW "lesstalkmorewifi"

// Connection Parameters
#define MAX_CONNECT_ATTEMPTS 6
#define CONNECT_RETRY_DELAY_MS 1000

// Global WiFi manager instance
WiFiMulti wifiMulti;

/**
 * @brief Check if WiFi client is currently connected and in station mode
 * 
 * Verifies both that WiFi is connected and that the device is operating
 * in station (client) mode rather than access point mode.
 * 
 * @return true if WiFi is connected and in station mode, false otherwise
 */
bool wifi_client_connected() {
    return WiFi.isConnected() && (WIFI_STA == (WiFi.getMode() & WIFI_STA));
}

/**
 * @brief Initialize and establish WiFi connection with retry logic
 * 
 * Attempts to connect to the configured WiFi network with a specified
 * number of retry attempts. Provides status feedback via Serial output.
 * 
 * @return true if connection successful, false if failed after all attempts
 */
bool setup_wifi() {
    int connect_attempts = MAX_CONNECT_ATTEMPTS;
    
    Serial.printf("INFO - WiFi: Attempting to connect to '%s'\n", WIFI_SSID);
    
    // Add the access point to the WiFi manager
    wifiMulti.addAP(WIFI_SSID, WIFI_PW);
    
    // Attempt connection with retry logic
    while (wifiMulti.run() != WL_CONNECTED && (connect_attempts-- > 0)) {
        delay(CONNECT_RETRY_DELAY_MS);
        Serial.printf("ERROR - WiFi: Connection failed, retrying... (%d attempts remaining)\n", connect_attempts);
    }
    
    // Report final connection status
    if (wifi_client_connected()) {
        Serial.printf("INFO - WiFi: Successfully connected to '%s' - IP: %s\n", 
                     WIFI_SSID, WiFi.localIP().toString().c_str());
    } else {
        Serial.printf("ERROR - WiFi: Failed to connect to '%s' after %d attempts\n", 
                     WIFI_SSID, MAX_CONNECT_ATTEMPTS);
    }
    
    return wifi_client_connected();
}

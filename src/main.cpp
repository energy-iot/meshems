/**
 * @file main.cpp
 * @brief Main application entry point for the Energy IoT Source firmware
 * @author Doug Mendonca, Liam O'Brien
 * @date April 18, 2025
 * 
 * This file contains the setup and main loop for the Energy IoT EMS Dev Platform 
 * controlling the OLED display, MODBUS interfaces, CAN bus interface, 
 * and button interfaces.
 * 
 *  _____                             _____ _____ _____   _____                  _____                          
 * |  ___|                           |_   _|  _  |_   _| |  _  |                /  ___|                         
 * | |__ _ __   ___ _ __ __ _ _   _    | | | | | | | |   | | | |_ __   ___ _ __ \ `--.  ___  _   _ _ __ ___ ___ 
 * |  __| '_ \ / _ \ '__/ _` | | | |   | | | | | | | |   | | | | '_ \ / _ \ '_ \ `--. \/ _ \| | | | '__/ __/ _ \
 * | |__| | | |  __/ | | (_| | |_| |  _| |_\ \_/ / | |   \ \_/ / |_) |  __/ | | /\__/ / (_) | |_| | | | (_|  __/
 * \____/_| |_|\___|_|  \__, |\__, |  \___/ \___/  \_/    \___/| .__/ \___|_| |_\____/ \___/ \__,_|_|  \___\___|
 *                       __/ | __/ |                           | |                                              
 *                      |___/ |___/                            |_|                                              
 *
 * Copyright 2025
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Arduino.h>
#include <WiFi.h>             // WiFi functionality
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <modbus.h>           // Modbus communication protocols
#include <buttons.h>          // Button input handling
#include <display.h>          // SH1106 OLED display
#include <console.h>          // Console UI for the display
#include <SPI.h>              // SPI communication for display/CAN
#include <can.h>              // Implementation of CAN bus communication
#include <wifi.h>
#include <mqtt_client.h>
#include <config.h>
#include <data_model.h>       // Access to current history data

// External declaration for the POLL_INTERVAL variable from modbus_master.cpp
extern unsigned int POLL_INTERVAL;

// Web server running on port 80
AsyncWebServer server(80);

// Setup web server
void setup_webserver() {
    // Initialize SPIFFS
    if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", String(), false);
    });
    
    // Route for JSON data
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{\"current\":";
        
        // Get the most recent current value
        float currentVal = 0.0;
        if (currentHistory.count > 0) {
            int lastIdx = (currentHistory.currentIndex - 1 + CURRENT_HISTORY_SIZE) % CURRENT_HISTORY_SIZE;
            currentVal = currentHistory.values[lastIdx];
        }
        json += String(currentVal, 2);
        
        // Add history data
        json += ",\"history\":[";
        
        if (currentHistory.count > 0) {
            // Start from the oldest value in the circular buffer
            int startIdx = 0;
            if (currentHistory.count >= CURRENT_HISTORY_SIZE) {
                startIdx = currentHistory.currentIndex % CURRENT_HISTORY_SIZE;
            }
            
            // Add each value
            for (int i = 0; i < currentHistory.count; i++) {
                int idx = (startIdx + i) % CURRENT_HISTORY_SIZE;
                json += String(currentHistory.values[idx], 2);
                if (i < currentHistory.count - 1) {
                    json += ",";
                }
            }
        }
        
        json += "],\"interval\":" + String(POLL_INTERVAL);
        json += "}";
        
        request->send(200, "application/json", json);
    });
    
    // Route for adjusting polling interval
    server.on("/interval", HTTP_GET, [](AsyncWebServerRequest *request){
        String message;
        
        if (request->hasParam("ms")) {
            String intervalStr = request->getParam("ms")->value();
            int newInterval = intervalStr.toInt();
            
            // Validate the new interval
            if (newInterval >= 50 && newInterval <= 10000) {
                // Change the poll interval
                POLL_INTERVAL = newInterval;
                
                message = "Polling interval set to " + String(POLL_INTERVAL) + "ms";
                _console.addLine(("Poll int: " + String(POLL_INTERVAL) + "ms").c_str());
            } else {
                message = "Invalid interval. Must be between 50ms and 10000ms.";
                request->send(400, "text/plain", message);
                return;
            }
        } else {
            message = "Missing 'ms' parameter";
            request->send(400, "text/plain", message);
            return;
        }
        
        request->send(200, "text/plain", message);
    });

    // Start server
    server.begin();
    Serial.println("Web server started");
    _console.addLine("Web server started");
}

void setup() {
    Serial.begin(115200);   // Initialize serial communication for debugging
    Serial.println("INFO - Booting...Setup in 3s");
    delay(3000);
    
    SPI.begin();

    generateDeviceID();
    setup_display();
    // Display startup splash screen (Rick image)
    drawBitmap(40, 5, RICK_WIDTH, RICK_HEIGHT, rick);
    delay(1000);
    
    drawBitmap(0, 0, LOGO_WIDTH, LOGO_HEIGHT, eIOT_logo); // Render Logo
    delay(1000);

    setup_wifi();
    setup_mqtt_client();
    
    // Initialize Modbus RTU master/client communication
    setup_modbus_master(); // This sets up communication with sensors like the SHT20 temp/humidity sensor or other devices
    setup_modbus_client();
    setup_can(); // Initialize CAN bus communication

    setup_buttons();
    _console.addLine(" EMS In-service Ready!");
    _console.addLine("  CHECK MQTT @");
    _console.addLine("  public.cloud.shiftr.io"); //TODO grab the setup strings from the config file
    _console.addLine("  filter OPENAMI/#");       //TODO grab the setup strings from the config file
    
    // Display IP address for the web interface
    if (wifi_client_connected()) {
        String ipAddress = "Web UI: http://" + get_wifi_ip();
        _console.addLine(ipAddress.c_str());
        Serial.println("Web UI accessible at: " + ipAddress);
    } else {
        _console.addLine("  WiFi not connected");
        Serial.println("WiFi not connected - web interface unavailable");
    }
    
    _console.addLine("  Push a button?");

    // Initialize web server for displaying data on laptop
    // Comment out web server for now to focus on USB plotting
    // setup_webserver();
}

/**
 * @brief Main program loop that runs continuously
 * 
 * Handles periodic tasks and polling:
 * - Check button inputs
 * - Update display
 * - Process CAN bus messages
 * - Handle Modbus master polling
 * - Handle Modbus client requests
 * 
 */

unsigned long lastMillis = 0;

void loop() {
   loop_buttons();
   
   if (millis() - lastMillis > 1000) {
        loop_modbus_master();
        lastMillis = millis();
        for(int i=0;i<MODBUS_NUM_METERS;i++) {
            loop_mqtt(dds238_meters[i]->last_reading); //publish to MQTT
        }
   }
    loop_buttons();             //multiple calls to loop_buttons() makes them more responsive
    loop_modbus_client();
    loop_buttons(); 
    loop_display();
    loop_can();
    // No need to call loop_webserver() with AsyncWebServer
}

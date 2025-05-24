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
#include <modbus.h>     // Modbus communication protocols
#include <buttons.h>    // Button input handling
#include <display.h>    // SH1106 OLED display
#include <console.h>    // Console UI for the display
#include <SPI.h>        // SPI communication for display/CAN
#include <can.h>        // Implementation of CAN bus communication
#include <mqtt_client.h> // MQTT client for publishing sensor data
#include <rs422.h>      // RS422 serial communication

void setup() {
    Serial.begin(115200);   // Initialize serial communication for debugging
    Serial.println("INFO - Booting...Setup in 3s");
    delay(3000);

    SPI.begin();        // Initialize SPI bus for display only
    setup_display();    // Initialize and configure the OLED display
    
    // Display startup splash screen (Rick image)
    drawBitmap(40, 5, RICK_WIDTH, RICK_HEIGHT, rick);
    delay(1000);
    drawBitmap(0, 0, LOGO_WIDTH, LOGO_HEIGHT, eIOT_logo); // Render Logo
    delay(1000);
    
    // Initialize console and display
    _console.addLine(" EMS Dev Kit Starting...");
    
    // Initialize RS422 serial communication
    setup_rs422();
    
    // Initialize MQTT client if enabled
    setup_mqtt();
    
    // Initialize buttons
    setup_buttons();
    
    _console.addLine(" EMS In-service Ready!");
    _console.addLine(" RS422 Monitoring Active");
#if ENABLE_MQTT
    _console.addLine(" MQTT publishing enabled");
#else
    _console.addLine(" MQTT publishing disabled");
#endif
}

/**
 * @brief Main program loop that runs continuously
 * 
 * Handles periodic tasks and polling:
 * - Update display
 * - Process RS422 serial data
 * - Maintain MQTT connection and publish data (if enabled)
 * - Check button inputs
 */
void loop() {
    // Update the display
    loop_display();
    
    // Process RS422 data first to ensure we don't miss any incoming data
    loop_rs422();
    
    
    // Handle MQTT if enabled
    loop_mqtt();
    
    // Process button inputs
    loop_buttons();
}

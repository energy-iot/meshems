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
#include <WiFi.h>       // WiFi functionality for TCP/IP

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
    
    // Initialize Modbus RTU master/client communication
    
    // This sets up the Modbus/RTU client (master) device as well as communication with connected devices
    setup_modbus_client_interface();
    // This sets up the Modbus/RTU server (slave) device
    setup_modbus_server();

    setup_buttons();
    _console.addLine("EMS Gatway Ready!");
    _console.addLine("Sol-Ark -> SunSpec");
    _console.addLine("Model 1, 701, 713 Active");
    
    // Display TCP/IP information
    if (WiFi.status() == WL_CONNECTED) {
        String ipInfo = "IP:" + WiFi.localIP().toString() + ":8502";
        _console.addLine(ipInfo);
    } else {
        _console.addLine("WiFi not connected");
    }
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
void loop() {
    loop_buttons();
    loop_modbus_client();
    loop_modbus_client();
    loop_display();
}

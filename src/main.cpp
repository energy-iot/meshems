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
#include <modbus.h>         // Modbus communication protocols
#include <buttons.h>        // Button input handling
#include <display.h>        // SH1106 OLED display
#include <console.h>        // Console UI for the display
#include <SPI.h>            // SPI communication for display/CAN
#include <can.h>            // Implementation of CAN bus communication
#include <wifi.h>           // Implement basic WiFi connection
#include <mqtt_client.h>    // Implement mqtt client
#include <config.h>         // Define required variables for device ID, mqtt connection 


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
    
    setup_wifi();           // Initialize WiFi Connection to the network defined in wifi.cpp
    setup_mqtt_client();    // Initialize MQTT Connection to the network

    // Initialize Modbus RTU master/client communication
    setup_modbus_master(); // This sets up communication with the EVSE controller and other devices
    setup_modbus_client();
    //setup_can(); // Initialize CAN bus communication

    setup_buttons();
    _console.addLine(" EMS In-service Ready!");
    _console.addLine("  Check MQTT @");
    _console.addLine("  public.cloud.shiftr.io");
    _console.addLine("  filter evse/#");
    _console.addLine("  Push a button?");

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

 unsigned long mqttlastMillis = 0;

void loop() {
    loop_buttons();
    loop_modbus_master();
    loop_modbus_client();
    
    if (millis() - mqttlastMillis > MQTT_PUBLISH_INTERVAL) {
        mqttlastMillis = millis();
        loop_mqtt();
    }
    loop_display();
    //loop_can();
}

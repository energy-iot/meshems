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
#include <wifi.h>
#include <mqtt_client.h>
#include <config.h>
#include <data_model.h>

void setup() {
    Serial.begin(115200);   // Initialize serial communication for debugging
    Serial.println("INFO - Booting...Setup in 3s");
    delay(3000);
    
    SPI.begin();

    generateDeviceID();
    setup_display();
    _console.addLine(" Display up! next is WiFi/Eth, ");
    _console.addLine(" NTP, MQTT, Modbus, Buttons... ");
    // Display startup splash screen (Rick image)
    drawBitmap(40, 5, RICK_WIDTH, RICK_HEIGHT, rick);
    delay(1000);
    
    drawBitmap(0, 0, LOGO_WIDTH, LOGO_HEIGHT, eIOT_logo); // Render Logo
    delay(1000);

    setup_wifi();
    setup_mqtt_client();
    //setup_powerData_caches(); //TODO maybe? openami to have 4-6 subtopics each with a cache planned , needs cache/buffered data for detecting a pattern change 
    //      for optional use each the time its polled or before each time it gets published  - iterate also for  rules and automation
    
    // Initialize Modbus RTU master/client communication
    setup_modbus_master(); // This sets up communication with sensors like the SHT20 temp/humidity sensor or other devices
    setup_modbus_client();
    //setup_gpio  // ssr, temp_humid, door contact/tamper. shock, imaging)
    setup_can(); // Initialize CAN bus communication

    setup_buttons();
    _console.addLine(" EMS In-service Ready!");
    _console.addLine("  CHECK MQTT @");
    _console.addLine("  public.cloud.shiftr.io"); //TODO grab the setup strings from the config file
    _console.addLine("  filter OPENAMI/#");       //TODO grab the setup strings from the config file
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
 * - Handle MQTT Publish
 * - Handle MQTT cmd responses 
 * 
 */

unsigned long lastModbusMillis = 0;
unsigned long lastMQTTMillis = 0;


void loop() {
   loop_buttons();
   
   if (millis() - lastModbusMillis > ModbusMaster_pollrate) {
        lastModbusMillis = millis();
        loop_modbus_master();
   }

   if (millis() - lastMQTTMillis > MQTTPublish_rootrate) {
        lastMQTTMillis = millis();
        /*
            TODO  calculate which are the normal periodic work items for this loop based on configurable MQTT periodic and adaptive 
            PUBLISH operations 
            perhaps  12 or so adaptive Publish global bools turned on or off per loop, the periodic  tasks in the subloops can then be targeted to run 
            this allows for adaptive rate of openami topics to be published , these adaptive rates can have a defualt periodicity
            but then time of day schedule can chnage the periodicity of the tasks.
            for example publish a base rate of 30 seconds, publish leaks at period of hourly , publish 3 ph summaries and single tenant meters 
             every 15 min itervals, publish harmnics every 15 mins, publish environmentals every 15 mins , 
             dont publish stuff on same 15 min  cadence (other than the 3Phase and meters must be on hourly edge cadence)  to minmize peak bandwidths
        */
        loop_mqtt();
   }
  
    loop_modbus_client();
    loop_buttons(); 
    loop_display();
    loop_can();
    // TODO loop_IFTTT();
    // TODO loop_alerts();
    
}
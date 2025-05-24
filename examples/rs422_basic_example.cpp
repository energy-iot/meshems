/*****************************************************************************
 * @file rs422_basic_example.cpp
 * @brief Basic example demonstrating RS422 serial communication
 * 
 * This example shows how to use the RS422 module with a QYF-998 RS422 to TTL
 * serial converter. The example demonstrates:
 * 
 * 1. Basic setup and initialization
 * 2. Sending and receiving messages
 * 3. Command processing
 * 4. Periodic status updates
 * 
 * Hardware Setup:
 * - Connect QYF-998 module RX+ and RX- to your RS422 device's TX+ and TX-
 * - Connect QYF-998 module TX+ and TX- to your RS422 device's RX+ and RX-
 * - Connect QYF-998 module RXD pin to ESP32 pin defined as RS422_RX_1 in pins.h
 * - Connect QYF-998 module TXD pin to ESP32 pin defined as RS422_TX_1 in pins.h
 * - Connect QYF-998 module VCC to 3.3V or 5V (check module specifications)
 * - Connect QYF-998 module GND to ESP32 GND
 * 
 * Usage:
 * 1. The system automatically sends heartbeat messages every 10 seconds
 * 2. Send "PING" to receive "PONG" response
 * 3. Send "STATUS" to get current message statistics
 * 4. Send "ECHO <your message>" to receive the message back
 * 5. Any other message will receive "UNKNOWN COMMAND" response
 * 
 * Author(s): Liam O'Brien
 *****************************************************************************/

#include <Arduino.h>
#include <rs422.h>
#include <pins.h>
#include <console.h>
#include <display.h>

// Example usage counter
int exampleCounter = 0;

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    Serial.println("RS422 Basic Example Starting...");
    
    // Initialize display (if available)
    setup_display();
    
    // Initialize RS422 communication
    setup_rs422();
    
    Serial.println("RS422 Basic Example Ready!");
    Serial.println("Try sending these commands via RS422:");
    Serial.println("  PING");
    Serial.println("  STATUS");
    Serial.println("  ECHO Hello World");
}

void loop() {
    // Process RS422 communication
    loop_rs422();
    
    // Update display
    loop_display();
    
    // Example: Send a custom message every 60 seconds
    static unsigned long lastCustomMessage = 0;
    if (millis() - lastCustomMessage > 60000) {
        exampleCounter++;
        String customMsg = "Example counter: " + String(exampleCounter);
        sendRS422CustomMessage(customMsg);
        Serial.println("Sent custom message: " + customMsg);
        lastCustomMessage = millis();
    }
    
    // Example: Print statistics every 20 seconds
    static unsigned long lastStatsTime = 0;
    if (millis() - lastStatsTime > 20000) {
        int rxCount, txCount;
        getRS422Stats(rxCount, txCount);
        Serial.println("RS422 Stats - RX: " + String(rxCount) + ", TX: " + String(txCount));
        lastStatsTime = millis();
    }
    
    // Small delay to prevent overwhelming the system
    delay(10);
}

/*
 * Expected Output on Serial Monitor:
 * 
 * RS422 Basic Example Starting...
 * INFO - RS422 Basic Example initialized
 * INFO - RS422 Baud Rate: 9600
 * INFO - RS422 RX Pin: [pin number]
 * INFO - RS422 TX Pin: [pin number]
 * INFO - RS422 Commands: PING, STATUS, ECHO <message>
 * Sent: RS422 Basic Example Started
 * RS422 Basic Example Ready!
 * Try sending these commands via RS422:
 *   PING
 *   STATUS
 *   ECHO Hello World
 * RS422 Sent: HEARTBEAT 10s
 * RS422 Stats - RX: 0, TX: 2
 * RS422 Sent: HEARTBEAT 20s
 * RS422 Sent: AUTO_STATUS: RX=0 TX=3 Uptime=30s
 * ...
 * 
 * If you send "PING" via RS422, you'll see:
 * RS422 Received: PING
 * RS422 Sent: PONG
 * 
 * If you send "STATUS" via RS422, you'll see:
 * RS422 Received: STATUS
 * RS422 Sent: STATUS: RX=2 TX=5
 * 
 * If you send "ECHO Test Message" via RS422, you'll see:
 * RS422 Received: ECHO Test Message
 * RS422 Sent: ECHO: Test Message
 */

/*****************************************************************************
 * @file rs422.cpp
 * @brief Simple RS422 serial communication interface - receive only
 * 
 * This file implements a simple RS422 receiver for communicating with a 
 * QYF-998 RS422 to TTL serial module connected to the ESP32.
 * 
 * Author(s): Liam O'Brien
 *****************************************************************************/

#include <rs422.h>
#include <pins.h>
#include <SoftwareSerial.h>
#include <console.h>

// Fixed baud rate for RS422 communication
const int RS422_BAUD_RATE = 9600;

// Define the software serial port for RS422 communication
SoftwareSerial RS422Serial(RS422_RX_1, RS422_TX_1);

// Buffer for storing received data
const int RS422_BUFFER_SIZE = 256;
char rs422Buffer[RS422_BUFFER_SIZE];
int rs422BufferIndex = 0;

// Counter for received messages
int messageCount = 0;

/**
 * @brief Initialize the RS422 serial communication
 */
void setup_rs422() {
    // Initialize RS422 software serial port
    RS422Serial.begin(RS422_BAUD_RATE);
    
    Serial.println("INFO - RS422 receiver initialized");
    Serial.print("INFO - RS422 Baud Rate: ");
    Serial.println(RS422_BAUD_RATE);
    Serial.print("INFO - RS422 RX Pin: ");
    Serial.println(RS422_RX_1);
    Serial.print("INFO - RS422 TX Pin: ");
    Serial.println(RS422_TX_1);
    
    // Add to console display
    _console.addLine("RS422 receiver ready");
    _console.addLine("Baud: " + String(RS422_BAUD_RATE));
}

/**
 * @brief Process incoming RS422 serial data
 */
void loop_rs422() {
    // Check if data is available from RS422
    if (RS422Serial.available()) {
        // Read a byte from RS422
        char c = RS422Serial.read();
        
        // Add to buffer if there's space
        if (rs422BufferIndex < RS422_BUFFER_SIZE - 1) {
            rs422Buffer[rs422BufferIndex++] = c;
        }
        
        // If we receive a newline or carriage return, process the message
        if (c == '\n' || c == '\r' || rs422BufferIndex >= RS422_BUFFER_SIZE - 1) {
            // Null-terminate the string
            rs422Buffer[rs422BufferIndex] = '\0';
            
            // Only output non-empty messages
            if (rs422BufferIndex > 0) {
                // Output the received message to debug serial
                Serial.print("RS422 Received: ");
                Serial.println(rs422Buffer);
                
                // Add to console display
                messageCount++;
                String displayMsg = "Msg #" + String(messageCount);
                _console.addLine(displayMsg);
                
                // If the message is short enough, display it
                if (rs422BufferIndex < 20) {
                    _console.addLine(rs422Buffer);
                } else {
                    // Otherwise display a truncated version
                    char shortMsg[21];
                    strncpy(shortMsg, rs422Buffer, 20);
                    shortMsg[20] = '\0';
                    _console.addLine(String(shortMsg) + "...");
                }
            }
            
            // Reset buffer for next message
            rs422BufferIndex = 0;
        }
    }
}

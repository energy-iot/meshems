/*****************************************************************************
 * @file rs422.h
 * @brief Simple RS422 serial communication interface - receive only
 * 
 * This header defines functions for simple RS422 message reception using a 
 * QYF-998 RS422 to TTL serial module connected to the ESP32.
 * 
 * Author(s): Liam O'Brien
 *****************************************************************************/

#pragma once

#include <Arduino.h>

/**
 * @brief Initialize the RS422 serial communication
 * 
 * Sets up the software serial port for communication with the QYF-998 RS422 module
 * using the pins defined in pins.h. Initializes at 9600 baud rate.
 */
void setup_rs422();

/**
 * @brief Process incoming RS422 serial data
 * 
 * Reads data from the RS422 serial port and outputs received messages
 * to the debug serial port and OLED display. Should be called regularly 
 * in the main loop.
 */
void loop_rs422();

/*****************************************************************************
 * @file pins.h
 * @brief Pin definitions for dev board
 * 
 * This header defines all GPIO pin assignments for the system peripherals:
 * - Analog button array (using voltage divider)
 * - SPI OLED display (SH1106)
 * - RS-485 interfaces (dual channels using HW-519 modules)
 * - Relay control
 * - CAN bus interface (MCP2515)
 * 
 * 
 * Author(s): Doug Mendonca, Liam O'Brien
 *****************************************************************************/

#pragma once

// analog button array (voltage divider)
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define ANALOG_BTN_PIN  A0 //GPIO1
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
#define ANALOG_BTN_PIN  A0
#else
#define ANALOG_BTN_PIN  A7
#endif

//SPI OLED display
//#define DISPLAY_RST_PIN 2
//#define DISPLAY_DC_PIN 42
//#define DISPLAY_CS_PIN 41

// ==================== SPI DISPLAY ====================
#define DISPLAY_RST_PIN 46  //Reset
#define DISPLAY_DC_PIN 3    //Data clock
#define DISPLAY_CS_PIN 9    //Chip select

// ==================== RS485 INTERFACE ================
//BLUE TERMINALS - MASTER DEVICE
#define RS485_RX_1             GPIO_NUM_15   // RX here maps to RS485 HW-519 module's silk screen "RXD"
#define RS485_TX_1             GPIO_NUM_16   // TX here maps to RS485 HW-519 module's silk screen "TXD"
//GREEN TERMINALS - SLAVE DEVICE
#define RS485_RX_2             GPIO_NUM_6  // RX here maps to RS485 HW-519 module's silk screen "RXD"
#define RS485_TX_2             GPIO_NUM_7  // TX here maps to RS485 HW-519 module's silk screen "TXD"

// ==================== RELAY ==========================
#define RELAY_1_PIN 38  //Pin to toggle the onboard SSR

// ==================== CAN INTERFACE ==================
#define CAN0_CS     2   //SPI chip select
#define CAN0_SO     42  //SPI MISO
#define CAN0_SI     41  //SPI MOSI
#define CAN0_SCK    8   //SPI clock
#define CAN0_INT    17  //Message interrupt output

// ==================== RS422 INTERFACE ==================
#define RS422_RX_1     21   //RX here maps to RS422 QYF-998 module's silk screen "RXD"
#define RS422_TX_1     47  //RX here maps to RS422 QYF-998 module's silk screen "TXD"

//#define RS422_RX_1     47   //RX here maps to RS422 QYF-998 module's silk screen "RXD"
//#define RS422_TX_1     21  //RX here maps to RS422 QYF-998 module's silk screen "TXD"
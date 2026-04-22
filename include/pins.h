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

#ifdef PCB865B

#define BATTERY_VOLTAGE_PIN A0
#define CTRL_LED_PIN 1

//LED
#define RGBLED_DATA_PIN 48
#ifdef SEEED_XIAO_C3
#define PAIR_BUTTON 4        // button for pairing
#define PAIR_LED 5           // LED used to signal pairing
#define PAIR_LED_ON LOW       // Blue LED on Nano BLE has inverted logic
#elif CONFIG_IDF_TARGET_ESP32S3
#define PAIR_LED_PIN 2           // LED used to signal pairing
#define PAIR_LED_ON HIGH
#else
#define PAIR_LED_PIN 2           // LED used to signal pairing
#define PAIR_LED_ON HIGH
#endif

//sensors
#define BME280_SDA_PIN 4
#define BME280_SCL_PIN 5

// analog button array (voltage divider)
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define ANALOG_BTN_PIN  A0
#else
#define ANALOG_BTN_PIN  A7
#endif

//display
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define DISPLAY_RST_PIN 8
#define DISPLAY_DC_PIN  18
#define DISPLAY_CS_PIN  17  
#else
#define DISPLAY_RST_PIN 4
#define DISPLAY_DC_PIN  22
#define DISPLAY_CS_PIN  2
#endif

//energy meter 3 stack circuitsetup.u
// or 1 phase 18 cts , or 2 phases 6/9/12 cts
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define CS1_BOARD1 15 // cs.us pin cs  Phase A or 1 CTs 1-3
#define CS2_BOARD1 16 // cs.us pin cs2 Phase A or 1 CTs 4-6
#define CS1_BOARD2 47 // cs.us pin 0   Phase B or 2 CTs 1-3
#define CS2_BOARD2 2  // cs.us pin 16  Phase B or 2 CTs 4-6
#define CS1_BOARD3 21 // cs.us pin 15  Phase C or 3 CTs 1-3
#define CS2_BOARD3 19 // cs.us pin 17  Phase C or 3 CTs 4-6
#else
// 16 and 17 are RX2/TX2 on devkit, remapped in main::setup
#define CS1_BOARD1 27 // red:33 ok};// drm - free up extra pins: , 0, 27, 2, 13, 14, 15 };
#define CS2_BOARD1 26 // blue:32 ok}; // drm - free up extra pins:, 16, 17, 21, 22, 25, 26 };
#endif

//ethernet
#define ETHER_PIN_RANDOM_SEED     A0 //A10 //was A0 on devkit
#define ETHER_PIN_CHIP_SELECT     9 //8 works //46-must be low for boot! //SS //SPI default SS/CS (5 on devkit/nodeMCU)

#ifdef CONFIG_IDF_TARGET_ESP32S3
#define PIN_ETHERNET_RESET  3//9 //15 on nodemcu //33 also works
#else
#define PIN_ETHERNET_RESET  15 //on nodemcu //33 also works
#endif

//GPS
#define GPS_SERIAL_RX 46
#define GPS_SERIAL_TX 40

//SSR
#define SSR1_PIN 20

// RS-485 
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define MAX485_RE_DE_TOGGLE_A   GPIO_NUM_5   
#define RS485_RX_A              GPIO_NUM_6
#define RS485_TX_A              GPIO_NUM_4
#define MAX485_RE_DE_TOGGLE_B  GPIO_NUM_41
#define RS485_RX_B             GPIO_NUM_42
#define RS485_TX_B             GPIO_NUM_7
#else
#define MAX485_RE_DE_TOGGLE_A 25    
#define RS485_RX_A 26
#define RS485_TX_A 27 
#define MAX485_RE_DE_TOGGLE_B   0 
#define RS485_RX_B 0
#define RS485_TX_B 0
#endif

// CAN INTERFACE (PCB865B - verify against schematic; SI moved off GPIO12 to avoid SPI2 SCK conflict)
#define CAN0_CS     5
#define CAN0_SO     35
#define CAN0_SI     38   // was 12 — conflicts with ESP32-S3 default SPI2 SCK (display)
#define CAN0_SCK    14
#define CAN0_INT    34

#else //PCB865B

#ifdef ESP32_POE_ISO
// ==================== ESP32-POE-ISO PIN DEFINITIONS ====================
// Olimex ESP32-POE-ISO: ESP32-WROVER, Ethernet PHY on GPIO17/23, limited free GPIOs

#define RGBLED_DATA_PIN 2   // onboard LED on POE-ISO

// analog button array (voltage divider)
#define ANALOG_BTN_PIN  36  // GPIO36 - POE-ISO variant has no A0 macro

// SPI DISPLAY (remapped for POE-ISO available GPIOs)
#define DISPLAY_RST_PIN 15
#define DISPLAY_DC_PIN  13
#define DISPLAY_CS_PIN  14

// RS485 INTERFACE (remapped for POE-ISO available GPIOs)
#define RS485_RX_1             GPIO_NUM_36  // RX - input only pin
#define RS485_TX_1             GPIO_NUM_4   // TX
#define RS485_RX_2             GPIO_NUM_39  // RX - input only pin
#define RS485_TX_2             GPIO_NUM_33  // TX

// RELAY
#define RELAY_1_PIN 32

// CAN INTERFACE (remapped for POE-ISO available GPIOs)
#define CAN0_CS     5
#define CAN0_SO     35
#define CAN0_SI     12
#define CAN0_SCK    14
#define CAN0_INT    34

#else
// ==================== ESP32-S3 DEVKITC PIN DEFINITIONS =================

// esp32s3 devkitc n16r8 onboard rgbw led
#define RGBLED_DATA_PIN 48

// analog button array (voltage divider)
#define ANALOG_BTN_PIN  A0  // GPIO1 on S3

// SPI DISPLAY
#define DISPLAY_RST_PIN 46  //Reset
#define DISPLAY_DC_PIN 3    //Data clock
#define DISPLAY_CS_PIN 9    //Chip select

// RS485 INTERFACE
#define RS485_RX_1             GPIO_NUM_6   // RX here maps to RS485 HW-519 module's silk screen "RXD"
#define RS485_TX_1             GPIO_NUM_7   // TX here maps to RS485 HW-519 module's silk screen "TXD"
#define RS485_RX_2             GPIO_NUM_15  // RX here maps to RS485 HW-519 module's silk screen "RXD"
#define RS485_TX_2             GPIO_NUM_16  // TX here maps to RS485 HW-519 module's silk screen "TXD"

// RELAY
#define RELAY_1_PIN 38  //Pin to toggle the onboard SSR, solid state relay - 5 vdc TTL TBD for larger ssr

// CAN INTERFACE
#define CAN0_CS     2   //SPI chip select
#define CAN0_SO     42  //SPI MISO
#define CAN0_SI     41  //SPI MOSI
#define CAN0_SCK    8   //SPI clock
#define CAN0_INT    17  //Message interrupt output

#endif // ESP32_POE_ISO

#endif // PCB865B
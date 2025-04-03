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

#define DISPLAY_RST_PIN 46
#define DISPLAY_DC_PIN 3
#define DISPLAY_CS_PIN 9

// RS-485 
#define RS485_RX_1             GPIO_NUM_6
#define RS485_TX_1             GPIO_NUM_7 
#define RS485_RX_2             GPIO_NUM_16
#define RS485_TX_2             GPIO_NUM_15

// Relays
#define RELAY_1_PIN 8
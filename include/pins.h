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
#define RS485_RX_1             GPIO_NUM_6   // RX here maps to RS485 HW-519 module's silk screen "RXD"
#define RS485_TX_1             GPIO_NUM_7   // TX here maps to RS485 HW-519 module's silk screen "TXD"
#define RS485_RX_2             GPIO_NUM_15  // RX here maps to RS485 HW-519 module's silk screen "RXD"
#define RS485_TX_2             GPIO_NUM_16  // TX here maps to RS485 HW-519 module's silk screen "TXD"

// Relays
#define RELAY_1_PIN 18

//CAN
#define CAN0_CS     2
#define CAN0_SO     42  //SPI MISO
#define CAN0_SI     41  //SPI MOSI
#define CAN0_SCK    8   //SPI clock
#define CAN0_INT    17
#pragma once
#include <stdint.h>

// 10-channel I2C SSR driver — two PCF8574 expanders, active-LOW outputs
// Chip A (0x20): channels 1-8   Chip B (0x21): channels 9-10

void setup_i2c_ssr();
void i2c_ssr_set(uint8_t channel, bool on);   // channel 1–10
void i2c_ssr_set_all(uint16_t mask);          // bit0=ch1 … bit9=ch10, 1=ON

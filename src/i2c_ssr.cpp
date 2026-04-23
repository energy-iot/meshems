#include <Arduino.h>
#include <Wire.h>
#include "i2c_ssr.h"
#include "pins.h"

static TwoWire ssrWire(1);

#define PCF_ADDR_A  0x20    // channels 1-8
#define PCF_ADDR_B  0x21    // channels 9-10

static uint8_t stateA = 0xFF;   // PCF8574 active-LOW: 0xFF = all relays off
static uint8_t stateB = 0xFF;

static void pcf_write(uint8_t addr, uint8_t val) {
    ssrWire.beginTransmission(addr);
    ssrWire.write(val);
    ssrWire.endTransmission();
}

void setup_i2c_ssr() {
    ssrWire.begin(I2C_SSR_SDA_PIN, I2C_SSR_SCL_PIN);
    pcf_write(PCF_ADDR_A, stateA);
    pcf_write(PCF_ADDR_B, stateB);
}

void i2c_ssr_set(uint8_t channel, bool on) {
    if (channel < 1 || channel > 10) return;
    if (channel <= 8) {
        uint8_t bit = 1 << (channel - 1);
        stateA = on ? (stateA & ~bit) : (stateA | bit);
        pcf_write(PCF_ADDR_A, stateA);
    } else {
        uint8_t bit = 1 << (channel - 9);
        stateB = on ? (stateB & ~bit) : (stateB | bit);
        pcf_write(PCF_ADDR_B, stateB);
    }
}

void i2c_ssr_set_all(uint16_t mask) {
    stateA = ~(uint8_t)(mask & 0xFF);
    stateB = ~(uint8_t)((mask >> 8) & 0x03);
    pcf_write(PCF_ADDR_A, stateA);
    pcf_write(PCF_ADDR_B, stateB);
}

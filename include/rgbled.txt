#ifndef _RGB_LEDS_H_
#define _RGB_LEDS_H_

#include <FastLED.h>

#define LED_ETHERNET 0
#define LED_BLUETOOTH 1
#define LED_THREE 2
#define LED_FOUR 3

void setup_rgbled();
void loop_rgbled();
void setLED_RED();
void setLED_BLUE();
void setLED_GREEN();
void setLED_WHITE();
void setLED_OFF();
int getNumLEDS();

extern CRGB leds[];  // Replaces Adafruit_NeoPixel pixel object

#endif
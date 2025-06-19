#include <rgbled.h>
#include <debug.h>
#include <pins.h>

#define NUM_LEDS    1
#define BRIGHTNESS  25

CRGB leds[NUM_LEDS];  // FastLED LED array

int getNumLEDS() {
    return NUM_LEDS;
}

void setup_rgbled() {
    FastLED.addLeds<NEOPIXEL, RGBLED_DATA_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    setLED_RED();
    delay(500);
    setLED_GREEN();
    delay(500);
    setLED_BLUE();
    delay(500);
    setLED_WHITE();
    delay(500);
    setLED_OFF();

    // TODO: Add diagnostic patterns as noted in original comments
}

void loop_rgbled() {
    // You can add blinking logic here later
}

void setLED_RED() {
    leds[0] = CRGB::Red;
    FastLED.show();
}

void setLED_GREEN() {
    leds[0] = CRGB::Green;
    FastLED.show();
}

void setLED_BLUE() {
    leds[0] = CRGB::Blue;
    FastLED.show();
}

void setLED_WHITE() {
    leds[0] = CRGB::White;
    FastLED.show();
}

void setLED_OFF() {
    leds[0] = CRGB::Black;
    FastLED.show();
}
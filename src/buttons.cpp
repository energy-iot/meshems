#include <Arduino.h>
#include <buttons.h>
#include <pins.h>

// Calibrated for two-button voltage divider array:
//   Button 1 reads ~1290, Button 2 reads ~1980
#define THOLD_NONE  2400   // above = no press (idle high with pullup)
#define THOLD_BTN2  1635   // midpoint between btn1 (~1290) and btn2 (~1980)
#define THOLD_BTN1   900   // floor for btn1 detection

bool pressed = false;
void (*button1_cb)();
void (*button2_cb)();

void button1_pushed() {
}
void button2_pushed() {
}

void setup_buttons() {
   pinMode(ANALOG_BTN_PIN, INPUT_PULLUP);
}

void loop_buttons() {
    int val = analogRead(ANALOG_BTN_PIN);
    //static int printCount = 0;
    //if (++printCount >= 20) { Serial.printf("A0 VCC: %d\n", val); printCount = 0; }
    if (val >= THOLD_NONE) {
        pressed = false;
        return;
    } else if (val > THOLD_BTN2 && !pressed) {
        Serial.printf("BUTTON 2 (%d)\n", val);
        button2_pushed();
        pressed = true;
    } else if (val > THOLD_BTN1 && !pressed) {
        Serial.printf("BUTTON 1 (%d)\n", val);
        button1_pushed();
        pressed = true;
    }
}

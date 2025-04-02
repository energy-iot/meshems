#include <Arduino.h>
#include <buttons.h>
#include <pins.h>

/* use multimeter to obtain thresholds for different resistor values
Voltage divider button array using 1K resistors: */
#define THOLD_NONE 4000
#define THOLD_BTN4 3800
#define THOLD_BTN3 3300
#define THOLD_BTN2 2500
#define THOLD_BTN1 200

bool pressed = false;
int last_btn_press_timestamp = 0;
void (*button1_cb)();
void (*button2_cb)();
void (*button3_cb)();
void (*button4_cb)();

void button1_pushed() {
}
void button2_pushed() {
}
void button3_pushed() {
}
void button4_pushed() {
}

void setup_buttons() {
   pinMode(ANALOG_BTN_PIN, INPUT_PULLUP);
}

void loop_buttons() {
    int val = analogRead(ANALOG_BTN_PIN);
    //Serial.printf("A0 VCC: %d\n", val);
    if (val >= THOLD_NONE){
        pressed = false;
        return;
    
    //BUTTON 4
    } else if (val>THOLD_BTN4 && !pressed) {
        Serial.printf("BUTTON 4\n");
        button4_pushed();
        pressed = true;

    //BUTTON 3
    } else if (val > THOLD_BTN3 && !pressed) {
        Serial.printf("BUTTON 3\n");
        button3_pushed();
        pressed = true;

    //BUTTON 2
    } else if (val > THOLD_BTN2 && !pressed) {
        Serial.printf("BUTTON 2\n");
        button2_pushed();
        pressed = true;

    //BUTTON 1
    } else if (val > THOLD_BTN1 && !pressed ) {
        Serial.printf("BUTTON 1\n");
        button1_pushed();
        pressed = true;
    }
}
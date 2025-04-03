#include <Arduino.h>
#include <relay.h>
#include <pins.h>

bool btn_toggle = false;

void toggle_relay_1() {
    //actuate relay 
    if (btn_toggle) {
        digitalWrite(RELAY_1_PIN,LOW);
    } else {
        digitalWrite(RELAY_1_PIN,HIGH);
    }
    btn_toggle = !btn_toggle;
}

void setup_relays() {
    pinMode(RELAY_1_PIN,OUTPUT);
    digitalWrite(RELAY_1_PIN,LOW);
}
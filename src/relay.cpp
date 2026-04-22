#include <Arduino.h>
#include <relay.h>
#include <pins.h>

bool btn_toggle = false;

void toggle_relay_1() {
    //actuate relay 
    if (btn_toggle) {
        digitalWrite(SSR1_PIN,LOW);
    } else {
        digitalWrite(SSR1_PIN,HIGH);
    }
    btn_toggle = !btn_toggle;
}

void setup_relays() {
    pinMode(SSR1_PIN,OUTPUT);
    digitalWrite(SSR1_PIN,LOW);
}
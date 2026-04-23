#include <Arduino.h>
#include <buttons.h>
#include <pins.h>

#define THOLD_NONE  2400
#define THOLD_BTN2  1635
#define THOLD_BTN1   900
#define BTN_SCAN_MS   20   // scan interval — fast enough to feel instant

static volatile bool pressed = false;
void (*button1_cb)();
void (*button2_cb)();

void button1_pushed() {Serial.printf("BUTTON 1 PRESSED\n");}
void button2_pushed() {Serial.printf("BUTTON 2 PRESSED\n");}

static void button_task(void* param) {
    //int printCount = 0;
    for (;;) {
        int val = analogRead(ANALOG_BTN_PIN);
        //if (++printCount >= 20) { Serial.printf("A0 VCC: %d\n", val); printCount = 0; }

        if (val >= THOLD_NONE) {
            pressed = false;
        } else if (val > THOLD_BTN2 && !pressed) {
            //Serial.printf("BUTTON 2 (%d)\n", val);
            button2_pushed();
            pressed = true;
        } else if (val > THOLD_BTN1 && !pressed) {
            //Serial.printf("BUTTON 1 (%d)\n", val);
            button1_pushed();
            pressed = true;
        }
        vTaskDelay(pdMS_TO_TICKS(BTN_SCAN_MS));
    }
}

void setup_buttons() {
    pinMode(ANALOG_BTN_PIN, INPUT_PULLUP);
    xTaskCreatePinnedToCore(button_task, "buttons", 2048, NULL, 2, NULL, 0);
}
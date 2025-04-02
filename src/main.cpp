#include <Arduino.h>
#include <modbus.h>
#include <buttons.h>

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");
    setup_modbus_master();
    setup_buttons();
}

void loop() {
    loop_buttons();
    loop_modbus_master();
}

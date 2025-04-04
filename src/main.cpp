#include <Arduino.h>
#include <modbus.h>
#include <buttons.h>
#include <display.h>
#include <console.h>
#include <SPI.h>
#include <can.h>

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");
    delay(4000);

    SPI.begin();
    setup_display();
    
    drawBitmap(40, 5, RICK_WIDTH, RICK_HEIGHT, rick); 
    delay(1000);

    setup_modbus_master();
    setup_can();

    setup_buttons();
    _console.addLine("      EMS Devkit");

}

void loop() {
    loop_buttons();
    loop_modbus_master();
    loop_display();
    loop_can(); //uncomment when ready to talk to transceiver
}

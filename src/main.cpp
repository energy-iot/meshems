#include <Arduino.h>
#include <modbus.h>
#include <buttons.h>
#include <display.h>
#include <console.h>
#include <SPI.h>
#include <can.h>
#include <wifi.h>
#include <mqtt_client.h>
#include <config.h>

void setup() {
    Serial.begin(115200);
    Serial.println("INFO - Booting...Setup in 4s");
    delay(4000);

    SPI.begin();

    generateDeviceID();

    setup_wifi();
    setup_mqtt_client();
    setup_display();
    
    drawBitmap(40, 5, LOGO_WIDTH, LOGO_HEIGHT, rick); // Render Rick Bitmap Array
    delay(1000);
    
    // drawBitmap(40, 5, LOGO_WIDTH, LOGO_HEIGHT, eIOT_logo_7060); // Render EIOT Logo 70x70 Array
    // delay(1000);

    drawBitmap(40, 5, LOGO_WIDTH, LOGO_HEIGHT, eIOT_logo_6048); // Render EIOT Logo 60x48 Array
    delay(1000);

    setup_modbus_master();
    setup_modbus_client();
    setup_can();

    setup_buttons();
    _console.addLine("   EMS Setup Done!");

}

//All we have to decide is what to do with the time that is given us. -GtG

//buttons are more responsive 
void loop() {
   loop_buttons();
    loop_modbus_master();
    loop_modbus_client();
   loop_buttons(); 
    loop_display();
    loop_can();
    loop_mqtt();
}

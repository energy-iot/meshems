/**The MIT License (MIT)

Copyright (c) 2016 by Rene-Martin Tudyka
Based on the SSD1306 OLED library Copyright (c) 2015 by Daniel Eichhorn (http://blog.squix.ch),
available at https://github.com/squix78/esp8266-oled-ssd1306

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at http://blog.squix.ch
*/
#include <display.h>
#include <console.h>
#include <pins.h>

// Uncomment one of the following based on OLED type
// SH1106(bool HW_SPI, int rst, int dc, int cs );
SH1106 display(true, DISPLAY_RST_PIN, DISPLAY_DC_PIN, DISPLAY_CS_PIN); // FOR SPI
//SH1106   display(OLED_ADDR, OLED_SDA, OLED_SDC);    // For I2C
SH1106Ui ui( &display );

char rssi_overlay[12];
int second_interval = 0;
int start_millis = 0;

void display_set_rssi(const char* rssi) {
  strncpy(rssi_overlay,rssi,12);
}

bool msOverlay(SH1106 *display, SH1106UiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, String(millis()/1000));
  return true;
}
bool rssiOverlay(SH1106 *display, SH1106UiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, rssi_overlay);
  return true;
}
/*
bool drawThermostatFrame(SH1106 *display, SH1106UiState* state, int x, int y) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);

  char str[32];
  uint8_t  t = thermostat[0].getDisplayTempF();
    if (t == 0) {
    sprintf(str, "MB_A TempF: ? ");
  } else {
    sprintf(str, "MB_A TempF: %2u ", t);
  }
  display->drawString(0, 0, str);

  sprintf(str, "MB_A Mode: %s ", thermostat[0].getModeStr().c_str());
  display->drawString(0, 15, str);

  t = thermostat[1].getDisplayTempF();
  if (t == 0) {
    sprintf(str, "MB_B TempF: ? ");
  } else {
    sprintf(str, "MB_B TempF: %2u ", thermostat[1].getDisplayTempF());
  }
  display->drawString(0, 30, str);

  sprintf(str, "MB_B Mode: %s ", thermostat[1].getModeStr().c_str());
  display->drawString(0, 45, str);

  return false;
}*/

bool drawFrame1(SH1106 *display, SH1106UiState* state, int x, int y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  // if this frame need to be refreshed at the targetFPS you need to
  // return true
  display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  return false;
}

/*
bool drawEnergyFrame(SH1106 *display, SH1106UiState* state, int x, int y) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0, 0, "System/Meter status");
  display->drawString(0, 10, "SS 1: S0:0x" + String(energy_model.sys0, HEX) + " S1:0x" + String(energy_model.sys1, HEX));
  display->drawString(0, 20, "MS 1: E0:0x" + String(energy_model.en0, HEX) + " E1:0x" + String(energy_model.en1, HEX));
  display->drawString(0, 30, "SS 2: S0:0x" + String(energy_model.sys0_2, HEX) + " S1:0x" + String(energy_model.sys1_2, HEX));
  display->drawString(0, 40, "MS 2: E0:0x" + String(energy_model.en0_2, HEX) + " E1:0x" + String(energy_model.en1_2, HEX));

  return false;
}
bool drawEnergyFrame2(SH1106 *display, SH1106UiState* state, int x, int y) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  char buf[32];
  sprintf(buf, "Temp: %02.02f C", energy_model.temp);
  display->drawString(0, 0, buf);
  sprintf(buf, "Freq: %02.02f Hz", energy_model.freq);
  display->drawString(0, 16, buf);
  sprintf(buf, "V1: %02.02f V", energy_model.voltage1);
  display->drawString(0, 32, buf);
  sprintf(buf, "V2: %02.02f V", energy_model.voltage2);
  display->drawString(0, 48, buf);

  return false;
}

*/

bool drawFrame3(SH1106 *display, SH1106UiState* state, int x, int y) {
  // Text alignment demo
  display->clear();
  display->setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 11 + y, "Left aligned (0,10)");

  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 22, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 33, "Right aligned (128,33)");
  return false;
}

bool drawFrame4(SH1106 *display, SH1106UiState* state, int x, int y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore.");
  return false;
}

bool drawConsoleFrame(SH1106 *display, SH1106UiState* state, int x, int y) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);  
  _console.redrawConsoleFrame(display);
  return false;
}

// how many frames are there?
int frameCount = 1;
// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
//bool (*frames[])(SH1106 *display, SH1106UiState* state, int x, int y) = { drawThermostatFrame, drawEnergyFrame2, drawEnergyFrame3, drawEnergyFrame4}; //, drawFrame2, drawFrame3, drawFrame4 };
bool (*frames[])(SH1106 *display, SH1106UiState* state, int x, int y) = {drawConsoleFrame};//drawFrame3, drawFrame4 };
//bool (*frames[])(SH1106 *display, SH1106UiState* state, int x, int y) = { drawEnergyFrame3, drawEnergyFrame4}; //, drawFrame2, drawFrame3, drawFrame4 };
//bool (*frames[])(SH1106 *display, SH1106UiState* state, int x, int y) = { drawConsoleFrame , drawThermostatFrame, drawEnergyFrame, drawEnergyFrame2, drawEnergyFrame3}; //, drawFrame2, drawFrame3, drawFrame4 };

void drawFrame_Text(int x, int y, String text) {
  ui.getDisplay()->clear();
  ui.getDisplay()->setTextAlignment(TEXT_ALIGN_LEFT);
  ui.getDisplay()->setFont(ArialMT_Plain_10);
  ui.getDisplay()->drawStringMaxWidth(0 + x, 10 + y, 128, text);
  ui.getDisplay()->drawString(x, y, text);
  ui.render();

  }

void drawFrame_NetworkInfo(String ssid, String ip_addr, String mac_addr, String extra) {
  ui.getDisplay()->clear();
  ui.getDisplay()->setTextAlignment(TEXT_ALIGN_LEFT);
  ui.getDisplay()->setFont(ArialMT_Plain_10);  
  ui.getDisplay()->drawStringMaxWidth(0, 5, 128, ssid);  
  ui.getDisplay()->drawStringMaxWidth(0, 15, 128, ip_addr);  
  ui.getDisplay()->drawStringMaxWidth(0, 25, 128, mac_addr);  
  ui.getDisplay()->drawStringMaxWidth(0, 35, 128, extra);  
  ui.render();
}

void drawFrame_SixLines(String one, String two, String three, String four, String five, String six) {
  ui.getDisplay()->clear();
  ui.getDisplay()->setTextAlignment(TEXT_ALIGN_LEFT);
  ui.getDisplay()->setFont(ArialMT_Plain_10);  
  ui.getDisplay()->drawStringMaxWidth(0, 0, 128, one);  
  ui.getDisplay()->drawStringMaxWidth(0, 10, 128, two);  
  ui.getDisplay()->drawStringMaxWidth(0, 20, 128, three);  
  ui.getDisplay()->drawStringMaxWidth(0, 30, 128, four);  
  ui.getDisplay()->drawStringMaxWidth(0, 40, 128, five);  
  ui.getDisplay()->drawStringMaxWidth(0, 50, 128, six);  
  ui.render();
}
void drawBitmap(int x, int y, int width, int height, const char *bitmap) {
  ui.getDisplay()->clear();
  ui.drawBitmap(x,y,width,height,bitmap);
  ui.render();
}
void drawRect(int x, int y, int width, int height) {
  ui.getDisplay()->clear();
  ui.getDisplay()->drawRect(x,y,width,height);
  ui.render();
}
bool (*overlays[])(SH1106 *display, SH1106UiState* state)             = { rssiOverlay };
//bool (*overlays[])(SH1106 *display, SH1106UiState* state)             = { };//thermostat_overlay };
int overlaysCount = 1;

SH1106Ui* getUI() {
  return &ui;
}

SH1106Ui* setup_display() {

  pinMode(DISPLAY_CS_PIN, OUTPUT);
  digitalWrite(DISPLAY_CS_PIN, HIGH);

  Serial.println("Starting display");
  ui.setTargetFPS(30);
  ui.setActiveSymbole(activeSymbole);
  ui.setInactiveSymbole(inactiveSymbole);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);
  //ui.disableAutoTransition();

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Inital UI takes care of initalising the display too.
  ui.init();

  display.flipScreenVertically();

  start_millis = millis();

  return &ui;
}
SH1106* getDisplay() {return &display;};

void show_seconds_counter() {
  //print seconds counter in the overlay
  if (millis() - second_interval > 1000) {
    char tbuf[16] = {0};
    sprintf(tbuf, "t:%d", (millis()-start_millis/1000)/1000);
    display_set_rssi(tbuf);
    second_interval = millis();
  }
}

void loop_display() {
  ui.update();
  //int remainingTimeBudget = ui.update();
  //if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    //delay(remainingTimeBudget);
  //}
  show_seconds_counter();
}
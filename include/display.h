#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Wire.h>
#include "SH1106.h"
#include "SH1106Ui.h"
#include "images.h"
#include "rick.h"

void loop_display();
SH1106Ui* setup_display();

bool msOverlay(SH1106 *display, SH1106UiState* state);

bool thermostat_overlay(SH1106 *display, SH1106UiState* state);
bool drawConsoleFrame(SH1106 *display, SH1106UiState* state, int x, int y);

bool drawFrame1(SH1106 *display, SH1106UiState* state, int x, int y);
bool drawThermostatFrame(SH1106 *display, SH1106UiState* state, int x, int y);
bool drawFrame3(SH1106 *display, SH1106UiState* state, int x, int y);
bool drawFrame4(SH1106 *display, SH1106UiState* state, int x, int y);
void drawFrame_Text(int x, int y, String text);
void drawFrame_NetworkInfo(String ssid, String ip_addr, String mac_addr, String extra);
void drawFrame_FiveLines(String one, String two, String three, String four, String five);
void drawFrame_SixLines(String one, String two, String three, String four, String five, String six);
void drawBitmap(int x, int y, int width, int height, const char *bitmap);
void drawRect(int x, int y, int width, int height);
void display_set_rssi(const char* rssi);
SH1106Ui* getUI();

#endif

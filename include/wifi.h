#pragma once

bool setup_wifi();
bool setup_wifi(String SSID, String PW);
bool wifi_client_connected();
void loop_wifi();
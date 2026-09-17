#pragma once
#include <Arduino.h>

enum Screen { SCREEN_MAIN, SCREEN_MENU, SCREEN_WIFI_INFO };
extern Screen currentScreen;

void initDisplay();
void drawMainScreen();
void drawMenu();
void drawWiFiInfo();
void handleJoystick();
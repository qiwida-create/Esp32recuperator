#pragma once
#include <Arduino.h>

struct RelayCfg {
  uint8_t sensorIdx;
  float   tOn;
  float   tOff;
  uint8_t mode;
  bool    state;
};

extern RelayCfg relays[2];
extern float    outdoorMinT;
extern bool     outdoorEnabled;
extern bool     systemBlocked;

void loadSettings();
void saveSettings();
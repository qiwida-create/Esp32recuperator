#pragma once
#include <Arduino.h>
#include "config.h"

extern float dhtT[2];
extern float dhtH[2];
extern float dsT[DS_COUNT];

void  initSensors();
void  readSensors();
float getOutdoorTemp();
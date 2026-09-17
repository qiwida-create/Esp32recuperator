#include "storage.h"
#include "config.h"
#include <Preferences.h>

Preferences prefs;

RelayCfg relays[2] = {
  {0, 30.0f, 28.0f, MODE_AUTO, false},
  {1, 40.0f, 38.0f, MODE_AUTO, false}
};

float outdoorMinT    = -25.0f;
bool  outdoorEnabled = true;
bool  systemBlocked  = false;

void loadSettings() {
  prefs.begin("meteo", true);
  relays[0].sensorIdx = prefs.getUChar("r1_s", 0);
  relays[0].tOn       = prefs.getFloat("r1_on", 30.0f);
  relays[0].tOff      = prefs.getFloat("r1_off", 28.0f);
  relays[0].mode      = prefs.getUChar("r1_mode", MODE_AUTO);
  relays[1].sensorIdx = prefs.getUChar("r2_s", 1);
  relays[1].tOn       = prefs.getFloat("r2_on", 40.0f);
  relays[1].tOff      = prefs.getFloat("r2_off", 38.0f);
  relays[1].mode      = prefs.getUChar("r2_mode", MODE_AUTO);
  outdoorMinT    = prefs.getFloat("o_min", -25.0f);
  outdoorEnabled = prefs.getBool ("o_en",  true);
  prefs.end();

  if (relays[0].sensorIdx > 3) relays[0].sensorIdx = 0;
  if (relays[1].sensorIdx > 3) relays[1].sensorIdx = 0;
  if (relays[0].mode > 3) relays[0].mode = MODE_AUTO;
  if (relays[1].mode > 3) relays[1].mode = MODE_AUTO;
}

void saveSettings() {
  prefs.begin("meteo", false);
  prefs.putUChar("r1_s",   relays[0].sensorIdx);
  prefs.putFloat("r1_on",  relays[0].tOn);
  prefs.putFloat("r1_off", relays[0].tOff);
  prefs.putUChar("r1_mode",relays[0].mode);
  prefs.putUChar("r2_s",   relays[1].sensorIdx);
  prefs.putFloat("r2_on",  relays[1].tOn);
  prefs.putFloat("r2_off", relays[1].tOff);
  prefs.putUChar("r2_mode",relays[1].mode);
  prefs.putFloat("o_min",  outdoorMinT);
  prefs.putBool ("o_en",   outdoorEnabled);
  prefs.end();
}
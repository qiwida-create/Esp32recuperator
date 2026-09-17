#include "relays.h"
#include "config.h"
#include "sensors.h"
#include "storage.h"

void initRelays() {
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  digitalWrite(RELAY2_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
}

void updateRelays() {
  bool blocked = false;
  if (outdoorEnabled) {
    float ot = getOutdoorTemp();
    if (!isnan(ot) && ot < outdoorMinT) blocked = true;
  }
  systemBlocked = blocked;

  for (int i = 0; i < 2; i++) {
    if (blocked) {
      relays[i].state = false;
    } else if (relays[i].mode == MODE_MANUAL_ON) {
      relays[i].state = true;
    } else if (relays[i].mode == MODE_MANUAL_OFF) {
      relays[i].state = false;
    } else if (relays[i].mode == MODE_WINDOW) {
      float t = dsT[relays[i].sensorIdx];
      if (!isnan(t)) {
        float lo = min(relays[i].tOn, relays[i].tOff);
        float hi = max(relays[i].tOn, relays[i].tOff);
        const float H = 0.5f;
        if (t > lo + H && t < hi - H) relays[i].state = true;
        else if (t < lo || t > hi)    relays[i].state = false;
      }
    } else {
      float t = dsT[relays[i].sensorIdx];
      if (!isnan(t)) {
        if (relays[i].tOn > relays[i].tOff) {
          if (t >= relays[i].tOn)       relays[i].state = true;
          else if (t <= relays[i].tOff) relays[i].state = false;
        } else {
          if (t <= relays[i].tOn)       relays[i].state = true;
          else if (t >= relays[i].tOff) relays[i].state = false;
        }
      }
    }
    int pin = (i == 0) ? RELAY1_PIN : RELAY2_PIN;
    if (RELAY_ACTIVE_LOW) digitalWrite(pin, relays[i].state ? LOW  : HIGH);
    else                  digitalWrite(pin, relays[i].state ? HIGH : LOW);
  }
}
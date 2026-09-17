/*
 * Метеостанция + 2 реле + джойстик + Wi-Fi
 * Модульная версия (Этап 1: разделение на модули)
 */

#include "config.h"
#include "storage.h"
#include "sensors.h"
#include "relays.h"
#include "display.h"
#include "web.h"
#include "wifi.h"

#include <SPI.h>

void setup() {
  Serial.begin(115200);
  Serial.println("\nMeteostation starting...");

  loadSettings();

  initRelays();
  initDisplay();
  initSensors();

  startWiFiAP();
  setupWebServer();

  readSensors();
  updateRelays();

  drawMainScreen();

  Serial.print("Free heap after setup: ");
  Serial.println(ESP.getFreeHeap());
}

void loop() {
  handleWebServer();

  static unsigned long lastRead = 0;
  unsigned long now = millis();

  handleJoystick();

  if (now - lastRead >= 2000) {
    lastRead = now;
    readSensors();
    updateRelays();
    if (currentScreen == SCREEN_MAIN) drawMainScreen();
  }
}
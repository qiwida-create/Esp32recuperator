#include "wifi.h"
#include "config.h"
#include <WiFi.h>

void startWiFiAP() {
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  bool ok = WiFi.softAP(AP_SSID, AP_PASS, 6);
  if (ok) {
    Serial.print("AP started. IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("AP start FAILED!");
  }
}
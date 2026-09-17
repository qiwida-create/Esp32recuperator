#include "sensors.h"
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

DHT dhtOut(DHT22_PIN, DHTTYPE_OUT);
DHT dhtIn (DHT11_PIN, DHTTYPE_IN);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float dhtT[2]       = {NAN, NAN};
float dhtH[2]       = {NAN, NAN};
float dsT[DS_COUNT] = {NAN, NAN, NAN, NAN};

void initSensors() {
  dhtOut.begin();
  dhtIn.begin();
  sensors.begin();
  sensors.setResolution(12);
}

void readSensors() {
  dhtT[0] = dhtOut.readTemperature();
  dhtH[0] = dhtOut.readHumidity();
  dhtT[1] = dhtIn.readTemperature();
  dhtH[1] = dhtIn.readHumidity();

  sensors.requestTemperatures();
  for (int i = 0; i < DS_COUNT; i++) {
    float t = sensors.getTempCByIndex(i);
    if (t == DEVICE_DISCONNECTED_C || t < -100.0f) dsT[i] = NAN;
    else dsT[i] = t;
  }
}

float getOutdoorTemp() { return dhtT[0]; }
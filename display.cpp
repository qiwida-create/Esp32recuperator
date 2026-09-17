#include "display.h"
#include "config.h"
#include "sensors.h"
#include "storage.h"
#include "relays.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

Screen currentScreen = SCREEN_MAIN;
int menuCursor = 0;
const int MENU_ITEMS = 11;

// ---------- Джойстик ----------
enum JoyKey { KEY_NONE = 0, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_OK };

JoyKey stableKey    = KEY_NONE;
JoyKey releasedKey  = KEY_NONE;
unsigned long stableSince = 0;
const unsigned long STABLE_MS  = 100;
const unsigned long KEY_REPEAT = 200;

// ---------- Кэш отрисовки ----------
int  prevDhtT[2]      = {999, 999};
int  prevDhtH[2]      = {999, 999};
int  prevDsT[4]       = {999, 999, 999, 999};
int  prevRelaySns[2]  = {255, 255};
int  prevRelayMode[2] = {255, 255};
bool prevRelayState[2]= {false, false};
bool prevBlocked      = false;
bool screenDirty      = true;

// ---------- Прототипы (forward declarations) ----------
void changeMenuItem(int dir);
void onKeyPress(JoyKey k);
JoyKey readJoyRaw();
JoyKey readJoyStable();
int readJoyMedian();

// ============================================================
void initDisplay() {
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(JOY_PIN, INPUT);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(COLOR_BG);

  screenDirty = true;
}

// ============================================================
// Основной экран
// ============================================================
void drawMainScreen() {
  if (screenDirty) {
    tft.fillScreen(COLOR_BG);
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TITLE, COLOR_BG);
    tft.setCursor(30, 2);
    tft.print("METEO STATION");
    tft.drawFastHLine(0, 12, tft.width(), COLOR_TITLE);

    tft.setTextColor(COLOR_LABEL, COLOR_BG);
    tft.setCursor(2, 14); tft.print("DHT22 OUT:");
    tft.setCursor(2, 44); tft.print("DHT11 IN :");
    tft.setCursor(2, 74); tft.print("DS18B20:");
  }

  int t1 = isnan(dhtT[0]) ? -1000 : (int)dhtT[0];
  int h1 = isnan(dhtH[0]) ? -1000 : (int)dhtH[0];
  if (screenDirty || t1 != prevDhtT[0] || h1 != prevDhtH[0]) {
    prevDhtT[0] = t1; prevDhtH[0] = h1;
    tft.setTextSize(2);
    tft.setCursor(2, 24);
    char buf[16];
    if (isnan(dhtT[0]) || isnan(dhtH[0])) {
      tft.setTextColor(COLOR_ERROR, COLOR_BG);
      snprintf(buf, sizeof(buf), "T:--- H:-- ");
    } else {
      tft.setTextColor(COLOR_DHT1, COLOR_BG);
      int tv = (int)dhtT[0]; int hv = (int)dhtH[0];
      if (tv < -99 || tv > 99) tv = 0;
      if (hv < -99 || hv > 99) hv = 0;
      snprintf(buf, sizeof(buf), "T:%3d H:%2d ", tv, hv);
    }
    tft.print(buf);
  }

  int t2 = isnan(dhtT[1]) ? -1000 : (int)dhtT[1];
  int h2 = isnan(dhtH[1]) ? -1000 : (int)dhtH[1];
  if (screenDirty || t2 != prevDhtT[1] || h2 != prevDhtH[1]) {
    prevDhtT[1] = t2; prevDhtH[1] = h2;
    tft.setTextSize(2);
    tft.setCursor(2, 54);
    char buf[16];
    if (isnan(dhtT[1]) || isnan(dhtH[1])) {
      tft.setTextColor(COLOR_ERROR, COLOR_BG);
      snprintf(buf, sizeof(buf), "T:--- H:-- ");
    } else {
      tft.setTextColor(COLOR_DHT2, COLOR_BG);
      int tv = (int)dhtT[1]; int hv = (int)dhtH[1];
      if (tv < -99 || tv > 99) tv = 0;
      if (hv < -99 || hv > 99) hv = 0;
      snprintf(buf, sizeof(buf), "T:%3d H:%2d ", tv, hv);
    }
    tft.print(buf);
  }

  for (int i = 0; i < DS_COUNT; i++) {
    int v = isnan(dsT[i]) ? -100000 : (int)(dsT[i] * 10);
    if (screenDirty || v != prevDsT[i]) {
      prevDsT[i] = v;
      int col = i % 2, row = i / 2;
      int x = 4 + col * 78;
      int y = 86 + row * 14;
      tft.setTextSize(1);
      tft.setCursor(x, y);
      char buf[16];
      if (isnan(dsT[i])) {
        tft.setTextColor(COLOR_ERROR, COLOR_BG);
        snprintf(buf, sizeof(buf), "%d: ERR   ", i + 1);
      } else {
        tft.setTextColor(COLOR_DS, COLOR_BG);
        float tv = dsT[i];
        if (tv < -99.9f) tv = -99.9f;
        if (tv > 999.9f) tv = 999.9f;
        snprintf(buf, sizeof(buf), "%d:%6.1fC", i + 1, tv);
      }
      tft.print(buf);
    }
  }

  // ----- Статус реле -----
  for (int i = 0; i < 2; i++) {
    if (!screenDirty &&
        prevRelaySns[i]   == relays[i].sensorIdx &&
        prevRelayState[i] == relays[i].state &&
        prevRelayMode[i]  == relays[i].mode &&
        prevBlocked       == systemBlocked) continue;

    prevRelaySns[i]   = relays[i].sensorIdx;
    prevRelayState[i] = relays[i].state;
    prevRelayMode[i]  = relays[i].mode;

    int x = (i == 0) ? 2 : 82;
    tft.setTextSize(1);
    tft.setCursor(x, 118);
    tft.setTextColor(COLOR_LABEL, COLOR_BG);

    char mc;
    if      (relays[i].mode == MODE_AUTO)   mc = 'A';
    else if (relays[i].mode == MODE_WINDOW) mc = 'W';
    else                                    mc = 'M';

    char buf[16];
    snprintf(buf, sizeof(buf), "R%d%c>S%d:", i + 1, mc, relays[i].sensorIdx + 1);
    tft.print(buf);

    if (systemBlocked) {
      tft.setTextColor(COLOR_BLOCK, COLOR_BG);
      tft.print("OFF*");
    } else {
      tft.setTextColor(relays[i].state ? COLOR_ON : COLOR_OFF, COLOR_BG);
      tft.print(relays[i].state ? "ON " : "OFF");
    }
  }
  prevBlocked = systemBlocked;
  screenDirty = false;
}

// ============================================================
// Меню
// ============================================================
void drawMenu() {
  tft.fillScreen(COLOR_BG);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_TITLE, COLOR_BG);
  tft.setCursor(2, 2);
  tft.print("MENU (OK=save)");
  tft.drawFastHLine(0, 12, 160, COLOR_TITLE);

  const char* labels[MENU_ITEMS] = {
    "R1 Mode", "R1 Sns ", "R1 On  ", "R1 Off ",
    "R2 Mode", "R2 Sns ", "R2 On  ", "R2 Off ",
    "Out T  ", "Out EN ", "Save & Exit"
  };
  const char* modeNames[4] = {"Auto", "ON  ", "OFF ", "Wind"};

  for (int i = 0; i < MENU_ITEMS; i++) {
    int y = 14 + i * 10;
    tft.setCursor(2, y);
    tft.setTextColor(i == menuCursor ? COLOR_SEL : COLOR_LABEL, COLOR_BG);
    tft.print(i == menuCursor ? ">" : " ");
    tft.setCursor(12, y);
    tft.print(labels[i]);
    tft.print(":");

    if (i < MENU_ITEMS - 1) {
      tft.setTextColor(COLOR_LABEL, COLOR_BG);
      switch (i) {
        case 0: tft.print(modeNames[relays[0].mode]); break;
        case 1: tft.print((int)relays[0].sensorIdx + 1); break;
        case 2: tft.print(relays[0].tOn, 1);  tft.print("C"); break;
        case 3: tft.print(relays[0].tOff, 1); tft.print("C"); break;
        case 4: tft.print(modeNames[relays[1].mode]); break;
        case 5: tft.print((int)relays[1].sensorIdx + 1); break;
        case 6: tft.print(relays[1].tOn, 1);  tft.print("C"); break;
        case 7: tft.print(relays[1].tOff, 1); tft.print("C"); break;
        case 8: tft.print(outdoorMinT, 1); tft.print("C"); break;
        case 9: tft.print(outdoorEnabled ? "ON " : "OFF"); break;
      }
    }
  }
}

// ============================================================
// Wi-Fi info
// ============================================================
void drawWiFiInfo() {
  tft.fillScreen(COLOR_BG);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_TITLE, COLOR_BG);
  tft.setCursor(2, 2);
  tft.print("Wi-Fi Info");
  tft.drawFastHLine(0, 12, 160, COLOR_TITLE);

  tft.setTextColor(COLOR_LABEL, COLOR_BG);
  tft.setCursor(4, 22);  tft.print("SSID:");
  tft.setCursor(4, 34);  tft.print(AP_SSID);
  tft.setCursor(4, 52);  tft.print("PASS:");
  tft.setCursor(4, 64);  tft.print(AP_PASS);
  tft.setCursor(4, 84);  tft.print("URL:");

  tft.setTextColor(COLOR_DS, COLOR_BG);
  tft.setCursor(4, 96);  tft.print("http://192.168.4.1");

  tft.setTextColor(COLOR_LABEL, COLOR_BG);
  tft.setCursor(4, 116); tft.print("OK=back");
}

// ============================================================
// Джойстик
// ============================================================
int readJoyMedian() {
  const int N = 9;
  int buf[N];
  for (int i = 0; i < N; i++) {
    buf[i] = analogRead(JOY_PIN);
    delayMicroseconds(150);
  }
  for (int i = 1; i < N; i++) {
    int key = buf[i], j = i - 1;
    while (j >= 0 && buf[j] > key) { buf[j + 1] = buf[j]; j--; }
    buf[j + 1] = key;
  }
  return buf[N / 2];
}

JoyKey readJoyRaw() {
  int v = readJoyMedian();
  static int lastV = -1;
  static unsigned long lastPrint = 0;
  if (abs(v - lastV) > 80 && millis() - lastPrint > 200) {
    Serial.print("Joy median = "); Serial.println(v);
    lastV = v; lastPrint = millis();
  }
  if (v > 3700) return KEY_NONE;
  if (v > 2750) return KEY_OK;
  if (v > 1750) return KEY_RIGHT;
  if (v > 1050) return KEY_DOWN;
  if (v > 250)  return KEY_UP;
  return KEY_LEFT;
}

JoyKey readJoyStable() {
  JoyKey raw = readJoyRaw();
  unsigned long now = millis();
  if (raw == stableKey) { stableSince = now; return stableKey; }
  if (now - stableSince >= STABLE_MS) {
    stableKey = raw; stableSince = now; return stableKey;
  }
  return stableKey;
}

void onKeyPress(JoyKey k) {
  if (currentScreen == SCREEN_MAIN) {
    if (k == KEY_OK) {
      currentScreen = SCREEN_MENU;
      menuCursor = 0;
      drawMenu();
    } else if (k == KEY_UP) {
      currentScreen = SCREEN_WIFI_INFO;
      drawWiFiInfo();
    }
    return;
  }
  if (currentScreen == SCREEN_WIFI_INFO) {
    if (k == KEY_OK || k == KEY_DOWN) {
      currentScreen = SCREEN_MAIN;
      screenDirty = true;
      drawMainScreen();
    }
    return;
  }
  // SCREEN_MENU
  if (k == KEY_UP) {
    menuCursor = (menuCursor - 1 + MENU_ITEMS) % MENU_ITEMS; drawMenu();
  } else if (k == KEY_DOWN) {
    menuCursor = (menuCursor + 1) % MENU_ITEMS; drawMenu();
  } else if (k == KEY_LEFT) {
    changeMenuItem(-1); drawMenu(); updateRelays();
  } else if (k == KEY_RIGHT) {
    changeMenuItem(+1); drawMenu(); updateRelays();
  } else if (k == KEY_OK) {
    if (menuCursor == MENU_ITEMS - 1) {
      saveSettings();
      currentScreen = SCREEN_MAIN;
      screenDirty = true;
      drawMainScreen();
    }
  }
}

void changeMenuItem(int dir) {
  float step = 0.5f;
  switch (menuCursor) {
    case 0: {
      int m = (int)relays[0].mode + dir;
      if (m < 0) m = 3; if (m > 3) m = 0;
      relays[0].mode = (uint8_t)m; break;
    }
    case 1: {
      int v = (int)relays[0].sensorIdx + dir;
      if (v < 0) v = 3; if (v > 3) v = 0;
      relays[0].sensorIdx = (uint8_t)v; break;
    }
    case 2: { relays[0].tOn  += dir*step; if(relays[0].tOn>100)relays[0].tOn=100; if(relays[0].tOn<-50)relays[0].tOn=-50; break; }
    case 3: { relays[0].tOff += dir*step; if(relays[0].tOff>100)relays[0].tOff=100; if(relays[0].tOff<-50)relays[0].tOff=-50; break; }
    case 4: {
      int m = (int)relays[1].mode + dir;
      if (m < 0) m = 3; if (m > 3) m = 0;
      relays[1].mode = (uint8_t)m; break;
    }
    case 5: {
      int v = (int)relays[1].sensorIdx + dir;
      if (v < 0) v = 3; if (v > 3) v = 0;
      relays[1].sensorIdx = (uint8_t)v; break;
    }
    case 6: { relays[1].tOn  += dir*step; if(relays[1].tOn>100)relays[1].tOn=100; if(relays[1].tOn<-50)relays[1].tOn=-50; break; }
    case 7: { relays[1].tOff += dir*step; if(relays[1].tOff>100)relays[1].tOff=100; if(relays[1].tOff<-50)relays[1].tOff=-50; break; }
    case 8: {
      outdoorMinT += dir * 1.0f;
      if (outdoorMinT > 60.0f)  outdoorMinT = 60.0f;
      if (outdoorMinT < -40.0f) outdoorMinT = -40.0f;
      break;
    }
    case 9: { outdoorEnabled = !outdoorEnabled; break; }
    default: break;
  }
}

void handleJoystick() {
  JoyKey k = readJoyStable();
  unsigned long now = millis();
  static unsigned long lastEvent = 0;
  if (k != KEY_NONE) {
    if (releasedKey == KEY_NONE && (now - lastEvent) > KEY_REPEAT) {
      lastEvent = now;
      releasedKey = k;
      onKeyPress(k);
    }
  } else {
    releasedKey = KEY_NONE;
  }
}
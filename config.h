#pragma once

// ---------- Пины TFT ----------
#define TFT_CS    5
#define TFT_RST   4
#define TFT_DC    2
#define TFT_MOSI  23
#define TFT_SCLK  18

// ---------- Пины датчиков ----------
#define DHT22_PIN   15
#define DHT11_PIN   16
#define DHTTYPE_OUT DHT22
#define DHTTYPE_IN  DHT11
#define ONE_WIRE_BUS 13
#define DS_COUNT     4

// ---------- Реле ----------
#define RELAY1_PIN 32
#define RELAY2_PIN 33
#define RELAY_ACTIVE_LOW true

// ---------- Джойстик ----------
#define JOY_PIN 34

// ---------- Режимы реле ----------
#define MODE_AUTO        0
#define MODE_MANUAL_ON   1
#define MODE_MANUAL_OFF  2
#define MODE_WINDOW      3

// ---------- Wi-Fi точка доступа ----------
#define AP_SSID "MeteoStation"
#define AP_PASS "meteo1234"

// ---------- Цвета (значения ST77XX_*) ----------
#define COLOR_BG    0x0000  // BLACK
#define COLOR_TITLE 0x07FF  // CYAN
#define COLOR_LABEL 0xFFFF  // WHITE
#define COLOR_DHT1  0xFFE0  // YELLOW
#define COLOR_DHT2  0xF81F  // MAGENTA
#define COLOR_DS    0xFD20  // ORANGE
#define COLOR_ERROR 0xF800  // RED
#define COLOR_SEL   0x07E0  // GREEN
#define COLOR_ON    0x07E0  // GREEN
#define COLOR_OFF   0xFFFF  // WHITE
#define COLOR_BLOCK 0xF800  // RED
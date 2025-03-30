#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP32_SPI
#define FASTLED_ESP32_SPI_PINS 1

#include "misc/pin_config.h"
#include "misc/pinconfig.h"
#include <TFT_eSPI.h>
#include <OneButton.h>
#include <FastLED.h>
#include <Preferences.h>
#include "EEPROM.h"
#include "ble/ble.h"
#include <WebServer.h>
#include "display/display.h"
#include "display/inspiration.h"
#include "wifi/time_get.h"

extern void startWebPortal();
extern WebServer server;

OneButton button(BTN_PIN, true);
CRGB leds;
#define TOUCH_CS -1

unsigned long lastRestart = 0;
const unsigned long restartInterval = 60000;

void nextPosition() {}

void blinkLED() {
    leds = CRGB::White;
    FastLED.setBrightness(64);
    FastLED.show();
    delay(100);
    leds = CRGB::Black;
    FastLED.show();
}

void setup() {
    // necessary to keep BLE running
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_sleep_enable_timer_wakeup(0);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    pinMode(TFT_LEDA_PIN, OUTPUT);
    digitalWrite(TFT_LEDA_PIN, 1);

    Serial.begin(115200);
    EEPROM.begin(1);

    // Initialize TFT display and LVGL
    initDisplay();

    // setup the LED
    leds = CRGB(0, 0, 0);
    FastLED.addLeds<APA102, LED_DI_PIN, LED_CI_PIN, BGR>(&leds, 1);
    FastLED.show();

    // setup the button
    button.attachClick(nextPosition);
    digitalWrite(TFT_LEDA_PIN, 0);

    // Start config portal
    startWebPortal();

    // Load hotspot name and use for ID and beacon name
    Preferences prefs;
    prefs.begin("config", true);
    String suffix = prefs.getString("wifi_hotspot_name", "T-Dongle");
    prefs.end();
    String id = suffix;
    String beaconName = "geogram-" + suffix;
    String msg = "";

    // Start BLE beacon
    startBeacon(beaconName.c_str(), id.c_str(), 0x1234, msg.c_str());

    // Init time
    initTime();

    // all done
    blinkLED();
    lastRestart = millis();

    generateInspiration();
}

void loop() {
    button.tick();
    server.handleClient();
    updateDisplay();  // needed for LVGL animations/input
    updateTime(); 
    delay(5);

    if (millis() - lastRestart > restartInterval) {
        restartBeacon();
        lastRestart = millis();
    }
}

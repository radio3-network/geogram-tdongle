#include <TFT_eSPI.h>
#include <OneButton.h>
#include <FastLED.h>
#include "EEPROM.h"
#include "pinconfig.h"
#include "Config.h"
#include "ble.h"

// UI
TFT_eSPI screen = TFT_eSPI();
OneButton button(BTN_PIN, true);
CRGB leds;
#define TOUCH_CS -1

// Timing
unsigned long lastRestart = 0;
const unsigned long restartInterval = 60000; // 60 sec

void nextPosition() {
    // Placeholder for button action
}

void blinkLED() {
    leds = CRGB::White;
    FastLED.show();
    delay(100);
    leds = CRGB::Black;
    FastLED.show();
}

void setup() {
    // Disable sleep
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_sleep_enable_timer_wakeup(0);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    // IO
    pinMode(TFT_LEDA_PIN, OUTPUT);
    digitalWrite(TFT_LEDA_PIN, 1);

    Serial.begin(115200);
    EEPROM.begin(1);

    // Screen
    screen.begin();
    screen.setRotation(1);
    screen.setSwapBytes(true);
    screen.fillScreen(TFT_BLACK);
    screen.initDMA();

    // LED
    leds = CRGB(0,0,0);
    FastLED.addLeds<APA102, LED_DI_PIN, LED_CI_PIN, BGR>(&leds, 1);
    FastLED.show();

    // Button
    button.attachClick(nextPosition);
    digitalWrite(TFT_LEDA_PIN, 0); // enable backlight

    // Config + Beacon
    config.load();
    startBeacon(config.beaconName.c_str(),
                config.deviceId.c_str(),
                config.manufacturerId,
                config.payload.c_str());
    blinkLED(); // startup signal
    lastRestart = millis();
}

void loop() {
    button.tick();

    if (millis() - lastRestart > restartInterval) {
        restartBeacon();
        blinkLED();
        lastRestart = millis();
    }
}

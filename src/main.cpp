#include <TFT_eSPI.h>
#include <OneButton.h>
#include <FastLED.h>
#include "EEPROM.h"
#include "pinconfig.h"


TFT_eSPI screen = TFT_eSPI();
OneButton button(BTN_PIN, true);
CRGB leds;


void nextPosition() {
}

void setup() {
    // Hardware initialization
    pinMode(TFT_LEDA_PIN, OUTPUT);
    digitalWrite(TFT_LEDA_PIN, 1);
    
 
    // Serial and EEPROM
    Serial.begin(115200);
    EEPROM.begin(1);
 
    // Screen setup
    screen.begin();
    screen.setRotation(0);
    screen.setSwapBytes(true);
    screen.fillScreen(TFT_WHITE);
    screen.initDMA();

    // LED setup
    leds = CRGB(0,0,0);
    FastLED.addLeds<APA102, LED_DI_PIN, LED_CI_PIN, BGR>(&leds, 1);
    FastLED.show();

    // Button setup
    button.attachClick(nextPosition);
    // turn on the LED
    digitalWrite(TFT_LEDA_PIN, 0);
}

void loop() {
    // Button handling
    button.tick();
    
}
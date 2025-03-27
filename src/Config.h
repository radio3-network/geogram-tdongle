#pragma once
#include <Arduino.h>

struct Config {
    String beaconName;
    String deviceId;
    uint16_t manufacturerId;
    String payload;

    void load();  // Load from flash
    void save();  // Save to flash
};

extern Config config;

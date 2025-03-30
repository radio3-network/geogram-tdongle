#include "Config.h"
#include <Preferences.h>

Config config;

void Config::load() {
    Preferences prefs;
    prefs.begin("beacon", true); // read-only
    beaconName     = prefs.getString("name", "T-Dongle");
    deviceId       = prefs.getString("id", "NODE-01");
    manufacturerId = prefs.getUShort("mfg", 0x1234);
    payload        = prefs.getString("msg", "Hello from ESP32");
    prefs.end();
}

void Config::save() {
    Preferences prefs;
    prefs.begin("beacon", false); // read-write
    prefs.putString("name", beaconName);
    prefs.putString("id", deviceId);
    prefs.putUShort("mfg", manufacturerId);
    prefs.putString("msg", payload);
    prefs.end();
}

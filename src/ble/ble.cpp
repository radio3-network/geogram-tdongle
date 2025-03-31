#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include <BLEScan.h>
#include <TFT_eSPI.h>
#include <Preferences.h>
#include "apps/presence.h"
#include "wifi/time_get.h"

extern TFT_eSPI screen;

// Persistent advertising object
static BLEAdvertising* adv = nullptr;
static BLEAdvertisementData advData;

void scanNearbyEddystoneUIDs() {
    BLEScan* scanner = BLEDevice::getScan();
    scanner->setActiveScan(true);
    scanner->setInterval(100);
    scanner->setWindow(99);

    BLEScanResults results = scanner->start(5, false); // 5 seconds scan
    int count = results.getCount();

    Serial.printf("Found %d BLE devices\n", count);
    int eddystoneCount = 0;

    for (int i = 0; i < count; ++i) {
        BLEAdvertisedDevice d = results.getDevice(i);
        if (d.haveServiceData()) {
            std::string data = d.getServiceData();
            if (d.haveServiceUUID() && d.getServiceUUID().toString() == "0000feaa-0000-1000-8000-00805f9b34fb") {
                uint8_t frameType = data[0];
                if (frameType == 0x00 && data.length() >= 18) { // UID frame
                    eddystoneCount++;
                    Serial.println("Eddystone UID found:");
                    Serial.print("  Namespace ID: ");
                    char deviceId[21] = {0}; // 10 bytes x 2 + null
                    for (int j = 2; j < 12; ++j) {
                        Serial.printf("%02X", (uint8_t)data[j]);
                        sprintf(&deviceId[(j - 2) * 2], "%02X", (uint8_t)data[j]);
                    }

                    Serial.print("\n  Instance ID: ");
                    for (int j = 12; j < 18; ++j) {
                        Serial.printf("%02X", (uint8_t)data[j]);
                    }
                    Serial.println();

                    updatePresence(String(deviceId), getCurrentTimestamp());
                }
            }
        }
    }

    Preferences prefs;
    prefs.begin("stats", false);
    prefs.putInt("users_detected", eddystoneCount);
    prefs.end();
    Serial.printf("[BLE] Eddystone devices detected: %d\n", eddystoneCount);

    scanner->clearResults();
}

void startBeacon(const char* beaconName, const char* deviceId, uint16_t manufacturerId, const char* payload) {
    BLEDevice::init(beaconName);
    BLEServer* pServer = BLEDevice::createServer();
    adv = BLEDevice::getAdvertising();

    advData = BLEAdvertisementData(); // Reset
    advData.setFlags(0x06); // General Discoverable, BR/EDR Not Supported

    std::string mfgData;
    mfgData += (char)(manufacturerId & 0xFF);
    mfgData += (char)((manufacturerId >> 8) & 0xFF);
    if (deviceId) {
        mfgData += "[";
        mfgData += deviceId;
        mfgData += "] ";
    }
    mfgData += payload;

    advData.addData(std::string("\xFF") + mfgData);

    adv->setAdvertisementData(advData);
    adv->start();

    Serial.println("Beacon started.");
}

void restartBeacon() {
    if (adv) {
        adv->stop();
        delay(10);
        scanNearbyEddystoneUIDs();
        delay(10);
        adv->start();
    }
}

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include <BLEScan.h>
#include <TFT_eSPI.h>

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
    for (int i = 0; i < count; ++i) {
        BLEAdvertisedDevice d = results.getDevice(i);
        if (d.haveServiceData()) {
            std::string data = d.getServiceData();
            if (d.haveServiceUUID() && d.getServiceUUID().toString() == "0000feaa-0000-1000-8000-00805f9b34fb") {
                uint8_t frameType = data[0];
                if (frameType == 0x00 && data.length() >= 18) { // UID frame
                    Serial.println("Eddystone UID found:");
                    Serial.print("  Namespace ID: ");
                    for (int j = 2; j < 12; ++j) {
                        Serial.printf("%02X", (uint8_t)data[j]);
                    }
                    Serial.print("\n  Instance ID: ");
                    for (int j = 12; j < 18; ++j) {
                        Serial.printf("%02X", (uint8_t)data[j]);
                    }
                    Serial.println();
                }
            }
        }
    }

    scanner->clearResults();
}

void startBeacon(const char* beaconName, const char* deviceId, uint16_t manufacturerId, const char* payload) {
    BLEDevice::init(beaconName);
    BLEServer* pServer = BLEDevice::createServer();
    adv = BLEDevice::getAdvertising();

    advData = BLEAdvertisementData(); // Reset
    advData.setFlags(0x06); // General Discoverable, BR/EDR Not Supported

    // Construct manufacturer-specific payload
    std::string mfgData;
    mfgData += (char)(manufacturerId & 0xFF);        // LSB
    mfgData += (char)((manufacturerId >> 8) & 0xFF); // MSB
    if (deviceId) {
        mfgData += "[";
        mfgData += deviceId;
        mfgData += "] ";
    }
    mfgData += payload;

    advData.addData(std::string("\xFF") + mfgData); // 0xFF = Manufacturer Specific Data

    adv->setAdvertisementData(advData);
    adv->start();

    Serial.println("Beacon started.");
}

void restartBeacon() {
    if (adv) {
        adv->stop();
        delay(10);
        scanNearbyEddystoneUIDs();  // Scan before restarting beacon
        delay(10);
        adv->start();
    }
}

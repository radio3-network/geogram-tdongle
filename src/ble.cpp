#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include <TFT_eSPI.h>

extern TFT_eSPI screen;

// Persistent advertising object
static BLEAdvertising* adv = nullptr;
static BLEAdvertisementData advData;

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

    /*
    String mac = BLEDevice::getAddress().toString().c_str();
    Serial.println("BLE MAC address: " + mac);
    screen.fillScreen(TFT_BLACK);
    screen.setTextColor(TFT_WHITE, TFT_BLACK);
    screen.setTextSize(1);
    screen.setCursor(10, 5);
    screen.print("BLE: " + mac);
    screen.setCursor(10, 20);
    screen.print("ID: " + String(deviceId));
    screen.setCursor(10, 35);
    screen.print("MSG:");
    screen.setCursor(10, 45);
    screen.print(payload);
*/

    Serial.println("Beacon started.");
}

void restartBeacon() {
    if (adv) {
        adv->stop();
        delay(10);
        adv->start();
        //Serial.println("Beacon restarted.");
    }
}
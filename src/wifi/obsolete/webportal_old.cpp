#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <esp_bt_device.h>
#include "../time_get.h"
#include "../webfiles.h"
#include "API/API.h"
/*
WebServer server(80);
Preferences prefs;

void handleFileRequest() {
    String path = server.uri();
    if (path == "/") path = "/index.html";

    if (LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        String contentType = "text/plain";
        if (path.endsWith(".html")) contentType = "text/html";
        else if (path.endsWith(".css")) contentType = "text/css";
        else if (path.endsWith(".js")) contentType = "application/javascript";

        server.streamFile(file, contentType);
        file.close();
    } else {
        server.send(404, "text/plain", "File Not Found");
    }
}

void handleSave() {
    if (server.method() != HTTP_POST) {
        server.send(405, "text/plain", "Method Not Allowed");
        return;
    }

    std::vector<std::pair<String, String>> args;
    for (int i = 0; i < server.args(); ++i) {
        args.emplace_back(server.argName(i), server.arg(i));
    }

    auto result = handleRequest("/config/set", args);

    bool ok = false;
    for (const auto& kv : result) {
        if (kv.first == "status" && kv.second == "ok") {
            ok = true;
            break;
        }
    }

    if (ok) {
        server.send(200, "text/plain", "OK");
        delay(500);
        ESP.restart();
    } else {
        String json = "{";
        for (size_t i = 0; i < result.size(); ++i) {
            json += "\"" + result[i].first + "\":\"" + result[i].second + "\"";
            if (i < result.size() - 1) json += ",";
        }
        json += "}";
        server.send(400, "application/json", json);
    }
}

void handleLoad() {
    auto result = handleRequest("/config/get", {});

    String json = "{";
    for (size_t i = 0; i < result.size(); ++i) {
        json += "\"" + result[i].first + "\":\"" + result[i].second + "\"";
        if (i < result.size() - 1) json += ",";
    }
    json += "}";

    server.send(200, "application/json", json);
}

void handleStatus() {
    unsigned long secs = millis() / 1000;
    unsigned long mins = secs / 60;
    unsigned long hrs = mins / 60;
    secs %= 60;
    mins %= 60;

    char uptime[32];
    snprintf(uptime, sizeof(uptime), "%lu:%02lu:%02lu", hrs, mins, secs);

    String wifiMac = WiFi.softAPmacAddress();

    const uint8_t* btMacRaw = esp_bt_dev_get_address();
    char btMac[18];
    snprintf(btMac, sizeof(btMac), "%02X:%02X:%02X:%02X:%02X:%02X",
             btMacRaw[0], btMacRaw[1], btMacRaw[2],
             btMacRaw[3], btMacRaw[4], btMacRaw[5]);

    String json = "{";
    json += "\"uptime\":\"" + String(uptime) + "\",";
    json += "\"wifi_mac\":\"" + wifiMac + "\",";
    json += "\"bt_mac\":\"" + String(btMac) + "\"";
    json += "}";

    server.send(200, "application/json", json);
}

void setupCaptivePortalRoutes() {
    server.on("/generate_204", []() {
        server.sendHeader("Location", "/index.html", true);
        server.send(302, "text/plain", "");
    });
    server.on("/hotspot-detect.html", []() {
        server.sendHeader("Location", "/index.html", true);
        server.send(302, "text/html", "");
    });
    server.on("/ncsi.txt", []() {
        server.send(200, "text/plain", "Microsoft NCSI");
    });
    server.on("/", []() {
        server.sendHeader("Location", "/index.html", true);
        server.send(302, "text/plain", "");
    });
}

void startWebPortal() {
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed");
        return;
    }

    prefs.begin("config", true);
    String suffix = prefs.getString("wifi_hotspot_name", "");
    String wifi_ssid = prefs.getString("wifi_ssid", "");
    String wifi_password = prefs.getString("wifi_password", "");
    prefs.end();

    String hotspotSSID = (suffix.length() > 0) ? ("geogram-" + suffix) : "geogram";
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(hotspotSSID.c_str(), "");

    if (wifi_ssid.length() > 0) {
        Serial.print("Connecting to Wi-Fi: ");
        Serial.println(wifi_ssid);
        WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 5000) {
            delay(100);
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("Connected, IP: ");
            Serial.println(WiFi.localIP());
            updateTime();
        } else {
            Serial.println("Wi-Fi connection failed.");
        }
    }

    setupCaptivePortalRoutes();
    server.on("/api/save", HTTP_POST, handleSave);
    server.on("/api/load", HTTP_GET, handleLoad);
    server.on("/api/status", HTTP_GET, handleStatus);
    setupFileBrowserRoutes();

    server.onNotFound(handleFileRequest);
    server.begin();

    Serial.println("Web portal active at: http://192.168.4.1");
}
*/
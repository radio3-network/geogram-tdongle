#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <Preferences.h>

WebServer server(80);
Preferences prefs;

void handleFileRequest() {
    String path = server.uri();
    Serial.println("Requesting: " + path);
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

    String deviceId = server.arg("deviceId");
    String payload = server.arg("payload");

    prefs.begin("config", false);
    prefs.putString("deviceId", deviceId);
    prefs.putString("payload", payload);
    prefs.end();

    server.send(200, "text/plain", "OK");
}

void handleLoad() {
    prefs.begin("config", true);
    String deviceId = prefs.getString("deviceId", "T-Dongle");
    String payload = prefs.getString("payload", "Hello");
    prefs.end();

    String json = "{\"deviceId\":\"" + deviceId + "\",\"payload\":\"" + payload + "\"}";
    server.send(200, "application/json", json);
}

void startWebPortal() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("T-Dongle", "");

    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed");
        return;
    }

    server.onNotFound(handleFileRequest);
    server.on("/api/save", HTTP_POST, handleSave);
    server.on("/api/load", HTTP_GET, handleLoad);
    server.begin();

    Serial.println("Web portal active at: http://192.168.4.1");
}

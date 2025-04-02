/**
 * @file webportal.cpp
 * @brief Async web portal implementation using ESPAsyncWebServer and LittleFS
 */

 #include <WiFi.h>
 #include <LittleFS.h>
 #include <Preferences.h>
 #include <AsyncTCP.h>
 #include <ESPAsyncWebServer.h>
 #include "time_get.h"
 #include <esp_bt_device.h>
 #include "../API/API.h"  // Added for /api/homepage handler
 
 static AsyncWebServer server(80);
 
 /**
  * @brief Sets up captive portal-style routes to redirect clients to index.html.
  */
 void setupCaptivePortalRoutes() {
     server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest* request) {
         request->redirect("/index.html");
     });
     server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest* request) {
         request->redirect("/index.html");
     });
     server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest* request) {
         request->send(200, "text/plain", "Microsoft NCSI");
     });
     server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
         request->redirect("/index.html");
     });
 }
 
 /**
  * @brief Serves static files from LittleFS. Defaults to index.html if path not found.
  */
 void setupStaticFileHandler() {
     server.onNotFound([](AsyncWebServerRequest* request) {
         String path = request->url();
         if (path == "/") path = "/index.html";
         if (LittleFS.exists(path)) {
             request->send(LittleFS, path, String(), false);
         } else {
             request->send(404, "text/plain", "File Not Found");
         }
     });
 }
 
 /**
  * @brief Handles uptime status response as JSON at /api/status
  */
/**
 * @brief Handles uptime status response as JSON at /api/status
 */
void setupStatusHandler() {
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        // Get status data from the API handler
        auto pairs = handleRequest("/status/get", {});
        
        // Convert to JSON format
        String jsonResponse = "{";
        bool first = true;
        
        for (const auto& pair : pairs) {
            if (!first) {
                jsonResponse += ",";
            }
            first = false;
            
            // Properly escape string values
            if (pair.first == "error" || pair.first == "status") {
                jsonResponse += "\"" + pair.first + "\":\"" + pair.second + "\"";
            } else {
                // For numeric values or special cases
                if (pair.second.length() == 0) {
                    // Empty fields should be null
                    jsonResponse += "\"" + pair.first + "\":null";
                } else if (pair.first == "uptime" || pair.first == "storage") {
                    // String values that aren't strictly numeric
                    jsonResponse += "\"" + pair.first + "\":\"" + pair.second + "\"";
                } else {
                    // Try to parse as number if possible
                    bool isNumeric = true;
                    for (unsigned int i = 0; i < pair.second.length(); i++) {
                        if (!isdigit(pair.second[i])) {
                            isNumeric = false;
                            break;
                        }
                    }
                    
                    if (isNumeric) {
                        jsonResponse += "\"" + pair.first + "\":" + pair.second;
                    } else {
                        jsonResponse += "\"" + pair.first + "\":\"" + pair.second + "\"";
                    }
                }
            }
        }
        jsonResponse += "}";
        
        // Send the response
        request->send(200, "application/json", jsonResponse);
    });
}
 
 /**
  * @brief Handles homepage API response at /api/homepage
  */
 void setupHomepageHandler() {
     server.on("/api/homepage", HTTP_GET, [](AsyncWebServerRequest* request) {
         auto pairs = handleRequest("/homepage", {});
         for (const auto& pair : pairs) {
             if (pair.first == "json") {
                 request->send(200, "application/json", pair.second);
                 return;
             }
         }
         request->send(500, "application/json", "{\"error\":\"failed to build homepage\"}");
     });
 }
 
 /**
  * @brief Starts the web portal, initializes FS, Wi-Fi, and Async server.
  */
 void startWebPortal() {
     if (!LittleFS.begin()) {
         Serial.println("LittleFS mount failed");
         return;
     }
 
     Preferences prefs;
     prefs.begin("config", true);
     String suffix = prefs.getString("wifi_hotspot_name", "");
     String wifi_ssid = prefs.getString("wifi_ssid", "");
     String wifi_password = prefs.getString("wifi_password", "");
     prefs.end();
 
     String hotspotSSID = (suffix.length() > 0) ? ("geogram-" + suffix) : "geogram";
 
     // Start Access Point
     WiFi.mode(WIFI_AP_STA);
     WiFi.softAP(hotspotSSID.c_str(), "");
 
     // Attempt STA Wi-Fi connection if credentials are saved
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
 
     // Register routes
     setupCaptivePortalRoutes();
     setupStaticFileHandler();
     setupStatusHandler();
     setupHomepageHandler();
 
     server.begin();
 
     Serial.println("Async Web portal active at: http://192.168.4.1");
 }
 
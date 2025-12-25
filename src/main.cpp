#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "secrets.h" // Contains server IP and WiFi credentials
#include "DisplayManager.h"
#include "WifiManager.h"

// --- GLOBAL VARIABLE FOR DYNAMIC HOST ---
// Holds the IP address selected based on the connected WiFi network.
const char *targetHost;

// --- GLOBAL OBJECTS ---
// The WebSocket client instance
WebSocketsClient webSocket;

// Memory buffer for JSON parsing.
StaticJsonDocument<200> doc;

// --- WEBSOCKET EVENT HANDLER ---
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    // CRITICAL: Declare variables at the top to avoid compiler errors
    DeserializationError error;
    const char *msgType;
    const char *mode;
    const char *macro;

    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.println("[WS] Disconnected!");

        // UX: Update the OLED immediately so the user knows server is down
        showConnectionStatus("Connection Lost!");
        break;

    case WStype_CONNECTED:
        Serial.println("[WS] Connected!");

        // Handshake: Send a simple JSON to tell the server who we are.
        webSocket.sendTXT("{\"type\":\"auth\", \"device\":\"ESP32_v1\"}");
        break;

    case WStype_TEXT:
        // This triggers when we receive a text message (JSON) from Python.
        Serial.printf("[WS] Recv: %s\n", payload);

        // 1. PARSE JSON
        error = deserializeJson(doc, payload);

        if (!error)
        {
            // 2. CHECK MESSAGE TYPE
            msgType = doc["type"];

            // If the message is a UI Update command...
            if (msgType && strcmp(msgType, "ui_update") == 0)
            {
                // 3. EXTRACT DATA
                mode = doc["mode"];   // e.g., "DEFAULT"
                macro = doc["macro"]; // e.g., "Desktop"

                // 4. UPDATE DISPLAY
                updateUI(mode, macro);
            }
        }
        else
        {
            Serial.println("JSON Parse Error!");
        }
        break;
    }
}

// --- SYSTEM SETUP ---
void setup()
{
    Serial.begin(115200);

    // 1. Initialize Hardware (OLED)
    initDisplay();
    showStartupScreen();
    delay(1000);

    // 2. Connect to Network
    // This function blocks until WiFi is connected.
    setupWiFi();

    // --- SMART HOST SELECTION LOGIC ---
    // Check WHICH network we are on to select the correct PC IP.

    String currentSSID = WiFi.SSID();

    Serial.print("Connected to SSID: ");
    Serial.println(currentSSID);

    if (currentSSID == WIFI_SSID_1)
    {
        // Scenario A: Primary Location (e.g. Home) -> IP .33
        targetHost = WS_SERVER_HOST_1;
        Serial.println("Location Detected: PRIMARY (Using Host 1)");
    }
    else if (currentSSID == WIFI_SSID_2)
    {
        // Scenario B: Secondary Location (e.g. Ankara) -> IP .20
        targetHost = WS_SERVER_HOST_2;
        Serial.println("Location Detected: SECONDARY (Using Host 2)");
    }
    else
    {
        // Fallback
        targetHost = WS_SERVER_HOST_1;
        Serial.println("Location Unknown: Defaulting to Host 1");
    }

    // 3. Connect to WebSocket Server
    Serial.print("Connecting to Server: ");
    Serial.print(targetHost);
    Serial.print(":");
    Serial.println(WS_SERVER_PORT);

    // Configure connection
    webSocket.begin(targetHost, WS_SERVER_PORT, "/");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
}

// --- MAIN LOOP ---
void loop()
{
    webSocket.loop();
}
#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "secrets.h" // Contains server IP and WiFi credentials
#include "DisplayManager.h"
#include "WifiManager.h"

// --- GLOBAL OBJECTS ---
// The WebSocket client instance
WebSocketsClient webSocket;

// Memory buffer for JSON parsing.
// 200 bytes is enough for simple messages like {"type":"ui_update", ...}
StaticJsonDocument<200> doc;

// --- WEBSOCKET EVENT HANDLER ---
// This function runs automatically whenever a WebSocket event occurs
// (Connected, Disconnected, or Message Received).
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    // CRITICAL: Declare variables at the top of the function.
    // Declaring them inside 'switch' cases can cause "jump to case label" compiler errors.
    DeserializationError error;
    const char *msgType;
    const char *mode;
    const char *macro;

    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.println("[WS] Disconnected!");
        // Optional: You could update the OLED here to say "Disconnected"
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
        // Convert the raw payload (string) into a usable JSON object.
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
    delay(1000); // Short pause to let user see the boot screen

    // 2. Connect to Network
    // This function blocks until WiFi is connected.
    setupWiFi();

    // 3. Connect to WebSocket Server
    Serial.print("Connecting to Server: ");
    Serial.print(WS_SERVER_HOST); // Defined in secrets.h
    Serial.print(":");
    Serial.println(WS_SERVER_PORT); // Defined in secrets.h

    // Configure the connection
    webSocket.begin(WS_SERVER_HOST, WS_SERVER_PORT, "/");

    // Register the event handler function defined above
    webSocket.onEvent(webSocketEvent);

    // Auto-reconnect every 5 seconds if connection is lost
    webSocket.setReconnectInterval(5000);
}

// --- MAIN LOOP ---
void loop()
{
    // CRITICAL: Keep the socket alive!
    // This handles sending pings, receiving data, and maintaining the connection.
    webSocket.loop();
}
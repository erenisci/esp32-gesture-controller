#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "DisplayManager.h"
#include "secrets.h"

WebSocketsClient ws;
DisplayManager display;

String currentTitle = "Waiting...";
String currentArtist = "Spotify";
long currentProgressMs = 0;
long totalDurationMs = 0;
bool isPlaying = false;

unsigned long lastTickTime = 0;

void onMessage(WStype_t type, uint8_t *payload, size_t length)
{
    if (type != WStype_TEXT)
        return;

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    if (doc["type"] == "now_playing")
    {
        currentTitle = doc["track"].as<String>();
        currentArtist = doc["artist"].as<String>();
        currentProgressMs = doc["progress"];
        totalDurationMs = doc["duration"];
        isPlaying = doc["playing"];
    }
}

void setup()
{
    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected!");

    display.begin();
    ws.begin(WS_HOST, WS_PORT, "/");
    ws.onEvent(onMessage);

    lastTickTime = millis();
}

void loop()
{
    ws.loop();

    unsigned long now = millis();
    if (isPlaying && totalDurationMs > 0)
    {
        long delta = now - lastTickTime;
        if (delta > 0)
        {
            currentProgressMs += delta;
            if (currentProgressMs > totalDurationMs)
            {
                currentProgressMs = totalDurationMs;
            }
            lastTickTime = now;
        }
    }
    else
    {
        lastTickTime = now;
    }

    display.update(currentTitle, currentArtist, currentProgressMs, totalDurationMs);
}

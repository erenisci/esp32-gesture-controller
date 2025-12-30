/**
 * @file main.cpp
 */
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DisplayManager.h"
#include "NetworkManager.h"

// BURAYA VERCEL PROJE ADRESİNİ YAZACAKSIN
// Örn: "https://spotify-iot-projen.vercel.app/api/player?device_id="
const char *SERVER_URL = "https://spotify-setup-tool.vercel.app/api/player?device_id=";

DisplayManager display;
unsigned long lastCheckTime = 0;
const unsigned long CHECK_INTERVAL = 1000; // 1 saniyede bir sorgula

void setup()
{
    Serial.begin(115200);

    // Ekranı Başlat
    display.begin();
    display.update("System Init", "Please Wait...", 0, 0);

    // Ağa Bağlan
    setupNetwork();

    // Kurulum için MAC adresini göster
    String mac = WiFi.macAddress();
    display.update("Setup Required", mac.c_str(), 0, 0);
    delay(2000);
}

void loop()
{
    // WiFi koptuysa tekrar bağlanmayı dene
    if (WiFi.status() != WL_CONNECTED)
    {
        display.update("WiFi Lost", "Reconnecting...", 0, 0);
        WiFi.reconnect();
        delay(1000);
        return;
    }

    unsigned long now = millis();
    if (now - lastCheckTime >= CHECK_INTERVAL)
    {
        lastCheckTime = now;

        HTTPClient http;
        String url = String(SERVER_URL) + WiFi.macAddress();

        // Hız için Insecure (SSL sertifikası kontrolü kapalı)
        WiFiClientSecure *client = new WiFiClientSecure;
        client->setInsecure();
        http.begin(*client, url);

        int httpCode = http.GET();

        if (httpCode == 200)
        {
            String payload = http.getString();

            // JSON Parse
            JsonDocument doc; // ArduinoJson v7
            DeserializationError error = deserializeJson(doc, payload);

            if (!error)
            {
                bool isConfigured = doc["is_configured"];

                if (!isConfigured)
                {
                    // Cihaz veritabanında yoksa, ekranda MAC adresini gösterip bekle
                    String mac = WiFi.macAddress();
                    display.update("Go to Setup", mac.c_str(), 0, 0);
                }
                else
                {
                    bool isPlaying = doc["is_playing"];
                    if (isPlaying)
                    {
                        const char *title = doc["title"];
                        const char *artist = doc["artist"];
                        long duration = doc["duration"];
                        long progress = doc["progress"];
                        display.update(title, artist, progress, duration);
                    }
                    else
                    {
                        display.update("Paused", "Spotify", 0, 0);
                    }
                }
            }
        }
        else
        {
            Serial.printf("HTTP Error: %d\n", httpCode);
        }

        http.end();
        delete client;
    }
}
#include <Arduino.h>
#include <WiFiManager.h>
#include "NetworkManager.h"

void setupNetwork()
{
    WiFiManager wm;
    wm.setDebugOutput(true);

    // Bağlanamazsa 'Spotify_Player_Setup' adında WiFi açar.
    bool res = wm.autoConnect("Spotify_Player_Setup");

    if (!res)
    {
        Serial.println("Failed to connect");
    }
    else
    {
        Serial.println("WiFi Connected!");
    }
}
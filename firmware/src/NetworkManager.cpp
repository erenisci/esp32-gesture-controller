/**
 * @file NetworkManager.cpp
 * @brief Implementation of network connectivity logic using WiFiManager.
 * * This file handles the initial WiFi connection process. It utilizes the
 * blocking "autoConnect" feature to ensure the device has a working
 * internet connection before proceeding to the main application loop.
 * * @author erenisci
 */

#include <Arduino.h>
#include <WiFiManager.h>
#include "NetworkManager.h"

/**
 * @brief Establishes a WiFi connection via a Captive Portal if necessary.
 * * Execution Flow:
 * 1. Tries to connect using previously saved credentials (SSID/Password).
 * 2. If connection fails or no credentials exist, it starts an Access Point (AP).
 * 3. The AP is named "Spotify_Player_Setup".
 * 4. The execution BLOCKS here until the user connects to the AP and enters
 * valid WiFi credentials via the web interface (192.168.4.1).
 */
void setupNetwork()
{
    // Initialize the WiFiManager instance
    WiFiManager wm;

    // Enable debug output to Serial for troubleshooting connection issues
    wm.setDebugOutput(true);

    // --- Blocking Connection Attempt ---
    // 'autoConnect' will try to connect to saved WiFi.
    // If it fails, it opens an AP named "Spotify_Player_Setup" (no password).
    // The code halts here until WiFi is connected.
    bool res = wm.autoConnect("Spotify_Player_Setup");

    if (!res)
    {
        // Theoretically, we might hit this if the timeout is set and reached,
        // but by default, autoConnect waits indefinitely.
        Serial.println("Failed to connect");
    }
    else
    {
        // Connection successful, proceed to main application
        Serial.println("WiFi Connected!");
    }
}
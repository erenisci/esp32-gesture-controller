#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
// Include Guards: Prevents this file from being included multiple times,
// which would cause "redefinition" compiler errors.

// --- LIBRARIES ---
// The core ESP32 library for handling Wireless LAN connections.
#include <WiFi.h>

// --- FUNCTION PROTOTYPES ---

// This function handles the entire connection logic:
// 1. Sets WiFi mode to Station (Client).
// 2. Uses SSID/PASS from secrets.h.
// 3. Blocks execution (waits) until the connection is established.
void setupWiFi();

#endif // WIFI_MANAGER_H
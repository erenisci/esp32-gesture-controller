#include "WifiManager.h"
#include "secrets.h"        // Provides WIFI_SSID and WIFI_PASS
#include "DisplayManager.h" // Allows updating the OLED during connection

void setupWiFi()
{
    // 1. CONFIGURATION
    // Set ESP32 to Station Mode (WIFI_STA).
    // This means it acts like a client (like your phone), not a router.
    WiFi.mode(WIFI_STA);

    // Start the connection process using credentials from secrets.h
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Debugging: Print to Serial Monitor
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);

    // UX: Show initial status on OLED
    showConnectionStatus("Searching Network...");

    // 2. CONNECTION LOOP (BLOCKING)
    // The code will stay inside this 'while' loop until connection is successful.
    int retryCount = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);       // Wait 1 second before checking again
        Serial.print("."); // Print dots to Serial Monitor to show activity

        // UX: Toggle OLED message every second
        // This visual feedback reassures the user that the device hasn't crashed.
        if (retryCount % 2 == 0)
            showConnectionStatus("Negotiating...");
        else
            showConnectionStatus("Waiting IP...");

        retryCount++;
    }

    // 3. SUCCESS
    // If we reached here, it means we are connected!
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP()); // Print the IP assigned by the router
}
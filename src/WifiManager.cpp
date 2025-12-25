#include "WifiManager.h"
#include "secrets.h"        // Provides WIFI_SSID_1, WIFI_PASS_1, etc.
#include "DisplayManager.h" // Allows updating the OLED during connection

// --- HELPER FUNCTION: TRY CONNECTING ---
// This function attempts to connect to a specific network for a limited time (timeout).
// It returns 'true' if connected successfully, or 'false' if it fails.
bool tryNetwork(const char *ssid, const char *password, int timeoutSeconds)
{
    // Debugging: Show which network we are trying
    Serial.print("Attempting to connect to: ");
    Serial.println(ssid);

    // UX: Update OLED to show the current target network
    // We use the dedicated function to ensure proper formatting (multi-line)
    showWifiAttempt(ssid);

    // Start the connection process
    WiFi.begin(ssid, password);

    int attempts = 0;

    // CONNECTION LOOP (BLOCKING)
    // Wait until connected OR until timeout is reached
    while (WiFi.status() != WL_CONNECTED && attempts < timeoutSeconds)
    {
        delay(1000);       // Wait 1 second
        Serial.print("."); // Print progress dots

        // UX: Visual feedback to reassure user
        if (attempts % 2 == 0)
            showConnectionStatus("Negotiating...");
        else
            showConnectionStatus("Waiting IP...");

        attempts++;
    }

    // Check result
    if (WiFi.status() == WL_CONNECTED)
    {
        return true; // Success!
    }
    else
    {
        Serial.println("\nTimeout! Could not connect.");
        return false; // Failed, go to next network
    }
}

void setupWiFi()
{
    // 1. CONFIGURATION
    // Set ESP32 to Station Mode (WIFI_STA).
    WiFi.mode(WIFI_STA);

    // 2. REDUNDANCY LOOP (SMART CONNECTION)
    // We enter an infinite loop that tries Network 1, then Network 2.
    // It only breaks (exits) if a connection is established.
    while (true)
    {
        // --- ATTEMPT 1: PRIMARY NETWORK ---
        // Try to connect to the first network defined in secrets.h
        if (tryNetwork(WIFI_SSID_1, WIFI_PASS_1, 10))
        {
            break; // Connected! Exit the loop immediately.
        }

        // --- ATTEMPT 2: SECONDARY NETWORK ---
        // If the first network failed, try the second one.
        if (tryNetwork(WIFI_SSID_2, WIFI_PASS_2, 10))
        {
            break; // Connected! Exit the loop immediately.
        }

        // --- FAILURE HANDLING ---
        // If both networks failed, show an error message and wait before restarting.
        Serial.println("\nAll networks failed. Retrying cycle...");
        showConnectionStatus("All Failed. Retrying...");
        delay(2000);
    }

    // 3. SUCCESS
    // If we reached here, it means we are connected!
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}
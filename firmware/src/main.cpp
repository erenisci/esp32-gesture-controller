/**
 * @file main.cpp
 * @brief Main firmware entry point for the Spotify IoT Display.
 * * This file implements a Dual-Core architecture to ensure smooth UI rendering
 * independent of network latency.
 * * - **Core 0 (Network Task):** Handles blocking HTTP requests, JSON parsing,
 * and updates global state variables.
 * - **Core 1 (Main Loop):** Handles high-speed OLED rendering (25 FPS) and
 * performs time-based interpolation (Dead Reckoning) for smooth progress bars.
 * * @author erenisci
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DisplayManager.h"
#include "NetworkManager.h"

// --- Configuration ---
// The endpoint of the Vercel BFF (Backend-for-Frontend)
const char *SERVER_URL = "https://spotify-setup-tool.vercel.app/api/player?device_id=";

// --- Global State (Shared Memory) ---
// These variables are written by Core 0 and read by Core 1.
// They act as the synchronization layer between the Network and UI threads.
DisplayManager display;

String g_title = "Ready";             ///< Current Song Title
String g_artist = "Spotify IoT";      ///< Current Artist Name
long g_songDuration = 0;              ///< Total duration in ms
long g_serverProgress = 0;            ///< Progress snapshot received from server
unsigned long g_dataReceivedTime = 0; ///< Timestamp (millis) when data arrived
bool g_isPlaying = false;             ///< Is music currently playing?
bool g_isConfigured = false;          ///< Is the device linked to a user?
bool g_wifiConnected = false;         ///< Network status flag
bool g_hasData = false;               ///< Flag to prevent displaying empty data on boot

// Handle for the FreeRTOS Network Task
TaskHandle_t NetworkTask;

/**
 * @brief The Network Worker Task (Runs on Core 0).
 * * This function runs in an infinite loop on the secondary processor core.
 * It is responsible for fetching data from the API without blocking the
 * main UI thread.
 * * @param parameter FreeRTOS task parameters (unused).
 */
void codeForNetworkTask(void *parameter)
{
    for (;;) // Infinite loop (standard for FreeRTOS tasks)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            g_wifiConnected = true;

            HTTPClient http;
            // Append MAC address to identify this specific device
            String url = String(SERVER_URL) + WiFi.macAddress();

            // Create a secure client but skip certificate validation for speed.
            // In embedded systems, full SSL validation can add significant latency.
            WiFiClientSecure *client = new WiFiClientSecure;
            client->setInsecure();

            http.begin(*client, url);
            http.setTimeout(3000); // 3s timeout to prevent hanging

            int httpCode = http.GET();

            if (httpCode == 200)
            {
                String payload = http.getString();

                // Parse JSON using ArduinoJson v7
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, payload);

                if (!error)
                {
                    // Update configuration status
                    g_isConfigured = doc["is_configured"];

                    if (g_isConfigured)
                    {
                        bool playingState = doc["is_playing"];

                        // Safety Check: Ensure "title" exists and is a string before reading.
                        // This prevents crashes if the API returns unexpected null values.
                        if (doc["title"].is<const char *>())
                        {
                            String newTitle = doc["title"].as<String>();
                            String newArtist = doc["artist"].as<String>();
                            long newDuration = doc["duration"];
                            long newProgress = doc["progress"];

                            // Critical Section: Update Shared Global Variables
                            g_title = newTitle;
                            g_artist = newArtist;
                            g_songDuration = newDuration;
                            g_serverProgress = newProgress;

                            // Capture the exact moment this data arrived.
                            // This is used by Core 1 to calculate "Real-Time" progress.
                            g_dataReceivedTime = millis();

                            g_hasData = true;
                        }

                        g_isPlaying = playingState;
                    }
                }
            }
            http.end();
            delete client; // Prevent memory leaks
        }
        else
        {
            g_wifiConnected = false;
        }

        // Delay for 1000ms (1 second) to respect API rate limits.
        // vTaskDelay is non-blocking for other tasks but blocking for this thread.
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Standard Arduino Setup.
 * Initializes hardware and spawns the Network Task.
 */
void setup()
{
    Serial.begin(115200);

    // Initialize OLED
    display.begin();
    display.update("System Init", "Starting...", 0, 0);

    // Connect to WiFi (Blocking call via WiFiManager)
    setupNetwork();

    // Spawn the Network Task on Core 0
    // Priority 1 (Low), Stack Size 10KB
    xTaskCreatePinnedToCore(
        codeForNetworkTask,
        "NetworkWorker",
        10000,
        NULL,
        1,
        &NetworkTask,
        0 // Pin to Core 0
    );

    // Show Setup Info briefly
    String mac = WiFi.macAddress();
    display.update("Setup Required", mac.c_str(), 0, 0);
    delay(2000);
}

/**
 * @brief Main UI Loop (Runs on Core 1).
 * * This loop handles the visual output. It runs independently of the network
 * task, ensuring animations remain smooth even if the internet is slow.
 */
void loop()
{
    static unsigned long lastScreenUpdate = 0;
    unsigned long now = millis();

    // Limit Frame Rate to ~25 FPS (40ms) to save CPU cycles
    if (now - lastScreenUpdate >= 40)
    {
        lastScreenUpdate = now;

        // --- Logic Branching for UI States ---

        if (!g_wifiConnected && millis() > 15000)
        {
            // Only show error if disconnected for > 15 seconds
            display.update("WiFi Lost", "Reconnecting...", 0, 0);
        }
        else if (!g_isConfigured)
        {
            // Device not linked to a Spotify account
            String mac = WiFi.macAddress();
            display.update("Go to Setup", mac.c_str(), 0, 0);
        }
        else if (!g_hasData)
        {
            // Connected but waiting for first API response
            display.update("Waiting...", "Spotify", 0, 0);
        }
        else
        {
            // --- Interpolation / Dead Reckoning Logic ---
            // The server only updates us every 1 second.
            // To make the progress bar smooth, we calculate the estimated
            // progress based on how much time passed since the last update.

            long currentProgress = g_serverProgress;

            // Only advance the timer if music is actually playing
            if (g_isPlaying)
            {
                long timePassed = now - g_dataReceivedTime;
                currentProgress += timePassed;
            }

            // Clamp value to not exceed total duration
            if (currentProgress > g_songDuration)
            {
                currentProgress = g_songDuration;
            }

            // Render the frame
            display.update(g_title.c_str(), g_artist.c_str(), currentProgress, g_songDuration);
        }
    }
}
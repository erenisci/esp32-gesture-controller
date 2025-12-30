/**
 * @file DisplayManager.h
 * @brief Manages the OLED display rendering logic for the Spotify IoT Player.
 * * This class acts as a wrapper around the Adafruit_SSD1306 library.
 * It abstracts away the low-level pixel drawing and provides high-level
 * methods to render song information, scrolling text effects, and
 * progress bars.
 * * @author erenisci
 */

#pragma once

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

/**
 * @class DisplayManager
 * @brief Handles all visual output to the SSD1306 OLED screen.
 */
class DisplayManager
{
public:
    /**
     * @brief Constructor for DisplayManager.
     * Initializes internal state variables (scrolling timers, last track info).
     */
    DisplayManager();

    /**
     * @brief Initializes the I2C interface and the display hardware.
     * * This method must be called once in the setup() function.
     * It sets up the I2C clock speed and initializes the OLED driver.
     */
    void begin();

    /**
     * @brief Updates the screen with the current song information.
     * * This function handles the logic for:
     * 1. Scrolling text if the title or artist is too long.
     * 2. Formatting the time (MM:SS).
     * 3. Drawing and filling the progress bar based on the song duration.
     * * @param title      The title of the current track.
     * @param artist     The artist name.
     * @param progressMs Current playback progress in milliseconds.
     * @param durationMs Total duration of the track in milliseconds.
     */
    void update(const char *title, const char *artist, long progressMs, long durationMs);

private:
    Adafruit_SSD1306 display; ///< Instance of the low-level display driver.

    // --- State Tracking for Scrolling Logic ---
    String lastTitle;              ///< Stores the last known title to detect song changes.
    String lastArtist;             ///< Stores the last known artist to detect song changes.
    unsigned long scrollStartTime; ///< Timestamp when the current scroll cycle started.

    /**
     * @brief Helper function to draw a line of text with scrolling effect.
     * * @param y       Vertical position (Y-coordinate) on the screen.
     * @param text    The string to display.
     * @param isTitle If true, draws a music note icon; otherwise, draws a user icon.
     * @param offset  Horizontal scroll offset in pixels.
     */
    void drawScrollingLine(int y, const char *text, bool isTitle, int offset);

    /**
     * @brief Helper function to format milliseconds into a "MM:SS" string.
     * * @param ms Time in milliseconds.
     * @return String Formatted time string (e.g., "3:45").
     */
    String formatTime(long ms);
};
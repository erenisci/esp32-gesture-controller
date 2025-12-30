/**
 * @file DisplayManager.cpp
 * @brief Implementation of the OLED display rendering logic.
 * * This file contains the low-level graphics routines to draw text, icons,
 * and the dynamic progress bar on the SSD1306 display. It handles the
 * complex timing logic required for smooth scrolling text animations.
 * * @author erenisci
 */

#include "DisplayManager.h"
#include <Wire.h>

// --- Configuration Constants ---
#define SCREEN_WIDTH 128    ///< OLED display width, in pixels
#define SCREEN_HEIGHT 64    ///< OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< I2C address of the display

// --- UI Layout ---
#define ICON_WIDTH 20   ///< Width reserved for the icon on the left
#define BOTTOM_ROW_Y 52 ///< Y-coordinate for the progress bar area

// --- Animation Timing ---
#define SCROLL_SPEED 80    ///< Delay in ms between scroll steps (Lower is faster)
#define WAIT_AT_START 5000 ///< How long to wait before scrolling starts (ms)
#define WAIT_AT_END 2000   ///< How long to wait after scrolling ends (ms)

// --- Icon Bitmaps (Stored in Flash Memory) ---
// 12x12px Music Note Icon
const unsigned char icon_note[] PROGMEM = {
    0x00, 0x00, 0x18, 0x00, 0x1C, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00,
    0x70, 0x00, 0xF0, 0x00, 0xE0, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00};

// 12x12px User/Artist Icon
const unsigned char icon_user[] PROGMEM = {
    0x00, 0x00, 0x30, 0x00, 0x78, 0x00, 0x78, 0x00, 0x30, 0x00, 0x00, 0x00,
    0x78, 0x00, 0xCC, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/**
 * @brief Constructor: Initializes the display object and state variables.
 */
DisplayManager::DisplayManager()
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
      lastTitle(""),
      lastArtist(""),
      scrollStartTime(0)
{
}

/**
 * @brief Sets up the hardware interface.
 */
void DisplayManager::begin()
{
    Wire.begin();

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        return; // Allocation failed
    }

    display.setTextWrap(false); // Disable wrapping to allow scrolling
    display.clearDisplay();
    display.display();
}

/**
 * @brief Utility to format milliseconds into "MM:SS".
 */
String DisplayManager::formatTime(long ms)
{
    int sec = ms / 1000;
    int m = sec / 60;
    int s = sec % 60;
    char buf[8];
    sprintf(buf, "%d:%02d", m, s);
    return String(buf);
}

/**
 * @brief Draws a single line of text with an icon, handling scroll offset.
 * * This function uses a "masking" technique:
 * 1. Draws the text at the calculated offset.
 * 2. Clears the area where the icon sits (to wipe out any text behind it).
 * 3. Draws the icon on top.
 */
void DisplayManager::drawScrollingLine(int y, const char *text, bool isTitle, int offset)
{
    // 1. Draw Text (Potentially shifted left by 'offset')
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(ICON_WIDTH - offset, y + 3);
    display.print(text);

    // 2. Clear the Icon Area (Masking)
    // This ensures text doesn't overlap with the static icon on the left.
    display.fillRect(0, y, ICON_WIDTH, 14, SSD1306_BLACK);

    // 3. Draw the Static Icon
    display.drawBitmap(2, y + 1, isTitle ? icon_note : icon_user, 12, 12, SSD1306_WHITE);
}

/**
 * @brief Main rendering loop. Calculates animations and draws the frame.
 */
void DisplayManager::update(const char *title, const char *artist, long progressMs, long durationMs)
{
    String currentTitle = String(title);
    String currentArtist = String(artist);

    // --- State Check: Did the song change? ---
    if (currentTitle != lastTitle || currentArtist != lastArtist)
    {
        lastTitle = currentTitle;
        lastArtist = currentArtist;
        scrollStartTime = millis(); // Reset scroll timer
    }

    display.clearDisplay();

    // --- SCROLLING LOGIC CALCULATIONS ---
    int viewWidth = SCREEN_WIDTH - ICON_WIDTH;
    int titleLen = currentTitle.length() * 6; // Approx 6px per char
    int artistLen = currentArtist.length() * 6;

    // Calculate how far we need to scroll for each line
    int maxScrollDist = max(max(0, titleLen - viewWidth), max(0, artistLen - viewWidth));
    int globalOffset = 0;

    // If scrolling is needed, calculate the current offset based on time
    if (maxScrollDist > 0)
    {
        long scrollDuration = maxScrollDist * SCROLL_SPEED;
        long totalCycle = WAIT_AT_START + scrollDuration + WAIT_AT_END;
        long currentStep = (millis() - scrollStartTime) % totalCycle;

        // Phase 1: Wait at Start
        if (currentStep < WAIT_AT_START)
        {
            globalOffset = 0;
        }
        // Phase 2: Scrolling
        else if (currentStep < (WAIT_AT_START + scrollDuration))
        {
            globalOffset = (currentStep - WAIT_AT_START) / SCROLL_SPEED;
        }
        // Phase 3: Wait at End
        else
        {
            globalOffset = maxScrollDist;
        }
    }

    // --- DRAW TEXT LINES ---
    // Apply offset only if the text is actually longer than the screen
    int titleOffset = (globalOffset > max(0, titleLen - viewWidth)) ? max(0, titleLen - viewWidth) : globalOffset;
    drawScrollingLine(4, title, true, titleOffset);

    int artistOffset = (globalOffset > max(0, artistLen - viewWidth)) ? max(0, artistLen - viewWidth) : globalOffset;
    drawScrollingLine(20, artist, false, artistOffset);

    // --- DRAW PROGRESS BAR ---
    String currStr = formatTime(progressMs);
    String totalStr = formatTime(durationMs);
    int totalW = totalStr.length() * 6;

    // 1. Draw Timestamps (Left and Right)
    display.setCursor(0, BOTTOM_ROW_Y);
    display.print(currStr);
    display.setCursor(SCREEN_WIDTH - totalW, BOTTOM_ROW_Y);
    display.print(totalStr);

    // 2. Calculate Bar Dimensions
    int barX = (currStr.length() * 6) + 6;         // Start after current time
    int barW = (SCREEN_WIDTH - totalW - 6) - barX; // Width up to total time

    // 3. Draw Bar
    if (barW > 0)
    {
        // Draw Outline
        display.drawRoundRect(barX, BOTTOM_ROW_Y + 2, barW, 4, 2, SSD1306_WHITE);

        // Draw Fill (Only if duration is valid to avoid div by zero)
        if (durationMs > 0)
        {
            long fillW = (progressMs * barW) / durationMs;
            fillW = constrain(fillW, 0, barW); // Safety clamp

            if (fillW >= 2) // Only fill if visible
            {
                display.fillRoundRect(barX, BOTTOM_ROW_Y + 2, fillW, 4, 2, SSD1306_WHITE);
            }
        }
    }

    // Push buffer to OLED
    display.display();
}
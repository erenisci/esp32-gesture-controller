#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

// --- LIBRARY IMPORTS ---
// Adafruit SSD1306: Handles the hardware communication (I2C) with the chip.
// Adafruit GFX: Handles the mathematics of drawing (lines, circles, text).
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// --- GLOBAL OBJECT DECLARATION ---
// The keyword 'extern' is crucial here.
// It tells the compiler: "There is an object named 'display' created in the .cpp file.
// Allow other files (like main.cpp) to use it without creating a duplicate."
extern Adafruit_SSD1306 display;

// --- FUNCTION PROTOTYPES ---

// Initializes the I2C connection and resets the screen.
void initDisplay();

// Shows the initial "SYSTEM BOOTING..." message.
void showStartupScreen();

// Updates the screen with WiFi connection progress (e.g., "Connecting...", "Failed").
void showConnectionStatus(const char *status);

// The MAIN UI function.
// Displays the Mode Name (Header) and the Macro Name (Body).
void updateUI(const char *modeName, const char *macroName);

#endif // DISPLAY_MANAGER_H
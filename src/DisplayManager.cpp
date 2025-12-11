#include "DisplayManager.h"

// --- OLED CONFIGURATION ---
// Screen dimensions in pixels
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Reset pin (-1 means sharing Arduino reset pin)
#define OLED_RESET -1

// I2C Address (Usually 0x3C or 0x3D)
#define SCREEN_ADDRESS 0x3C

// Create the display object to control hardware
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- INITIALIZATION ---
void initDisplay()
{
    // Try to initialize with internal 3.3V charge pump
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        // If connection fails, stop everything (Infinite Loop)
        for (;;)
            ;
    }

    // Clear the buffer (remove Adafruit logo)
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.display();                   // Apply changes
}

// --- BOOT SCREEN ---
void showStartupScreen()
{
    display.clearDisplay();
    display.setTextSize(1); // Normal font size

    // 1. Header Text
    display.setCursor(0, 0);
    display.println("SYSTEM BOOTING...");

    // 2. Separator Line
    // Draws a line from (0,10) to (128,10)
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    display.display();
}

// --- CONNECTION STATUS ---
// Shows "Connecting..." and the dynamic status message below it
void showConnectionStatus(const char *status)
{
    display.clearDisplay();
    display.setTextSize(1);

    // Header
    display.setCursor(0, 0);
    display.println("CONNECTING...");

    // Line
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    // Status Message (e.g., "Negotiating...")
    // Placed at Y=15 to avoid overlapping with the line
    display.setCursor(0, 15);
    display.println(status);

    display.display();
}

// --- MAIN UI ---
// This is the main interface showing the Mode and Active Macro
void updateUI(const char *modeName, const char *macroName)
{
    display.clearDisplay();

    // 1. HEADER: CENTER ALIGNMENT (< DEFAULT >)
    // Size 1 character is approx 6 pixels wide
    int charWidthSmall = 6;

    String headerText = "< ";
    headerText += modeName;
    headerText += " >";

    // Math: Calculate X to center the text
    // (ScreenWidth - TextWidth) / 2
    int headerLen = headerText.length();
    int headerX = (SCREEN_WIDTH - (headerLen * charWidthSmall)) / 2;
    if (headerX < 0)
        headerX = 0; // Safety check

    display.setTextSize(1);
    display.setCursor(headerX, 0);
    display.println(headerText);

    // Separator Line
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    // 2. BODY LABEL: "ACTION:" (LEFT ALIGNED)
    // Placed at Y=20
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("ACTION:");

    // 3. BODY VALUE: MACRO NAME (BIG & CENTERED)
    // Size 2 character is approx 12 pixels wide
    int charWidthBig = 12;
    String macroText = macroName;
    int macroLen = macroText.length();

    // Math: Calculate X for the big text
    int macroX = (SCREEN_WIDTH - (macroLen * charWidthBig)) / 2;
    if (macroX < 0)
        macroX = 0;

    display.setTextSize(2);        // Double size font
    display.setCursor(macroX, 35); // Placed at Y=35 (Below "ACTION:")
    display.println(macroText);

    display.display(); // Push everything to screen
}
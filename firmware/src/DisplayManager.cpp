/**
 * @file DisplayManager.cpp
 */
#include "DisplayManager.h"
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define ICON_WIDTH 20
#define BOTTOM_ROW_Y 52
#define SCROLL_SPEED 80
#define WAIT_AT_START 5000
#define WAIT_AT_END 2000

// Icons
const unsigned char icon_note[] PROGMEM = {0x00, 0x00, 0x18, 0x00, 0x1C, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x70, 0x00, 0xF0, 0x00, 0xE0, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00};
const unsigned char icon_user[] PROGMEM = {0x00, 0x00, 0x30, 0x00, 0x78, 0x00, 0x78, 0x00, 0x30, 0x00, 0x00, 0x00, 0x78, 0x00, 0xCC, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

DisplayManager::DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1), lastTitle(""), lastArtist(""), scrollStartTime(0) {}

void DisplayManager::begin()
{
    Wire.begin();
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
        return;
    display.setTextWrap(false);
    display.clearDisplay();
    display.display();
}

String DisplayManager::formatTime(long ms)
{
    int sec = ms / 1000;
    int m = sec / 60;
    int s = sec % 60;
    char buf[8];
    sprintf(buf, "%d:%02d", m, s);
    return String(buf);
}

void DisplayManager::drawScrollingLine(int y, const char *text, bool isTitle, int offset)
{
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(ICON_WIDTH - offset, y + 3);
    display.print(text);
    display.fillRect(0, y, ICON_WIDTH, 14, SSD1306_BLACK); // Clear icon area
    display.drawBitmap(2, y + 1, isTitle ? icon_note : icon_user, 12, 12, SSD1306_WHITE);
}

void DisplayManager::update(const char *title, const char *artist, long progressMs, long durationMs)
{
    String currentTitle = String(title);
    String currentArtist = String(artist);

    if (currentTitle != lastTitle || currentArtist != lastArtist)
    {
        lastTitle = currentTitle;
        lastArtist = currentArtist;
        scrollStartTime = millis();
    }

    display.clearDisplay();

    // Scrolling Logic
    int viewWidth = SCREEN_WIDTH - ICON_WIDTH;
    int titleLen = currentTitle.length() * 6;
    int artistLen = currentArtist.length() * 6;
    int maxScrollDist = max(max(0, titleLen - viewWidth), max(0, artistLen - viewWidth));
    int globalOffset = 0;

    if (maxScrollDist > 0)
    {
        long scrollDuration = maxScrollDist * SCROLL_SPEED;
        long totalCycle = WAIT_AT_START + scrollDuration + WAIT_AT_END;
        long currentStep = (millis() - scrollStartTime) % totalCycle;
        if (currentStep >= WAIT_AT_START && currentStep < (WAIT_AT_START + scrollDuration))
            globalOffset = (currentStep - WAIT_AT_START) / SCROLL_SPEED;
        else if (currentStep >= (WAIT_AT_START + scrollDuration))
            globalOffset = maxScrollDist;
    }

    drawScrollingLine(4, title, true, (globalOffset > max(0, titleLen - viewWidth)) ? max(0, titleLen - viewWidth) : globalOffset);
    drawScrollingLine(20, artist, false, (globalOffset > max(0, artistLen - viewWidth)) ? max(0, artistLen - viewWidth) : globalOffset);

    // Always draw Progress Bar (Even if empty)
    String currStr = formatTime(progressMs);
    String totalStr = formatTime(durationMs);
    int totalW = totalStr.length() * 6;

    display.setCursor(0, BOTTOM_ROW_Y);
    display.print(currStr);
    display.setCursor(SCREEN_WIDTH - totalW, BOTTOM_ROW_Y);
    display.print(totalStr);

    int barX = (currStr.length() * 6) + 6;
    int barW = (SCREEN_WIDTH - totalW - 6) - barX;

    if (barW > 0)
    {
        display.drawRoundRect(barX, BOTTOM_ROW_Y + 2, barW, 4, 2, SSD1306_WHITE);
        if (durationMs > 0)
        {
            long fillW = (progressMs * barW) / durationMs;
            fillW = constrain(fillW, 0, barW);
            if (fillW >= 2)
                display.fillRoundRect(barX, BOTTOM_ROW_Y + 2, fillW, 4, 2, SSD1306_WHITE);
        }
    }

    display.display();
}
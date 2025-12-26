#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

class DisplayManager
{
public:
    DisplayManager();
    void begin();
    void update(const String &title,
                const String &artist,
                long progressMs,
                long durationMs);

private:
    Adafruit_SSD1306 display;

    void drawScrollingLine(int y, const String &text, bool isTitle);
    String formatTime(long ms);
};

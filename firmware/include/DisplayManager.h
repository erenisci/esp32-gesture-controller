#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

class DisplayManager
{
public:
    DisplayManager();
    void begin();
    void update(const char *title, const char *artist, long progressMs, long durationMs);

private:
    Adafruit_SSD1306 display;
    String lastTitle;
    String lastArtist;
    unsigned long scrollStartTime;
    void drawScrollingLine(int y, const char *text, bool isTitle, int offset);
    String formatTime(long ms);
};
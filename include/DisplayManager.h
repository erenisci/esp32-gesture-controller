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

    // --- YENİ EKLENEN DEĞİŞKENLER ---
    String lastTitle;              // Bir önceki döngüdeki şarkı ismi
    String lastArtist;             // Bir önceki döngüdeki sanatçı
    unsigned long scrollStartTime; // Şarkı değiştiği anki zaman (millis)
    // -------------------------------

    void drawScrollingLine(int y, const String &text, bool isTitle, int offset);
    String formatTime(long ms);
};
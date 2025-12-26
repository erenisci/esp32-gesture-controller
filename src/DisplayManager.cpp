#include "DisplayManager.h"
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// --- AYARLAR ---
#define ICON_WIDTH 20
#define SCROLL_SPEED 80
#define BOTTOM_ROW_Y 52

#define WAIT_AT_START 5000 // 5 saniye bekle
#define WAIT_AT_END 2000   // 2 saniye bekle

const unsigned char icon_note[] PROGMEM = {
    0x00, 0x00, 0x18, 0x00, 0x1C, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00,
    0x70, 0x00, 0xF0, 0x00, 0xE0, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00};

const unsigned char icon_user[] PROGMEM = {
    0x00, 0x00, 0x30, 0x00, 0x78, 0x00, 0x78, 0x00, 0x30, 0x00, 0x00, 0x00,
    0x78, 0x00, 0xCC, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Constructor'da değişkenleri sıfırlıyoruz
DisplayManager::DisplayManager()
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1), lastTitle(""), lastArtist(""), scrollStartTime(0) {}

void DisplayManager::begin()
{
    Wire.begin();
    Wire.setClock(400000);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        for (;;)
            ;
    }

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

void DisplayManager::drawScrollingLine(int y, const String &text, bool isTitle, int offset)
{
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(ICON_WIDTH - offset, y + 3);
    display.print(text);

    display.fillRect(0, y, ICON_WIDTH, 14, SSD1306_BLACK);

    if (isTitle)
        display.drawBitmap(2, y + 1, icon_note, 12, 12, SSD1306_WHITE);
    else
        display.drawBitmap(2, y + 1, icon_user, 12, 12, SSD1306_WHITE);
}

void DisplayManager::update(const String &title, const String &artist, long progressMs, long durationMs)
{
    // --- YENİ EKLENEN MANTIK: ŞARKI DEĞİŞİM KONTROLÜ ---
    // Eğer gelen şarkı veya sanatçı ismi, hafızadakinden farklıysa:
    if (title != lastTitle || artist != lastArtist)
    {
        lastTitle = title;          // Yeni şarkıyı hafızaya al
        lastArtist = artist;        // Yeni sanatçıyı hafızaya al
        scrollStartTime = millis(); // Kronometreyi ŞU AN'a sıfırla
    }
    // ----------------------------------------------------

    display.clearDisplay();

    int viewWidth = SCREEN_WIDTH - ICON_WIDTH;
    int titleLen = title.length() * 6;
    int artistLen = artist.length() * 6;

    int titleScrollDist = titleLen - viewWidth;
    int artistScrollDist = artistLen - viewWidth;

    if (titleScrollDist < 0)
        titleScrollDist = 0;
    if (artistScrollDist < 0)
        artistScrollDist = 0;

    int maxScrollDist = (titleScrollDist > artistScrollDist) ? titleScrollDist : artistScrollDist;
    int globalOffset = 0;

    if (maxScrollDist > 0)
    {
        long scrollDuration = maxScrollDist * SCROLL_SPEED;
        long totalCycle = WAIT_AT_START + scrollDuration + WAIT_AT_END;

        unsigned long now = millis();

        // --- ÖNEMLİ DÜZELTME ---
        // Artık "now" değil, "now - scrollStartTime" kullanıyoruz.
        // Yani şarkı başladığından beri geçen süreye bakıyoruz.
        long timeSinceChange = now - scrollStartTime;
        long currentStep = timeSinceChange % totalCycle;

        if (currentStep < WAIT_AT_START)
        {
            globalOffset = 0;
        }
        else if (currentStep < (WAIT_AT_START + scrollDuration))
        {
            long timeInScroll = currentStep - WAIT_AT_START;
            globalOffset = timeInScroll / SCROLL_SPEED;
        }
        else
        {
            globalOffset = maxScrollDist;
        }
    }

    int titleOffset = (globalOffset > titleScrollDist) ? titleScrollDist : globalOffset;
    drawScrollingLine(4, title, true, titleOffset);

    int artistOffset = (globalOffset > artistScrollDist) ? artistScrollDist : globalOffset;
    drawScrollingLine(20, artist, false, artistOffset);

    // --- ALT KISIM (Aynı kaldı) ---
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    String currStr = formatTime(progressMs);
    int currW = currStr.length() * 6;
    display.setCursor(0, BOTTOM_ROW_Y);
    display.print(currStr);

    String totalStr = formatTime(durationMs);
    int totalW = totalStr.length() * 6;
    display.setCursor(SCREEN_WIDTH - totalW, BOTTOM_ROW_Y);
    display.print(totalStr);

    int barX = currW + 6;
    int barEnd = SCREEN_WIDTH - totalW - 6;
    int barW = barEnd - barX;
    int barY = BOTTOM_ROW_Y + 2;
    int barH = 4;

    if (barW > 0)
    {
        display.drawRoundRect(barX, barY, barW, barH, 2, SSD1306_WHITE);
        if (durationMs > 0)
        {
            long fillW = (progressMs * barW) / durationMs;
            if (fillW < 0)
                fillW = 0;
            if (fillW > barW)
                fillW = barW;
            if (fillW >= 2)
                display.fillRoundRect(barX, barY, fillW, barH, 2, SSD1306_WHITE);
        }
    }

    display.display();
}
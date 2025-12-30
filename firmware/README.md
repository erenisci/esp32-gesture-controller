# Spotify IoT Firmware

This directory contains the **C++ firmware** for the ESP32, built with **PlatformIO**.

It handles WiFi connectivity, communicates with the Backend API, and drives the OLED display using a **Dual-Core Architecture** for lag-free performance.

## Tech Stack

- **Framework:** Arduino
- **Build System:** PlatformIO
- **Key Libraries:** `ArduinoJson`, `Adafruit_SSD1306`, `WiFiManager`

## Key Features

- **Multi-threading:**
  - **Core 0:** Network tasks (HTTP requests, JSON parsing).
  - **Core 1:** UI rendering (60 FPS smooth scrolling).
- **Dead Reckoning:** Local interpolation for smooth progress bars between API updates (prevents stuttering).
- **Captive Portal:** On-the-fly WiFi configuration via `Spotify_Player_Setup` hotspot.

## Hardware & Pinout

| Component    | ESP32 Pin |
| :----------- | :-------- |
| **OLED SDA** | GPIO 21   |
| **OLED SCL** | GPIO 22   |
| **OLED VCC** | 3.3V      |
| **OLED GND** | GND       |

## Quick Start

1.  **Open in PlatformIO:** Open this `firmware` folder in VS Code.
2.  **Configure URL:**
    Open `src/main.cpp`. By default, it points to the public setup tool. If you are self-hosting, update it:
    ```cpp
    // Default Public Server:
    const char *SERVER_URL = "https://spotify-setup-tool.vercel.app/api/player?device_id=";
    ```
3.  **Upload:** Connect your ESP32 via USB and click the **Upload** (&rarr;) button in the PlatformIO toolbar.

## Structure

- `src/main.cpp`: Entry point & Dual-Core logic.
- `src/DisplayManager`: OLED graphics & scrolling engine.
- `src/NetworkManager`: WiFi & Captive Portal logic.
- `platformio.ini`: Dependencies & Board config.

---

_For the full project documentation and backend setup, please refer to the root `README.md`._

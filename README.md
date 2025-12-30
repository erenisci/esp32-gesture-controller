# Spotify IoT Dashboard: Dual-Core ESP32 & Serverless Bridge

This project is an end-to-end IoT solution that bridges physical hardware with digital music streaming. It transforms a standard ESP32 into a professional dashboard that visualizes Spotify playback data in **real-time**.

---

## System Architecture (Technical Overview)

To overcome the resource limitations of microcontrollers and maximize performance, a **BFF (Backend-for-Frontend)** architecture was implemented.

1.  **Firmware (ESP32 - C++):** Manages asynchronous data handling and smooth UI rendering at the hardware level.
2.  **BFF Bridge (Node.js - Vercel):** Handles complex OAuth2 flows, token refreshing, and JSON pruning in the cloud, reducing the workload on the ESP32 by 90%.
3.  **Setup Tool (Web):** A modern web interface powered by JS automation that allows users to securely link their physical device (MAC Address) to their Spotify account.

---

## Advanced Features & Engineering Solutions

### 1. Dead Reckoning Interpolation (Smooth Progress Bar)

The Spotify API updates playback data at intervals of a second or more, which typically causes stuttering on progress bars. I developed an interpolation algorithm that synchronizes with the device's internal clock (millis) the moment data arrives. It predicts the sub-second progress until the next update. Result: **Zero stutter, perfectly fluid animation.**

### 2. Dual-Core Asynchronous Processing

Both cores of the ESP32 are actively utilized:

- **Core 0 (Network Thread):** Handles HTTP requests, SSL certificate validation, and JSON parsing. These blocking operations occur in the background without affecting the display.
- **Core 1 (Main UI Thread):** Refreshes the OLED screen at 25+ FPS, rendering scrolling text animations and the progress bar.

### 3. Smart Data Pruning

A raw Spotify API response is a complex JSON object exceeding 10KB. Our server layer processes this data into an optimized **~100-byte** packet. This minimizes the device's RAM usage and eliminates the risk of memory-related crashes.

---

## Repository Structure (Monorepo)

```bash
├── firmware/                 # PlatformIO project (C++ / Arduino)
│   ├── src/                  # Core source code (Main, Display & Network Manager)
│   └── include/              # Header files (.h)
├── backend/
│   └── spotify-setup-tool/   # Vercel Backend & Frontend
│       ├── api/              # Serverless Functions (Node.js)
│       └── public/           # Web Pairing Interface (with MAC auto-format JS)
└── README.md                 # Main documentation
```

---

## Hardware Setup & Pinout

| SSD1306 OLED (I2C) | ESP32 DevKit | Function         |
| :----------------- | :----------- | :--------------- |
| **VCC**            | 3.3V         | Power Supply     |
| **GND**            | GND          | Ground           |
| **SDA**            | GPIO 21      | Data Line (I2C)  |
| **SCL**            | GPIO 22      | Clock Line (I2C) |

---

## Quick Start

1. **Backend Deployment**: Follow the instructions in the `/backend/spotify-setup-tool` directory to set up your Vercel and Supabase instances.
2. **Firmware Upload**: Update the `SERVER_URL` variable in `firmware/src/main.cpp` with your own URL:
   `const char *SERVER_URL = "https://spotify-setup-tool.vercel.app/api/player?device_id=";`
3. **Pairing**: Enter the MAC address shown on the device screen into the web interface. The interface automatically formats the MAC address for you. Log in with Spotify, and your device is ready!

---

## License

This project is protected under the [**MIT License**](./LICENSE).

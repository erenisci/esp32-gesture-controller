# Spotify IoT Display - Full Stack Project

An end-to-end IoT solution that visualizes your real-time Spotify playback on an ESP32-driven OLED display. This project combines embedded systems engineering with modern web architecture.

---

## Project Architecture

This project is built using a **BFF (Backend-for-Frontend)** pattern to solve the limitations of microcontrollers.

1.  **Firmware (ESP32):** A C++ application utilizing dual-core processing. It focuses on high-speed UI rendering and simple data fetching.
2.  **Backend (Node.js):** A serverless API that handles Spotify's OAuth2 complex authentication and data pruning.
3.  **Setup Tool:** A web interface used to securely link a physical device (MAC Address) to a Spotify account.

---

## Repository Structure

The project is organized as a monorepo for easy management:

- **[`/firmware`](./firmware):** ESP32 C++ source code, libraries, and hardware wiring diagrams.
- **[`/tools/spotify-setup-tool`](./tools/spotify-setup-tool):** Backend API (Vercel Functions) and the Setup Tool website.

---

## How It Works

### 1. Zero-Lag UI Logic

The firmware uses **Dead Reckoning Interpolation**. Instead of waiting for the API to update the progress bar every second, the ESP32 calculates the sub-second progress locally based on its internal clock. This results in a "butter-smooth" 60 FPS animation.

### 2. Dual-Core Multithreading

- **Core 0:** Dedicated to networking. It handles SSL handshakes and JSON parsing without blocking the UI.
- **Core 1:** Dedicated to the OLED display. It ensures the text always scrolls smoothly and the bar never stutters.

### 3. Secure OAuth2 Bridge

By offloading authentication to the cloud backend, we never store Spotify `Client Secrets` on the hardware. Tokens are stored securely in a PostgreSQL database and mapped to the device's unique MAC Address.

---

## Quick Installation

1.  **Deploy the Backend:** Follow the instructions in the [Backend README](./backend/README.md) to set up your Vercel and Supabase instance.
2.  **Flash the Firmware:** Open the [Firmware directory](./firmware) in PlatformIO, set your `SERVER_URL`, and upload the code to your ESP32.
3.  **Link Your Account:** Visit your deployed web tool, enter your device's MAC Address, and log in with Spotify.

---

## Hardware Requirements

- **ESP32** (Any standard DevKit)
- **SSD1306 OLED Display** (128x64 I2C)
- **WiFi Connection** (2.4GHz)

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

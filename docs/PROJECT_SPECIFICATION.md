# ESP32 Gesture Controller — Project Specification

## Overview

This document describes the architecture, features, and implementation plan for the **ESP32 Gesture Controller** — a wireless macro pad system. The system links a Windows desktop application (handling **Camera Gestures + Macro Logic**) with an ESP32 device (**OLED Display + LED Ring + Buttons**) using WebSocket communication over the local network.

Unlike typical macro pads, this device acts as a **dynamic status dashboard**, displaying the active **Mode** and the last triggered **Action** in real-time, receiving updates directly from the PC.

---

# High-Level Components

## 1. ESP32 Device (Hardware + Firmware)

### **Hardware**

- ESP32 (DOIT ESP32 DEVKIT V1)
- 0.96" I2C OLED (SSD1306)
- 16-LED WS2812/NeoPixel RGB Ring
- **5 tactile buttons:**

  - NEXT
  - PREV
  - TOGGLE_LED
  - TOGGLE_OLED
  - TOGGLE_CAMERA

- 1 hardware power switch (device ON/OFF)

### **Firmware Responsibilities**

- Connect to Local WiFi (Station Mode)
- Connect to Desktop WebSocket Server (Client Mode)
- **Display Dashboard**:

  - Centered Mode Name (Header)
  - Centered Macro Action (Body)

- **Visual Feedback** using LED ring
- **Input System**:

  - Read 5 hardware buttons
  - Send button events to Desktop App

---

## 2. Desktop Application (Windows / Python)

The desktop application acts as the **brain** of the entire system.

### **Camera & Gesture Processing**

- Webcam video capture (OpenCV)
- Hand landmark detection (MediaPipe)
- Gesture classification
- Gesture smoothing and debouncing

### **Macro Engine**

- Map **Gestures → Macros** (e.g., _Fist → Mute_)
- Map **Buttons → Macros** (e.g., _NEXT → Next Track_)
- Execute keystrokes & mouse events (pynput)

### **WebSocket Server**

- Hosts a WebSocket server on 0.0.0.0
- Sends UI updates (ui_update) to ESP32
- Receives button events from ESP32

---

## 3. Architecture Change (Cloud Removed)

### **Old Model:** ESP32 → Cloud (Dropped)

Too much latency, complexity, and dependency.

### **New Model:** ESP32 ↔ PC (Local Network)

- Stable, low-latency communication
- PC handles everything heavy (macros, gesture detection, cloud sync if ever added)

ESP32 acts strictly as a **"dumb terminal"**:

- Display
- Button input
- LED feedback

---

# Message Formats (WebSocket JSON)

## 1. Desktop → ESP32 (UI Update)

```json
{
  "type": "ui_update",
  "mode": "DEFAULT",
  "macro": "Desktop"
}
```

## 2. Desktop → ESP32 (LED Update)

Future extension.

```json
{
  "type": "led_update",
  "pattern": "pulse",
  "color": "#FF0000"
}
```

## 3. ESP32 → Desktop (Button Events)

```json
{
  "type": "button",
  "id": "BTN_NEXT",
  "state": "pressed"
}
```

## 4. ESP32 → Desktop (Auth)

```json
{
  "type": "auth",
  "device": "ESP32_v1"
}
```

---

# ESP32 Firmware — Technical Details

## Libraries & Tools

- PlatformIO (VS Code)
- Arduino Framework
- Adafruit_SSD1306 + Adafruit_GFX
- WebSockets Client
- ArduinoJson
- Adafruit NeoPixel

## Modular File Structure

```
src/
├─ main.cpp
├─ DisplayManager.cpp
├─ WifiManager.cpp
include/
├─ DisplayManager.h
├─ WifiManager.h
├─ secrets.h   # SSID, Password, Server IP (not committed)
```

## Key Behaviors

- **Boot Sequence:** Init → Boot Screen → WiFi Connect → WS Connect
- **Auto-Reconnect** on WiFi drop
- **OLED UI Rendering**:

  - Header centered
  - "ACTION:" label left aligned
  - Macro/action text centered in size 2 font

---

# Desktop Application — Technical Details

## Technology Stack

- Python 3.10+
- asyncio
- websockets
- opencv-python
- mediapipe
- pynput

## Gesture Engine Logic

- Detect hand landmarks
- Classify gesture
- Smooth gesture over multiple frames
- Trigger Windows Shortcut
- Send JSON UI Update to ESP32

Example:

```json
{
  "type": "ui_update",
  "mode": "FPS",
  "macro": "Shoot"
}
```

---

# Pinout & Hardware Mapping

- **OLED (I2C)**

  - SDA → GPIO21
  - SCL → GPIO22

- **NeoPixel Ring**

  - DATA → GPIO15

- **Buttons**

  - Pullup inputs on GPIO: 13, 12, 14, 27, 26

---

# Development Roadmap (Revised)

### ✅ Phase 1 — Connectivity

- ESP32 connects to WiFi
- ESP32 connects to WebSocket server
- OLED displays "Connected"

### ✅ Phase 2 — UI Protocol

- Python sends ui_update
- ESP32 parses JSON
- OLED renders formatted text

### 🚧 Phase 3 — Buttons

- Wire buttons to ESP32
- Send button events to Python
- Python responds

### ⏳ Phase 4 — Gesture Integration

- Implement camera pipeline
- Real-time gestures → macros
- UI updates to ESP32

### ⏳ Phase 5 — LED Ring

- Add NeoPixel effects
- Success animation, mode color coding

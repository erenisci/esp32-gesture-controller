import asyncio
import json

import websockets

# --- CONNECTION HANDLER ---
# This function runs automatically whenever a new device (ESP32) connects.


async def handler(websocket):
    # Log the IP address of the device that just connected
    print(f"[NEW CONNECTION] Client: {websocket.remote_address}")

    try:
        # 1. INITIALIZATION: Send the first screen layout to ESP32
        # As soon as the device connects, we tell it what to display.
        print("[SENT] Sending UI update to ESP32...")

        # Create the data packet (Dictionary format)
        ui_command = {
            "type": "ui_update",   # Tells ESP32 this is a screen update
            "mode": "DEFAULT",     # The text for the header
            "macro": "Desktop"     # The text for the main label
        }

        # Convert the dictionary to a JSON string and send it
        await websocket.send(json.dumps(ui_command))
        print("[SENT] Initial state sent.")

        # 2. LISTENING LOOP
        # This loop runs forever, waiting for messages FROM the ESP32.
        # It does not block the rest of the program.
        async for message in websocket:
            print(f"[RECEIVED] From ESP32: {message}")

            # Simple Heartbeat Logic:
            # If ESP32 sends "ping", we reply "pong" to keep the connection alive.
            if "ping" in message:
                await websocket.send("pong")

    # Handle accidental disconnections (e.g., ESP32 turned off)
    except websockets.ConnectionClosed:
        print("[DISCONNECTED] Connection lost!")


# --- MAIN SERVER STARTER ---
async def main():
    print("[SYSTEM] Server starting...")

    # "0.0.0.0" is CRITICAL. It means "Listen on all network interfaces".
    # If you use "localhost", the ESP32 (external device) cannot connect.
    print("[INFO] IP: 0.0.0.0 (Listening on all interfaces)")
    print("[INFO] Port: 8765")

    # Start the WebSocket server
    async with websockets.serve(handler, "0.0.0.0", 8765):
        # Keep the script running indefinitely
        await asyncio.Future()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        # Handle Ctrl+C gracefully
        print("\n[SYSTEM] Server stopped by user.")

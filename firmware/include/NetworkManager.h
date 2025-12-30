/**
 * @file NetworkManager.h
 * @brief Network connectivity interface for the Spotify IoT Display.
 * * This module is responsible for managing the WiFi lifecycle. It abstracts
 * the complexity of the captive portal (WiFiManager) and provides a single
 * entry point to establish an internet connection.
 * * @author erenisci
 */

#pragma once

/**
 * @brief Initializes the WiFi connection and handles the Captive Portal logic.
 * * This function utilizes the WiFiManager library to manage network credentials.
 * - **Scenario A:** If saved credentials exist, it connects automatically.
 * - **Scenario B:** If connection fails, it launches an Access Point (AP) named
 * "Spotify_Player_Setup". The user must connect to this AP to configure
 * their home WiFi credentials.
 * * @note This function is blocking. It will not return until a connection
 * is established or the configuration portal is exited.
 */
void setupNetwork();
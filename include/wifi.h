#pragma once

/**
 * @file wifi.h
 * @brief WiFi connection management for EMS device
 * 
 * This module handles WiFi connectivity including connection setup,
 * status monitoring, and connection maintenance.
 */

/**
 * @brief Check if WiFi client is currently connected
 * @return true if WiFi is connected and in station mode, false otherwise
 */
bool wifi_client_connected();

/**
 * @brief Initialize and establish WiFi connection
 * @return true if connection successful, false if failed after all attempts
 */
bool setup_wifi();

/**
 * @brief WiFi maintenance loop (currently not implemented)
 * 
 * This function is intended for periodic WiFi connection monitoring
 * and reconnection handling in the main loop.
 */
void loop_wifi();

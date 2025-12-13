/*
 * settings.h
 *
 *  Created on: Dec 13, 2025
 *      Author: Settings management for persistent storage
 */

#ifndef INC_SETTINGS_H_
#define INC_SETTINGS_H_

#include <stdint.h>
#include <stdbool.h>

// Settings structure version for future compatibility
#define SETTINGS_VERSION 1

// Display mode enumeration
typedef enum {
    SETTINGS_DISPLAY_BOTH = 0,
    SETTINGS_DISPLAY_TIME_ONLY = 1,
    SETTINGS_DISPLAY_DATE_ONLY = 2
} SettingsDisplayMode;

// Settings structure - keep it small and aligned for flash operations
typedef struct __attribute__((packed, aligned(4))) {
    uint32_t magic;                     // Magic number to validate settings (0x53455454 = "SETT")
    uint8_t version;                    // Settings version
    uint8_t display_mode;               // Display mode (0=both, 1=time, 2=date)
    uint8_t backlight_state;            // Backlight state (0=off, 1=on)
    uint8_t reserved1;                  // Reserved for future use
    uint32_t checksum;                  // Simple checksum for validation
    uint32_t reserved2;                 // Padding to ensure 16-byte alignment
} Settings;

// Initialize settings system
void Settings_Init(void);

// Load settings from flash
bool Settings_Load(Settings* settings);

// Save settings to flash
bool Settings_Save(const Settings* settings);

// Get current settings
Settings* Settings_Get(void);

// Set display mode and save - returns true if successful
bool Settings_SetDisplayMode(SettingsDisplayMode mode);

// Set backlight state and save - returns true if successful
bool Settings_SetBacklightState(bool state);

// Get display mode
SettingsDisplayMode Settings_GetDisplayMode(void);

// Get backlight state
bool Settings_GetBacklightState(void);

// Debug function to show current settings state (for troubleshooting)
void Settings_Debug(void);

#endif /* INC_SETTINGS_H_ */
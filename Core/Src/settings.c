/*
 * settings.c
 *
 *  Created on: Dec 13, 2025
 *      Author: Settings management for persistent storage
 */

#include "settings.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash_ex.h"
#include "16_2_display.h"
#include <string.h>
#include <stdio.h>

// STM32F411CE Flash Memory Map:
// Sector 0-3: 16KB each (0x08000000 - 0x0800FFFF) - Used by application code
// Sector 4: 64KB (0x08010000 - 0x0801FFFF) - Used by application code
// Sector 5: 128KB (0x08020000 - 0x0803FFFF) - Used by application code
// Sector 6: 128KB (0x08040000 - 0x0805FFFF) - Used by application code
// Sector 7: 128KB (0x08060000 - 0x0807FFFF) - Use END of this for settings
// 
// We'll use the VERY END of Sector 7 (last sector) to be safe
#define SETTINGS_FLASH_ADDRESS  0x0807FF00  // End of Sector 7, 256 bytes before end
#define SETTINGS_MAGIC          0x53455454  // "SETT" in hex

// Use Sector 7 for settings (safest - last sector)
#define SETTINGS_FLASH_SECTOR   FLASH_SECTOR_7

// Current settings in RAM
static Settings current_settings = {0};
static bool settings_initialized = false;

// Calculate simple checksum
static uint32_t calculate_checksum(const Settings* settings) {
    uint32_t checksum = 0;
    const uint8_t* data = (const uint8_t*)settings;
    
    // Calculate checksum for all fields except the checksum field itself
    for (size_t i = 0; i < sizeof(Settings) - sizeof(uint32_t) - sizeof(uint16_t); i++) {
        checksum += data[i];
    }
    
    return checksum;
}

// Validate settings
static bool validate_settings(const Settings* settings) {
    // Check magic number
    if (settings->magic != SETTINGS_MAGIC) {
        return false;
    }
    
    // Check version
    if (settings->version != SETTINGS_VERSION) {
        return false;
    }
    
    // Check checksum
    uint32_t expected_checksum = calculate_checksum(settings);
    if (settings->checksum != expected_checksum) {
        return false;
    }
    
    // Check display mode is valid
    if (settings->display_mode > 2) {
        return false;
    }
    
    // Check backlight state is valid
    if (settings->backlight_state > 1) {
        return false;
    }
    
    return true;
}

// Initialize with default settings
static void load_default_settings(Settings* settings) {
    settings->magic = SETTINGS_MAGIC;
    settings->version = SETTINGS_VERSION;
    settings->display_mode = SETTINGS_DISPLAY_BOTH;
    settings->backlight_state = 1;  // On by default
    settings->reserved1 = 0;
    settings->reserved2 = 0;
    settings->checksum = calculate_checksum(settings);
}

void Settings_Init(void) {
    if (!Settings_Load(&current_settings)) {
        // Load failed, use defaults
        load_default_settings(&current_settings);
        // Try to save defaults
        Settings_Save(&current_settings);
    }
    settings_initialized = true;
}

bool Settings_Load(Settings* settings) {
    // Read settings from flash
    const Settings* flash_settings = (const Settings*)SETTINGS_FLASH_ADDRESS;
    
    // Copy from flash to RAM
    memcpy(settings, flash_settings, sizeof(Settings));
    
    // Validate
    if (!validate_settings(settings)) {
        return false;
    }
    
    return true;
}

bool Settings_Save(const Settings* settings) {
    HAL_StatusTypeDef status;
    Settings settings_to_save;
    
    // Copy settings and update checksum
    memcpy(&settings_to_save, settings, sizeof(Settings));
    settings_to_save.magic = SETTINGS_MAGIC;
    settings_to_save.version = SETTINGS_VERSION;
    settings_to_save.checksum = calculate_checksum(&settings_to_save);
    
    // Clear any existing flash errors
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                          FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    
    // Unlock flash
    HAL_FLASH_Unlock();
    
    // Erase the sector
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0;
    
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector = SETTINGS_FLASH_SECTOR;
    EraseInitStruct.NbSectors = 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return false;
    }
    
    // Ensure erase was successful by checking if sector is blank
    uint32_t* check_addr = (uint32_t*)SETTINGS_FLASH_ADDRESS;
    for (int i = 0; i < 4; i++) {
        if (check_addr[i] != 0xFFFFFFFF) {
            HAL_FLASH_Lock();
            return false;  // Erase failed
        }
    }
    
    // Write settings to flash (word by word) with proper alignment
    uint32_t* src = (uint32_t*)&settings_to_save;
    uint32_t address = SETTINGS_FLASH_ADDRESS;
    size_t words = (sizeof(Settings) + 3) / 4;  // Round up to word count
    
    for (size_t i = 0; i < words; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, src[i]);
        if (status != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }
        
        // Verify each word was written correctly
        if (*(uint32_t*)address != src[i]) {
            HAL_FLASH_Lock();
            return false;
        }
        
        address += 4;
    }
    
    // Lock flash
    HAL_FLASH_Lock();
    
    // Final verification - reload and compare
    Settings verification_settings;
    if (!Settings_Load(&verification_settings)) {
        return false;
    }
    
    // Compare critical fields (not the full structure due to potential padding)
    if (verification_settings.magic != settings_to_save.magic ||
        verification_settings.version != settings_to_save.version ||
        verification_settings.display_mode != settings_to_save.display_mode ||
        verification_settings.backlight_state != settings_to_save.backlight_state ||
        verification_settings.checksum != settings_to_save.checksum) {
        return false;
    }
    
    // Update current settings in RAM only after successful verification
    memcpy(&current_settings, &settings_to_save, sizeof(Settings));
    
    return true;
}

Settings* Settings_Get(void) {
    if (!settings_initialized) {
        Settings_Init();
    }
    return &current_settings;
}

bool Settings_SetDisplayMode(SettingsDisplayMode mode) {
    if (!settings_initialized) {
        Settings_Init();
    }
    
    current_settings.display_mode = (uint8_t)mode;
    return Settings_Save(&current_settings);
}

bool Settings_SetBacklightState(bool state) {
    if (!settings_initialized) {
        Settings_Init();
    }
    
    current_settings.backlight_state = state ? 1 : 0;
    return Settings_Save(&current_settings);
}

SettingsDisplayMode Settings_GetDisplayMode(void) {
    if (!settings_initialized) {
        Settings_Init();
    }
    
    return (SettingsDisplayMode)current_settings.display_mode;
}

bool Settings_GetBacklightState(void) {
    if (!settings_initialized) {
        Settings_Init();
    }
    
    return current_settings.backlight_state != 0;
}

// Debug function to show current settings state
void Settings_Debug(void) {
    if (!settings_initialized) {
        Settings_Init();
    }
    
    // Show settings on LCD for debugging
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("Settings Debug:");
    
    char debug_line[17];
    snprintf(debug_line, sizeof(debug_line), "D:%d B:%d V:%d", 
             current_settings.display_mode, 
             current_settings.backlight_state,
             current_settings.version);
    
    LCD16X2_Set_Cursor(2, 1);
    LCD16X2_Write_String(debug_line);
    
    HAL_Delay(3000);  // Show for 3 seconds
    
    // Show magic and checksum on second screen
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("Magic & Checksum");
    
    snprintf(debug_line, sizeof(debug_line), "%08lX %08lX", 
             current_settings.magic, 
             current_settings.checksum);
    
    LCD16X2_Set_Cursor(2, 1);
    LCD16X2_Write_String(debug_line);
    
    HAL_Delay(3000);  // Show for 3 seconds
}
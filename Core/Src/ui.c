/*
 * ui.c
 *
 *  Created on: Nov 15, 2025
 *      Author: DRIFTYY777
 */

#include <ui.h>
#include <16_2_display.h>
#include <backLight.h>
#include <buttons.h>
#include <settings.h>


#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// for Data.csv file
#include "main.h"

extern uint8_t getDateTime(void);
extern void formatTimestamp(char *buffer, size_t bufSize);
extern RTC_HandleTypeDef hrtc;

// ============ UI STATE MANAGEMENT ============
typedef enum {
    DISPLAY_BOTH,
    DISPLAY_TIME_ONLY,
    DISPLAY_DATE_ONLY
} DisplayMode;

static DisplayMode home_display_mode = DISPLAY_BOTH;
static UIMode current_ui_mode = UI_MODE_HOME;
static uint32_t last_home_update = 0;
#define HOME_UPDATE_INTERVAL_MS 1000  // Update home screen every 1 second

void UI_Init(void) {
    // Initialize settings system and load saved settings
    Settings_Init();
    
    // Load display mode from settings
    SettingsDisplayMode saved_mode = Settings_GetDisplayMode();
    home_display_mode = (DisplayMode)saved_mode;
    
    // Load and apply backlight state
    bool backlight_state = Settings_GetBacklightState();
    setBacklight(backlight_state);
    
    current_ui_mode = UI_MODE_HOME;
    UI_ShowHomeScreen();
}

UIMode UI_GetMode(void) {
    return current_ui_mode;
}

void UI_EnterMenu(void) {
    current_ui_mode = UI_MODE_MENU;
    Menu_Init(main_menu, 3);  // Changed from 2 to 3 to include Back button
}

void UI_ExitMenu(void) {
    current_ui_mode = UI_MODE_HOME;
    UI_ShowHomeScreen();
}

void UI_ShowHomeScreen(void) {
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    
    // Get current time and date from RTC
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    
    char line1[17] = {0};
    char line2[17] = {0};
    
    LCD16X2_Clear();
    
    switch(home_display_mode) {
        case DISPLAY_BOTH:
            // Line 1: Date
            snprintf(line1, sizeof(line1), "  %02d/%02d/20%02d", 
                     sDate.Date, sDate.Month, sDate.Year);
            // Line 2: Time
            snprintf(line2, sizeof(line2), "    %02d:%02d:%02d", 
                     sTime.Hours, sTime.Minutes, sTime.Seconds);
            LCD16X2_Set_Cursor(1, 1);
            LCD16X2_Write_String(line1);
            LCD16X2_Set_Cursor(2, 1);
            LCD16X2_Write_String(line2);
            break;
            
        case DISPLAY_TIME_ONLY:
            snprintf(line1, sizeof(line1), "    Time:");
            snprintf(line2, sizeof(line2), "   %02d:%02d:%02d", 
                     sTime.Hours, sTime.Minutes, sTime.Seconds);
            LCD16X2_Set_Cursor(1, 1);
            LCD16X2_Write_String(line1);
            LCD16X2_Set_Cursor(2, 1);
            LCD16X2_Write_String(line2);
            break;
            
        case DISPLAY_DATE_ONLY:
            snprintf(line1, sizeof(line1), "    Date:");
            snprintf(line2, sizeof(line2), " %02d/%02d/20%02d", 
                     sDate.Date, sDate.Month, sDate.Year);
            LCD16X2_Set_Cursor(1, 1);
            LCD16X2_Write_String(line1);
            LCD16X2_Set_Cursor(2, 1);
            LCD16X2_Write_String(line2);
            break;
    }
}

void UI_Update(void) {
    // Check for long press of ENTER button to enter menu
    if (current_ui_mode == UI_MODE_HOME) {
        if (get_btn_long_press(ENTER_BUTTON)) {
            UI_EnterMenu();
            return;
        }
        
        // Update home screen periodically
        uint32_t now = HAL_GetTick();
        if ((now - last_home_update) >= HOME_UPDATE_INTERVAL_MS) {
            UI_ShowHomeScreen();
            last_home_update = now;
        }
    }
    else if (current_ui_mode == UI_MODE_MENU) {
        // Check for BACK button long press to exit menu
        if (get_btn_long_press(BACK_BUTTON)) {
            UI_ExitMenu();
            return;
        }
        
        // Update menu system
        Menu_Update();
    }
}


// Action: Toggle Back light
void action_ToggleBacklight(void) {
	static bool backlight_state_initialized = false;
	static bool backlight_state = true;
	
	// Initialize from saved settings on first call
	if (!backlight_state_initialized) {
		backlight_state = Settings_GetBacklightState();
		backlight_state_initialized = true;
	}
	
	// Toggle state
	backlight_state = !backlight_state;
	setBacklight(backlight_state);
	
	// Save to flash
	Settings_SetBacklightState(backlight_state);

	LCD16X2_Clear();
	LCD16X2_Set_Cursor(1, 1);
	if (backlight_state) {
		LCD16X2_Write_String("Backlight: ON ");
	} else {
		LCD16X2_Write_String("Backlight: OFF");
	}
	HAL_Delay(1500);
}

// Action: Show System Info
void action_SystemInfo(void) {
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("STM32F411 Clock");
    LCD16X2_Set_Cursor(2, 1);
    LCD16X2_Write_String("Ver 1.0 2025");
    HAL_Delay(2000);
}

// ============ HELPER FUNCTIONS FOR UNIFIED DATE/TIME EDITOR ============

// Helper function to update a specific field on the LCD with blinking
void update_field(uint8_t field, bool visible, uint8_t hour, uint8_t minute, uint8_t second, 
                  uint8_t day, uint8_t month, uint8_t year) {
    char buffer[3];
    
    switch(field) {
        case 0: // Hour
            if (visible) {
                snprintf(buffer, sizeof(buffer), "%02u", hour);
            } else {
                snprintf(buffer, sizeof(buffer), "  ");
            }
            LCD16X2_Set_Cursor(2, 1);
            LCD16X2_Write_String(buffer);
            break;
            
        case 1: // Minute
            if (visible) {
                snprintf(buffer, sizeof(buffer), "%02u", minute);
            } else {
                snprintf(buffer, sizeof(buffer), "  ");
            }
            LCD16X2_Set_Cursor(2, 4);
            LCD16X2_Write_String(buffer);
            break;
            
        case 2: // Second
            if (visible) {
                snprintf(buffer, sizeof(buffer), "%02u", second);
            } else {
                snprintf(buffer, sizeof(buffer), "  ");
            }
            LCD16X2_Set_Cursor(2, 7);
            LCD16X2_Write_String(buffer);
            break;
            
        case 3: // Day
            if (visible) {
                snprintf(buffer, sizeof(buffer), "%02u", day);
            } else {
                snprintf(buffer, sizeof(buffer), "  ");
            }
            LCD16X2_Set_Cursor(1, 1);
            LCD16X2_Write_String(buffer);
            break;
            
        case 4: // Month
            if (visible) {
                snprintf(buffer, sizeof(buffer), "%02u", month);
            } else {
                snprintf(buffer, sizeof(buffer), "  ");
            }
            LCD16X2_Set_Cursor(1, 4);
            LCD16X2_Write_String(buffer);
            break;
            
        case 5: // Year
            if (visible) {
                snprintf(buffer, sizeof(buffer), "%02u", year);
            } else {
                snprintf(buffer, sizeof(buffer), "  ");
            }
            LCD16X2_Set_Cursor(1, 9);
            LCD16X2_Write_String(buffer);
            break;
    }
}

// Helper function to display the date/time format template
void show_datetime_template(void) {
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("  /  /20  ");
    LCD16X2_Set_Cursor(2, 1);
    LCD16X2_Write_String("  :  :  ");
}

// ============ NEW UNIFIED DATE/TIME EDITOR ============

void action_UpdateDateTime(void) {
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    
    // Get current date and time as starting point
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    
    uint8_t hour_edit = sTime.Hours;
    uint8_t minute_edit = sTime.Minutes;
    uint8_t second_edit = sTime.Seconds;
    uint8_t day_edit = sDate.Date;
    uint8_t month_edit = sDate.Month;
    uint8_t year_edit = sDate.Year;
    
    // Field being edited (0=hour, 1=minute, 2=second, 3=day, 4=month, 5=year, 6=save)
    uint8_t field = 0;
    bool editing = true;
    bool blink_state = true;
    uint32_t last_blink_time = HAL_GetTick();
    
    // Initial display setup - only once
    show_datetime_template();
    
    // Initial display of all values
    for (int i = 0; i < 6; i++) {
        update_field(i, true, hour_edit, minute_edit, second_edit, day_edit, month_edit, year_edit);
    }
    
    while (editing) {
        uint32_t current_time = HAL_GetTick();
        
        // Fast blink handling - 300ms intervals
        if (current_time - last_blink_time >= 300) {
            blink_state = !blink_state;
            last_blink_time = current_time;
            
            if (field < 6) {
                update_field(field, blink_state, hour_edit, minute_edit, second_edit, day_edit, month_edit, year_edit);
            }
        }
        
        HAL_Delay(50);  // Reduced loop frequency for better button handling
        
        // Handle button inputs
        if (get_btn(UP_BUTTON)) {
            if (field < 6) update_field(field, true, hour_edit, minute_edit, second_edit, day_edit, month_edit, year_edit);
            
            switch(field) {
                case 0: hour_edit = (hour_edit + 1) % 24; break;
                case 1: minute_edit = (minute_edit + 1) % 60; break;
                case 2: second_edit = (second_edit + 1) % 60; break;
                case 3: {
                    day_edit++;
                    uint8_t max_days = 31;
                    if (month_edit == 2) max_days = 29;
                    else if (month_edit == 4 || month_edit == 6 || month_edit == 9 || month_edit == 11) max_days = 30;
                    if (day_edit > max_days) day_edit = 1;
                    break;
                }
                case 4: month_edit = (month_edit % 12) + 1; break;
                case 5: year_edit++; if (year_edit > 99) year_edit = 0; break;
            }
            
            if (field < 6) update_field(field, true, hour_edit, minute_edit, second_edit, day_edit, month_edit, year_edit);
            blink_state = true;
            last_blink_time = HAL_GetTick();
            HAL_Delay(200);  // Wait for button release
        }
        else if (get_btn(DOWN_BUTTON)) {
            if (field < 6) update_field(field, true, hour_edit, minute_edit, second_edit, day_edit, month_edit, year_edit);
            
            switch(field) {
                case 0: hour_edit = (hour_edit == 0) ? 23 : hour_edit - 1; break;
                case 1: minute_edit = (minute_edit == 0) ? 59 : minute_edit - 1; break;
                case 2: second_edit = (second_edit == 0) ? 59 : second_edit - 1; break;
                case 3: {
                    if (day_edit > 1) {
                        day_edit--;
                    } else {
                        if (month_edit == 2) day_edit = 29;
                        else if (month_edit == 4 || month_edit == 6 || month_edit == 9 || month_edit == 11) day_edit = 30;
                        else day_edit = 31;
                    }
                    break;
                }
                case 4: month_edit = (month_edit == 1) ? 12 : month_edit - 1; break;
                case 5: year_edit = (year_edit == 0) ? 99 : year_edit - 1; break;
            }
            
            if (field < 6) update_field(field, true, hour_edit, minute_edit, second_edit, day_edit, month_edit, year_edit);
            blink_state = true;
            last_blink_time = HAL_GetTick();
            HAL_Delay(200);  // Wait for button release
        }
        else if (get_btn(ENTER_BUTTON)) {
            if (field < 6) {
                update_field(field, true, hour_edit, minute_edit, second_edit, day_edit, month_edit, year_edit);
                field++;
                
                if (field == 6) {
                    LCD16X2_Clear();
                    LCD16X2_Set_Cursor(1, 1);
                    LCD16X2_Write_String("Save Date/Time?");
                    LCD16X2_Set_Cursor(2, 1);
                    char confirm[17];
                    snprintf(confirm, sizeof(confirm), "%02u/%02u/%02u %02u:%02u", 
                             day_edit, month_edit, year_edit, hour_edit, minute_edit);
                    LCD16X2_Write_String(confirm);
                } else {
                    blink_state = true;
                    last_blink_time = HAL_GetTick();
                }
            } else {
                // Save date and time
                sTime.Hours = hour_edit;
                sTime.Minutes = minute_edit;
                sTime.Seconds = second_edit;
                sDate.Date = day_edit;
                sDate.Month = month_edit;
                sDate.Year = year_edit;
                sDate.WeekDay = RTC_WEEKDAY_MONDAY;
                
                HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
                HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
                
                LCD16X2_Clear();
                LCD16X2_Set_Cursor(1, 1);
                LCD16X2_Write_String("Saved!");
                LCD16X2_Set_Cursor(2, 1);
                char save_msg[17];
                snprintf(save_msg, sizeof(save_msg), "%02u/%02u/%02u %02u:%02u:%02u", 
                         day_edit, month_edit, year_edit, hour_edit, minute_edit, second_edit);
                LCD16X2_Write_String(save_msg);
                HAL_Delay(2000);
                editing = false;
            }
            HAL_Delay(250);  // Longer delay after ENTER to prevent double-press
        }
        else if (get_btn(BACK_BUTTON)) {
            if (field > 0) {
                if (field == 6) {
                    show_datetime_template();
                    for (int i = 0; i < 6; i++) {
                        update_field(i, true, hour_edit, minute_edit, second_edit, day_edit, month_edit, year_edit);
                    }
                } else {
                    update_field(field, true, hour_edit, minute_edit, second_edit, day_edit, month_edit, year_edit);
                }
                
                field--;
                blink_state = true;
                last_blink_time = HAL_GetTick();
            } else {
                LCD16X2_Clear();
                LCD16X2_Set_Cursor(1, 1);
                LCD16X2_Write_String("Cancelled");
                LCD16X2_Set_Cursor(2, 1);
                LCD16X2_Write_String("No changes made");
                HAL_Delay(1500);
                editing = false;
            }
            HAL_Delay(250);  // Longer delay after BACK to prevent double-press
        }
    }
}

// ============ DISPLAY MODE ACTIONS ============

void action_DisplayBoth(void) {
    home_display_mode = DISPLAY_BOTH;
    
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("Display Mode:");
    LCD16X2_Set_Cursor(2, 1);
    LCD16X2_Write_String("Date & Time");
    HAL_Delay(1000);
    
    // Save to flash with feedback
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("Saving...");
    
    if (Settings_SetDisplayMode(SETTINGS_DISPLAY_BOTH)) {
        LCD16X2_Set_Cursor(2, 1);
        LCD16X2_Write_String("Saved!");
    } else {
        LCD16X2_Set_Cursor(2, 1);
        LCD16X2_Write_String("Save Failed!");
    }
    HAL_Delay(1500);
}

void action_DisplayTimeOnly(void) {
    home_display_mode = DISPLAY_TIME_ONLY;
    
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("Display Mode:");
    LCD16X2_Set_Cursor(2, 1);
    LCD16X2_Write_String("Time Only");
    HAL_Delay(1000);
    
    // Save to flash with feedback
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("Saving...");
    
    if (Settings_SetDisplayMode(SETTINGS_DISPLAY_TIME_ONLY)) {
        LCD16X2_Set_Cursor(2, 1);
        LCD16X2_Write_String("Saved!");
    } else {
        LCD16X2_Set_Cursor(2, 1);
        LCD16X2_Write_String("Save Failed!");
    }
    HAL_Delay(1500);
}

void action_DisplayDateOnly(void) {
    home_display_mode = DISPLAY_DATE_ONLY;
    
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("Display Mode:");
    LCD16X2_Set_Cursor(2, 1);
    LCD16X2_Write_String("Date Only");
    HAL_Delay(1000);
    
    // Save to flash with feedback
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("Saving...");
    
    if (Settings_SetDisplayMode(SETTINGS_DISPLAY_DATE_ONLY)) {
        LCD16X2_Set_Cursor(2, 1);
        LCD16X2_Write_String("Saved!");
    } else {
        LCD16X2_Set_Cursor(2, 1);
        LCD16X2_Write_String("Save Failed!");
    }
    HAL_Delay(1500);
}
void back_to_home(void) {
	current_ui_mode = UI_MODE_HOME;
	UI_ShowHomeScreen();
}


// ============ MENU DEFINITIONS ============

// Display Options sub menu
MenuItem display_menu[] = {
    {.text = "Date and Time", .callback = action_DisplayBoth, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "Time Only", .callback = action_DisplayTimeOnly, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "Date Only", .callback = action_DisplayDateOnly, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "Back", .callback = NULL, .submenu = NULL, .submenu_size = 0, .is_back = true}
};

// Settings sub menu
MenuItem settings_menu[] = {
    {.text = "Set Date/Time", .callback = action_UpdateDateTime, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "Toggle BLight", .callback = action_ToggleBacklight, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "System Info", .callback = action_SystemInfo, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "Debug Settings", .callback = Settings_Debug, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "Back", .callback = NULL, .submenu = NULL, .submenu_size = 0, .is_back = true}
};

// Main menu
MenuItem main_menu[] = {
    {.text = "Display Options", .callback = NULL, .submenu = display_menu, .submenu_size = 4, .is_back = false},
    {.text = "Settings", .callback = NULL, .submenu = settings_menu, .submenu_size = 4, .is_back = false},
    {.text = "Back", .callback = UI_ExitMenu, .submenu = NULL, .submenu_size = 0, .is_back = false}
};
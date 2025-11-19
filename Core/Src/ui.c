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


#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// for Data.csv file
#include "main.h"

extern uint8_t getDateTime(void);
extern void formatTimestamp(char *buffer, size_t bufSize);

// ============ MENU ACTIONS ============

//// Action: View Current Data
//void action_ViewData(void) {
//    char line1[17];
//    char line2[17];
//    snprintf(line1, sizeof(line1), "T:%.1fC F:%.1f", temperature, flow);
//    snprintf(line2, sizeof(line2), "Total:%.2f", totalizer);
//
//    LCD16X2_Clear();
//    LCD16X2_Set_Cursor(1, 1);
//    LCD16X2_Write_String(line1);
//    LCD16X2_Set_Cursor(2, 1);
//    LCD16X2_Write_String(line2);
//    HAL_Delay(2000);
//}
//
//// Action: View Date/Time
//void action_ViewDateTime(void) {
//    char timestampBuf[32];
//    getDateTime();
//    formatTimestamp(timestampBuf, sizeof(timestampBuf));
//
//    LCD16X2_Clear();
//    LCD16X2_Set_Cursor(1, 1);
//    LCD16X2_Write_String("Current Time:");
//    LCD16X2_Set_Cursor(2, 1);
//    LCD16X2_Write_String(timestampBuf);
//    HAL_Delay(2000);
//}
//
//// Action: SD Card Info
//void action_SDInfo(void) {
//    LCD16X2_Clear();
//    LCD16X2_Set_Cursor(1, 1);
//    LCD16X2_Write_String("SD Card Status:");
//    LCD16X2_Set_Cursor(2, 1);
//    if (sdCard_isInitialized()) {
//		LCD16X2_Write_String("OK - Inserted");
//	} else {
//		LCD16X2_Write_String("ERROR - Missing");
//	}
//    HAL_Delay(2000);
//}
//
//void action_setDateTime(void) {
//    // Get current time as starting point
//    getDateTime();
//
//    // Local variables for date/time editing
//    uint16_t year_edit = DS3231_GetYear();
//    uint8_t month_edit = DS3231_GetMonth();
//    uint8_t day_edit = DS3231_GetDate();
//    uint8_t hour_edit = DS3231_GetHour();
//    uint8_t minute_edit = DS3231_GetMinute();
//    uint8_t second_edit = DS3231_GetSecond();
//
//    // Field being edited (0=year, 1=month, 2=day, 3=hour, 4=minute, 5=second, 6=save)
//    uint8_t field = 0;
//    bool done = false;
//
//    char line1[18];
//    char line2[18];
//
//    while (!done) {
//        // Display current values with cursor on active field
//        switch(field) {
//            case 0: // Year
//                snprintf(line1, sizeof(line1), "Year: >%04u<", year_edit);
//                snprintf(line2, sizeof(line2), "UP/DN ENTER=Next");
//                break;
//            case 1: // Month
//                snprintf(line1, sizeof(line1), "Month: >%02u<", month_edit);
//                snprintf(line2, sizeof(line2), "UP/DN ENTER=Next");
//                break;
//            case 2: // Day
//                snprintf(line1, sizeof(line1), "Day: >%02u<", day_edit);
//                snprintf(line2, sizeof(line2), "UP/DN ENTER=Next");
//                break;
//            case 3: // Hour
//                snprintf(line1, sizeof(line1), "Hour: >%02u<", hour_edit);
//                snprintf(line2, sizeof(line2), "UP/DN ENTER=Next");
//                break;
//            case 4: // Minute
//                snprintf(line1, sizeof(line1), "Minute: >%02u<", minute_edit);
//                snprintf(line2, sizeof(line2), "UP/DN ENTER=Next");
//                break;
//            case 5: // Second
//                snprintf(line1, sizeof(line1), "Second: >%02u<", second_edit);
//                snprintf(line2, sizeof(line2), "UP/DN ENTER=Save");
//                break;
//            case 6: // Confirm
//                snprintf(line1, sizeof(line1), "Save DateTime?");
//                snprintf(line2, sizeof(line2), "ENTER=Yes BACK=No");
//                break;
//        }
//
//        LCD16X2_Clear();
//        LCD16X2_Set_Cursor(1, 1);
//        LCD16X2_Write_String(line1);
//        LCD16X2_Set_Cursor(2, 1);
//        LCD16X2_Write_String(line2);
//
//        // Wait for button press
//        HAL_Delay(200); // Debounce
//
//        // Handle button input
//        if (get_btn(UP_BUTTON)) {
//            // Increment current field
//            switch(field) {
//                case 0: year_edit++; if (year_edit > 2099) year_edit = 2020; break;
//                case 1: month_edit++; if (month_edit > 12) month_edit = 1; break;
//                case 2: day_edit++; if (day_edit > 31) day_edit = 1; break;
//                case 3: hour_edit++; if (hour_edit > 23) hour_edit = 0; break;
//                case 4: minute_edit++; if (minute_edit > 59) minute_edit = 0; break;
//                case 5: second_edit++; if (second_edit > 59) second_edit = 0; break;
//            }
//        }
//
//        if (get_btn(DOWN_BUTTON)) {
//            // Decrement current field
//            switch(field) {
//                case 0: if (year_edit > 2020) year_edit--; else year_edit = 2099; break;
//                case 1: if (month_edit > 1) month_edit--; else month_edit = 12; break;
//                case 2: if (day_edit > 1) day_edit--; else day_edit = 31; break;
//                case 3: if (hour_edit > 0) hour_edit--; else hour_edit = 23; break;
//                case 4: if (minute_edit > 0) minute_edit--; else minute_edit = 59; break;
//                case 5: if (second_edit > 0) second_edit--; else second_edit = 59; break;
//            }
//        }
//
//        if (get_btn(ENTER_BUTTON)) {
//            if (field < 6) {
//                field++; // Move to next field
//            } else {
//                // Save the date/time
//                DS3231_SetFullDate(day_edit, month_edit, 1, year_edit); // day, month, dayOfWeek(1=Mon), year
//                DS3231_SetFullTime(hour_edit, minute_edit, second_edit);
//
//                LCD16X2_Clear();
//                LCD16X2_Set_Cursor(1, 1);
//                LCD16X2_Write_String("DateTime Saved!");
//                char buf[22];
//
//                snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u", year_edit, month_edit, day_edit, hour_edit, minute_edit);
//                LCD16X2_Set_Cursor(2, 1);
//                LCD16X2_Write_String(buf);
//                HAL_Delay(2000);
//                done = true;
//            }
//        }
//
//        if (get_btn(BACK_BUTTON)) {
//            // Cancel without saving
//            LCD16X2_Clear();
//            LCD16X2_Set_Cursor(1, 1);
//            LCD16X2_Write_String("Cancelled");
//            HAL_Delay(1000);
//            done = true;
//        }
//        // clear all buffers
//        memset(line1, 0, sizeof(line1));
//        memset(line2, 0, sizeof(line2));
//    }
//}

// Action: Toggle Back light
void action_ToggleBacklight(void) {
	static bool backlight_state = true; // Assume backlight starts ON
	backlight_state = !backlight_state;
	setBacklight(backlight_state);

	LCD16X2_Clear();
	LCD16X2_Set_Cursor(1, 1);
	if (backlight_state) {
		LCD16X2_Write_String("Backlight: ON ");
	} else {
		LCD16X2_Write_String("Backlight: OFF");
	}
	HAL_Delay(1000);
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

// Action: Show Time
void action_ShowTime(void) {
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("Current Time:");
    LCD16X2_Set_Cursor(2, 1);
    LCD16X2_Write_String("12:34:56"); // Replace with actual RTC read
    HAL_Delay(2000);
}

// Action: Show Date
void action_ShowDate(void) {
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("Current Date:");
    LCD16X2_Set_Cursor(2, 1);
    LCD16X2_Write_String("2025-11-19"); // Replace with actual RTC read
    HAL_Delay(2000);
}

// Action: Test buttons (temporary debug function)
void action_TestButtons(void) {
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String("Button Test Mode");
    LCD16X2_Set_Cursor(2, 1);
    LCD16X2_Write_String("Press any button");
    
    uint32_t start_time = HAL_GetTick();
    bool button_found = false;
    
    // Test for 5 seconds
    while ((HAL_GetTick() - start_time) < 5000 && !button_found) {
        if (get_btn(UP_BUTTON)) {
            LCD16X2_Clear();
            LCD16X2_Set_Cursor(1, 1);
            LCD16X2_Write_String("UP Button");
            LCD16X2_Set_Cursor(2, 1);
            LCD16X2_Write_String("Working!");
            button_found = true;
        }
        else if (get_btn(DOWN_BUTTON)) {
            LCD16X2_Clear();
            LCD16X2_Set_Cursor(1, 1);
            LCD16X2_Write_String("DOWN Button");
            LCD16X2_Set_Cursor(2, 1);
            LCD16X2_Write_String("Working!");
            button_found = true;
        }
        else if (get_btn(ENTER_BUTTON)) {
            LCD16X2_Clear();
            LCD16X2_Set_Cursor(1, 1);
            LCD16X2_Write_String("ENTER Button");
            LCD16X2_Set_Cursor(2, 1);
            LCD16X2_Write_String("Working!");
            button_found = true;
        }
        else if (get_btn(BACK_BUTTON)) {
            LCD16X2_Clear();
            LCD16X2_Set_Cursor(1, 1);
            LCD16X2_Write_String("BACK Button");
            LCD16X2_Set_Cursor(2, 1);
            LCD16X2_Write_String("Working!");
            button_found = true;
        }
        HAL_Delay(10); // Small delay to prevent overwhelming the system
    }
    
    if (button_found) {
        HAL_Delay(2000); // Show result for 2 seconds
    } else {
        LCD16X2_Clear();
        LCD16X2_Set_Cursor(1, 1);
        LCD16X2_Write_String("No button");
        LCD16X2_Set_Cursor(2, 1);
        LCD16X2_Write_String("detected!");
        HAL_Delay(2000);
    }
}


// ============ MENU DEFINITIONS ============

// Time/Date sub menu
MenuItem datetime_menu[] = {
    {.text = "Show Time", .callback = action_ShowTime, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "Show Date", .callback = action_ShowDate, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "Back", .callback = NULL, .submenu = NULL, .submenu_size = 0, .is_back = true}
};

// Settings sub menu
MenuItem settings_menu[] = {
    {.text = "Toggle BLight", .callback = action_ToggleBacklight, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "System Info", .callback = action_SystemInfo, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "Test Buttons", .callback = action_TestButtons, .submenu = NULL, .submenu_size = 0, .is_back = false},
    {.text = "Back", .callback = NULL, .submenu = NULL, .submenu_size = 0, .is_back = true}
};

// Main menu
MenuItem main_menu[] = {
    {.text = "Date/Time", .callback = NULL, .submenu = datetime_menu, .submenu_size = 3, .is_back = false},
    {.text = "Settings", .callback = NULL, .submenu = settings_menu, .submenu_size = 4, .is_back = false}
};
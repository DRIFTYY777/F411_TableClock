/*
 * ui.c
 *
 * Created on: 24-Jul-2026
 * Author: dhima
 */

#include <ui.h>

#include "inputs.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define MENU_TIMEOUT_MS 30000U

Lcd_HandleTypeDef *lcd_ptr;        // Pointer to the LCD handle for use in UI functions
RTC_HandleTypeDef *hrtc_ptr;       // Pointer to the RTC handle for use in UI functions
TimeFormat_t _time_format = _NONE; // Default to 24-hour mode

typedef struct
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t dayofweek;
    uint8_t dayofmonth;
    uint8_t month;
    uint8_t year;

    TimeFormat_t format; // 24-hour or 12-hour format

} TimeDate_t;
TimeDate_t current_time_date; // Global variable to hold the current time and date

/// @brief Enum for different screen states in the UI.
typedef enum
{
    HOME_SCREEN,
    MENU_SCREEN,
    ALARM_SCREEN,
    DATE_SCREEN,
    FORMAT_SCREEN,
    TIME_SCREEN,
    STOPWATCH_SCREEN,
    TIMER_SCREEN,
    BRIGHTNESS
} SCREEN_STATE;

SCREEN_STATE current_screen = HOME_SCREEN;   // current screen state
SCREEN_STATE last_screen = (SCREEN_STATE)-1; // last screen state

// Removed "Menu" header string so item indexes align 1:1 with enum actions
char *menu[] = {
    "Menu",       // 0
    "Alarm",      // 1
    "Date",       // 2
    "Time",       // 3
    "Format",     // 4
    "Stop Watch", // 5
    "Timer",      // 6
    "Brightness", // 7
    "Back"};      // 8

static const uint8_t menu_size = sizeof(menu) / sizeof(menu[0]);

uint8_t menu_index = 0; // Selected menu item index (0 to menu_size - 1)
uint8_t menu_needs_redraw = 1;
uint32_t menu_timeout = 0;

// this is used to keep track of the last cursor position on the screen, so we can clear it when the cursor moves
uint8_t re_draws = 1;
int8_t last_cursor = -1;

uint8_t cursor_position = 0; // 0 = hundreds, 1 = tens, 2 = ones

/* Private define */
void display_menu(char *menu[], uint8_t menu_count, uint8_t selected_index);
void handle_menu(Switch_t sw);
void home_screen(Switch_t sw);

void set_date_screen();
void set_time_screen();
void set_format(Switch_t sw);

void set_brightness(Switch_t sw);

Switch_t handle_date_input(Switch_t sw, TimeDate_t *time);
Switch_t handle_time_input(Switch_t sw, TimeDate_t *time);

void display_menu(char *menu[], uint8_t menu_count, uint8_t menu_index)
{
    static char last_lines[2][17] = {{0}};
    char line[17]; // 16 characters + '\0'

    if (menu_needs_redraw)
    {
        memset(last_lines, 0xFF, sizeof(last_lines));
        menu_needs_redraw = 0;
    }

    for (uint8_t row = 0; row < 2; row++)
    {
        uint8_t item = menu_index + row;

        Lcd_cursor(lcd_ptr, row, 0);

        if (item < menu_count)
        {
            // 1-char selector '>' or ' ' + 15-char left-aligned string = 16 chars total
            snprintf(line, sizeof(line), "%c%-15s",
                     (row == 0) ? '>' : ' ',
                     menu[item]);
        }
        else
        {
            // Blank line padded to 16 spaces
            snprintf(line, sizeof(line), "%-16s", "");
        }

        // Skip redraw if the line hasn't changed to avoid screen flicker
        if (memcmp(last_lines[row], line, 16) == 0)
            continue;

        memcpy(last_lines[row], line, 17);

        Lcd_string(lcd_ptr, line);
    }
}

void handle_menu(Switch_t sw)
{
    if (sw == SW_UP)
    {
        if (menu_index > 0)
            menu_index--;
    }

    if (sw == SW_DOWN)
    {
        if (menu_index < menu_size - 1)
            menu_index++;
    }

    if (sw == SW_BACK)
    {
        current_screen = HOME_SCREEN;
        menu_index = 0;
        menu_needs_redraw = 1;
        Lcd_clear(lcd_ptr);
        return;
    }

    if (sw == SW_ENTER)
    {
        switch (menu_index)
        {
        case 0:
            current_screen = MENU_SCREEN;
            break;
        case 1:
            current_screen = ALARM_SCREEN;
            break;
        case 2:
            current_screen = DATE_SCREEN;
            break;
        case 3:
            current_screen = TIME_SCREEN;
            break;
        case 4:
            current_screen = FORMAT_SCREEN;
            break;
        case 5:
            current_screen = STOPWATCH_SCREEN;
            break;
        case 6:
            current_screen = TIMER_SCREEN;
            break;
        case 7:
            current_screen = BRIGHTNESS;
            break;
        case 8: // "Back" option inside menu
            current_screen = HOME_SCREEN;
            break;
        default:
            break;
        }
    }
}

char *get_weekday_name(uint8_t weekday)
{
    switch (weekday)
    {
    case 1:
        return "Sun";
    case 2:
        return "Mon";
    case 3:
        return "Tue";
    case 4:
        return "Wed";
    case 5:
        return "Thu";
    case 6:
        return "Fri";
    case 7:
        return "Sat";
    default:
        return "";
    }
}

void ui_init(Lcd_HandleTypeDef *lcd, RTC_HandleTypeDef *hrtc)
{
    lcd_ptr = lcd;   // Store the LCD handle pointer for use in UI functions
    hrtc_ptr = hrtc; // Store the RTC handle pointer for use in UI functions
}

uint8_t ui_is_home(void)
{
    return current_screen == HOME_SCREEN;
}

void ui_task(void)
{
    Switch_t sw = read_switch(); // Read the switch input

    /* Handle menu Sleep timeout */
    if (current_screen != HOME_SCREEN)
    {
        if (sw != SW_NONE)
        {
            menu_timeout = HAL_GetTick() + MENU_TIMEOUT_MS;
        }
        else if ((int32_t)(HAL_GetTick() - menu_timeout) >= 0)
        {
            current_screen = HOME_SCREEN;
            menu_index = 0;
            menu_needs_redraw = 1;
            Lcd_clear(lcd_ptr);

            home_screen(SW_NONE); // Update the home screen display after timeout
        }
    }

    // Handle screen transition cleans
    if (current_screen != last_screen)
    {
        last_screen = current_screen;
        menu_index = 0;

        if (current_screen == MENU_SCREEN)
        {
            menu_needs_redraw = 1;
        }

        if (current_screen != HOME_SCREEN)
        {
            menu_timeout = HAL_GetTick() + MENU_TIMEOUT_MS;
        }

        /* Update backlight based on current screen */
        HAL_GPIO_WritePin(BACKLIGHT_PORT, BACKLIGHT_PIN,
                          (current_screen == HOME_SCREEN) ? GPIO_PIN_RESET : GPIO_PIN_SET);

        Lcd_clear(lcd_ptr);
    }

    switch (current_screen)
    {
    case HOME_SCREEN: // Home Screen
        home_screen(sw);
        break;
    case MENU_SCREEN: // Menu Screen
        // FIX: Process input FIRST so menu_index updates, then draw immediately
        handle_menu(sw);
        if (current_screen == MENU_SCREEN)
        {
            display_menu(menu, menu_size, menu_index);
        }
        break;
    case DATE_SCREEN: // Date Screen
        set_date_screen();
        if (handle_date_input(sw, &current_time_date) == SW_ENTER)
        {
            // apply date changes to RTC
            RTC_DateTypeDef sDate = {0};

            sDate.WeekDay = current_time_date.dayofweek;
            sDate.Date = current_time_date.dayofmonth;
            sDate.Month = current_time_date.month;
            sDate.Year = current_time_date.year;

            HAL_RTC_SetDate(hrtc_ptr, &sDate, RTC_FORMAT_BIN);
        }
        break;
    case TIME_SCREEN:
        set_time_screen();
        if (handle_time_input(sw, &current_time_date) == SW_ENTER)
        {
            // apply date and time changes to RTC
            RTC_TimeTypeDef sTime = {0};

            sTime.Hours = current_time_date.hours;
            sTime.Minutes = current_time_date.minutes;

            HAL_RTC_SetTime(hrtc_ptr, &sTime, RTC_FORMAT_BIN);
        }
        break;
    case BRIGHTNESS:
        set_brightness(sw);
        break;
    case ALARM_SCREEN:
        break;
    case FORMAT_SCREEN:
        set_format(sw);
        break;
    case STOPWATCH_SCREEN:
        break;
    case TIMER_SCREEN:
        break;
    };
}

void home_screen(Switch_t sw)
{
    if (sw == SW_ENTER_LONG)
    {
        current_screen = MENU_SCREEN;
        menu_index = 0;
        menu_needs_redraw = 1;
        return;
    }
    RTC_TimeTypeDef t;
    RTC_DateTypeDef d;

    _time_format = get_time_format(); // Get the current time format

    // Read and display current RTC time/date
    HAL_RTC_GetTime(hrtc_ptr, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(hrtc_ptr, &d, RTC_FORMAT_BIN);

    char row1[17];
    char row2[17];

    // snprintf(row1, sizeof(row1), "%02d:%02d", t.Hours, t.Minutes);

    // MM:HH day of the week
    if (_time_format == _12_HOUR_MODE)
    {
        uint8_t display_hours = t.Hours % 12;
        if (display_hours == 0)
            display_hours = 12; // Handle midnight and noon
        snprintf(row1, sizeof(row1), "%02d:%02d %s %s", display_hours, t.Minutes, (t.Hours >= 12) ? "PM" : "AM", get_weekday_name(d.WeekDay));
    }
    else
    {
        snprintf(row1, sizeof(row1), "%02d:%02d %s", t.Hours, t.Minutes, get_weekday_name(d.WeekDay));
    }

    snprintf(row2, sizeof(row2), "%02d/%02d/%04d", d.Date, d.Month, 2000 + d.Year);

    Lcd_cursor(lcd_ptr, 0, (16 - strlen(row1)) / 2); // Center row1
    Lcd_string(lcd_ptr, row1);

    Lcd_cursor(lcd_ptr, 1, (16 - strlen(row2)) / 2); // Center row2
    Lcd_string(lcd_ptr, row2);
}

void set_date_screen()
{
    static char line1[17];
    static char last_line2[17] = "";

    if (re_draws)
    {
        // get the actual date from RTC
        RTC_DateTypeDef d;
        HAL_RTC_GetDate(hrtc_ptr, &d, RTC_FORMAT_BIN);
        current_time_date.dayofweek = d.WeekDay;
        current_time_date.dayofmonth = d.Date;
        current_time_date.month = d.Month;
        current_time_date.year = d.Year;

        // Force redraw flags
        memset(last_line2, 0xFF, sizeof(last_line2));
        re_draws = 0;
        last_cursor = -1;
    }

    // Display the date input screen
    snprintf(line1, sizeof(line1), "%02d/%02d/%02d %s",
             current_time_date.dayofmonth,
             current_time_date.month,
             current_time_date.year,
             get_weekday_name(current_time_date.dayofweek));
    Lcd_cursor(lcd_ptr, 0, 0);
    Lcd_string(lcd_ptr, line1);

    // line 2 is the cursor line, which will be drawn below the selected field
    // Line 3: Display cursor position
    if (last_cursor != cursor_position)
    {
        if (last_cursor >= 0)
        {
            Lcd_cursor(lcd_ptr, 1, 0);
            Lcd_string(lcd_ptr, "                    "); // Clear line
        }

        // Calculate cursor position based on the selected field
        uint8_t cursor_col = 0;
        switch (cursor_position)
        {
        case 0: // Day
            cursor_col = 0;
            break;
        case 1: // Month
            cursor_col = 3;
            break;
        case 2: // Year
            cursor_col = 6;
            break;
        case 3: // Day of week
            cursor_col = 10;
            break;
        }

        Lcd_cursor(lcd_ptr, 1, cursor_col);
        Lcd_string(lcd_ptr, "^");

        last_cursor = cursor_position;
    }
}

void set_time_screen()
{
    static char line1[17];
    static char last_line2[17] = "";

    if (re_draws)
    {
        _time_format = get_time_format(); // Get the current time format

        // get the actual date from RTC
        RTC_TimeTypeDef d;
        HAL_RTC_GetTime(hrtc_ptr, &d, RTC_FORMAT_BIN);

        current_time_date.hours = d.Hours;
        current_time_date.minutes = d.Minutes;

        // Force redraw flags
        memset(last_line2, 0xFF, sizeof(last_line2));
        re_draws = 0;
        last_cursor = -1;
    }

    uint8_t display_hours = current_time_date.hours;
    const char *period = "";

    if (_time_format == _12_HOUR_MODE)
    {
        display_hours = current_time_date.hours % 12;
        if (display_hours == 0)
            display_hours = 12;
        period = (current_time_date.hours >= 12) ? "PM" : "AM";
    }

    snprintf(line1, sizeof(line1), "%02d:%02d %s",
             display_hours, current_time_date.minutes, period);

    Lcd_cursor(lcd_ptr, 0, 0);
    Lcd_string(lcd_ptr, line1);

    // line 2 is the cursor line, which will be drawn below the selected field
    // Line 3: Display cursor position
    if (last_cursor != cursor_position)
    {
        if (last_cursor >= 0)
        {
            Lcd_cursor(lcd_ptr, 1, 0);
            Lcd_string(lcd_ptr, "                    "); // Clear line
        }

        // Calculate cursor position based on the selected field
        uint8_t cursor_col = 0;
        switch (cursor_position)
        {
        case 0: // Hours
            cursor_col = 0;
            break;
        case 1: // Minutes
            cursor_col = 3;
            break;
        case 2: // AM/PM
            cursor_col = 6;
            break;
        }

        Lcd_cursor(lcd_ptr, 1, cursor_col);
        Lcd_string(lcd_ptr, "^");

        last_cursor = cursor_position;
    }
}

/* Returns the number of days in a given month and year */
static uint8_t days_in_month(uint8_t month, uint8_t year)
{
    if (month == 2)
    {
        return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) ? 29 : 28;
    }

    if (month == 4 || month == 6 || month == 9 || month == 11)
    {
        return 30;
    }

    return 31;
}

Switch_t handle_date_input(Switch_t sw, TimeDate_t *time)
{
    switch (sw)
    {
    case SW_UP:
    {
        switch (cursor_position)
        {
        case 0: // Day
            if (time->dayofmonth < days_in_month(time->month, time->year))
                (time->dayofmonth)++;
            break;
        case 1: // Month
            if (time->month < 12)
                (time->month)++;
            if (time->dayofmonth > days_in_month(time->month, time->year))
                time->dayofmonth = days_in_month(time->month, time->year);
            break;
        case 2: // Year
            if (time->year < 99)
                (time->year)++;
            if (time->dayofmonth > days_in_month(time->month, time->year))
                time->dayofmonth = days_in_month(time->month, time->year);
            break;
        case 3: // DOW
            if (time->dayofweek < 7)
                (time->dayofweek)++;
            break;
        }
        break;
    }

    case SW_DOWN:
    {
        switch (cursor_position)
        {
        case 0: // Day
            if (time->dayofmonth > 1)
                (time->dayofmonth)--;
            break;
        case 1: // Month
            if (time->month > 1)
                (time->month)--;
            if (time->dayofmonth > days_in_month(time->month, time->year))
                time->dayofmonth = days_in_month(time->month, time->year);
            break;
        case 2: // Year
            if (time->year > 26)
                (time->year)--;
            if (time->dayofmonth > days_in_month(time->month, time->year))
                time->dayofmonth = days_in_month(time->month, time->year);
            break;
        case 3: // DOW
            if (time->dayofweek > 1)
                (time->dayofweek)--;
            break;
        }
        break;
    }

    case SW_ENTER:
    {
        if (cursor_position < 3)
        {
            cursor_position++;
        }
        else
        {
            // Finalize the date and time input
            cursor_position = 0;
            re_draws = 1;
            current_screen = MENU_SCREEN;
            Lcd_clear(lcd_ptr);

            re_draws = 1; // reset re_draws counter

            return SW_ENTER;
        }
        break;
    }

    case SW_BACK:
    {
        if (cursor_position > 0)
        {
            cursor_position--;
        }
        else
        {
            cursor_position = 0;
            current_screen = MENU_SCREEN;
            Lcd_clear(lcd_ptr);

            re_draws = 1; // reset re_draws counter

            return SW_BACK;
        }
        break;
    };

    default:
        break;
    }

    return SW_NONE;
}

Switch_t handle_time_input(Switch_t sw, TimeDate_t *time)
{
    switch (sw)
    {
    case SW_UP:
    {
        if (_time_format == _12_HOUR_MODE) // 12-hour mode
        {
            switch (cursor_position)
            {
            case 0: // Hours
                if (time->hours % 12 == 11)
                    time->hours -= 11;
                else
                    time->hours++;
                break;
            case 1: // Minutes
                if (time->minutes < 59)
                    (time->minutes)++;
                else
                    time->minutes = 0; // Wrap around to 0
                break;
            case 2: // hadnle AM/PM toggle
                time->hours = (time->hours + 12) % 24;
                break;
            }
            break;
        }
        else // 24-hour mode
        {
            switch (cursor_position)
            {
            case 0: // Hours
                if (time->hours < 23)
                    (time->hours)++;
                else
                    time->hours = 0; // Wrap around to 0
                break;
            case 1: // Minutes
                if (time->minutes < 59)
                    (time->minutes)++;
                else
                    time->minutes = 0; // Wrap around to 0
                break;
            }
            break;
        }
        break;
    }
    case SW_DOWN:
    {
        // handle 24 and 12 hour modes
        if (_time_format == _12_HOUR_MODE) // 12-hour mode
        {
            switch (cursor_position)
            {
            case 0: // Hours
                if (time->hours % 12 == 1)
                    time->hours += 11;
                else
                    time->hours--;
                break;
            case 1: // Minutes
                if (time->minutes > 0)
                    (time->minutes)--;
                else
                    time->minutes = 59; // Wrap around to 59
                break;
            case 2: // handle AM/PM toggle
                time->hours = (time->hours + 12) % 24;
                break;
            }
            break;
        }
        else // 24-hour mode
        {
            switch (cursor_position)
            {
            case 0: // Hours
                if (time->hours > 0)
                    (time->hours)--;
                else
                    time->hours = 23; // Wrap around to 23
                break;
            case 1: // Minutes
                if (time->minutes > 0)
                    (time->minutes)--;
                else
                    time->minutes = 59; // Wrap around to 59
                break;
            }
            break;
        }
        break;
    }
    case SW_ENTER:
    {
        uint8_t last_cursor_position = (_time_format == _12_HOUR_MODE) ? 2 : 1;

        if (cursor_position < last_cursor_position)
        {
            cursor_position++;
        }
        else
        {
            // Finalize the time input
            cursor_position = 0;
            re_draws = 1;
            current_screen = MENU_SCREEN;
            Lcd_clear(lcd_ptr);

            re_draws = 1; // reset re_draws counter

            return SW_ENTER;
        }
        break;
    }
    case SW_BACK:
    {
        if (cursor_position > 0)
        {
            cursor_position--;
        }
        else
        {
            cursor_position = 0;
            current_screen = MENU_SCREEN;
            Lcd_clear(lcd_ptr);

            re_draws = 1; // reset re_draws counter

            return SW_BACK;
        }
        break;
    }

    default:
        break;
    }

    return SW_NONE;
}

void set_format(Switch_t sw)
{
    TimeFormat_t local_format = get_time_format(); // Get the current time format

    static uint8_t cursor_pos = 0; // 0 = 24-hour, 1 = 12-hour

    if (re_draws)
    {
        // Force redraw flags
        re_draws = 0;
        last_cursor = -1;

        local_format = get_time_format(); // Get the current time format

        // Set cursor position based on current time format
        if (local_format == _24_HOUR_MODE)
        {
            cursor_pos = 0;
        }
        else if (local_format == _12_HOUR_MODE)
        {
            cursor_pos = 1;
        }
    }

    if (sw == SW_UP)
    {
        if (cursor_pos > 0)
            cursor_pos--;
    }
    else if (sw == SW_DOWN)
    {
        if (cursor_pos < 1)
            cursor_pos++;
    }
    else if (sw == SW_ENTER)
    {
        // Toggle the bypass state based on cursor position
        if (cursor_pos == 0)
        {
            set_time_format(_24_HOUR_MODE);
        }
        else if (cursor_pos == 1)
        {
            set_time_format(_12_HOUR_MODE);
        }

        // Update the display to reflect the new state
        re_draws = 1;     // Force redraw to update the display
        last_cursor = -1; // Reset the last cursor position
        cursor_pos = -1;
        current_screen = MENU_SCREEN; // Return to main menu after selection
    }
    else if (sw == SW_BACK)
    {
        current_screen = MENU_SCREEN;
        menu_index = 0; // reset menu index
        menu_needs_redraw = 1;
        re_draws = 1;    // reset re_draws counter
        last_cursor = 0; // reset last_cursor when entering home screen
        cursor_pos = 0;  // reset cursor position when entering home screen
        Lcd_clear(lcd_ptr);
    }

    if (cursor_pos == 0)
    {
        Lcd_cursor(lcd_ptr, 0, 0);
        Lcd_string(lcd_ptr, "> 24-HOUR MODE");

        Lcd_cursor(lcd_ptr, 1, 0);
        Lcd_string(lcd_ptr, "  12-HOUR MODE");
    }
    else if (cursor_pos == 1)
    {
        Lcd_cursor(lcd_ptr, 0, 0);
        Lcd_string(lcd_ptr, "  24-HOUR MODE");

        Lcd_cursor(lcd_ptr, 1, 0);
        Lcd_string(lcd_ptr, "> 12-HOUR MODE");
    }
}

void set_brightness(Switch_t sw)
{
    static uint8_t brightness_level = 5; // Default brightness level (0-10)
    static char line1[17];

    if (re_draws)
    {
        re_draws = 0;
        last_cursor = -1;
    }

    // Display the brightness input screen
    snprintf(line1, sizeof(line1), "Brightness: %d", brightness_level);
    Lcd_cursor(lcd_ptr, 0, 0);
    Lcd_string(lcd_ptr, line1);

    // Handle user input for brightness adjustment
    if (sw == SW_UP)
    {
        if (brightness_level < 10)
            brightness_level++;
    }
    else if (sw == SW_DOWN)
    {
        if (brightness_level > 0)
            brightness_level--;
    }
}

/*
 * menu.c
 *
 *  Created on: Nov 15, 2025
 *      Author: DRIFTYY777
 */

#include <menu.h>
#include <16_2_display.h>
#include <buttons.h>

#include <stdio.h>
#include <string.h>

// Menu state
static MenuState menu_state = {0};

// Flag to prevent rapid menu updates
static uint32_t last_update_time = 0;
#define MENU_UPDATE_DELAY_MS 200

void Menu_Init(MenuItem* main_menu, uint8_t menu_size) {
    menu_state.current_menu = main_menu;
    menu_state.current_menu_size = menu_size;
    menu_state.selected_index = 0;
    menu_state.parent_menu = NULL;
    menu_state.parent_menu_size = 0;
    menu_state.parent_index = 0;
    
    Menu_Display();
}

void Menu_Display(void) {
    if (menu_state.current_menu == NULL || menu_state.current_menu_size == 0) {
        return;
    }
    
    // Get current selected item
    MenuItem* selected = &menu_state.current_menu[menu_state.selected_index];
    
    // Prepare display strings
    char line1[17] = {0};
    char line2[17] = {0};
    
    // Line 1: Show previous item if not first
    if (menu_state.selected_index > 0) {
        snprintf(line1, sizeof(line1), " %s", 
                 menu_state.current_menu[menu_state.selected_index - 1].text);
    } else {
        snprintf(line1, sizeof(line1), " --- MENU ---");
    }
    
    // Line 2: Show current selected item with cursor
    snprintf(line2, sizeof(line2), ">%s", selected->text);
    
    // Display on LCD
    LCD16X2_Clear();
    LCD16X2_Set_Cursor(1, 1);
    LCD16X2_Write_String(line1);
    LCD16X2_Set_Cursor(2, 1);
    LCD16X2_Write_String(line2);
}

void Menu_EnterSubmenu(MenuItem* submenu, uint8_t submenu_size) {
    if (submenu == NULL || submenu_size == 0) {
        return;
    }
    
    // Save current state as parent
    menu_state.parent_menu = menu_state.current_menu;
    menu_state.parent_menu_size = menu_state.current_menu_size;
    menu_state.parent_index = menu_state.selected_index;
    
    // Enter sub menu
    menu_state.current_menu = submenu;
    menu_state.current_menu_size = submenu_size;
    menu_state.selected_index = 0;
    
    Menu_Display();
}

void Menu_GoBack(void) {
    if (menu_state.parent_menu != NULL) {
        // Restore parent menu
        menu_state.current_menu = menu_state.parent_menu;
        menu_state.current_menu_size = menu_state.parent_menu_size;
        menu_state.selected_index = menu_state.parent_index;
        
        // Clear parent reference (we're now at parent level)
        menu_state.parent_menu = NULL;
        menu_state.parent_menu_size = 0;
        menu_state.parent_index = 0;
        
        Menu_Display();
    }
}

void Menu_ExecuteSelected(void) {
    if (menu_state.current_menu == NULL || menu_state.selected_index >= menu_state.current_menu_size) {
        return;
    }
    
    MenuItem* selected = &menu_state.current_menu[menu_state.selected_index];
    
    // Check if it's a back button
    if (selected->is_back) {
        Menu_GoBack();
        return;
    }
    
    // Check if it has a sub menu
    if (selected->submenu != NULL && selected->submenu_size > 0) {
        Menu_EnterSubmenu(selected->submenu, selected->submenu_size);
        return;
    }
    
    // Execute callback if it exists
    if (selected->callback != NULL) {
        selected->callback();
        // Redisplay menu after action
        Menu_Display();
    }
}

void Menu_Update(void) {
    // Throttle updates to prevent too rapid changes
    uint32_t current_time = HAL_GetTick();
    if (current_time - last_update_time < MENU_UPDATE_DELAY_MS) {
        return;
    }
    
    bool needs_update = false;
    
    // UP button - move selection up
    if (get_btn(UP_BUTTON)) {
        if (menu_state.selected_index > 0) {
            menu_state.selected_index--;
        } else {
            // Wrap to bottom
            menu_state.selected_index = menu_state.current_menu_size - 1;
        }
        needs_update = true;
        last_update_time = current_time;
    }
    
    // DOWN button - move selection down
    if (get_btn(DOWN_BUTTON)) {
        if (menu_state.selected_index < menu_state.current_menu_size - 1) {
            menu_state.selected_index++;
        } else {
            // Wrap to top
            menu_state.selected_index = 0;
        }
        needs_update = true;
        last_update_time = current_time;
    }
    
    // ENTER button - execute selected item
    if (get_btn(ENTER_BUTTON)) {
        Menu_ExecuteSelected();
        last_update_time = current_time;
        return; // ExecuteSelected already updates display
    }
    
    // BACK button (physical button) - go back to parent menu
    if (get_btn(BACK_BUTTON)) {
        if (menu_state.parent_menu != NULL) {
            Menu_GoBack();
        }
        last_update_time = current_time;
        return; // GoBack already updates display
    }
    
    // Update display if selection changed
    if (needs_update) {
        Menu_Display();
    }
}
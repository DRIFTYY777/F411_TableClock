/*
 * menu.h
 *
 *  Created on: Nov 15, 2025
 *      Author: DRIFTYY777
 */

#ifndef INC_MENU_H_
#define INC_MENU_H_

#include <stdint.h>
#include <stdbool.h>

// Menu item callback function type
typedef void (*MenuCallback)(void);

// Menu item structure
typedef struct MenuItem {
    const char* text;           // Display text for the menu item
    MenuCallback callback;      // Function to execute when selected (NULL if submenu)
    struct MenuItem* submenu;   // Pointer to submenu (NULL if action item)
    uint8_t submenu_size;       // Number of items in submenu
    bool is_back;               // True if this is a "Back" menu item
} MenuItem;

// Menu state structure
typedef struct {
    MenuItem* current_menu;     // Current menu being displayed
    uint8_t current_menu_size;  // Size of current menu
    uint8_t selected_index;     // Currently selected menu item
    MenuItem* parent_menu;      // Parent menu for back navigation
    uint8_t parent_menu_size;   // Size of parent menu
    uint8_t parent_index;       // Selected index in parent menu
} MenuState;

// Initialize the menu system with main menu
void Menu_Init(MenuItem* main_menu, uint8_t menu_size);

// Update menu (call this in main loop) - handles button input
void Menu_Update(void);

// Display the current menu on LCD
void Menu_Display(void);

// Navigate to a submenu
void Menu_EnterSubmenu(MenuItem* submenu, uint8_t submenu_size);

// Go back to parent menu
void Menu_GoBack(void);

// Execute the selected menu item
void Menu_ExecuteSelected(void);

#endif /* INC_MENU_H_ */
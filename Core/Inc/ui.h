/*
 * ui.h
 *
 *  Created on: Nov 15, 2025
 *      Author: DRIFTYY777
 */

#ifndef INC_UI_H_
#define INC_UI_H_

#include <stddef.h>
#include <menu.h>
#include <stdbool.h>

// UI Mode enumeration
typedef enum {
    UI_MODE_HOME,
    UI_MODE_MENU
} UIMode;

/* We have to send this values to menu creator */
extern MenuItem settings_menu[];
extern MenuItem data_menu[];
extern MenuItem main_menu[];

// Initialize UI system
void UI_Init(void);

// Update UI (call in main loop)
void UI_Update(void);

// Get current UI mode
UIMode UI_GetMode(void);

// Switch to menu mode
void UI_EnterMenu(void);

// Switch to home mode
void UI_ExitMenu(void);

// Display home screen
void UI_ShowHomeScreen(void);

#endif /* INC_UI_H_ */
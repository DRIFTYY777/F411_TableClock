/*
 * ui.h
 *
 *  Created on: 05-Jul-2026
 *      Author: DRIFTYY
 */

#ifndef INC_UI_H_
#define INC_UI_H_

typedef enum
{
	HOME_SCREEN,
	MENU_SCREEN,

	DATE_SCREEN,
	TIME_SCREEN,

	BACKLIGHT_SCREEN,

	INFO_SCREEN
};

char *menu[] = {
	"MENU",
	"Set Date",
	"Set Time",
	"Back-Light"};

void ui_task();

#endif /* INC_UI_H_ */

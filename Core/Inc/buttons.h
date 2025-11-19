/*
 * buttons.h
 *
 *  Created on: Nov 15, 2025
 *      Author: DRIFTYY
 */

#ifndef INC_BUTTONS_H_
#define INC_BUTTONS_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>

// for button enumeration
enum Button {
	UP_BUTTON,
	DOWN_BUTTON,
	BACK_BUTTON,
	ENTER_BUTTON
};

// Initialize all button GPIOs
void init_buttons(void);

// returns true if button is pressed with debouncing (only once per press)
bool get_btn(enum Button button);

#endif /* INC_BUTTONS_H_ */

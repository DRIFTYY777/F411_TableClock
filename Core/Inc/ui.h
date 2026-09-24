/*
 * ui.h
 *
 *  Created on: 24-Jul-2026
 *      Author: dhima
 */

#ifndef INC_UI_H_
#define INC_UI_H_

#include "lcd.h"
#include "main.h"

void ui_init(Lcd_HandleTypeDef *lcd, RTC_HandleTypeDef *hrtc);
void ui_task(void);
uint8_t ui_is_home(void);

#endif /* INC_UI_H_ */

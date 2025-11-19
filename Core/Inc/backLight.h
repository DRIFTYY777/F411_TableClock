/*
 * backLight.h
 *
 *  Created on: Nov 15, 2025
 *      Author: DRIFTYY777
 */

#ifndef INC_BACKLIGHT_H_
#define INC_BACKLIGHT_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>

// EEPROM address for backlight state storage
#define BACKLIGHT_EEPROM_ADDR 0x0001

void beginBacklight(GPIO_TypeDef* BL_GPIOx, uint16_t BL_PINx);
void setBacklight(bool state);

#endif /* INC_BACKLIGHT_H_ */

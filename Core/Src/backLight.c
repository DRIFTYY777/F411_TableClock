/*
 * backLight.c
 *
 *  Created on: Nov 15, 2025
 *      Author: DRIFTYY777
 */

#include <backLight.h>

GPIO_TypeDef* _BL_GPIOx;
uint16_t _BL_PINx;

void beginBacklight(GPIO_TypeDef* BL_GPIOx, uint16_t BL_PINx){
	// Initialize the GPIO for backlight control
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(BL_GPIOx, BL_PINx, GPIO_PIN_RESET);

	/*Configure GPIO pin : BL_Pin */
	GPIO_InitStruct.Pin = BL_PINx;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(BL_GPIOx, &GPIO_InitStruct);

	// Store GPIO references for later use
	_BL_GPIOx = BL_GPIOx;
	_BL_PINx = BL_PINx;

}

void setBacklight(bool state){
	// Set the pin state and save new value to EEPROM
	if(state){
		HAL_GPIO_WritePin(_BL_GPIOx, _BL_PINx, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(_BL_GPIOx, _BL_PINx, GPIO_PIN_RESET);
	}
}

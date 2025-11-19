/*
 * 16_2_display.h
 *
 *  Created on: Nov 15, 2025
 *      Author: DRIFTYY777
 */

#ifndef INC_16_2_DISPLAY_H_
#define INC_16_2_DISPLAY_H_
#include "stm32f4xx_hal.h"

typedef struct {
	GPIO_TypeDef* rs_Port;
	uint16_t rs_Pin;
	GPIO_TypeDef* rw_Port;
	uint16_t rw_Pin;
	GPIO_TypeDef* en_Port;
	uint16_t en_Pin;
	GPIO_TypeDef* d4_Port;
	uint16_t d4_Pin;
	GPIO_TypeDef* d5_Port;
	uint16_t d5_Pin;
	GPIO_TypeDef* d6_Port;
	uint16_t d6_Pin;
	GPIO_TypeDef* d7_Port;
	uint16_t d7_Pin;
} LCD16X2_HandleTypeDef;

void LCD16X2_Init(LCD16X2_HandleTypeDef* lcd);
void LCD16X2_Clear();
void LCD16X2_Set_Cursor(uint8_t row, uint8_t col);
void LCD16X2_Write_String(const char* str);

#endif /* INC_16_2_DISPLAY_H_ */

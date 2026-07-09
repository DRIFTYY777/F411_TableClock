/**
 * lcd.h
 *
 *  Created on: Feb 14, 2026
 *      Author: DRIFTYY777
 */

#ifndef LCD_H_
#define LCD_H_

#include "main.h"

typedef struct
{
    GPIO_TypeDef *rs_Port;
    uint16_t rs_Pin;
    GPIO_TypeDef *rw_Port;
    uint16_t rw_Pin;
    GPIO_TypeDef *en_Port;
    uint16_t en_Pin;

#ifdef LCD_8_BIT_MODE
    GPIO_TypeDef *d0_Port;
    uint16_t d0_Pin;
    GPIO_TypeDef *d1_Port;
    uint16_t d1_Pin;
    GPIO_TypeDef *d2_Port;
    uint16_t d2_Pin;
    GPIO_TypeDef *d3_Port;
    uint16_t d3_Pin;
    GPIO_TypeDef *d4_Port;
    uint16_t d4_Pin;
    GPIO_TypeDef *d5_Port;
    uint16_t d5_Pin;
    GPIO_TypeDef *d6_Port;
    uint16_t d6_Pin;
    GPIO_TypeDef *d7_Port;
    uint16_t d7_Pin;
#else
    GPIO_TypeDef *d4_Port;
    uint16_t d4_Pin;
    GPIO_TypeDef *d5_Port;
    uint16_t d5_Pin;
    GPIO_TypeDef *d6_Port;
    uint16_t d6_Pin;
    GPIO_TypeDef *d7_Port;
    uint16_t d7_Pin;
#endif
} LCD_HandleTypeDef;

/* Initialize */
void LCD_Init(LCD_HandleTypeDef *lcd);
void LCD_Clear();
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *str);
void LCD_Printf(const char *format, ...);

#endif /* LCD_H_ */

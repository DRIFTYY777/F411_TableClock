/*
 * 16_2_display.c
 *
 *  Created on: Nov 15, 2025
 *      Author: DRIFTYY777
 */

#include <16_2_display.h>
#include <stdbool.h>

// private copy of lcd handle
static LCD16X2_HandleTypeDef *MyLCD = NULL;

// Mutex flag to prevent concurrent display updates
static volatile uint8_t display_updating = 0;

// Flag used by button ISR to avoid interfering with LCD timing
volatile uint8_t lcd_busy = 0;

/*
 * @brief Send command and data to LCD
 * @param value: 8-bit command or data value
 *
 * @note if we want to send command, isData = false
 * @note if we want to send data, isData = true
 *
 */
void LCD16X2_Send(uint8_t value, bool isData)
{
    lcd_busy = 1;

    // RS: 0 = Command, 1 = Data
    HAL_GPIO_WritePin(MyLCD->rs_Port, MyLCD->rs_Pin,
                      isData ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // RW = 0 (always write)
    HAL_GPIO_WritePin(MyLCD->rw_Port, MyLCD->rw_Pin, GPIO_PIN_RESET);

    // ---------- Send high nibble ----------
    HAL_GPIO_WritePin(MyLCD->d4_Port, MyLCD->d4_Pin, (value & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d5_Port, MyLCD->d5_Pin, (value & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d6_Port, MyLCD->d6_Pin, (value & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d7_Port, MyLCD->d7_Pin, (value & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Pulse EN
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_RESET);

    // ---------- Send low nibble ----------
    HAL_GPIO_WritePin(MyLCD->d4_Port, MyLCD->d4_Pin, (value & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d5_Port, MyLCD->d5_Pin, (value & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d6_Port, MyLCD->d6_Pin, (value & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d7_Port, MyLCD->d7_Pin, (value & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Pulse EN
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_RESET);

    HAL_Delay(2); // wait for LCD internal processing
    lcd_busy = 0;
}

void LCD16X2_Begin(){
    HAL_Delay(50);

    // Ensure all pins start LOW
	HAL_GPIO_WritePin(MyLCD->rw_Port, MyLCD->rw_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MyLCD->d4_Port, MyLCD->d4_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MyLCD->d5_Port, MyLCD->d5_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MyLCD->d6_Port, MyLCD->d6_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MyLCD->d7_Port, MyLCD->d7_Pin, GPIO_PIN_RESET);

	// Initialization sequence
	LCD16X2_Send(0x03, false);
	HAL_Delay(5);

	LCD16X2_Send(0x03, false);
	HAL_Delay(5);

	LCD16X2_Send(0x02, false); // Set to 4-bit mode
	HAL_Delay(5);

	LCD16X2_Send(0x28, false); // 4-bit, 2 line, 5x8 dots
	LCD16X2_Send(0x0C, false); // Display ON, Cursor OFF

	LCD16X2_Send(0x06, false); // Entry mode set: Increment cursor
	LCD16X2_Clear();
}

void init_pins(){

	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	 // init all the pins be ready for any port on any pin
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	  // RS Pin
	  GPIO_InitStruct.Pin = MyLCD->rs_Pin;
	  HAL_GPIO_Init(MyLCD->rs_Port, &GPIO_InitStruct);

	  // RW Pin
	  GPIO_InitStruct.Pin = MyLCD->rw_Pin;
	  HAL_GPIO_Init(MyLCD->rw_Port, &GPIO_InitStruct);

	  // EN Pin
	  GPIO_InitStruct.Pin = MyLCD->en_Pin;
	  HAL_GPIO_Init(MyLCD->en_Port, &GPIO_InitStruct);

	  // D4 Pin
	  GPIO_InitStruct.Pin = MyLCD->d4_Pin;
	  HAL_GPIO_Init(MyLCD->d4_Port, &GPIO_InitStruct);

	  // D5 Pin
	  GPIO_InitStruct.Pin = MyLCD->d5_Pin;
	  HAL_GPIO_Init(MyLCD->d5_Port, &GPIO_InitStruct);

	  // D6 Pin
	  GPIO_InitStruct.Pin = MyLCD->d6_Pin;
	  HAL_GPIO_Init(MyLCD->d6_Port, &GPIO_InitStruct);

	  // D7 Pin
	  GPIO_InitStruct.Pin = MyLCD->d7_Pin;
	  HAL_GPIO_Init(MyLCD->d7_Port, &GPIO_InitStruct);
}

void LCD16X2_Init(LCD16X2_HandleTypeDef* lcd){
	// store the handle so the others can use it
	MyLCD = lcd; // store the handle
	init_pins();
	// wait for power to stabilize
	HAL_Delay(50);

	LCD16X2_Begin();
}

void LCD16X2_Clear(){
	LCD16X2_Send(0x01, false); // Clear display command
	HAL_Delay(2); // Wait for clear to complete
}

void LCD16X2_Set_Cursor(uint8_t row, uint8_t col){
		uint8_t address;

	// Calculate address based on row and column
	switch (row) {
		case 1:
			address = 0x00 + (col - 1);
			break;
		case 2:
			address = 0x40 + (col - 1);
			break;
		default:
			address = 0x00 + (col - 1); // Default to first row
			break;
	}

	LCD16X2_Send(0x80 | address, false); // Set DDRAM address command
}

void LCD16X2_Write_String(const char* str){
    // Try to acquire display lock atomically
    __disable_irq();
    if (display_updating) {
        // Display is already being updated, skip this request
        __enable_irq();
        return;
    }
    display_updating = 1;
    __enable_irq();

	while (*str) {
		LCD16X2_Send((uint8_t)(*str++), true);
	}
    display_updating = 0;
}



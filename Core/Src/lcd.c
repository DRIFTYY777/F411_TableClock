#include "lcd.h"
#include <stdbool.h>

static LCD_HandleTypeDef *MyLCD = NULL;

static volatile uint8_t display_updating = 0;
volatile uint8_t lcd_busy = 0;

// ...existing code...
void LCD_send(uint8_t data, bool is_command)
{
    lcd_busy = 1;

    // RS: 0 = Command, 1 = Data
    HAL_GPIO_WritePin(MyLCD->rs_Port, MyLCD->rs_Pin,
                      is_command ? GPIO_PIN_RESET : GPIO_PIN_SET);

    // RW = 0 (always write)
    HAL_GPIO_WritePin(MyLCD->rw_Port, MyLCD->rw_Pin, GPIO_PIN_RESET);

#ifdef LCD_8_BIT_MODE
    // Send high byte
    HAL_GPIO_WritePin(MyLCD->d0_Port, MyLCD->d0_Pin, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d1_Port, MyLCD->d1_Pin, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d2_Port, MyLCD->d2_Pin, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d3_Port, MyLCD->d3_Pin, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d4_Port, MyLCD->d4_Pin, (data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d5_Port, MyLCD->d5_Pin, (data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d6_Port, MyLCD->d6_Pin, (data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d7_Port, MyLCD->d7_Pin, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Pulse EN
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_RESET);

    HAL_Delay(2); // wait for LCD internal processing
    lcd_busy = 0;

#else
    // ---------- Send high nibble ----------
    HAL_GPIO_WritePin(MyLCD->d4_Port, MyLCD->d4_Pin, (data & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d5_Port, MyLCD->d5_Pin, (data & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d6_Port, MyLCD->d6_Pin, (data & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d7_Port, MyLCD->d7_Pin, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Pulse EN
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_RESET);

    // ---------- Send low nibble ----------
    HAL_GPIO_WritePin(MyLCD->d4_Port, MyLCD->d4_Pin, (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d5_Port, MyLCD->d5_Pin, (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d6_Port, MyLCD->d6_Pin, (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d7_Port, MyLCD->d7_Pin, (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Pulse EN
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_RESET);

    HAL_Delay(2); // wait for LCD internal processing
    lcd_busy = 0;

#endif
}

void init_pins()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // init all the pins be ready for any port on any pin
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = MyLCD->rs_Pin;
    HAL_GPIO_Init(MyLCD->rs_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MyLCD->en_Pin;
    HAL_GPIO_Init(MyLCD->en_Port, &GPIO_InitStruct);

    if (MyLCD->rw_Port != NULL && MyLCD->rw_Pin != -1)
    {
        GPIO_InitStruct.Pin = MyLCD->rw_Pin;
        HAL_GPIO_Init(MyLCD->rw_Port, &GPIO_InitStruct);
    }

#ifdef LCD_8_BIT_MODE
    GPIO_InitStruct.Pin = MyLCD->d0_Pin;
    HAL_GPIO_Init(MyLCD->d0_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MyLCD->d1_Pin;
    HAL_GPIO_Init(MyLCD->d1_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MyLCD->d2_Pin;
    HAL_GPIO_Init(MyLCD->d2_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MyLCD->d3_Pin;
    HAL_GPIO_Init(MyLCD->d3_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MyLCD->d4_Pin;
    HAL_GPIO_Init(MyLCD->d4_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MyLCD->d5_Pin;
    HAL_GPIO_Init(MyLCD->d5_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MyLCD->d6_Pin;
    HAL_GPIO_Init(MyLCD->d6_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MyLCD->d7_Pin;
    HAL_GPIO_Init(MyLCD->d7_Port, &GPIO_InitStruct);

#else
    GPIO_InitStruct.Pin = MyLCD->d4_Pin;
    HAL_GPIO_Init(MyLCD->d4_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MyLCD->d5_Pin;
    HAL_GPIO_Init(MyLCD->d5_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MyLCD->d6_Pin;
    HAL_GPIO_Init(MyLCD->d6_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MyLCD->d7_Pin;
    HAL_GPIO_Init(MyLCD->d7_Port, &GPIO_InitStruct);

#endif
}

void LCD_Clear()
{
    LCD_send(0x01, true); // Clear display command
    HAL_Delay(2);         // Wait for clear command to process
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
uint8_t address = 0;
#if defined(LCD16X2)

    address = (row == 0) ? 0x00 : 0x40; // DDRAM address for row 1 and row 2
    address += col;                             // Add column offset

#elif defined(LCD20X4)
    switch (row)
    {
    case 0:
        address = 0x00;
        break;
    case 1:
        address = 0x40;
        break;
    case 2:
        address = 0x14;
        break;
    case 3:
        address = 0x54;
        break;
    default:
        address = 0x00;
    }
    address += col; // Add column offset
#endif
    LCD_send(0x80 | address, true); // Set DDRAM address command
}

void LCD_Print(const char *str)
{
    while (*str)
    {
        LCD_send(*str++, false); // Send character data
    }
}


void display_begin()
{
    // Wait for LCD to power up
    HAL_Delay(50);

#ifdef LCD_8_BIT_MODE // Initialization sequence for 8-bit mode
    // make all pins low
    HAL_GPIO_WritePin(MyLCD->rs_Port, MyLCD->rs_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->rw_Port, MyLCD->rw_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d0_Port, MyLCD->d0_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d1_Port, MyLCD->d1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d2_Port, MyLCD->d2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d3_Port, MyLCD->d3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d4_Port, MyLCD->d4_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d5_Port, MyLCD->d5_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d6_Port, MyLCD->d6_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d7_Port, MyLCD->d7_Pin, GPIO_PIN_RESET);

#if defined(LCD16X2) // 8 bit mode initialization for 16x2 LCD
    // Initialization sequence
    LCD_send(0x30, false); // Function set: 8-bit mode
    HAL_Delay(5);

    LCD_send(0x30, false);
    HAL_Delay(5);

    LCD_send(0x30, false);
    HAL_Delay(5);

    LCD_send(0x38, false); // 8-bit, 2 line, 5x8 dots
    LCD_send(0x0C, false); // Display ON, Cursor OFF
    LCD_send(0x06, false); // Entry mode set: Increment cursor

#elif defined(LCD20X4) // 8 bit mode initialization for 20x4 LCD
    // Initialization sequence
    LCD_send(0x30, false); // Function set: 8-bit mode
    HAL_Delay(5);

    LCD_send(0x30, false);
    HAL_Delay(5);

    LCD_send(0x30, false);
    HAL_Delay(5);

    LCD_send(0x38, false); // 8-bit, 2 line, 5x8 dots
    LCD_send(0x0C, false); // Display ON, Cursor OFF
    LCD_send(0x06, false); // Entry mode set: Increment cursor

#endif

#else // 4-bit mode initialization
    // make all pins low
    HAL_GPIO_WritePin(MyLCD->rs_Port, MyLCD->rs_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->en_Port, MyLCD->en_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d4_Port, MyLCD->d4_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d5_Port, MyLCD->d5_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d6_Port, MyLCD->d6_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MyLCD->d7_Port, MyLCD->d7_Pin, GPIO_PIN_RESET);

#ifdef LCD16X2 // Initialization sequence for 16x2 LCD in 4-bit mode
    // Initialization sequence
    LCD_send(0x03, false); // Function set: 8-bit mode
    HAL_Delay(5);

    LCD_send(0x03, false);
    HAL_Delay(5);

    LCD_send(0x02, false); // Set to 4-bit mode
    HAL_Delay(5);

    LCD_send(0x28, false); // 4-bit, 2 line, 5x8 dots
    LCD_send(0x0C, false); // Display ON, Cursor OFF

    LCD_send(0x06, false); // Entry mode set: Increment cursor

#endif

#ifdef LCD20X4 // Initialization sequence for 20x4 LCD in 4-bit mode
    // Initialization sequence
    LCD_send(0x28, false); // Function set: 8-bit mode
    HAL_Delay(5);

    LCD_send(0x08, false);
    HAL_Delay(5);

    LCD_send(0x06, false); // Set to 4-bit mode
    HAL_Delay(5);

    LCD_send(0x06, false); // Entry mode set: Increment cursor
    LCD_send(0x0C, false); // Display ON, Cursor OFF

#endif
    LCD_Clear();
#endif
}

void LCD_Init(LCD_HandleTypeDef *lcd)
{
    MyLCD = lcd;
    init_pins();
    HAL_Delay(50); // Wait for LCD to power up
    display_begin();
}

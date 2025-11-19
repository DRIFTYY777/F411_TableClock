/*
 * buttons.c
 *
 *  Created on: Nov 15, 2025
 *      Author: DRIFTYY777
 */

#include <buttons.h>

// for all the pin definitions
#include "main.h"


// Button debouncing constants and variables
#define BUTTON_DEBOUNCE_TIME_MS 50  // 50ms ISR debounce
#define BUTTON_LONG_PRESS_MS 1000   // 1s long press

static volatile bool button_event[4] = {false};        // edge event set by ISR
static volatile uint32_t button_last_isr[4] = {0};     // last ISR timestamp for debounce
static volatile uint32_t button_press_time[4] = {0};   // time when button was pressed

// Map GPIO pin to button index
static int pin_to_index(uint16_t GPIO_Pin)
{
    switch(GPIO_Pin)
    {
        case UP_Pin:    return UP_BUTTON;
        case DOWN_Pin:  return DOWN_BUTTON;
        case BACK_Pin:  return BACK_BUTTON;
        case ENTER_Pin: return ENTER_BUTTON;
        default: return -1;
    }
}

void init_buttons(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIO clocks for all button ports
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure each button pin as input with pull-up and EXTI (falling edge trigger)
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING; // button pulls line to GND when pressed
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    // Initialize UP button (PB6)
    GPIO_InitStruct.Pin = UP_Pin;
    HAL_GPIO_Init(UP_GPIO_Port, &GPIO_InitStruct);

    // Initialize DOWN button (PB8)
    GPIO_InitStruct.Pin = DOWN_Pin;
    HAL_GPIO_Init(DOWN_GPIO_Port, &GPIO_InitStruct);

    // Initialize BACK button (PB5)
    GPIO_InitStruct.Pin = BACK_Pin;
    HAL_GPIO_Init(BACK_GPIO_Port, &GPIO_InitStruct);

    // Initialize ENTER button (PB7)
    GPIO_InitStruct.Pin = ENTER_Pin;
    HAL_GPIO_Init(ENTER_GPIO_Port, &GPIO_InitStruct);

    // Enable and set EXTI line Interrupts for different pin ranges
    // PB5 uses EXTI9_5_IRQn
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    
    // PB6, PB7, PB8 use EXTI9_5_IRQn as well (pins 5-9)
    // So we only need to enable EXTI9_5_IRQn for all buttons
}

// This callback is called from HAL_GPIO_EXTI_IRQHandler in the IRQ handler
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    int idx = pin_to_index(GPIO_Pin);
    if (idx < 0) return;

    uint32_t now = HAL_GetTick();
    // ISR-level debounce: ignore events that occur too close together
    if ((now - button_last_isr[idx]) < BUTTON_DEBOUNCE_TIME_MS)
    {
        return;
    }
    button_last_isr[idx] = now;

    // Mark event and record press time
    button_event[idx] = true;
    button_press_time[idx] = now;
}

// EXTI handler for lines 5 to 9 (used by buttons on PB5, PB6, PB7, PB8)
void EXTI9_5_IRQHandler(void)
{
  /* Call HAL handler for each button pin so HAL_GPIO_EXTI_Callback is invoked */
  HAL_GPIO_EXTI_IRQHandler(UP_Pin);
  HAL_GPIO_EXTI_IRQHandler(DOWN_Pin);
  HAL_GPIO_EXTI_IRQHandler(BACK_Pin);
  HAL_GPIO_EXTI_IRQHandler(ENTER_Pin);
}

// Debounced edge-event check: returns true once per press (set by ISR). Non-blocking.
bool get_btn(enum Button button)
{
    int idx = (int)button;
    if (idx < 0 || idx > 3) return false;

    // Check and clear the event flag atomically
    __disable_irq();
    bool ev = button_event[idx];
    if (ev)
    {
        button_event[idx] = false;
    }
    __enable_irq();

    return ev;
}
#include "inputs.h"

#include "main.h"

#define BUTTON_REPEAT_DELAY 400U
#define BUTTON_REPEAT_INTERVAL 100U

void inputs_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    // init all pins for buttons
    GPIO_InitStruct.Pin = UP_Pin;
    HAL_GPIO_Init(UP_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DOWN_Pin;
    HAL_GPIO_Init(DOWN_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BACK_Pin;
    HAL_GPIO_Init(BACK_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ENTER_Pin;
    HAL_GPIO_Init(ENTER_GPIO_Port, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

Switch_t read_switch()
{
    // detect long press on enter button
    static uint32_t last_press_time = 0;
    static uint8_t enter_lock = 0;
    static uint8_t back_lock = 0;
    static uint8_t down_lock = 0;
    static uint8_t up_lock = 0;
    static uint32_t down_repeat_time = 0;
    static uint32_t up_repeat_time = 0;
    uint32_t current_time = HAL_GetTick();

    /* Returns the LONG PRESS ENTER */
    if (HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == 0)
    {
        if (current_time - last_press_time > 800) // 1 second
        {
            last_press_time = current_time;
            return SW_ENTER_LONG;
        }
    }
    else
    {
        last_press_time = current_time;
    }

    /* RETURNS THE SHORT PRESS ENTER */
    if (HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == 0)
    {
        HAL_Delay(5); // debounce delay
        if (HAL_GPIO_ReadPin(ENTER_GPIO_Port, ENTER_Pin) == 0)
        {
            if (!enter_lock)
            {
                enter_lock = 1;
                return SW_ENTER;
            }
        }
    }
    else
    {
        enter_lock = 0;
    }

    if (HAL_GPIO_ReadPin(BACK_GPIO_Port, BACK_Pin) == 0)
    {
        HAL_Delay(5); // debounce delay

        if (HAL_GPIO_ReadPin(BACK_GPIO_Port, BACK_Pin) == 0)
        {
            if (!back_lock)
            {
                back_lock = 1;
                return SW_BACK;
            }
        }
    }
    else
    {
        back_lock = 0;
    }

    if (HAL_GPIO_ReadPin(DOWN_GPIO_Port, DOWN_Pin) == 0)
    {
        HAL_Delay(5); // debounce delay

        if (HAL_GPIO_ReadPin(DOWN_GPIO_Port, DOWN_Pin) == 0)
        {
            if (!down_lock)
            {
                down_lock = 1;
                down_repeat_time = current_time + BUTTON_REPEAT_DELAY;
                return SW_DOWN;
            }

            if ((int32_t)(current_time - down_repeat_time) >= 0)
            {
                down_repeat_time = current_time + BUTTON_REPEAT_INTERVAL;
                return SW_DOWN;
            }
        }
    }
    else
    {
        down_lock = 0;
        down_repeat_time = 0;
    }

    if (HAL_GPIO_ReadPin(UP_GPIO_Port, UP_Pin) == 0)
    {
        HAL_Delay(5); // debounce delay

        if (HAL_GPIO_ReadPin(UP_GPIO_Port, UP_Pin) == 0)
        {
            if (!up_lock)
            {
                up_lock = 1;
                up_repeat_time = current_time + BUTTON_REPEAT_DELAY;
                return SW_UP;
            }

            if ((int32_t)(current_time - up_repeat_time) >= 0)
            {
                up_repeat_time = current_time + BUTTON_REPEAT_INTERVAL;
                return SW_UP;
            }
        }
    }
    else
    {
        up_lock = 0;
        up_repeat_time = 0;
    }
    return SW_NONE;
}
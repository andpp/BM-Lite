/*
 * Copyright (c) 2020 Fingerprint Cards AB
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modified by Andrey Perminov <andrey.ppp@gmail.com> 
 * for FPC BM-Lite applications
 */

/**
 * @file   hal_buttons.c
 * @brief  Initialization and access of button.
 */

#include <stdbool.h>
#include <stdint.h>

#include "stm32wbxx_hal.h"
#include "bmlite_hal.h"
#include "hal_config.h"

static uint32_t button_press_time;
static volatile uint32_t button_is_pressed = 0;

/* Initialize the button on the STM Nucleo board */
void board_button_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(DEMO_BUTTON_PORT, DEMO_BUTTON_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin         = DEMO_BUTTON_PIN;
    GPIO_InitStruct.Mode        = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull        = GPIO_PULLUP;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate   = 0;
    HAL_GPIO_Init(DEMO_BUTTON_PORT, &GPIO_InitStruct);

    // B4 & B13 are connected on Nucleo board. Make B13 as input
    GPIO_InitStruct.Pin         = GPIO_PIN_13;
    GPIO_InitStruct.Mode        = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull        = GPIO_NOPULL;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate   = 0;
    HAL_GPIO_Init(DEMO_BUTTON_PORT, &GPIO_InitStruct);


    HAL_NVIC_SetPriority(DEMO_BUTTON_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DEMO_BUTTON_IRQn);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
  __HAL_GPIO_EXTI_CLEAR_IT(DEMO_BUTTON_PIN);

}

/* Interrupt handler to handle button event */
void DEMO_BUTTON_IRQ_HANDLER(void)
{
    GPIO_PinState pin = HAL_GPIO_ReadPin(DEMO_BUTTON_PORT, DEMO_BUTTON_PIN);
    static uint32_t press_on = 0;

    if (!pin) {
        press_on = HAL_GetTick();
        button_press_time = 0;
        button_is_pressed = 1;
    } else {
        button_press_time = HAL_GetTick() - press_on;
        button_is_pressed = 0;
    }

    if(__HAL_GPIO_EXTI_GET_IT(DEMO_BUTTON_PIN) != 0x00u)  {
      __HAL_GPIO_EXTI_CLEAR_IT(DEMO_BUTTON_PIN);
    }
}

void hal_reset_button_press_time(void)
{
    button_press_time = 0;
}

uint32_t hal_get_button_press_time(void)
{
    uint32_t prim = __get_PRIMASK();
    
    __disable_irq();

    uint32_t tmp = button_press_time;
    button_press_time = 0;

    if (!prim) {
        __enable_irq();
    }

    return tmp;
}

uint32_t hal_check_button_pressed()
{
     return button_is_pressed;
}
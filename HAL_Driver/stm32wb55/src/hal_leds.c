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
 * @file   hal_leds.c
 * @brief  Initialization and access of LED.
 *
 *  The user LED on the Nucleo-L476RG is using the same pin as SPI SCK (PA5) so the LED
 *  functionality will only work between SPI transfers.
 */

#include <stdbool.h>
#include <stdint.h>

#include "stm32wbxx_hal.h"
#include "stm32wbxx_ll_gpio.h"

#include "bmlite_hal.h"
#include "hal_config.h"

/** LED ON time in ms */
#define LED_SOLID_ON_TIME_MS 1000

/** LED blink time in ms */
#define LED_BLINK_TIME_MS    200

void board_led_init()
{
  GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DEMO_LEDR_PORT, DEMO_LEDR_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DEMO_LEDG_PORT, DEMO_LEDG_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(DEMO_LEDB_PORT, DEMO_LEDB_PIN, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = DEMO_LEDR_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DEMO_LEDR_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DEMO_LEDG_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DEMO_LEDG_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = DEMO_LEDB_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DEMO_LEDB_PORT, &GPIO_InitStruct);

}


static void set_leds(uint8_t led)
{
    HAL_GPIO_WritePin(DEMO_LEDR_PORT, DEMO_LEDR_PIN, led & 0x01);
    HAL_GPIO_WritePin(DEMO_LEDG_PORT, DEMO_LEDG_PIN, led & 0x02);
    HAL_GPIO_WritePin(DEMO_LEDB_PORT, DEMO_LEDB_PIN, led & 0x04);
}

void hal_set_leds(platform_led_status_t status, uint16_t mode)
{
     switch(status) {
          case BMLITE_LED_STATUS_READY:
               set_leds(0);
               break;
          case BMLITE_LED_STATUS_MATCH:
               if (mode) {
                    set_leds(2);
               } else {
                    set_leds(1);
               }
               hal_timebase_busy_wait(500);
               break;
          case BMLITE_LED_STATUS_WAITTOUCH:
               if (mode) {
                    set_leds(4);
               }
               break;
          case BMLITE_LED_STATUS_ENROLL:
               if (mode) { 
                    // Start enroll
                    set_leds(5);
                    hal_timebase_busy_wait(500);
                    set_leds(6);
                    hal_timebase_busy_wait(500);
               } else {
                    // Finish enroll
                    set_leds(5);
                    hal_timebase_busy_wait(100);
                    set_leds(4);
                    hal_timebase_busy_wait(100);
                    set_leds(6);
                    hal_timebase_busy_wait(100);
               }
               break;
          case BMLITE_LED_STATUS_DELETE_TEMPLATES:
                    set_leds(4);
                    hal_timebase_busy_wait(100);
                    set_leds(0);
                    hal_timebase_busy_wait(100);
                    set_leds(4);
                    hal_timebase_busy_wait(100);
               break;
          case BMLITE_LED_STATUS_ERROR:
               if (mode) {
                    set_leds(3);
                    hal_timebase_busy_wait(70);
               } else {
                    set_leds(3);
                    hal_timebase_busy_wait(500);
                    set_leds(0);
                    hal_timebase_busy_wait(500);
                    set_leds(3);
                    hal_timebase_busy_wait(500);
               }
               break;
    }
}
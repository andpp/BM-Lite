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
 * @file    platform_timebase.c
 * @brief   timebase interface functions
 */


#include "stm32wbxx_hal.h"
#include "bmlite_hal.h"

void hal_timebase_init(void)
{
    HAL_InitTick(TICK_INT_PRIORITY);
}

void hal_timebase_busy_wait(uint32_t delay)
{
    /* Ensure minimum delay or skip if delay is zero*/
    if (delay) {
        delay++;
        HAL_Delay(delay);
    }
}

uint32_t hal_timebase_get_tick(void)
{
    return HAL_GetTick();
}

/**
 * This function handles System tick timer.
 */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

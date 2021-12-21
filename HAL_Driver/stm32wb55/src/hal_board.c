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
 * @file   hal_board.c
 * @brief  Board Specific functions
 */

#include <stdbool.h>

#include "stm32wbxx_hal.h"
#include "hal_config.h"

#include "bmlite_hal.h"

static volatile bool sensor_interrupt = false;

static void dma_init();
// static void flash_init();
static void system_irq_init();

void stm_spi_init(uint32_t speed_hz);
void stm_uart_init(uint32_t speed_hz);
void board_led_init();
void board_button_init();

uint32_t SecureFlashStartAddr;

fpc_bep_result_t hal_board_init(void *params)
{
    (void)params;
    
    uint32_t debugger = (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk);
    /* Disable fault exceptions */
    SCB->SHCSR &= ~SCB_SHCSR_MEMFAULTENA_Msk;
    /* Disable the MPU */
    MPU->CTRL  &= ~MPU_CTRL_ENABLE_Msk;

    /* Configure system flash */
    // flash_init();

    /* Configure system IRQs */
    system_irq_init();

    /* Initialize DMA */
    dma_init();

#ifdef BMLITE_ON_UART
    stm_uart_init(115200);
#else
#ifdef BMLITE_ON_SPI
    stm_spi_init(4000000);
#else
   #error "BMLITE_ON_SPI or BMLITE_ON_SPI must be defined"
#endif
#endif

    if (debugger) {
        HAL_DBGMCU_EnableDBGStandbyMode();
        HAL_DBGMCU_EnableDBGSleepMode();
        HAL_DBGMCU_EnableDBGStopMode();
    }

    board_led_init();
    board_button_init();

    return FPC_BEP_RESULT_OK;
}

void hal_bmlite_reset(bool state)
{
    if (state) {
        HAL_GPIO_WritePin(BMLITE_RST_PORT, BMLITE_RST_PIN, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(BMLITE_RST_PORT, BMLITE_RST_PIN, GPIO_PIN_SET);
    }
}

bool hal_bmlite_get_status(void)
{
    return HAL_GPIO_ReadPin(BMLITE_READY_PORT, BMLITE_READY_PIN);
}

void dma_init(void)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    // __HAL_RCC_DMA1_FORCE_RESET();
    // __HAL_RCC_DMA1_RELEASE_RESET();

    __HAL_RCC_DMA2_CLK_ENABLE();
    // __HAL_RCC_DMA2_FORCE_RESET();
    // __HAL_RCC_DMA2_RELEASE_RESET();
}

static void system_irq_init(void)
{
    /* Set Interrupt Group Priority */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    /* MemoryManagement_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);

    /* BusFault_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);

    /* UsageFault_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);

    /* SVCall_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);

    /* DebugMonitor_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);

    /* PendSV_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    /* RCC_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(RCC_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(RCC_IRQn);
}

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

#include "boards.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"

#include "bmlite_hal.h"

#define BMLITE_PIN_RESET   	ARDUINO_2_PIN
#define BMLITE_PIN_STATUS   ARDUINO_A2_PIN

static volatile bool sensor_interrupt = false;

static void nordic_bmlite_gpio_init(void);
void nordic_bmlite_spi_init(uint32_t speed_hz);


fpc_bep_result_t hal_board_init(void *params)
{
    (void)params;
    
	if (NRF_UICR->REGOUT0 != UICR_REGOUT0_VOUT_3V3)
	{
	    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
	    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
	    NRF_UICR->REGOUT0 = UICR_REGOUT0_VOUT_3V3;

	    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
	    while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
	}
	NRF_USBD->ENABLE = 1;

    nordic_bmlite_gpio_init();
    nordic_bmlite_spi_init(4000000);
    bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);

    return FPC_BEP_RESULT_OK;
}

void hal_bmlite_reset(bool state)
{
    if(!state) {
	    nrf_drv_gpiote_out_set(BMLITE_PIN_RESET);
    } else {
        nrf_drv_gpiote_out_clear(BMLITE_PIN_RESET);
    }
}

bool hal_bmlite_get_status(void)
{
    return nrf_drv_gpiote_in_is_set(BMLITE_PIN_STATUS);
}

static void nordic_bmlite_gpio_init(void)
{
    ret_code_t err_code;
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(true);
    err_code = nrf_drv_gpiote_out_init(BMLITE_PIN_RESET, &out_config);
    APP_ERROR_CHECK(err_code);
	nrf_drv_gpiote_out_task_enable(BMLITE_PIN_RESET); //Enable task for output pin (toggle)

    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    err_code = nrf_drv_gpiote_in_init(BMLITE_PIN_STATUS, &config, NULL);
    APP_ERROR_CHECK(err_code);
    nrf_gpiote_event_clear(NRF_GPIOTE_EVENTS_IN_0);
    return;
}


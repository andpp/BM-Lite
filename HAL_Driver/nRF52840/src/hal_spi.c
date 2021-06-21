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
 * @file    platform_spi.c
 * @brief   SPI functions
 */

#include "nrf_drv_spi.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_drv_gpiote.h"

#include "bmlite_hal.h"
#include "fpc_bep_types.h"

#define SPI_INSTANCE  0 /**< SPI instance index. */

#define BMLITE_CS_PIN   ARDUINO_8_PIN
#define BMLITE_MISO_PIN ARDUINO_12_PIN
#define BMLITE_MOSI_PIN ARDUINO_11_PIN
#define BMLITE_CLK_PIN  ARDUINO_13_PIN

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */
static uint8_t       *m_tx_buf;
static uint8_t       *m_rx_buf;

nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

/**
 * @brief SPI user event handler.
 * @param event
 */
static void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    if( p_event->type == NRF_DRV_SPI_EVENT_DONE) {
	    spi_xfer_done = true;
    }
}

static void spi_write_read(uint8_t *write, uint8_t *read, size_t size, bool leave_cs_asserted)
{

	spi_xfer_done = false;
	m_tx_buf =  write;
	m_rx_buf = read;

	nrf_drv_gpiote_out_clear(BMLITE_CS_PIN);

	nrf_drv_spi_transfer(&spi, m_tx_buf, size, m_rx_buf, size);

	while (!spi_xfer_done)
	{
		__WFE();
	}

	if(!leave_cs_asserted)
		nrf_drv_gpiote_out_set(BMLITE_CS_PIN);

}

fpc_bep_result_t hal_bmlite_spi_write_read(uint8_t *write, uint8_t *read, size_t size,
        bool leave_cs_asserted)
{

	uint8_t num_of_rounds = size/255, i;
	uint8_t *p_write = write, *p_read = read;

	for(i=0; i<num_of_rounds; i++){
		spi_write_read(p_write, p_read, 255, true);
		p_write += 255;
		p_read += 255;
		size -=255;
	}

	spi_write_read(p_write, p_read, size, leave_cs_asserted);

	return FPC_BEP_RESULT_OK;
}

void nordic_bmlite_spi_init(uint32_t speed_hz)
{
    //spi_config.ss_pin   = BMLITE_CS_PIN;
	spi_config.miso_pin = BMLITE_MISO_PIN;
    spi_config.mosi_pin = BMLITE_MOSI_PIN;
    spi_config.sck_pin  = BMLITE_CLK_PIN;
    spi_config.frequency  = NRF_SPI_FREQ_8M;// default to 8M for now
    nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);

    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(true);
    nrf_drv_gpiote_out_init(BMLITE_CS_PIN, &out_config);
}


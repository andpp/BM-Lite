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
 * @file    hal_spi.c
 * @brief   SPI HAL functions
 */

#ifdef BMLITE_ON_SPI

#include "stm32wbxx_hal.h"

#include "bmlite_hal.h"
#include "fpc_bep_types.h"
#include "hal_config.h"

SPI_HandleTypeDef bmlite_handle = { 0 };                //!< Sensor SPI handle.

static DMA_HandleTypeDef bmlite_dma_rx_handle = { 0 };  //!< Sensor SPI DMA Rx handle.
static DMA_HandleTypeDef bmlite_dma_tx_handle = { 0 };  //!< Sensor SPI DMA Tx handle.

volatile bool spi_rx_tx_done;


fpc_bep_result_t hal_bmlite_spi_write_read(uint8_t *write, uint8_t *read, size_t size,
        bool leave_cs_asserted)
{
    HAL_SPI_StateTypeDef spi_state;
    HAL_StatusTypeDef status;
	fpc_bep_result_t result = FPC_BEP_RESULT_IO_ERROR;

    do {
        spi_state = HAL_SPI_GetState(&bmlite_handle);
        if (spi_state == HAL_SPI_STATE_ERROR) {
            goto exit;
        }
    } while (spi_state != HAL_SPI_STATE_READY);

    HAL_GPIO_WritePin(BMLITE_SPI_NSS_PORT, BMLITE_SPI_NSS_PIN, GPIO_PIN_RESET);
    spi_rx_tx_done = false;

    do {
        status = HAL_SPI_TransmitReceive_DMA(&bmlite_handle, write, read, size);
        if (status == HAL_ERROR) {
            goto exit1;
        }
    } while (status == HAL_BUSY);

    if (status == HAL_OK) {
        result = FPC_BEP_RESULT_OK;
    }

    while (!spi_rx_tx_done) {
    }

exit1:
    if (!leave_cs_asserted) {
        HAL_GPIO_WritePin(BMLITE_SPI_NSS_PORT, BMLITE_SPI_NSS_PIN, GPIO_PIN_SET);
    }

exit:
    return result;

}

static void set_prescaler(uint32_t speed_hz)
{

    if (SystemCoreClock / 2 < speed_hz) {
        bmlite_handle.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_2;
    } else if (SystemCoreClock / 4 < speed_hz) {
        bmlite_handle.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_4;
    } else if (SystemCoreClock / 8 < speed_hz) {
        bmlite_handle.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_8;
    } else if (SystemCoreClock / 16 < speed_hz) {
        bmlite_handle.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_16;
    } else if (SystemCoreClock / 32 < speed_hz) {
        bmlite_handle.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_32;
    } else if (SystemCoreClock / 64 < speed_hz) {
        bmlite_handle.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_64;
    } else if (SystemCoreClock / 128 < speed_hz) {
        bmlite_handle.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_128;
    } else {
        bmlite_handle.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_256;
    }
}

void stm_spi_init(uint32_t speed_hz)
{
    /* Peripheral clock enable */
    BMLITE_SPI_CLK_ENABLE();
    bmlite_handle.Instance                = BMLITE_SPI;
    bmlite_handle.Init.Mode               = SPI_MODE_MASTER;
    bmlite_handle.Init.Direction          = SPI_DIRECTION_2LINES;
    bmlite_handle.Init.DataSize           = SPI_DATASIZE_8BIT;
    bmlite_handle.Init.CLKPolarity        = SPI_POLARITY_LOW;
    bmlite_handle.Init.CLKPhase           = SPI_PHASE_1EDGE;
    bmlite_handle.Init.NSS                = SPI_NSS_SOFT;
    bmlite_handle.Init.NSSPMode           = SPI_NSS_PULSE_DISABLE;
    bmlite_handle.Init.FirstBit           = SPI_FIRSTBIT_MSB;
    bmlite_handle.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLE;
    bmlite_handle.Init.CRCPolynomial      = 7;
    bmlite_handle.Init.CRCLength          = SPI_CRC_LENGTH_DATASIZE;

    set_prescaler(speed_hz);

    HAL_SPI_Init(&bmlite_handle);
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(BMLITE_SPI_MISO_PORT, BMLITE_SPI_MISO_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin         = BMLITE_SPI_MISO_PIN;
    GPIO_InitStruct.Mode        = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull        = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate   = BMLITE_SPI_AF;
    HAL_GPIO_Init(BMLITE_SPI_MISO_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(BMLITE_SPI_MOSI_PORT, BMLITE_SPI_MOSI_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin         = BMLITE_SPI_MOSI_PIN;
    GPIO_InitStruct.Mode        = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull        = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate   = BMLITE_SPI_AF;
    HAL_GPIO_Init(BMLITE_SPI_MOSI_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(BMLITE_SPI_NSS_PORT, BMLITE_SPI_NSS_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin         = BMLITE_SPI_NSS_PIN;
    GPIO_InitStruct.Mode        = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull        = GPIO_PULLUP;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate   = 0;
    HAL_GPIO_Init(BMLITE_SPI_NSS_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(BMLITE_SPI_SCK_PORT, BMLITE_SPI_SCK_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin         = BMLITE_SPI_SCK_PIN;
    GPIO_InitStruct.Mode        = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull        = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate   = BMLITE_SPI_AF;
    HAL_GPIO_Init(BMLITE_SPI_SCK_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(BMLITE_READY_PORT, BMLITE_READY_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin         = BMLITE_READY_PIN;
    GPIO_InitStruct.Mode        = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull        = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate   = 0;
    HAL_GPIO_Init(BMLITE_READY_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(BMLITE_RST_PORT, BMLITE_RST_PIN, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin         = BMLITE_RST_PIN;
    GPIO_InitStruct.Mode        = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull        = GPIO_PULLUP;
    GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate   = 0;
    HAL_GPIO_Init(BMLITE_RST_PORT, &GPIO_InitStruct);

    /* Peripheral DMA init*/

    bmlite_dma_rx_handle.Instance = BMLITE_SPI_DMA_CHANNEL_RX;
    bmlite_dma_rx_handle.Init.Request = BMLITE_SPI_DMA_REQ_RX;
    bmlite_dma_rx_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    bmlite_dma_rx_handle.Init.PeriphInc = DMA_PINC_DISABLE;
    bmlite_dma_rx_handle.Init.MemInc = DMA_MINC_ENABLE;
    bmlite_dma_rx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    bmlite_dma_rx_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    bmlite_dma_rx_handle.Init.Mode = DMA_NORMAL;
    bmlite_dma_rx_handle.Init.Priority = DMA_PRIORITY_MEDIUM;
    HAL_DMA_Init(&bmlite_dma_rx_handle);

    __HAL_LINKDMA(hspi, hdmarx, bmlite_dma_rx_handle);

    bmlite_dma_tx_handle.Instance = BMLITE_SPI_DMA_CHANNEL_TX;
    bmlite_dma_tx_handle.Init.Request = BMLITE_SPI_DMA_REQ_TX;
    bmlite_dma_tx_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;
    bmlite_dma_tx_handle.Init.PeriphInc = DMA_PINC_DISABLE;
    bmlite_dma_tx_handle.Init.MemInc = DMA_MINC_ENABLE;
    bmlite_dma_tx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    bmlite_dma_tx_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    bmlite_dma_tx_handle.Init.Mode = DMA_NORMAL;
    bmlite_dma_tx_handle.Init.Priority = DMA_PRIORITY_MEDIUM;
    HAL_DMA_Init(&bmlite_dma_tx_handle);

    __HAL_LINKDMA(hspi, hdmatx, bmlite_dma_tx_handle);

    /* DMA interrupt init */
    /* SPI Master RX DMA interrupt configuration */
    HAL_NVIC_SetPriority(BMLITE_SPI_DMA_IRQn_RX, 1, 0);
    HAL_NVIC_EnableIRQ(BMLITE_SPI_DMA_IRQn_RX);

    /* SPI Master TX DMA interrupt configuration */
    HAL_NVIC_SetPriority(BMLITE_SPI_DMA_IRQn_TX, 1, 0);
    HAL_NVIC_EnableIRQ(BMLITE_SPI_DMA_IRQn_TX);

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority(BMLITE_SPI_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(BMLITE_SPI_IRQn);

    /* Enable SPI */
    __HAL_SPI_ENABLE(hspi);
}

void BMLITE_SPI_IRQ_HANDLER(void)
{
   HAL_SPI_IRQHandler(&bmlite_handle);
}

void BMLITE_SPI_DMA_IRQ_HANDLER_RX(void)
{
    HAL_DMA_IRQHandler(&bmlite_dma_rx_handle);
}

void BMLITE_SPI_DMA_IRQ_HANDLER_TX(void)
{
   HAL_DMA_IRQHandler(&bmlite_dma_tx_handle);
}

/**
 * SPI RxTx callback routine.
 * @param hspi SPI handle.
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == &bmlite_handle) {
        spi_rx_tx_done = true;
    }
}

#endif

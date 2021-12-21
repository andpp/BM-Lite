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
 * @file    hal_uart.c
 * @brief   UART HAL functions
 */

#ifdef BMLITE_ON_UART

#include "stm32wbxx_hal.h"

#include "bmlite_hal.h"
#include "fpc_bep_types.h"
#include "hal_config.h"

#include <string.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

typedef struct
{
    volatile uint8_t flag;      /* Timeout event flag */
    uint16_t prev_cndtr;        /* Holds previous value of DMA_CNDTR (counts down) */
    uint32_t idle_irq_count;    /* Debug */
    uint32_t rx_complete_count; /* Debug */
    uint32_t rx_half_count;     /* Debug */
} dma_event_t;

UART_HandleTypeDef huart_host = { 0 };
static DMA_HandleTypeDef hdma_rx = { 0 };
static DMA_HandleTypeDef hdma_tx = { 0 };

static volatile bool tx_half;
static volatile bool tx_done;
static volatile bool error;
static volatile bool rx_available;

#define DMA_BUF_SIZE   128
#define DMA_TIMEOUT_MS 2

static uint8_t uart_rx_fifo[DMA_BUF_SIZE];
static dma_event_t dma_uart_rx = { 0, DMA_BUF_SIZE, 0, 0, 0 };

// static bool tx_active(void)
// {
//     return !tx_done;
// }

// static bool rx_data_available(void)
// {
//     bool avail = rx_available;

//     rx_available = false;

//     return avail;
// }

/**
 * This function handles Host USART global interrupt.
 */
void FPC_BMLITE_USART_IRQ_HANDLER(void)
{
    HAL_UART_IRQHandler(&huart_host);

    /* UART IDLE Interrupt */
    if ((huart_host.Instance->ISR & USART_ISR_IDLE) != RESET) {
        huart_host.Instance->ICR = UART_CLEAR_IDLEF;
        dma_uart_rx.flag = 1;
        dma_uart_rx.idle_irq_count++;
        if(hdma_rx.XferCpltCallback != NULL) {
            hdma_rx.XferCpltCallback(&hdma_rx);
        }
    }
}

/**
 * This function handles Host USART DMA RX global interrupt.
 */
void FPC_BMLITE_USART_DMA_IRQ_HANDLER_RX(void)
{
    HAL_DMA_IRQHandler(huart_host.hdmarx);
}

/**
 * This function handles Host USART DMA TX global interrupt.
 */
void FPC_BMLITE_USART_DMA_IRQ_HANDLER_TX(void)
{
    HAL_DMA_IRQHandler(huart_host.hdmatx);
}

/**
 * USART init function.
 * @param[in] baudrate Initial baud rate.
 */
void stm_uart_init(uint32_t baudrate)
{
    // HAL_StatusTypeDef status;

    huart_host.Instance           = FPC_BMLITE_USART;
    huart_host.Init.BaudRate      = baudrate;
    huart_host.Init.WordLength    = UART_WORDLENGTH_8B;
    huart_host.Init.StopBits      = UART_STOPBITS_1;
    huart_host.Init.Parity        = UART_PARITY_NONE;
    huart_host.Init.Mode          = UART_MODE_TX_RX;
    huart_host.Init.HwFlowCtl     = UART_HWCONTROL_NONE;
    huart_host.Init.OverSampling  = UART_OVERSAMPLING_16;
    huart_host.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart_host.Init.OneBitSampling          = UART_ONE_BIT_SAMPLE_DISABLE;
    huart_host.AdvancedInit.AdvFeatureInit  = UART_ADVFEATURE_NO_INIT;

    // huart_host.Init.BaudRate = 921600;

    HAL_UART_Init(&huart_host);
    HAL_UARTEx_EnableFifoMode(&huart_host);

    /* UART1 IDLE Interrupt Configuration */
    SET_BIT(FPC_BMLITE_USART->CR1, USART_CR1_IDLEIE);

    hal_timebase_busy_wait(100);

    /* Start UART RX */
    HAL_UART_Receive_DMA(&huart_host, uart_rx_fifo, DMA_BUF_SIZE);
}

void uart_host_reinit(void)
{
    if (huart_host.Init.BaudRate) {
        huart_host.Instance->BRR = (uint16_t)(UART_DIV_SAMPLING16(SystemCoreClock,
                huart_host.Init.BaudRate, huart_host.Init.ClockPrescaler));
    }
}

/**
 * Initialize UART GPIO.
 * @param[in, out] huart UART Handle.
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    // HAL_StatusTypeDef status;

    if (huart->Instance == FPC_BMLITE_USART) {
        /* Peripheral clock enable */
        FPC_BMLITE_USART_CLK_ENABLE();
        FPC_BMLITE_USART_RX_PORT_CLK_ENABLE();
        FPC_BMLITE_USART_TX_PORT_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();

        GPIO_InitStruct.Mode        = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull        = GPIO_PULLUP;
        GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate   = FPC_BMLITE_USART_AF;

        GPIO_InitStruct.Pin         = FPC_BMLITE_USART_RX_PIN;
        HAL_GPIO_Init(FPC_BMLITE_USART_RX_PORT, &GPIO_InitStruct);

        GPIO_InitStruct.Pin         = FPC_BMLITE_USART_TX_PIN;
        HAL_GPIO_Init(FPC_BMLITE_USART_TX_PORT, &GPIO_InitStruct);

        /* Peripheral DMA init*/
        hdma_rx.Instance = FPC_BMLITE_USART_DMA_CHANNEL_RX;
        hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_rx.Init.Mode = DMA_CIRCULAR;
        hdma_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
        hdma_rx.Init.Request = FPC_BMLITE_USART_DMA_REQ_RX;

        HAL_DMA_Init(&hdma_rx);
        __HAL_LINKDMA(huart, hdmarx, hdma_rx);

        hdma_tx.Instance = FPC_BMLITE_USART_DMA_CHANNEL_TX;
        hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_tx.Init.Mode = DMA_NORMAL;
        hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
        hdma_tx.Init.Request = FPC_BMLITE_USART_DMA_REQ_TX;

        HAL_DMA_Init(&hdma_tx);
        __HAL_LINKDMA(huart, hdmatx, hdma_tx);

        /* UART Host RX DMA interrupt configuration */
        HAL_NVIC_SetPriority(FPC_BMLITE_USART_DMA_IRQn_RX, 6, 0);
        HAL_NVIC_EnableIRQ(FPC_BMLITE_USART_DMA_IRQn_RX);

        /* UART Host TX DMA interrupt configuration */
        HAL_NVIC_SetPriority(FPC_BMLITE_USART_DMA_IRQn_TX, 6, 0);
        HAL_NVIC_EnableIRQ(FPC_BMLITE_USART_DMA_IRQn_TX);

        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(FPC_BMLITE_USART_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(FPC_BMLITE_USART_IRQn);

        HAL_GPIO_WritePin(BMLITE_RST_PORT, BMLITE_RST_PIN, GPIO_PIN_RESET);
        GPIO_InitStruct.Pin         = BMLITE_RST_PIN;
        GPIO_InitStruct.Mode        = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull        = GPIO_PULLUP;
        GPIO_InitStruct.Speed       = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate   = 0;
        HAL_GPIO_Init(BMLITE_RST_PORT, &GPIO_InitStruct);
    }
}

size_t hal_bmlite_uart_write(const uint8_t *data, size_t size)
{
    HAL_StatusTypeDef result = HAL_ERROR;

    tx_half = false;
    tx_done = false;
    error = false;

    result = HAL_UART_Transmit_DMA(&huart_host, data, size);
    if (result != HAL_OK) {
        return 0;
    }

    while (!tx_done) {

        if (error) {
            result = HAL_ERROR;
            return 0;
        }
    }

    return size;
}

size_t hal_bmlite_uart_read(uint8_t *data, size_t size)
{
    uint32_t n_sent = 0;

    /* Restart Host UART RX DMA transfer if it has stopped for some reason */
    if (huart_host.RxState != HAL_UART_STATE_BUSY_RX) {
        dma_uart_rx.prev_cndtr = DMA_BUF_SIZE;
        HAL_UART_Receive_DMA(&huart_host, uart_rx_fifo, DMA_BUF_SIZE);
    }

    if (!size) {
        return 0;
    }

    uint16_t curr_cndtr = __HAL_DMA_GET_COUNTER(&hdma_rx);

    if (curr_cndtr != dma_uart_rx.prev_cndtr) {
        uint32_t cur_pos = DMA_BUF_SIZE - curr_cndtr;
        uint32_t prev_pos;
        uint32_t length;

        /* Determine start position in DMA buffer based on previous CNDTR value */
        prev_pos = DMA_BUF_SIZE - dma_uart_rx.prev_cndtr;
        if (prev_pos < cur_pos) {
            length = MIN(cur_pos - prev_pos, size);
        } else {
            /* Copy until end of buffer first */
            length = MIN(DMA_BUF_SIZE - prev_pos, size);
        }
        memcpy(data, &uart_rx_fifo[prev_pos], length);
        data += length;
        n_sent += length;
        dma_uart_rx.prev_cndtr -= length;
        if (dma_uart_rx.prev_cndtr == 0) {
            dma_uart_rx.prev_cndtr = DMA_BUF_SIZE;
        }
        if (prev_pos > cur_pos) {
            /* Copy from start of buffer */
            length = MIN(cur_pos, size);
            memcpy(data, uart_rx_fifo, length);
            data += length;
            n_sent += length;
            dma_uart_rx.prev_cndtr -= length;
        }
    } 

    return n_sent;

}

bool uart_host_rx_data_available(void)
{
    return rx_available;
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == huart_host.Instance) {
        rx_available = true;
        dma_uart_rx.rx_half_count++;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == huart_host.Instance) {
        uint16_t curr_cndtr = __HAL_DMA_GET_COUNTER(huart->hdmarx);

        /*
         * Ignore IDLE Timeout when the received characters exactly filled up the DMA buffer and
         * DMA Rx Complete Interrupt is generated, but there is no new character during timeout.
         */
        if (dma_uart_rx.flag && curr_cndtr == DMA_BUF_SIZE) {
            dma_uart_rx.flag = 0;
            return;
        }

        if (!dma_uart_rx.flag) {
            dma_uart_rx.rx_complete_count++;
        }
        dma_uart_rx.flag = 0;

        rx_available = true;
    }
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == huart_host.Instance) {
        tx_half = true;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == huart_host.Instance) {
        tx_done = true;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == huart_host.Instance) {
        error = true;
    }
}

#endif

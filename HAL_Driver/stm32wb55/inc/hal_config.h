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
 */

/**
 * @file   bmlite_hal_config.h
 * @brief  HAL configuration for ST Nucleo-64 STM32WB55 with Sensor Shield.
 */

#ifndef BMLITE_HAL_CONFIG_H
#define BMLITE_HAL_CONFIG_H

#include "stm32wbxx_hal.h"

/**
 * Configuration of button and led.
 */
#define DEMO_BUTTON_PORT        GPIOC
#define DEMO_BUTTON_PIN         GPIO_PIN_4
#define DEMO_BUTTON_IRQn        EXTI4_IRQn
#define DEMO_BUTTON_IRQ_HANDLER EXTI4_IRQHandler

#define DEMO_LEDR_PORT           GPIOB
#define DEMO_LEDR_PIN            GPIO_PIN_1
#define DEMO_LEDG_PORT           GPIOB
#define DEMO_LEDG_PIN            GPIO_PIN_0
#define DEMO_LEDB_PORT           GPIOB
#define DEMO_LEDB_PIN            GPIO_PIN_5

/**
 * Configuration of SPI used for BM-Lite communication.
 */
#define BMLITE_SPI                     SPI1
#define BMLITE_SPI_AF                  GPIO_AF5_SPI1
#define BMLITE_SPI_CLK_ENABLE          __HAL_RCC_SPI1_CLK_ENABLE
#define BMLITE_SPI_IRQn                SPI1_IRQn
#define BMLITE_SPI_IRQ_HANDLER         SPI1_IRQHandler

#define BMLITE_SPI_SCK_PORT            GPIOA
#define BMLITE_SPI_SCK_PIN             GPIO_PIN_5
#define BMLITE_SPI_NSS_PORT            GPIOC
#define BMLITE_SPI_NSS_PIN             GPIO_PIN_12
#define BMLITE_SPI_MISO_PORT           GPIOA
#define BMLITE_SPI_MISO_PIN            GPIO_PIN_6
#define BMLITE_SPI_MOSI_PORT           GPIOA
#define BMLITE_SPI_MOSI_PIN            GPIO_PIN_7

#define BMLITE_SPI_DMA_CHANNEL_RX      DMA1_Channel2
#define BMLITE_SPI_DMA_REQ_RX          DMA_REQUEST_SPI1_RX
#define BMLITE_SPI_DMA_IRQn_RX         DMA1_Channel2_IRQn
#define BMLITE_SPI_DMA_IRQ_HANDLER_RX  DMA1_Channel2_IRQHandler
#define BMLITE_SPI_DMA_CHANNEL_TX      DMA1_Channel3
#define BMLITE_SPI_DMA_REQ_TX          DMA_REQUEST_SPI1_TX
#define BMLITE_SPI_DMA_IRQn_TX         DMA1_Channel3_IRQn
#define BMLITE_SPI_DMA_IRQ_HANDLER_TX  DMA1_Channel3_IRQHandler

/**
 * Configuration of UART used for BM-Lite communication.
*/
#define FPC_BMLITE_USART                     USART1
#define FPC_BMLITE_USART_AF                  GPIO_AF7_USART1
#define FPC_BMLITE_USART_CLK_ENABLE          __HAL_RCC_USART1_CLK_ENABLE
#define FPC_BMLITE_USART_IRQn                USART1_IRQn
#define FPC_BMLITE_USART_IRQ_HANDLER         USART1_IRQHandler

#define FPC_BMLITE_USART_RX_PORT             GPIOA
#define FPC_BMLITE_USART_RX_PIN              GPIO_PIN_10
#define FPC_BMLITE_USART_TX_PORT             GPIOA
#define FPC_BMLITE_USART_TX_PIN              GPIO_PIN_9
#define FPC_BMLITE_USART_RX_PORT_CLK_ENABLE  __HAL_RCC_GPIOA_CLK_ENABLE
#define FPC_BMLITE_USART_TX_PORT_CLK_ENABLE  __HAL_RCC_GPIOB_CLK_ENABLE

#define FPC_BMLITE_USART_DMA_CHANNEL_RX      DMA1_Channel6
#define FPC_BMLITE_USART_DMA_REQ_RX          DMA_REQUEST_USART1_RX
#define FPC_BMLITE_USART_DMA_IRQn_RX         DMA1_Channel6_IRQn
#define FPC_BMLITE_USART_DMA_IRQ_HANDLER_RX  DMA1_Channel6_IRQHandler
#define FPC_BMLITE_USART_DMA_CHANNEL_TX      DMA1_Channel7
#define FPC_BMLITE_USART_DMA_REQ_TX          DMA_REQUEST_USART1_TX
#define FPC_BMLITE_USART_DMA_IRQn_TX         DMA1_Channel7_IRQn
#define FPC_BMLITE_USART_DMA_IRQ_HANDLER_TX  DMA1_Channel7_IRQHandler


/**
 * Configuration of GPIO used for sensor reset / sensor interrupt.
 */
#define BMLITE_RST_PORT     GPIOC
#define BMLITE_RST_PIN      GPIO_PIN_6

#define BMLITE_READY_PORT     GPIOA
#define BMLITE_READY_PIN      GPIO_PIN_1

#endif /* BMLITE_HAL_CONFIG_H */

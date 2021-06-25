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


#ifndef PLATFORM_H
#define PLATFORM_H

/**
 * @file    platform.h
 * @brief   Platform specific function interface
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "fpc_bep_types.h"

/**
 * @brief Initializes board
 *
 * @param[in] params  - pointer to additional parameters.
 */
fpc_bep_result_t platform_init(void *params);

/**
 * @brief Does BM-Lite HW Reset
 *
 */
void platform_bmlite_reset(void);

/**
 * @brief Sends data over SPI port in blocking mode.
 *
 * @param[in]       size        Number of bytes to send.
 * @param[in]       data        Data buffer to send.
 * @param[in]       timeout     Timeout in ms. Use 0 for infinity.
 *
 * @return ::fpc_com_result_t
 */
fpc_bep_result_t platform_bmlite_spi_send(uint16_t size, const uint8_t *data, uint32_t timeout);

/**
 * @brief Sends data over UART port in blocking mode.
 *
 * @param[in]       size        Number of bytes to send.
 * @param[in]       data        Data buffer to send.
 * @param[in]       timeout     Timeout in ms. Use 0 for infinity.
 *
 * @return ::fpc_com_result_t
 */
fpc_bep_result_t platform_bmlite_uart_send(uint16_t size, const uint8_t *data, uint32_t timeout);

/**
 * @brief Receives data from SPI port in blocking mode.
 *
 * @param[in]       size        Number of bytes to receive.
 * @param[in, out]  data        Data buffer to fill.
 * @param[in]       timeout     Timeout in ms. Use 0 for infinity.
 *
 * @return ::fpc_com_result_t
 */
fpc_bep_result_t platform_bmlite_spi_receive(uint16_t size, uint8_t *data, uint32_t timeout);

/**
 * @brief Receives data from UART port in blocking mode.
 *
 * @param[in]       size        Number of bytes to receive.
 * @param[in, out]  data        Data buffer to fill.
 * @param[in]       timeout     Timeout in ms. Use 0 for infinity.
 *
 * @return ::fpc_com_result_t
 */
fpc_bep_result_t platform_bmlite_uart_receive(uint16_t size, uint8_t *data, uint32_t timeout);

/**
 * @brief Stops execution if a debug interface is attached.
 */
void platform_halt_if_debug(void);

/**
 * @brief Performs a software reset.
 */
void platform_sw_reset(void) __attribute__((__noreturn__));

#endif /* PLATFORM_H */

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

#ifndef PLATFORM_RPI_H
#define PLATFORM_RPI_H

/**
 * @file    platform.h
 * @brief   Platform specific function interface
 */

#include <stdint.h>
#include <stdbool.h>

#include "fpc_bep_types.h"
#include "hcp_tiny.h"

typedef enum {
   COM_INTERFACE = 0,
   SPI_INTERFACE
} interface_t;

typedef struct {
   interface_t iface;
   char *port;
   uint32_t baudrate;
   uint32_t timeout;
   HCP_comm_t *hcp_comm;
} rpi_initparams_t;

/*
* Pin definitions for RPI 3
*/
#define BMLITE_RESET_PIN    0
#define BMLITE_IRQ_PIN      22
#define SPI_CHANNEL         0

/**
 * @brief Initializes COM Physical layer.
 *
 * @param[in]       port        tty port to use.
 * @param[in]       baudrate    Baudrate.
 * @param[in]       timeout     Timeout in ms. Use 0 for infinity.
 */
bool rpi_com_init(char *port, int baudrate, int timeout);

/**
 * @brief Sends data over communication port in blocking mode.
 *
 * @param[in]       size        Number of bytes to send.
 * @param[in]       data        Data buffer to send.
 * @param[in]       timeout     Timeout in ms. Use 0 for infinity.
 *
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t rpi_com_send(uint16_t size, const uint8_t *data, uint32_t timeout);

/**
 * @brief Receives data from communication port in blocking mode.
 *
 * @param[in]       size        Number of bytes to receive.
 * @param[in, out]  data        Data buffer to fill.
 * @param[in]       timeout     Timeout in ms. Use 0 for infinity.
 *
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t rpi_com_receive(uint16_t size, uint8_t *data, uint32_t timeout);

/**
 * @brief Initializes SPI Physical layer.
 *
 * @param[in]       speed_hz    Baudrate.
 */
bool rpi_spi_init(uint32_t speed_hz);

/**
 * @brief Sends data over communication port in blocking mode.
 *
 * @param[in]       size        Number of bytes to send.
 * @param[in]       data        Data buffer to send.
 * @param[in]       timeout     Timeout in ms. Use 0 for infinity.
 *
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t platform_spi_send(uint16_t size, const uint8_t *data, uint32_t timeout,
        void *session);

/**
 * @brief Receives data from communication port in blocking mode.
 *
 * @param[in]       size        Number of bytes to receive.
 * @param[in, out]  data        Data buffer to fill.
 * @param[in]       timeout     Timeout in ms. Use 0 for infinity.
 *
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t platform_spi_receive(uint16_t size, uint8_t *data, uint32_t timeout,
        void *session);

/**
 * @brief Get time in micro seconds
 *
 * @return time in us.
 */
// uint64_t platform_get_time(void);

/**
 * @brief Clear console screen
 */
void rpi_clear_screen(void);

/**
 * @brief Busy wait.
 *
 * @param[in] ms  Time to wait [ms].
 * 0 => return immediately
 * 1 => wait at least 1ms etc.
 */
void hal_timebase_busy_wait(uint32_t ms);


#endif /* PLATFORM_RPI_H */

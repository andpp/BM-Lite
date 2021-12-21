/*
 * Copyright (c) 2020 Andrey Perminov <andrey.ppp@gmail.com>
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
 */

/**
 * @file   bmlite_hal.h
 * @brief  BM-Lite HAL functions.
 * 
 *    All functions must be implemented in order to support BM-Lite on a Board
 */

#ifndef BMLITE_H
#define BMLITE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "fpc_bep_types.h"

#ifdef __arm__
typedef uint32_t hal_tick_t;
#else 
typedef uint64_t hal_tick_t;
#endif

/**
 * @brief LED status.
 *
 * Different LED status.
 */
typedef enum {
    BMLITE_LED_STATUS_READY = 0,
    BMLITE_LED_STATUS_MATCH,
    BMLITE_LED_STATUS_WAITTOUCH,
    BMLITE_LED_STATUS_ENROLL,
    BMLITE_LED_STATUS_DELETE_TEMPLATES,
    BMLITE_LED_STATUS_ERROR,
} platform_led_status_t;


/*
 * @brief Board initialization
 * @param[in] params  - pointer to additional parameters
 */

fpc_bep_result_t hal_board_init(void *params);

/*
 * @brief Control BM-Lite Reset pin
 * @param[in] True  - Activate RESET
 *            False - Deactivate RESET
 */
void hal_bmlite_reset(bool state);

/*
 * @brief SPI write-read
 * @param[in] Write buffer
 * @param[in] Read buffer
 * @param[in] Size
 * @param[in] Leave CS asserted
 * @return ::fpc_bep_result_t
 */
fpc_bep_result_t hal_bmlite_spi_write_read(uint8_t *write, uint8_t *read, size_t size,
        bool leave_cs_asserted);

/*
 * @brief UART write
 * @param[in] Write buffer
 * @param[in] Size
 * @return ::size_t Number of bytes actually written
 */
size_t hal_bmlite_uart_write(const uint8_t *data, size_t size);

/*
 * @brief UART read
 * @param[in] Read buffer
 * @param[in] Size
 * @return ::size_t Number of butes actually read
 */
size_t hal_bmlite_uart_read(uint8_t *buff, size_t size);

/*
 * @brief Check if BM-Lite IRQ pin is set
 * @return ::bool
 */
bool hal_bmlite_get_status(void);

/**
 * @brief Initializes timebase. Starts system tick counter.
 */
void hal_timebase_init(void);

/**
 * @brief Reads the system tick counter.
 *
 * @return Tick count since hal_timebase_init() call. [ms]
 */
hal_tick_t hal_timebase_get_tick(void);

/**
 * @brief Busy wait.
 *
 * @param[in] ms  Time to wait [ms].
 * 0 => return immediately
 * 1 => wait at least 1ms etc.
 */
void hal_timebase_busy_wait(uint32_t ms);

/**
 *  Optional functions for Buttons & Leds control
 */

/**
 * @brief Get button press time (msec)
 * 
 * @return ::uint32_t
 */
uint32_t hal_get_button_press_time(void);

/**
 * @brief Check if button was pressed and released.
 *
 * @return Button press time in milli seconds.
 */
uint32_t hal_check_button_pressed(void);

/**
 * @brief Set LED(s) status
 * @param[in] Status
 * @param[in] Status modifier
 */
void hal_set_leds(platform_led_status_t status, uint16_t mode);


#endif /* BMLITE_H */

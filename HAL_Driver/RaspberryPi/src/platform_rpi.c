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
 * @file    platform_linux.c
 * @brief   Linux platform specific functions
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <sys/time.h>
#include <unistd.h>

#include "bmlite_hal.h"
#include "platform_rpi.h"
#include "platform.h"
#include "console_params.h"

hal_tick_t hal_timebase_get_tick(void)
{
    struct timeval current_time;
    uint64_t time_in_ms;

    gettimeofday(&current_time, NULL);
    /* Seconds and microseconds are converted to milliseconds */
    time_in_ms = current_time.tv_usec / 1000 + current_time.tv_sec * 1000;

    return time_in_ms;
}

void hal_timebase_busy_wait(uint32_t ms)
{
    usleep(ms * 1000);
}

void clear_screen(void)
{
    system("clear");
}

void hal_timebase_init()
{
}

fpc_bep_result_t hal_board_init(void *params)
{
    console_initparams_t *p = (console_initparams_t *)params;
        switch (p->iface) {
        case SPI_INTERFACE:
            if(!rpi_spi_init(p->baudrate)) {
                printf("SPI initialization failed\n");
                return FPC_BEP_RESULT_INTERNAL_ERROR;
            }
            break;
        case COM_INTERFACE:
            if (!rpi_com_init(p->port, p->baudrate, p->timeout)) {
                printf("Com initialization failed\n");
                return FPC_BEP_RESULT_INTERNAL_ERROR;
            }
            break;
        default:
            printf("Interface not specified'n");
            return FPC_BEP_RESULT_INTERNAL_ERROR;
    }

    if (p->iface == COM_INTERFACE) {
        p->hcp_comm->read = platform_bmlite_uart_receive;
        p->hcp_comm->write = platform_bmlite_uart_send;
    } else {
        p->hcp_comm->read = platform_bmlite_spi_receive;
        p->hcp_comm->write = platform_bmlite_spi_send;
    }

    p->hcp_comm->phy_rx_timeout = p->timeout*1000;

    return FPC_BEP_RESULT_OK;
}

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
 * @file    platform_linux.c
 * @brief   Linux platform specific functions
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "bmlite_hal.h"
#include "platform.h"
#include "console_params.h"
#include "platform_defs.h"

#define MAX_FNAME_LEN 128

static int fd_spi = -1;
static int fd_reset_value = -1;
static int fd_ready_value = -1;

static struct spi_ioc_transfer spi_tr = {
    .tx_buf = (unsigned long)0,
    .rx_buf = (unsigned long)0,
    .len = 0,
    .delay_usecs = 1000,
    .speed_hz = 500000,
    .bits_per_word = 8,
};

static fpc_bep_result_t platform_spi_init(char *device, uint32_t baudrate);
static fpc_bep_result_t platform_gpio_init();
static int gpio_init(uint32_t pin, gpio_dir_t dir);


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
            if(p->port == NULL)
               p->port = BMLITE_SPI_DEV;

            if(platform_spi_init(p->port, p->baudrate) != FPC_BEP_RESULT_OK) {
                printf("SPI initialization failed\n");
                return FPC_BEP_RESULT_INTERNAL_ERROR;
            }
            break;
        default:
            printf("Interface not specified'n");
            return FPC_BEP_RESULT_INTERNAL_ERROR;
    }

    p->hcp_comm->read = platform_bmlite_spi_receive;
    p->hcp_comm->write = platform_bmlite_spi_send;
    p->hcp_comm->phy_rx_timeout = p->timeout*1000;

    return platform_gpio_init();
}

void hal_bmlite_reset(bool state)
{
    /* The reset pin is controlled by WiringPis digitalWrite function*/
    if (state) {
        write(fd_reset_value, "0", 2);
    } else {
        write(fd_reset_value, "1", 2);
    }
}

bool hal_bmlite_get_status(void)
{
    char res[2];

    lseek (fd_ready_value, 0, SEEK_SET);
    read (fd_ready_value, res, 2);
    return res[0] == '1';
}

fpc_bep_result_t hal_bmlite_spi_write_read(uint8_t *write, uint8_t *read, size_t size,
    bool leave_cs_asserted)
{

    size_t status;

    spi_tr.tx_buf        = (unsigned long)write;
    spi_tr.rx_buf        = (unsigned long)read;
    spi_tr.len           = size;

    status = ioctl(fd_spi, SPI_IOC_MESSAGE(1), &spi_tr);

    /*
     * Status returns the number of bytes sent, if this number is different
     * from the the number of bytes written an error has occured.
     */
    if (status == size) {
        return FPC_BEP_RESULT_OK;
    }
    return FPC_BEP_RESULT_IO_ERROR;
}


static fpc_bep_result_t platform_spi_init(char *device, uint32_t baudrate)
{
    uint8_t mode = 0;
    uint32_t speed = baudrate;
    uint8_t bits = 8;

    spi_tr.bits_per_word = bits;
    spi_tr.speed_hz = baudrate;

    fd_spi = open(device, O_RDWR);
	if (fd_spi < 0) {
		printf("Can't open device %s\n", device);
        return FPC_BEP_RESULT_INTERNAL_ERROR;
    }

	if(ioctl(fd_spi, SPI_IOC_WR_MODE, &mode) < 0) {
		printf("Can't set spi mode");
        return FPC_BEP_RESULT_INTERNAL_ERROR;
    }

	if(ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
		printf("Can't set bits per word");
        return FPC_BEP_RESULT_INTERNAL_ERROR;
    }

	if(ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
		printf("Can't set speed hz");
        return FPC_BEP_RESULT_INTERNAL_ERROR;
    }


    return FPC_BEP_RESULT_OK;
}

static fpc_bep_result_t platform_gpio_init()
{
    fd_reset_value = gpio_init(BMLITE_RESET_PIN, GPIO_DIR_OUT);
    fd_ready_value = gpio_init(BMLITE_READY_PIN, GPIO_DIR_IN);

    if(fd_ready_value < 0 || fd_ready_value < 0) 
        return FPC_BEP_RESULT_INTERNAL_ERROR;
    else
        return FPC_BEP_RESULT_OK;
}

static int gpio_init(uint32_t pin, gpio_dir_t dir)
{

    char fn_val[MAX_FNAME_LEN];
    char tmp[MAX_FNAME_LEN];
    int fd;
    int flags;

    snprintf(fn_val, MAX_FNAME_LEN, "/sys/class/gpio/gpio%d/value", pin);

    // Export pin
    if((fd = open(fn_val, O_SYNC | O_WRONLY)) < 0) {
        fd = open("/sys/class/gpio/export", O_SYNC | O_WRONLY);
        if(fd < 0) {
            if(dir == GPIO_DIR_OUT)
                printf("Can't export BM-Lite RESET pin\n");
            else
                printf("Can't export BM-Lite READY pin\n");
            return FPC_BEP_RESULT_INTERNAL_ERROR;
        }
        int size = snprintf(tmp, MAX_FNAME_LEN, "%d", pin);
        write(fd, tmp, size);
        close(fd);
    } else {
        close(fd);
    }

    // Set pin direction
    snprintf (tmp, MAX_FNAME_LEN, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(tmp, O_SYNC | O_WRONLY);
    if(fd >= 0) {
	// Some GPIO doesn't allow to change pin direction
        if(dir == GPIO_DIR_OUT)
            write(fd, "out", 4);
        else 
            write(fd, "in", 4);
        close(fd);
    }

    if(dir == GPIO_DIR_OUT)
        flags = O_SYNC | O_WRONLY;
    else
        flags = O_SYNC | O_RDONLY;

    fd = open(fn_val, flags);
    if(fd < 0) {
        printf("Can't open %s\n", fn_val);
    }
    return fd;
}


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
 * @brief   Linux SPI platform specific functions
 */

#define _BSD_SOURCE

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <sys/time.h>

#include "platform.h"

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>

#include "wiringPi.h"
#include "wiringPiSPI.h"
#include "platform_rpi.h"

#define SPI_BUF_MIN_SIZE 40000
// This path is used to determine the SPI buffer size.
static char SPIBufPath[] = "/sys/module/spidev/parameters/bufsiz";

static const uint16_t    spiDelay = 0;
static const uint8_t     spiBPW   = 8;

uint32_t speed_hz_int;

void fpc_bmlite_reset(bool state);

static void raspberryPi_init()
{
    /* Start wiringPi functions. */
    wiringPiSetup();

    /* Set correct pin modes */
    pinMode(BMLITE_READY_PIN, INPUT);
    pinMode(BMLITE_RESET_PIN, OUTPUT);

    /* Set reset high */
    digitalWrite(BMLITE_RESET_PIN, 1);

}

void hal_bmlite_reset(bool state)
{
    /* The reset pin is controlled by WiringPis digitalWrite function*/
    if (state) {
        digitalWrite(BMLITE_RESET_PIN, 0);
    } else {
        digitalWrite(BMLITE_RESET_PIN, 1);
    }
}

bool hal_bmlite_get_status(void)
{
    return digitalRead(BMLITE_READY_PIN);
}


bool rpi_spi_init(uint32_t speed_hz)
{
    raspberryPi_init();

    /* In standard the SPI drivers buffer is 4096 bytes, the current buffer
     * size is read and compared to minimum required size.
     */

    FILE *fp;
    fp = fopen(SPIBufPath, "r");
    /* The file is opened, and if this failed the program exits. */
    if (fp == NULL) {
        printf("Could not open %s to get buffersize\n", SPIBufPath);
        exit(1);
    }

    /* The size of the file is determined, then the whole file is read. */
    fseek(fp, 0, SEEK_END);         // Seek to end of file.
    size_t filesize = ftell(fp);    // Get current file pointer.
    fseek(fp, 0, SEEK_SET);         // Seek back to beginning of file.

    char buffersize[filesize];
    if (fread(buffersize, 1, filesize, fp) == filesize) {
        printf("Failed to read data from %s.\n", SPIBufPath);
        exit(1);
    }
    fclose(fp);
    /* Verify that SPI buffer meets minimum size. */
    if (atoi(buffersize) < SPI_BUF_MIN_SIZE) {
        printf("SPI buffer size is too small, if the standard RPI driver is "
               "used, this can be changed by adding \"spidev.bufsiz=<size>\" "
               "to /boot/cmdline.txt. <size> needs to be larger than %d for this "
               "sensor.\n",
               SPI_BUF_MIN_SIZE);
        exit(1);
    }

    /*
     * If the buffer has been verified as large enough the SPI will initialize,
     *  the reset and IRQ pin will also be set up.
     */
    int SpiRef;
    SpiRef = wiringPiSPISetup(SPI_CHANNEL, speed_hz);
    speed_hz_int = speed_hz;

    if (SpiRef == -1) {
        printf("WiringPi GPIO setup failed with error %d", errno);
        return false;
    }

    return true;
}

fpc_bep_result_t hal_bmlite_spi_write_read(uint8_t *write, uint8_t *read, size_t size,
    bool leave_cs_asserted)
{
    /*
     * SPI data is transmitted using an edited version of wiringPiSPIDataRW,
     * since the original function does not have support for holding the CS
     * signal low after a transmission which is needed by bep-lib.
     */
    size_t status;
    struct spi_ioc_transfer spi;
    memset (&spi, 0, sizeof (spi));

    /* The file descriptor is fetched from wiringPi. */
    int spiFds = wiringPiSPIGetFd(SPI_CHANNEL);

    spi.tx_buf        = (unsigned long)write;
    spi.rx_buf        = (unsigned long)read;
    spi.len           = size;
    spi.delay_usecs   = spiDelay;
    spi.speed_hz      = speed_hz_int;
    spi.bits_per_word = spiBPW;
    spi.cs_change     = leave_cs_asserted;

    status = ioctl(spiFds, SPI_IOC_MESSAGE(1), &spi);

    /*
     * Status returns the number of bytes sent, if this number is different
     * from the the number of bytes written an error has occured.
     */
    if (status == size) {
        return FPC_BEP_RESULT_OK;
    }
    return FPC_BEP_RESULT_IO_ERROR;

}

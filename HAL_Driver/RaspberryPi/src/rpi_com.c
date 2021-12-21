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
 */

/**
 * @file    platform_com.c
 * @brief   Linux COM platform specific functions
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

#include "platform_rpi.h"

static int fd = -1;

static int set_interface_attribs(int fd, int speed, int timeout)
{
    struct termios tty;

    memset(&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0) {
        fprintf(stderr, "error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_iflag = 0; // Clear input modes
    tty.c_oflag = 0; // Clear output modes
    tty.c_cflag = 0; // Clear control modes
    tty.c_lflag = 0; // Clear local modes
    tty.c_cflag |= CS8; // Set size mask to 8 bits
    tty.c_cflag |= CREAD; // Enable receiver
    tty.c_cflag |= HUPCL; // Lower modem control lines after last process closes the device (hang up)
    tty.c_cflag |= CLOCAL; // Ignore modem control lines

    tty.c_cc[VMIN] = 0; // Minimum number of characters for non-canonical read
    tty.c_cc[VTIME] = 0; // Timeout in deciseconds for non-canonical read

    tcsetattr(fd, TCSANOW, &tty);

    return 0;
}

bool rpi_com_init(char *port, int baudrate, int timeout)
{

    int baud;

    fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        fprintf(stderr, "error %d opening %s: %s", errno, port, strerror (errno));
        return false;
    }

    if (baudrate == 921600) {
        baud = B921600;
    } else {
        baud = B115200;
    }
    set_interface_attribs(fd, baud, timeout);

    return true;
}

size_t hal_bmlite_uart_write(const uint8_t *data, size_t size)
{
    int n;

    if (fd < 0) {
        fprintf(stderr, "error invalid file descriptor");
        return 0;
    }

    return write(fd, data, size);

}

size_t hal_bmlite_uart_read(uint8_t *data, size_t size)
{
    fpc_bep_result_t res = FPC_BEP_RESULT_OK;
    int n_read = 0;
    int n = 0;
    fd_set rfds;
    int retval;
    unsigned char c;

    if (fd < 0) {
        fprintf(stderr, "error invalid file descriptor");
        return FPC_BEP_RESULT_INVALID_ARGUMENT;
    }

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    while (n_read < size) {
        retval = select(fd + 1, &rfds, NULL, NULL, NULL);

        if (retval == -1) {
            perror("select()");
            exit(EXIT_FAILURE);
        } else if (retval != 0) {
            n = read(fd, &c, sizeof(c));
            if (n > 0) {
                *data = c;
                data++;
                n_read++;
            }
        }
    }

    return n_read;
}



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

#ifndef PLATFORM_DEFS_H
#define PLATFORM_DEFS_H

#define BMLITE_SPI_DEV "/dev/spidev2.1"

#define BMLITE_RESET_PIN (504 + 3)
#define BMLITE_READY_PIN ((6 - 1) * 32 + 14)

typedef enum {
     GPIO_DIR_IN,
     GPIO_DIR_OUT 
} gpio_dir_t;

#endif
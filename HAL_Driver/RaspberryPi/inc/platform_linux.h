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

#ifndef PLATFORM_LINUX_H
#define PLATFORM_LINUX_H

/**
 * @file    platform.h
 * @brief   Platform specific function interface
 */

#include <stdint.h>
#include <stdbool.h>

#include "fpc_bep_types.h"
#include "hcp_tiny.h"

void clear_screen(void);

#endif /* PLATFORM_RPI_H */

#  Copyright (c) 2020 Fingerprint Cards AB
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Modified by Andrey Perminov <andrey.ppp@gmail.com> 
#  for FPC BM-Lite applications



HAL_PATH := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

CPU = cortex_m4
FPU ?= hard
CC := arm-none-eabi-gcc

CFLAGS +=\
	-mcpu=cortex-m4\
	-mthumb -mabi=aapcs\
 	-ffunction-sections -fdata-sections -fno-strict-aliasing\
 	-fno-builtin --short-enums \
	-Wno-expansion-to-defined \

# SDK definitions

CFLAGS +=\
	-DUSE_HAL_DRIVER \
	-DNUCLEO \
	-DARM_MATH_CM4

CFLAGS += -DSTM32WB55xx

ifeq ($(FPU),soft)
CFLAGS +=\
        -mfloat-abi=soft
else
CFLAGS +=\
        -mfloat-abi=hard \
        -mfpu=fpv4-sp-d16
endif

ifeq ($(DEBUG),y)
CFLAGS += -g -O0
LDFLAGS += -g
endif

HAL := $(HAL_PATH)
SDK_ROOT := $(HAL)/STM32WBxx_HAL_Driver

LDFILE = $(HAL)/spi_gcc_nrf52.ld
LDFLAGS += -T $(HAL)/STM32WB55RGVX_FLASH.ld

# # Suppress warnings in vendor files
$(OUT)/obj/stm32wbxx_hal_pwr_ex.o: CFLAGS+=-Wno-unused-parameter
$(OUT)/obj/stm32wbxx_hal_rtc_ex.o: CFLAGS+=-Wno-unused-parameter
$(OUT)/obj/syscalls.o: CFLAGS+=-Wno-unused-parameter
$(OUT)/obj/hal_uart.o: CFLAGS+=-Wno-discarded-qualifiers

ifeq ($(FPU),soft)
AR_LIBS +=\
	$(HAL)/CMSIS/Lib/libarm_cortexM4l_math.a
else
AR_LIBS +=\
	$(HAL)/CMSIS/Lib/libarm_cortexM4lf_math.a
endif

# C Sources
HAL_SRCS := \
	$(SDK_ROOT)/Src/stm32wbxx_hal.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_cortex.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_dma.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_flash.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_flash_ex.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_gpio.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_pwr.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_pwr_ex.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_rcc.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_rcc_ex.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_rtc.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_rtc_ex.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_spi.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_uart.c \
	$(SDK_ROOT)/Src/stm32wbxx_hal_uart_ex.c \


VPATH += $(dir $(HAL_SRCS))
C_SRCS += $(notdir $(HAL_SRCS))

# Source Folders
VPATH += $(HAL)/src/

# C Sources
C_SRCS += $(notdir $(wildcard $(HAL)/src/*.c))

# S Sources
S_SRCS += $(HAL)/src/startup_stm32wb55rgvx.s 


# Source Folders
VPATH += $(sort $(dir $(HAL_FPC_C_SRCS) $(HAL_VENDOR_C_SRCS)))

# Includes
C_INC +=\
	-I$(HAL)/inc \
	-I$(SDK_ROOT)/Inc \
	-I$(HAL)/CMSIS/Include \
	-I$(HAL)/CMSIS/Device/ST/STM32WBxx/Include

	

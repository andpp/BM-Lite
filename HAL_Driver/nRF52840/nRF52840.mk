#
# Copyright (c) 2020 Fingerprint Cards AB
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#
#  Modified by Andrey Perminov <andrey.ppp@gmail.com> 
#  for FPC BM-Lite applications
#


CPU = cortex_m4
LOG :=
CC := arm-none-eabi-gcc

CFLAGS +=\
	-mcpu=cortex-m4\
	-mthumb -mabi=aapcs\
 	-ffunction-sections -fdata-sections -fno-strict-aliasing\
 	-fno-builtin --short-enums \
	-Wno-expansion-to-defined \

# SDK definitions
CFLAGS +=\
	-DBOARD_PCA10056\
	-DCONFIG_GPIO_AS_PINRESET\
	-DNRF52840_XXAA\
	-D__STARTUP_CONFIG \
 	-DMALLOC_PROVIDED \
 	-DFPC_MALLOC \
 	-DBSP_SIMPLE \
 	-Wa,--defsym,_STARTUP_CONFIG=1 \
 	-DUART_CMDS \

CFLAGS +=\
	-DUSE_HAL_DRIVER \
	-DARM_MATH_CM4\

ifeq ($(FPU),soft)
CFLAGS +=\
        -mfloat-abi=soft
else
CFLAGS +=\
        -mfloat-abi=hard \
        -mfpu=fpv4-sp-d16
endif

HAL := $(HAL_PATH)
SDK_ROOT := $(HAL)/sdk

# Linker flags
LDFILE = $(HAL)/spi_gcc_nrf52.ld
TEMPLATE_PATH := $(SDK_ROOT)/modules/nrfx/mdk

LDFLAGS += -T $(LDFILE) -L $(TEMPLATE_PATH)

LDLAGS +=\
    -mthumb -mabi=aapcs\
	-mcpu=cortex-m4\
	-L $(TEMPLATE_PATH) 

LDFLAGS +=\
	-lc -lgcc \

AR_LIBS +=\
	$(HAL)/CMSIS/dsp/GCC/libarm_cortexM4lf_math.a 

HAL_SRCS += \
  $(SDK_ROOT)/components/boards/boards.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_rtt.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_uart.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_default_backends.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_str_formatter.c \
  $(SDK_ROOT)/components/libraries/button/app_button.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
  $(SDK_ROOT)/components/libraries/util/app_error_weak.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
  $(SDK_ROOT)/components/libraries/fifo/app_fifo.c \
  $(SDK_ROOT)/components/libraries/uart/app_uart_fifo.c \
  $(SDK_ROOT)/components/libraries/scheduler/app_scheduler.c \
  $(SDK_ROOT)/components/libraries/timer/app_timer.c \
  $(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
  $(SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
  $(SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
  $(SDK_ROOT)/components/libraries/ringbuf/nrf_ringbuf.c \
  $(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
  $(SDK_ROOT)/components/libraries/bsp/bsp.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_spi.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_uart.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_gpiote.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_spi.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_spim.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uart.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uarte.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_timer.c \
  $(SDK_ROOT)/modules/nrfx/soc/nrfx_atomic.c \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c \
  $(SDK_ROOT)/modules/nrfx/hal/nrf_nvmc.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf_format.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c \
  $(SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c \
#  $(PROJ_DIR)/main.c \
   	  
VPATH += $(dir $(HAL_SRCS))
C_SRCS += $(notdir $(HAL_SRCS))

# Source Folders
VPATH += $(HAL)/src/

# C Sources
C_SRCS += $(notdir $(wildcard $(HAL)/src/*.c))

# S Sources
S_SRCS += $(HAL)/src/gcc_startup_nrf52840.S 

# Includes
C_INC +=\
	-I$(HAL)/config \
	-I$(HAL)/inc \
	-I$(HAL)/CMSIS/Include \
	-I$(SDK_ROOT)/components \
	-I$(SDK_ROOT)/components/libraries/scheduler \
	-I$(SDK_ROOT)/components/libraries/timer \
	-I$(SDK_ROOT)/components/libraries/strerror \
	-I$(SDK_ROOT)/components/toolchain/cmsis/include \
	-I$(SDK_ROOT)/components/libraries/util \
	-I$(SDK_ROOT)/components/libraries/balloc \
	-I$(SDK_ROOT)/components/libraries/ringbuf \
	-I$(SDK_ROOT)/components/libraries/fifo \
	-I$(SDK_ROOT)/components/libraries/bsp \
	-I$(SDK_ROOT)/components/libraries/log \
	-I$(SDK_ROOT)/components/libraries/uart \
	-I$(SDK_ROOT)/components/libraries/button \
	-I$(SDK_ROOT)/components/libraries/experimental_section_vars \
	-I$(SDK_ROOT)/integration/nrfx/legacy \
	-I$(SDK_ROOT)/components/libraries/delay \
	-I$(SDK_ROOT)/integration/nrfx \
	-I$(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd \
	-I$(SDK_ROOT)/external/segger_rtt \
	-I$(SDK_ROOT)/components/boards \
	-I$(SDK_ROOT)/components/libraries/memobj \
	-I$(SDK_ROOT)/modules/nrfx/drivers/include \
	-I$(SDK_ROOT)/components/libraries/log/src \
	-I$(SDK_ROOT)/external/fprintf \
	-I$(SDK_ROOT)/components/libraries/atomic \
	-I$(SDK_ROOT)/modules/nrfx \
	-I$(SDK_ROOT)/modules/nrfx/hal \
	-I$(SDK_ROOT)/modules/nrfx/mdk \

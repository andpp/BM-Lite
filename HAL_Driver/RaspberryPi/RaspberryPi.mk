# Binary sources

HAL = $(ROOT)/HAL_Driver/RaspberryPi

PATH := /work/devtools/gcc-arm-hf/bin:$(PATH)
CC := arm-linux-gnueabihf-gcc

CFLAGS +=\
	-D_DEFAULT_SOURCE 

VPATH += $(HAL)
C_INC += -I$(HAL)/inc
LDFLAGS += -lwiringPi -L$(HAL)/lib/ 

# Source Folders
VPATH += $(HAL)/src/

# C Sources
C_SRCS += $(notdir $(wildcard $(HAL)/src/*.c))
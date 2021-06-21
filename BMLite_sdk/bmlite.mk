# Binary sources

BMLITE_SDK = $(ROOT)/BMLite_sdk

VPATH += $(BMLITE_SDK)

C_INC += -I$(BMLITE_SDK)/inc

# Source Folders
VPATH += $(BMLITE_SDK)/src/

# C Sources
C_SRCS += $(notdir $(wildcard $(BMLITE_SDK)/src/*.c))
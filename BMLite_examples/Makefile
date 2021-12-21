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


# Make sure that 'all' target become default target
.DEFAULT_GOAL := all

PLATFORM ?= RaspberryPi
APP ?= console_app

PRODUCT := $(APP)

# Setup paths
ROOT := ..
HAL_BASE := $(ROOT)/HAL_Driver
HAL_PATH := $(HAL_BASE)/$(PLATFORM)
BMLITE_PATH := $(ROOT)/BMLite_sdk
APP_PATH := $(ROOT)/BMLite_examples
OUT_BASE := $(APP_PATH)/out
OUT := $(OUT_BASE)/$(APP)/$(PLATFORM)

HAL_DIRS = $(subst $(HAL_BASE),, $(sort $(dir $(wildcard $(HAL_BASE)/*/))))
HAL_LIST = $(subst /,,$(HAL_DIRS))
APP_LIST = $(filter-out out, $(subst /,, $(sort $(dir $(wildcard */)))))

ifneq ($(PLATFORM),)
 ifeq ($(filter $(PLATFORM),$(HAL_LIST)),)  
   $(error "Unknown platform: $(PLATFORM). Use one of $(HAL_LIST)")
 endif
endif

ifneq ($(APP),)
 ifeq ($(filter $(APP),$(APP_LIST)),)  
   $(error "Unknown application: $(APP). Use one of $(APP_LIST)")
 endif
endif


ifeq ($(filter $(PLATFORM), RaspberryPi Linux),)
  ifeq ($(APP),console_app)
    $(error 'console_app is not supported for $(PLATFORM)')
  endif
else
  ifeq ($(APP),embedded_app)
    $(error 'embedded_app is not supported for $(PLATFORM)')
  endif
endif

# Main target
TARGET := $(OUT)/$(PRODUCT)

# Common flags
CFLAGS +=\
       -std=c99 \
       -Wall \
       -Wextra \
       -Werror \
       -Wno-unused-parameter \
       -fdata-sections \
       -ffunction-sections \
       -MMD \
       -MP 

ifeq ($(PORT),UART)
CFLAGS += -DBMLITE_ON_UART
else
CFLAGS += -DBMLITE_ON_SPI
endif


ifeq ($(DEBUG),y)
CFLAGS +=\
  -g3\
  -Og\
  -DDEBUG
endif

# Include main application
include $(APP_PATH)/$(APP)/app.mk
# Include BM-Lite SDK
include $(BMLITE_PATH)/bmlite.mk
# Include HAL driver
include $(HAL_PATH)/$(PLATFORM).mk

# Object files and search paths
VPATH += $(sort $(dir $(C_SRCS)))
OBJECTS = $(patsubst %.c,$(OUT)/obj/%.o,$(notdir $(C_SRCS)))

# Dependency files
DEP := $(OBJECTS:.o=.d)
DEP_CFLAGS=$(OUT)/dep_cflags.txt

all: $(TARGET)

# Create binary from object files and external libraries
$(TARGET): $(OBJECTS) $(DEP_CFLAGS) $(S_SRCS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(C_INC) $(S_SRCS) $(OBJECTS) $(LDFLAGS) -o $@

# Compile source files
$(OUT)/obj/%.o: %.c $(DEP_CFLAGS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(C_INC) -o $@ -c $<

# Detect changes in CFLAGS
$(DEP_CFLAGS): force
	@mkdir -p $(dir $@)
	@echo '$(CFLAGS)' | cmp -s - $@ || echo '$(CFLAGS)' > $@

-include $(DEP)

# Empty rule for dep files, they will be created when compiling
%.d: ;

list_apps:
	@echo Available apps: $(APP_LIST)

list_hals:
	@echo Available HAL: $(HAL_LIST)

clean:
	rm -rf $(OUT)

clean_all:
	rm -rf $(OUT_BASE)

.PHONY: clean clean_all force

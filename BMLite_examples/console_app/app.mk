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

# C source files
APP_DIR = $(APP_PATH)/$(APP)

C_SRCS += $(wildcard $(APP_DIR)/src/*.c)

# Include directories
PATH_INC += $(APP_DIR)/inc

C_INC += $(addprefix -I,$(PATH_INC))

CFLAGS +=\
	-DBMLITE_USE_CALLBACK \
	-DDEBUG_COMM


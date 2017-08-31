##
 # Copyright (C) 2016 YunOS Project. All rights reserved.
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #   http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
##

NAME := libmisc

$(NAME)_INCLUDES += \
    ./ \
    ../sdk-impl \
    ../LITE-log \
    ../LITE-utils
# don't modify to L_CFLAGS, because CONFIG_CJSON_WITHOUT_DOUBLE should enable global

$(NAME)_SOURCES := \
    utils_httpc.c \
    utils_epoch_time.c \
    utils_list.c \
    utils_net.c \
    utils_timer.c

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
PLATFORM_MQTT := linux
#NETWORK_MQTT := linuxsock
else ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
PLATFORM_MQTT := linux
#NETWORK_MQTT := linuxsock
else ifeq ($(findstring mk3060, $(BUILD_STRING)), mk3060)
PLATFORM_MQTT := rhino
#NETWORK_MQTT := rhinosock
endif

ifeq ($(CONFIG_OTA_CH),coap)
$(NAME)_SOURCES += ../hal/$(PLATFORM_MQTT)/HAL_TCP_$(PLATFORM_MQTT).c
ifneq ($(CONFIG_COAP_DTLS_SUPPORT), y)
$(NAME)_SOURCES += ../hal/$(PLATFORM_MQTT)/HAL_OS_$(PLATFORM_MQTT).c
$(NAME)_DEFINES += IOTX_WITHOUT_TLS
$(NAME)_COMPONENTS += utility.iotx-utils.mbedtls-lib
endif
endif

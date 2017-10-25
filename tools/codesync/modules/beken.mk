#
#  UNPUBLISHED PROPRIETARY SOURCE CODE
#  Copyright (c) 2016 MXCHIP Inc.
#
#  The contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of MXCHIP Corporation.
#

NAME := beken

HOST_OPENOCD := beken

ifeq ($(CONFIG_SOFTAP),1)
GLOBAL_CFLAGS += -DCONFIG_SOFTAP
endif

$(NAME)_TYPE := kernel

$(NAME)_COMPONENTS += platform/arch/arm/armv5
$(NAME)_COMPONENTS += rhino hal netmgr framework.common mbedtls cjson cli digest_algorithm
$(NAME)_COMPONENTS += protocols.net protocols.mesh
$(NAME)_COMPONENTS += platform/mcu/beken/aos/app_runtime
$(NAME)_COMPONENTS += platform/mcu/beken/aos/framework_runtime

GLOBAL_DEFINES += CONFIG_MX108
GLOBAL_DEFINES += CONFIG_AOS_KV_MULTIPTN_MODE
GLOBAL_DEFINES += CONFIG_AOS_KV_PTN=6
GLOBAL_DEFINES += CONFIG_AOS_KV_SECOND_PTN=7
GLOBAL_DEFINES += CONFIG_AOS_KV_PTN_SIZE=4096
GLOBAL_DEFINES += CONFIG_AOS_KV_BUFFER_SIZE=8192

GLOBAL_CFLAGS += -mcpu=arm968e-s \
                 -march=armv5te \
                 -mthumb -mthumb-interwork \
                 -mlittle-endian

GLOBAL_CFLAGS += -w

$(NAME)_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing


GLOBAL_INCLUDES += include/lwip-2.0.2/port \
                   include/common \
                   include/app/config \
                   include/func/include \
                   include/os/include \
                   include/driver/include \
                   include/driver/common \
                   include/ip/common \
                   include


GLOBAL_LDFLAGS += -mcpu=arm968e-s \
                 -march=armv5te \
                 -mthumb -mthumb-interwork\
                 -mlittle-endian \
                 --specs=nosys.specs \
                 -nostartfiles \
                 $(CLIB_LDFLAGS_NANO_FLOAT)

BINS ?=

ifeq ($(APP),bootloader)
GLOBAL_LDFLAGS += -T platform/mcu/beken/linkinfo/bk7231_boot.ld
else

ifeq ($(BINS),)
GLOBAL_LDS_FILES += platform/mcu/beken/linkinfo/bk7231.ld.S
else ifeq ($(BINS),app)
GLOBAL_LDS_FILES += platform/mcu/beken/linkinfo/bk7231_app.ld.S
else ifeq ($(BINS),kernel)
GLOBAL_LDS_FILES += platform/mcu/beken/linkinfo/bk7231_kernel.ld.S
endif

endif

$(NAME)_INCLUDES += aos
$(NAME)_SOURCES := aos/aos_main.c
$(NAME)_SOURCES += aos/soc_impl.c \
                   aos/trace_impl.c \
				   hal/mesh_wifi_hal.c

$(NAME)_PREBUILT_LIBRARY := libbeken.a

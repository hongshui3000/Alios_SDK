NAME := board_mk3060

JTAG := jlink

$(NAME)_TYPE := kernel
MODULE               := EMW3060
HOST_ARCH            := ARM968E-S
HOST_MCU_FAMILY      := beken

$(NAME)_SOURCES := board.c

GLOBAL_INCLUDES += .
GLOBAL_DEFINES += STDIO_UART=0

CURRENT_TIME = $(shell /bin/date +%Y%m%d.%H%M)
define get-os-version
YOS-R-$(CURRENT_TIME)
endef

CONFIG_SYSINFO_OS_VERSION := $(call get-os-version)

$(warning $(CONFIG_SYSINFO_OS_VERSION))

CONFIG_SYSINFO_PRODUCT_MODEL := ALI_YOS_MK3060
CONFIG_SYSINFO_DEVICE_NAME := MK3060
$(warning ${CONFIG_SYSINFO_OS_VERSION})

GLOBAL_CFLAGS += -DSYSINFO_OS_VERSION=\"$(CONFIG_SYSINFO_OS_VERSION)\"
GLOBAL_CFLAGS += -DSYSINFO_PRODUCT_MODEL=\"$(CONFIG_SYSINFO_PRODUCT_MODEL)\"
GLOBAL_CFLAGS += -DSYSINFO_DEVICE_NAME=\"$(CONFIG_SYSINFO_DEVICE_NAME)\"

GLOBAL_LDFLAGS  += -L $(SOURCE_ROOT)/board/mk3060

# Extra build target in mico_standard_targets.mk, include bootloader, and copy output file to eclipse debug file (copy_output_for_eclipse)
EXTRA_TARGET_MAKEFILES +=  $(MAKEFILES_PATH)/yos_standard_targets.mk
EXTRA_TARGET_MAKEFILES +=  $(SOURCE_ROOT)/platform/mcu/$(HOST_MCU_FAMILY)/gen_crc_bin.mk

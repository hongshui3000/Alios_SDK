NAME := board_b_l475e

JTAG := stlink-v2-1

$(NAME)_TYPE := kernel
MODULE               := 1062
HOST_ARCH            := Cortex-M4
HOST_MCU_FAMILY      := stm32l4xx

$(NAME)_SOURCES := board.c osa_flash.c

GLOBAL_INCLUDES += .
GLOBAL_DEFINES += STDIO_UART=0

CURRENT_TIME = $(shell /bin/date +%Y%m%d.%H%M)
CONFIG_SYSINFO_KERNEL_VERSION = YOS-R-1.0.1

ifeq (1,${BINS})
GLOBAL_CFLAGS += -DSYSINFO_OS_BINS
CONFIG_SYSINFO_APP_VERSION = APP-1.0.0-$(CURRENT_TIME)
define get-os-version
${CONFIG_SYSINFO_KERNEL_VERSION}_$(CONFIG_SYSINFO_APP_VERSION)
endef
else
define get-os-version
${CONFIG_SYSINFO_KERNEL_VERSION}-$(CURRENT_TIME)
endef
endif

CONFIG_SYSINFO_OS_VERSION := $(call get-os-version)

$(warning $(CONFIG_SYSINFO_OS_VERSION))

CONFIG_SYSINFO_PRODUCT_MODEL := ALI_YOS_B-L475E
CONFIG_SYSINFO_DEVICE_NAME := B-L475E


GLOBAL_CFLAGS += -DSYSINFO_OS_VERSION=\"$(CONFIG_SYSINFO_OS_VERSION)\"
GLOBAL_CFLAGS += -DSYSINFO_PRODUCT_MODEL=\"$(CONFIG_SYSINFO_PRODUCT_MODEL)\"
GLOBAL_CFLAGS += -DSYSINFO_DEVICE_NAME=\"$(CONFIG_SYSINFO_DEVICE_NAME)\"
GLOBAL_CFLAGS += -DSYSINFO_KERNEL_VERSION=\"$(CONFIG_SYSINFO_KERNEL_VERSION)\"
GLOBAL_CFLAGS += -DSYSINFO_APP_VERSION=\"$(CONFIG_SYSINFO_APP_VERSION)\"

GLOBAL_LDFLAGS  += -L $(SOURCE_ROOT)/board/b_l475e

# Global defines
# HSE_VALUE = STM32 crystal frequency = 26MHz (needed to make UART work correctly)
GLOBAL_DEFINES += $$(if $$(NO_CRLF_STDIO_REPLACEMENT),,CRLF_STDIO_REPLACEMENT)
GLOBAL_CFLAGS  += -DSTM32L475xx -DWITH_LWIP -mcpu=cortex-m4 -mthumb -mfloat-abi=soft

WIFI_FIRMWARE_SECTOR_START    := 2      #0x2000
FILESYSTEM_IMAGE_SECTOR_START := 256    #0x100000

# Extra build target in mico_standard_targets.mk, include bootloader, and copy output file to eclipse debug file (copy_output_for_eclipse)
EXTRA_TARGET_MAKEFILES +=  $(MAKEFILES_PATH)/yos_standard_targets.mk
#EXTRA_TARGET_MAKEFILES +=  $(SOURCE_ROOT)/platform/mcu/$(HOST_MCU_FAMILY)/gen_crc_bin.mk

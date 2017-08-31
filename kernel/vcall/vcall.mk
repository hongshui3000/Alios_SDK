NAME := vcall

$(NAME)_TYPE := kernel
GLOBAL_INCLUDES += ./mico/include

$(NAME)_CFLAGS += -Wall -Werror
ifeq ($(HOST_ARCH),ARM968E-S)
$(NAME)_CFLAGS += -marm
endif

ifneq ($(vcall),posix)
GLOBAL_DEFINES += VCALL_RHINO
$(NAME)_COMPONENTS += rhino

$(NAME)_SOURCES := \
    mico/mico_rhino.c

$(NAME)_SOURCES += \
    aos/yos_rhino.c
else
GLOBAL_DEFINES += VCALL_POSIX

$(NAME)_SOURCES += \
    aos/yos_posix.c
endif


NAME := sal
GLOBAL_DEFINES += WITH_SAL # for sal general use
$(NAME)_TYPE := kernel
$(NAME)_SOURCES := sal_sockets.c sal_err.c sal_arch.c
GLOBAL_INCLUDES += ./include

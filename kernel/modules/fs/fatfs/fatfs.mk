NAME := fatfs

$(NAME)_TYPE        := kernel

$(NAME)_SOURCES     := fatfs.c
$(NAME)_SOURCES     += ff/ff.c
$(NAME)_SOURCES     += ff/ffunicode.c
$(NAME)_CFLAGS      += -Wall -Werror

GLOBAL_INCLUDES     += include ff/include

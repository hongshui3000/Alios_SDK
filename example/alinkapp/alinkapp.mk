NAME := alinkapp

$(NAME)_SOURCES := alink_sample.c
$(NAME)_COMPONENTS := base64 hashtable log connectivity protocol.alink ywss modules.kv cli

ifneq (,$(filter linuxhost,$(COMPONENTS)))
gateway ?= 0
else
gateway ?= 1
endif

ifeq ($(gateway),1)

$(NAME)_COMPONENTS += gateway
ifneq (,$(filter linuxhost,$(COMPONENTS)))
DDA ?= 1
endif

ifneq (,$(filter armhflinux,$(COMPONENTS)))
DDA ?= 1
endif

endif

$(NAME)_INCLUDES := ../../framework/protocol/alink/system/ ../../framework/protocol/alink/json/
$(NAME)_INCLUDES += ../../kernel/modules/kv/include
$(NAME)_INCLUDES += ../../framework/gateway/
GLOBAL_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
GLOBAL_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized 
GLOBAL_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable 
GLOBAL_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing

ifeq ($(DDA),1)
GLOBAL_LDFLAGS += -lreadline -lncurses
$(NAME)_COMPONENTS  += dda
endif

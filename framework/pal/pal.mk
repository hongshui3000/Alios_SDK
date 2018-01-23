NAME := pal

$(NAME)_COMPONENTS :=  mbedtls alicrypto connectivity/websockets  cjson protocol.alink ywss netmgr

ifeq ($(HOST_ARCH), Cortex-M4F)
$(NAME)_PREBUILT_LIBRARY += lib/$(HOST_ARCH)/douglas.a

else ifeq ($(HOST_ARCH), linux)
PLATFORM := linuxhost
$(NAME)_PREBUILT_LIBRARY += lib/$(PLATFORM)/douglas.a

else
$(error "not find correct platform!")
endif


GLOBAL_INCLUDES     +=inc  util
#$(NAME)_INCLUDES  :=asr  ca    core

$(NAME)_DEFINES  +=PAL_DEBUG




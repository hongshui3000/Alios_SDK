NAME := wifimonitor

GLOBAL_INCLUDES += ./

GLOBAL_CFLAGS += -DCONFIG_WIFIMONITOR

$(NAME)_SOURCES += wifimonitor.c

$(NAME)_COMPONENTS += log cli

$(NAME)_DEFINES += DEBUG

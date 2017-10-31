NAME := netmgrapp

$(NAME)_SOURCES := netmgrapp.c

$(NAME)_COMPONENTS += netmgr yloop cli

GLOBAL_DEFINES += CONFIG_NO_TCPIP # temperaly used by esp32 <TODO>

ifneq (,${BINS})
GLOBAL_CFLAGS += -DSYSINFO_OS_BINS
endif

CURRENT_TIME = $(shell /bin/date +%Y%m%d.%H%M)
CONFIG_SYSINFO_APP_VERSION = APP-1.0.0-$(CURRENT_TIME)
$(info app_version:${CONFIG_SYSINFO_APP_VERSION})
GLOBAL_CFLAGS += -DSYSINFO_APP_VERSION=\"$(CONFIG_SYSINFO_APP_VERSION)\"

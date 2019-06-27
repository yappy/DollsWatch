#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := app-template
PROJECT_VER := $(shell git describe --always --tags --dirty)

CPPFLAGS += -D PROJECT_VER=\"$(PROJECT_VER)\"

include $(IDF_PATH)/make/project.mk


# ESP-IDF version check
IDF_VER_REQUIRED := v3.2.2
ifneq ($(IDF_VER), $(IDF_VER_REQUIRED))
$(error ESP-IDF version error: $(IDF_VER) != $(IDF_VER_REQUIRED))
endif

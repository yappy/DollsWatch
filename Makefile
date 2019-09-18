#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := app-template
PROJECT_VER := $(shell git describe --always --tags --dirty)
ARDUINO_VER := $(shell cd components/arduino-esp32 && git describe --always --tags --dirty)
M5STACK_VER := $(shell cd components/M5Stack && git describe --always --tags --dirty)

CPPFLAGS += -D PROJECT_VER=\"$(PROJECT_VER)\"
CPPFLAGS += -D ARDUINO_VER=\"$(ARDUINO_VER)\"
CPPFLAGS += -D M5STACK_VER=\"$(M5STACK_VER)\"

include $(IDF_PATH)/make/project.mk

$(info VER Project: $(PROJECT_VER))
$(info VER ESP-IDF: $(IDF_VER))
$(info VER arduino-esp32: $(ARDUINO_VER))
$(info VER M5Stack: $(M5STACK_VER))

# ESP-IDF version check
IDF_VER_REQUIRED := v3.3
ifneq ($(IDF_VER), $(IDF_VER_REQUIRED))
$(error ESP-IDF version error: $(IDF_VER) != $(IDF_VER_REQUIRED))
endif

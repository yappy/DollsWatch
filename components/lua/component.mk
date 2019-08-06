COMPONENT_SRCDIRS := src
COMPONENT_ADD_INCLUDEDIRS := src
# force C++ compile
CFLAGS := $(CPPFLAGS)
CFLAGS += -x c++ -mlongcalls

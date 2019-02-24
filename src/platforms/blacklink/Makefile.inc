SYS = $(shell $(CC) -dumpmachine)
CFLAGS += -DLIBFTDI
LDFLAGS += -lusb-1.0
ifneq (, $(findstring mingw, $(SYS)))
LDFLAGS += -lws2_32
CFLAGS += -Wno-cast-function-type
else ifneq (, $(findstring cygwin, $(SYS)))
LDFLAGS += -lws2_32
endif
SRC += 	timing.c stlinkv2.c

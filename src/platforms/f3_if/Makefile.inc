CROSS_COMPILE ?= arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS += -Istm32/include -mcpu=cortex-m4 -mthumb \
	-mfloat-abi=hard -mfpu=fpv4-sp-d16 \
	-DSTM32F3 -DF3_IF -I../libopencm3/include \
	-Iplatforms/stm32

LDFLAGS = -lopencm3_stm32f3 -Wl,--defsym,_stack=0x20008000 \
	-Wl,-T,platforms/stm32/stm32f303xc.ld -nostartfiles -lc -lnosys \
	-Wl,-Map=mapfile -mthumb -mcpu=cortex-m4 -Wl,-gc-sections \
	-mfloat-abi=hard -mfpu=fpv4-sp-d16 --specs=nano.specs \
	-L../libopencm3/lib

VPATH += platforms/stm32

SRC += 	cdcacm.c	\
	traceswo.c	\
	usbuart.c	\
	serialno.c	\
	timing.c	\
	timing_stm32.c	\

all:	blackmagic.bin

host_clean:
	-$(Q)$(RM) blackmagic.bin

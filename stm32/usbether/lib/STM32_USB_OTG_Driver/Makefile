BINPATH=/home/arthin/work/op/OpenPilot/tools/gcc-arm-none-eabi-4_7-2013q1/bin


CC=$(BINPATH)/arm-none-eabi-gcc
AR=$(BINPATH)/arm-none-eabi-ar

###################################################

# Check for valid float argument
ifneq ($(FLOAT_TYPE), hard)
ifneq ($(FLOAT_TYPE), soft)
override FLOAT_TYPE = hard
#override FLOAT_TYPE = soft
endif
endif

###################################################

vpath %.c src

CFLAGS  = -g -O2 -Wall
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -mcpu=cortex-m4

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif

CFLAGS += -ffreestanding -nostdlib

# Includes
CFLAGS += -I../../inc -Iinc
CFLAGS += -I../CMSIS/Device/ST/STM32F4xx/Include -I../CMSIS/Include

# Sources
SRCS = usb_core.c usb_dcd.c usb_dcd_int.c
#SRCS += usb_hcd.c usb_hcd_int.c
#SRCS += usb_otg.c

OBJS = $(SRCS:.c=.o)
LIBNAME = libusbcore.a

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)
	
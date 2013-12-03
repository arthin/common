# Sources

SRCS = main.c\
 usbd_cdc_r_core.c\
 usbd_desc.c\
 usbd_rndis.c\
 usbd_usr.c\
 usb_bsp.c

#arch
SRCS += stm32f4xx_it.c system_stm32f4xx.c
#FreeRTOS
SRCS += FreeRTOS/croutine.c\
 FreeRTOS/list.c\
 FreeRTOS/queue.c\
 FreeRTOS/tasks.c\
 FreeRTOS/timers.c\
 FreeRTOS/portable/GCC/ARM_CM4F/port.c\
 FreeRTOS/portable/MemMang/heap_2.c

# Project name

PROJ_NAME=usbe
OUTPATH=build

###################################################

# Check for valid float argument
# NOTE that you have to run make clan after
# changing these as hardfloat and softfloat are not
# binary compatible
ifneq ($(FLOAT_TYPE), hard)
ifneq ($(FLOAT_TYPE), soft)
override FLOAT_TYPE = hard
#override FLOAT_TYPE = soft
endif
endif

###################################################

BINPATH=/home/arthin/work/op/OpenPilot/tools/gcc-arm-none-eabi-4_7-2013q1/bin
CC=$(BINPATH)/arm-none-eabi-gcc
OBJCOPY=$(BINPATH)/arm-none-eabi-objcopy
SIZE=$(BINPATH)/arm-none-eabi-size

CFLAGS  = -std=gnu99 -g -O0 -Wall -Tstm32_flash.ld
CFLAGS += -mlittle-endian -mthumb -mthumb-interwork -nostartfiles -mcpu=cortex-m4

ifeq ($(FLOAT_TYPE), hard)
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard
else
CFLAGS += -msoft-float
endif

###################################################

vpath %.c src
vpath %.a lib

ROOT=$(shell pwd)

# Includes
CFLAGS += -Isrc -Ilib/CMSIS/Device/ST/STM32F4xx/Include -Ilib/CMSIS/Include
CFLAGS += -Iinc

# Library paths
LIBPATHS = -Llib/STM32F4xx_StdPeriph_Driver -Llib/STM32_USB_Device_Library/Core
LIBPATHS += -Llib/STM32_USB_Device_Library/Class/cdc -Llib/STM32_USB_OTG_Driver

# Libraries to link
LIBS = -lm -lstdperiph -lusbdevcore -lusbcore  -lusbcore
#LIBS += -lusbdevcdc

# STM32 library includes
CFLAGS += -Ilib/STM32F4xx_StdPeriph_Driver/inc
CFLAGS += -Ilib/STM32_USB_OTG_Driver/inc
CFLAGS += -Ilib/STM32_USB_Device_Library/Core/inc
CFLAGS += -Ilib/STM32_USB_Device_Library/Class/cdc/inc

# FreeRTOS inclides
CFLAGS += -IFreeRTOS/include
CFLAGS += -IFreeRTOS/portable/GCC/ARM_CM4F

# add startup file to build
SRCS += lib/startup_stm32f4xx.s

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj

all: lib proj
	$(SIZE) $(OUTPATH)/$(PROJ_NAME).elf

lib:
	$(MAKE) -C lib FLOAT_TYPE=$(FLOAT_TYPE)

proj: 	$(OUTPATH)/$(PROJ_NAME).elf

$(OUTPATH)/$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBPATHS) $(LIBS)
	$(OBJCOPY) -O ihex $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(OUTPATH)/$(PROJ_NAME).elf $(OUTPATH)/$(PROJ_NAME).bin

clean:
	rm -f *.o
	rm -f $(OUTPATH)/$(PROJ_NAME).elf
	rm -f $(OUTPATH)/$(PROJ_NAME).hex
	rm -f $(OUTPATH)/$(PROJ_NAME).bin
	$(MAKE) clean -C lib # Remove this line if you don't want to clean the libs as well

# Build all libraries

# Check for valid float argument
ifneq ($(FLOAT_TYPE), hard)
ifneq ($(FLOAT_TYPE), soft)
override FLOAT_TYPE = hard
#override FLOAT_TYPE = soft
endif
endif

all:
	$(MAKE) -C STM32F4xx_StdPeriph_Driver FLOAT_TYPE=$(FLOAT_TYPE)
	$(MAKE) -C STM32_USB_OTG_Driver FLOAT_TYPE=$(FLOAT_TYPE)
	$(MAKE) -C STM32_USB_Device_Library/Core FLOAT_TYPE=$(FLOAT_TYPE)
	$(MAKE) -C STM32_USB_Device_Library/Class/cdc FLOAT_TYPE=$(FLOAT_TYPE)
#	$(MAKE) -C USB_Host/Core FLOAT_TYPE=$(FLOAT_TYPE)
#	$(MAKE) -C USB_Host/Class/MSC FLOAT_TYPE=$(FLOAT_TYPE)
#	$(MAKE) -C fat_fs FLOAT_TYPE=$(FLOAT_TYPE)

clean:
	$(MAKE) clean -C STM32F4xx_StdPeriph_Driver
	$(MAKE) clean -C STM32_USB_OTG_Driver
	$(MAKE) clean -C STM32_USB_Device_Library/Core
	$(MAKE) clean -C STM32_USB_Device_Library/Class/cdc
#	$(MAKE) clean -C USB_Host/Core
#	$(MAKE) clean -C USB_Host/Class/MSC
#	$(MAKE) clean -C fat_fs

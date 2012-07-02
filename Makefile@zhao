	
# Makefile for the USB Mass Storage device drivers.	
# 05 MAY 2012
# My new USB driver Makefile



/***********************************************************/
EXTRA_CFLAGS := -Idrivers/scsi

ifneq ((CONFIG_USB_STORAGE) += myUSBDriver.o

 myUSBDriver-obj-$(CONFIG_USB_STORAGE_DEBUG) += debug.o
 myUSBDriver-obj-$(CONFIG_USB_STORAGE_USBAT) += shuttle_usbat.o
 myUSBDriver-obj-$(CONFIG_USB_STORAGE_SDDR09) += sddr09.o
 myUSBDriver-obj-$(CONFIG_USB_STORAGE_SDDR55) += sddr55.o
 myUSBDriver-obj-$(CONFIG_USB_STORAGE_FREECOM) += freecom.o
 myUSBDriver-obj-$(CONFIG_USB_STORAGE_DPCM) += dpcm.o
 myUSBDriver-obj-$(CONFIG_USB_STORAGE_ISD200) += isd200.o
 myUSBDriver-obj-$(CONFIG_USB_STORAGE_DATAFAB) += datafab.o
 myUSBDriver-obj-$(CONFIG_USB_STORAGE_JUMPSHOT) += jumpshot.o
 myUSBDriver-obj-$(CONFIG_USB_STORAGE_ALAUDA) += alauda.o
 myUSBDriver-obj-$(CONFIG_USB_STORAGE_ONETOUCH) += onetouch.o

 myUSBDriver-objs := scsiglue.o protocol.o transport.o usb.o \
 initializers.o (shell pwd)
KVER ?= (KVER)/build
all:
(KDIR) M=(CONFIG_USB_LIBUSUAL),)
obj-$(CONFIG_USB) += libusual.o
endif
clean:
rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions
/***********************************************************/

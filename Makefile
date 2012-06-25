



obj-m := abcc.o
 KERNELDIR := /lib/modules/3.2.16/build
 PWD := $(shell pwd)

modules:
	 $(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install 

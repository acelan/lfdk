CONFIG_LFDD=m

obj-$(CONFIG_LFDD) := lfdd_drv.o
lfdd_drv-objs := libpci_x86.o libmem.o libio.o lfdd.o
EXTRA_CFLAGS += -DLFDD_DEBUG

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.c .*.cmd .*.o.d .tmp_versions Module.symvers modules.order


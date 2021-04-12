ifneq ($(KERNELRELEASE),)
	obj-m := ps2.o
else
KERNEL_SOURCE :=/opt/work/fs5050/linux
	PWD := $(shell pwd)
default:
	make -C $(KERNEL_SOURCE) M=$(PWD) modules
endif
clean:
	rm -f *.o *.ko *.mod.*

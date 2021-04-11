obj-m:=key_pad.o
PWD:=$(shell pwd)
KDIR:=/home/swann/IMX_283A/linux-2.6.35.3
all:
	$(MAKE) -C $(KDIR) M=$(PWD) 
clean:
	rm -rf *.ko *.order *.symvers *.cmd *.o *.mod.c *.tmp_versions .*.cmd .tmp_versions

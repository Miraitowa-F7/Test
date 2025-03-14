ifneq ($(KERNELRELEASE),)
    obj-m := simple_char_driver2.o
else
    KERNELDIR ?= /home/ubuntu64-f7/Kernel/linux-6.6.68 # 内核源码路径
    PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- modules

clean:
	rm -rf *.o *.ko *.mod.c .tmp_versions *.symvers *.order
endif

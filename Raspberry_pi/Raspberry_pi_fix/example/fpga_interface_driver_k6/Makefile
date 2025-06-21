#Makefile for a basic kernel module
#export PATH=$PATH:~/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin

obj-m   := fpga_interface_driver.o

# KDIR :=/work/achro-em/kernel/
# KDIR :=~/linux
KDIR := /lib/modules/$(shell uname -r)/build
PWD :=$(shell pwd)

all: driver

driver:
#	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	$(MAKE) -C $(KDIR) M=$(PWD) modules

install_nfs:
	cp -a fpga_interface_driver.ko /nfsroot

install_scp:
	scp fpga_interface_driver.ko pi@127.0.0.1:/home/pi/Modules

clean:
#	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf *.o
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf .interface*
	rm -rf .tmp*

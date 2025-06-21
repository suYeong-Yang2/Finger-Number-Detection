#
# FPGA LED 드라이버와 사용자 앱을 빌드하는 Makefile
# (외부 fpga_interface_driver 모듈에 의존)
#

# 빌드할 커널 모듈 목록
obj-m := fpga_led_driver.o

# fpga_interface_driver가 컴파일된 디렉토리의 절대 경로
INTERFACE_DRIVER_PATH := /home/kjh/example/fpga_interface_driver_k6
export KBUILD_EXTRA_SYMBOLS := $(INTERFACE_DRIVER_PATH)/Module.symvers

# 커널 소스(헤더) 디렉토리 경로
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: modules app

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# 'fpga_test_led.c' 파일을 컴파일하여 'fpga_test_led' 실행 파일 생성
app:
	gcc -o fpga_test_led fpga_test_led.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.ko *.o.* *.mod.c *.order *.symvers fpga_test_led


#
# FPGA FND 드라이버를 외부 의존성 모듈과 연결하여 빌드하는 Makefile
#

# 여기서는 fpga_fnd_driver 모듈만 빌드하도록 지정합니다.
obj-m := fpga_fnd_driver.o

# fpga_interface_driver가 이미 컴파일된 디렉토리의 절대 경로를 지정합니다.
# 이 경로에 Module.symvers 파일이 있어야 합니다.
INTERFACE_DRIVER_PATH := /home/kjh/example/fpga_interface_driver_k6
export KBUILD_EXTRA_SYMBOLS := $(INTERFACE_DRIVER_PATH)/Module.symvers

# 커널 소스(헤더) 디렉토리 경로입니다.
KDIR := /lib/modules/$(shell uname -r)/build

# 현재 디렉토리 경로입니다.
PWD := $(shell pwd)

# 'make' 또는 'make all' 실행 시 기본적으로 수행될 작업들입니다.
all: modules app

# 커널 모듈을 빌드하는 규칙입니다.
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# 사용자 애플리케이션을 빌드하는 규칙입니다.
# 파일 이름은 'fpga_test_fnd.c'로 가정합니다.
app:
	gcc -o fpga_test_fnd fpga_test_fnd.c

# 'make clean' 실행 시 컴파일된 모든 결과물을 정리합니다.
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.ko *.o.* *.mod.c *.order *.symvers fpga_test_fnd


#
# FPGA Buzzer 드라이버를 외부 의존성 모듈과 연결하여 빌드하는 Makefile
#

# 여기서는 fpga_buzzer_driver 모듈만 빌드하도록 지정합니다.
obj-m := fpga_buzzer_driver.o

# 현재 디렉토리 경로를 저장합니다.
PWD := $(shell pwd)

# 'iom_fpga_itf_write'와 같은 외부 함수(심볼)를 찾기 위해,
# fpga_interface_driver가 이미 컴파일된 경로를 절대 경로로 지정합니다.
# PWD 변수를 사용하여 경로를 명확하게 만듭니다.
INTERFACE_DRIVER_PATH := /home/kjh/example/fpga_interface_driver_k6
export KBUILD_EXTRA_SYMBOLS := $(INTERFACE_DRIVER_PATH)/Module.symvers

# 커널 소스(헤더) 디렉토리 경로입니다.
KDIR := /lib/modules/$(shell uname -r)/build

# 'make' 또는 'make all' 실행 시 기본적으로 수행될 작업들입니다.
all: modules app

# 커널 모듈을 빌드하는 규칙입니다.
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# 사용자 애플리케이션을 빌드하는 규칙입니다.
# 파일 이름은 'fpga_test_buzzer.c'로 가정합니다.
app:
	gcc -o fpga_test_buzzer fpga_test_buzzer.c

# 'make install_nfs' 실행 시 /nfsroot 디렉토리로 파일을 복사합니다.
install_nfs:
	cp -a fpga_buzzer_driver.ko /nfsroot
	cp -a fpga_test_buzzer /nfsroot

# 'make install_scp' 실행 시 scp를 통해 파일을 복사합니다.
install_scp:
	scp fpga_buzzer_driver.ko fpga_test_buzzer pi@127.0.0.1:/home/pi/Modules

# 'make clean' 실행 시 컴파일된 모든 결과물을 정리합니다.
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.ko *.o.* *.mod.c *.order *.symvers fpga_test_buzzer


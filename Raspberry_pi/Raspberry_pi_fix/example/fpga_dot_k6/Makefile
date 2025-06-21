# 두 개 이상의 커널 모듈과 사용자 애플리케이션을 함께 빌드하기 위한 Makefile
#

# 빌드할 커널 모듈 목록입니다.
# fpga_dot_driver.o는 fpga_interface_driver.o에 의존하므로 함께 빌드합니다.
obj-m := fpga_interface_driver.o fpga_dot_driver.o

# 커널 소스(헤더) 디렉토리 경로입니다.
# 라즈베리파이에서 직접 컴파일하므로 `uname -r`을 사용합니다.
KDIR := /lib/modules/$(shell uname -r)/build

# 현재 디렉토리 경로입니다.
PWD := $(shell pwd)

# 'make' 또는 'make all' 실행 시 기본적으로 수행될 작업들입니다.
all: modules app

# 커널 모듈을 빌드하는 규칙입니다.
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# 사용자 애플리케이션을 빌드하는 규칙입니다.
# 라즈베리파이에서 직접 컴파일하므로 'gcc'를 사용합니다.
app:
	gcc -o fpga_test_dot fpga_test_dot.c

# 'make install_nfs' 실행 시 /nfsroot 디렉토리로 파일을 복사합니다.
install_nfs:
	cp -a fpga_interface_driver.ko fpga_dot_driver.ko /nfsroot
	cp -a fpga_test_dot /nfsroot

# 'make install_scp' 실행 시 scp를 통해 파일을 복사합니다.
install_scp:
	scp fpga_interface_driver.ko fpga_dot_driver.ko pi@127.0.0.1:/home/pi/Modules
	scp fpga_test_dot pi@127.0.0.1:/home/pi/Modules

# 'make clean' 실행 시 컴파일된 모든 결과물을 정리합니다.
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f *.ko *.o.* *.mod.c *.order *.symvers fpga_test_dot


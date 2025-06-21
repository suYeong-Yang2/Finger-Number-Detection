/*
 * FPGA Dot Matrix Driver for Linux Kernel 6.x
 *
 * This driver creates a character device (/dev/fpga_dot) to control the
 * dot matrix display. It relies on the 'fpga_interface_driver' for the
 * actual low-level hardware access.
 *
 * Updated for modern kernel conventions and stability.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // For copy_from_user

// The font data is expected to be in the same directory.
// 이 파일은 컴파일 시 같은 디렉토리에 있어야 합니다.
#include "fpga_dot_font.h"

#define IOM_FPGA_DOT_MAJOR 262
#define IOM_FPGA_DOT_NAME "fpga_dot"

#define IOM_FPGA_DOT_ADDRESS 0x210 // Dot Matrix의 물리 주소

/*
 * 이 함수들은 'fpga_interface_driver.ko' 모듈에 의해 제공됩니다.
 * 'extern'은 컴파일러에게 이 함수들이 다른 곳에 정의되어 있음을 알리고,
 * 모듈이 로드될 때 커널이 이들을 연결해 줍니다.
 * 중요: 이 드라이버를 insmod 하기 전에 반드시 fpga_interface_driver.ko를 먼저 로드해야 합니다.
 */
extern ssize_t iom_fpga_itf_write(unsigned int addr, unsigned char value);

// 여러 프로그램이 동시에 접근하는 것을 막기 위한 전역 변수
static int fpga_dot_port_usage = 0;

/* 함수 프로토타입 선언 */
static int iom_fpga_dot_open(struct inode *inode, struct file *file);
static int iom_fpga_dot_release(struct inode *inode, struct file *file);
static ssize_t iom_fpga_dot_write(struct file *file, const char __user *buf, size_t len, loff_t *off);

/* 파일 오퍼레이션 구조체 (최신 스타일로 정의) */
static const struct file_operations iom_fpga_dot_fops = {
    .owner   = THIS_MODULE,
    .open    = iom_fpga_dot_open,
    .write   = iom_fpga_dot_write,
    .release = iom_fpga_dot_release,
};

// dev/fpga_dot 장치 파일을 열 때 호출되는 함수
static int iom_fpga_dot_open(struct inode *inode, struct file *file)
{
    if (fpga_dot_port_usage != 0)
        return -EBUSY; // 이미 사용 중이면 오류 반환

    fpga_dot_port_usage = 1;
    return 0;
}

// dev/fpga_dot 장치 파일을 닫을 때 호출되는 함수
static int iom_fpga_dot_release(struct inode *inode, struct file *file)
{
    fpga_dot_port_usage = 0;
    return 0;
}

// dev/fpga_dot 장치 파일에 write()를 할 때 호출되는 함수
static ssize_t iom_fpga_dot_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    int i;
    unsigned char value[10];
    size_t length_to_copy = len > sizeof(value) ? sizeof(value) : len;

    if (copy_from_user(value, buf, length_to_copy)) {
        return -EFAULT;
    }

    // 사용자로부터 받은 데이터로 Dot Matrix의 각 라인을 제어
    for (i = 0; i < length_to_copy; i++) {
        iom_fpga_itf_write((unsigned int)IOM_FPGA_DOT_ADDRESS + i, value[i] & 0x7F);
    }

    return length_to_copy;
}

// 모듈이 커널에 로드될 때 호출되는 초기화 함수
static int __init iom_fpga_dot_init(void)
{
    int result;

    result = register_chrdev(IOM_FPGA_DOT_MAJOR, IOM_FPGA_DOT_NAME, &iom_fpga_dot_fops);
    if (result < 0) {
        pr_warn("Can't get major number %d for device %s\n", IOM_FPGA_DOT_MAJOR, IOM_FPGA_DOT_NAME);
        return result;
    }

    pr_info("init module, %s major number : %d\n", IOM_FPGA_DOT_NAME, IOM_FPGA_DOT_MAJOR);
    return 0;
}

// 모듈이 커널에서 제거될 때 호출되는 종료 함수
static void __exit iom_fpga_dot_exit(void)
{
    unregister_chrdev(IOM_FPGA_DOT_MAJOR, IOM_FPGA_DOT_NAME);
    pr_info("exit module, %s\n", IOM_FPGA_DOT_NAME);
}

module_init(iom_fpga_dot_init);
module_exit(iom_fpga_dot_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("FPGA Dot Matrix device driver");


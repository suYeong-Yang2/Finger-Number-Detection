/*
 * FPGA LED Driver for Linux Kernel 6.x
 *
 * This driver creates a character device (/dev/fpga_led) to control the
 * 8 LEDs on the FPGA board. It relies on 'fpga_interface_driver' for
 * the actual low-level hardware access.
 *
 * Updated for modern kernel conventions and stability.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // For copy_from_user/copy_to_user

#define IOM_LED_MAJOR 260
#define IOM_LED_NAME "fpga_led"

#define IOM_LED_ADDRESS 0x016 // LED의 물리 주소

/*
 * 이 함수들은 'fpga_interface_driver.ko' 모듈에 의해 제공됩니다.
 * 중요: 이 드라이버를 insmod 하기 전에 반드시 fpga_interface_driver.ko를 먼저 로드해야 합니다.
 */
extern unsigned char iom_fpga_itf_read(unsigned int addr);
extern ssize_t iom_fpga_itf_write(unsigned int addr, unsigned char value);

// 여러 프로그램이 동시에 접근하는 것을 막기 위한 전역 변수
static int ledport_usage = 0;

/* 함수 프로토타입 선언 */
static int iom_led_open(struct inode *inode, struct file *file);
static int iom_led_release(struct inode *inode, struct file *file);
static ssize_t iom_led_write(struct file *file, const char __user *buf, size_t len, loff_t *off);
static ssize_t iom_led_read(struct file *file, char __user *buf, size_t len, loff_t *off);

/* 파일 오퍼레이션 구조체 (최신 스타일로 정의) */
static const struct file_operations iom_led_fops = {
    .owner   = THIS_MODULE,
    .open    = iom_led_open,
    .write   = iom_led_write,
    .read    = iom_led_read,
    .release = iom_led_release,
};

// /dev/fpga_led 장치 파일을 열 때 호출되는 함수
static int iom_led_open(struct inode *inode, struct file *file)
{
    if (ledport_usage != 0)
        return -EBUSY;

    ledport_usage = 1;
    return 0;
}

// /dev/fpga_led 장치 파일을 닫을 때 호출되는 함수
static int iom_led_release(struct inode *inode, struct file *file)
{
    ledport_usage = 0;
    return 0;
}

// /dev/fpga_led 장치 파일에 write()를 할 때 호출되는 함수
static ssize_t iom_led_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    unsigned char value;

    if (copy_from_user(&value, buf, 1)) {
        return -EFAULT;
    }

    iom_fpga_itf_write((unsigned int)IOM_LED_ADDRESS, value);
    return 1;
}

// /dev/fpga_led 장치 파일에서 read()를 할 때 호출되는 함수
static ssize_t iom_led_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    unsigned char value;

    value = iom_fpga_itf_read((unsigned int)IOM_LED_ADDRESS);

    if (copy_to_user(buf, &value, 1)) {
        return -EFAULT;
    }

    return 1;
}

// 모듈 초기화 함수
static int __init iom_led_init(void)
{
    int result = register_chrdev(IOM_LED_MAJOR, IOM_LED_NAME, &iom_led_fops);
    if (result < 0) {
        pr_warn("Can't get major number %d for device %s\n", IOM_LED_MAJOR, IOM_LED_NAME);
        return result;
    }
    pr_info("init module, %s major number: %d\n", IOM_LED_NAME, IOM_LED_MAJOR);
    return 0;
}

// 모듈 종료 함수
static void __exit iom_led_exit(void)
{
    unregister_chrdev(IOM_LED_MAJOR, IOM_LED_NAME);
    pr_info("exit module, %s\n", IOM_LED_NAME);
}

module_init(iom_led_init);
module_exit(iom_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("FPGA LED device driver");


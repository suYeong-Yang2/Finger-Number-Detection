/*
 * FPGA Buzzer Driver for Linux Kernel 6.x
 *
 * This driver creates a character device (/dev/fpga_buzzer) to control the
 * buzzer. It relies on the 'fpga_interface_driver' for the actual low-level
 * hardware access.
 *
 * Updated for modern kernel conventions and stability.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // For copy_from_user/copy_to_user

#define IOM_BUZZER_MAJOR 264
#define IOM_BUZZER_NAME "fpga_buzzer"

#define IOM_BUZZER_ADDRESS 0x070 // Buzzer의 물리 주소

/*
 * 이 함수들은 'fpga_interface_driver.ko' 모듈에 의해 제공됩니다.
 * 'extern'은 컴파일러에게 이 함수들이 다른 곳에 정의되어 있음을 알리고,
 * 모듈이 로드될 때 커널이 이들을 연결해 줍니다.
 * 중요: 이 드라이버를 insmod 하기 전에 반드시 fpga_interface_driver.ko를 먼저 로드해야 합니다.
 */
extern unsigned char iom_fpga_itf_read(unsigned int addr);
extern ssize_t iom_fpga_itf_write(unsigned int addr, unsigned char value);

// 여러 프로그램이 동시에 접근하는 것을 막기 위한 전역 변수
static int buzzer_port_usage = 0;

/* 함수 프로토타입 선언 */
static int iom_buzzer_open(struct inode *inode, struct file *file);
static int iom_buzzer_release(struct inode *inode, struct file *file);
static ssize_t iom_buzzer_write(struct file *file, const char __user *buf, size_t len, loff_t *off);
static ssize_t iom_buzzer_read(struct file *file, char __user *buf, size_t len, loff_t *off);

/* 파일 오퍼레이션 구조체 (최신 스타일로 정의) */
static const struct file_operations iom_buzzer_fops = {
    .owner   = THIS_MODULE,
    .open    = iom_buzzer_open,
    .write   = iom_buzzer_write,
    .read    = iom_buzzer_read,
    .release = iom_buzzer_release,
};

// /dev/fpga_buzzer 장치 파일을 열 때 호출되는 함수
static int iom_buzzer_open(struct inode *inode, struct file *file)
{
    if (buzzer_port_usage != 0)
        return -EBUSY; // 이미 사용 중이면 오류 반환

    buzzer_port_usage = 1;
    return 0;
}

// /dev/fpga_buzzer 장치 파일을 닫을 때 호출되는 함수
static int iom_buzzer_release(struct inode *inode, struct file *file)
{
    buzzer_port_usage = 0;
    return 0;
}

// /dev/fpga_buzzer 장치 파일에 write()를 할 때 호출되는 함수
static ssize_t iom_buzzer_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    unsigned char value;

    if (copy_from_user(&value, buf, 1)) {
        return -EFAULT;
    }

    iom_fpga_itf_write((unsigned int)IOM_BUZZER_ADDRESS, value);
    return 1;
}

// /dev/fpga_buzzer 장치 파일에서 read()를 할 때 호출되는 함수
static ssize_t iom_buzzer_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    unsigned char value;

    value = iom_fpga_itf_read((unsigned int)IOM_BUZZER_ADDRESS);

    if (copy_to_user(buf, &value, 1)) {
        return -EFAULT;
    }

    return 1;
}

// 모듈이 커널에 로드될 때 호출되는 초기화 함수
static int __init iom_buzzer_init(void)
{
    int result;

    result = register_chrdev(IOM_BUZZER_MAJOR, IOM_BUZZER_NAME, &iom_buzzer_fops);
    if (result < 0) {
        pr_warn("Can't get major number %d for device %s\n", IOM_BUZZER_MAJOR, IOM_BUZZER_NAME);
        return result;
    }

    pr_info("init module, %s major number : %d\n", IOM_BUZZER_NAME, IOM_BUZZER_MAJOR);
    return 0;
}

// 모듈이 커널에서 제거될 때 호출되는 종료 함수
static void __exit iom_buzzer_exit(void)
{
    unregister_chrdev(IOM_BUZZER_MAJOR, IOM_BUZZER_NAME);
    pr_info("exit module, %s\n", IOM_BUZZER_NAME);
}

module_init(iom_buzzer_init);
module_exit(iom_buzzer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("FPGA Buzzer device driver");


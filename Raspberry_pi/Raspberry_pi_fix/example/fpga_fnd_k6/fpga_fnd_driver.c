/*
 * FPGA FND (7-Segment) Driver for Linux Kernel 6.x
 *
 * This driver creates a character device (/dev/fpga_fnd) to control the
 * 4-digit 7-segment display. It relies on the 'fpga_interface_driver' for
 * the actual low-level hardware access.
 *
 * Updated for modern kernel conventions and stability.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // For copy_from_user/copy_to_user

#define IOM_FND_MAJOR 261
#define IOM_FND_NAME "fpga_fnd"

// FND의 물리 주소 (두 개의 레지스터 사용)
#define IOM_FND1_ADDRESS 0x003
#define IOM_FND2_ADDRESS 0x004

/*
 * 이 함수들은 'fpga_interface_driver.ko' 모듈에 의해 제공됩니다.
 * 중요: 이 드라이버를 insmod 하기 전에 반드시 fpga_interface_driver.ko를 먼저 로드해야 합니다.
 */
extern unsigned char iom_fpga_itf_read(unsigned int addr);
extern ssize_t iom_fpga_itf_write(unsigned int addr, unsigned char value);

// 여러 프로그램이 동시에 접근하는 것을 막기 위한 전역 변수
static int fpga_fnd_port_usage = 0;

/* 함수 프로토타입 선언 */
static int iom_fnd_open(struct inode *inode, struct file *file);
static int iom_fnd_release(struct inode *inode, struct file *file);
static ssize_t iom_fnd_write(struct file *file, const char __user *buf, size_t len, loff_t *off);
static ssize_t iom_fnd_read(struct file *file, char __user *buf, size_t len, loff_t *off);

/* 파일 오퍼레이션 구조체 (최신 스타일로 정의) */
static const struct file_operations iom_fpga_fnd_fops = {
    .owner   = THIS_MODULE,
    .open    = iom_fnd_open,
    .write   = iom_fnd_write,
    .read    = iom_fnd_read,
    .release = iom_fnd_release,
};

// /dev/fpga_fnd 장치 파일을 열 때 호출되는 함수
static int iom_fnd_open(struct inode *inode, struct file *file)
{
    if (fpga_fnd_port_usage != 0)
        return -EBUSY; // 이미 사용 중이면 오류 반환

    fpga_fnd_port_usage = 1;
    return 0;
}

// /dev/fpga_fnd 장치 파일을 닫을 때 호출되는 함수
static int iom_fnd_release(struct inode *inode, struct file *file)
{
    fpga_fnd_port_usage = 0;
    return 0;
}

// /dev/fpga_fnd 장치 파일에 write()를 할 때 호출되는 함수
static ssize_t iom_fnd_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    unsigned char value[4];

    // 사용자 공간에서 4바이트 데이터를 복사해옵니다.
    if (copy_from_user(value, buf, 4)) {
        return -EFAULT;
    }

    // 4바이트 데이터를 2개의 8비트 레지스터 값으로 조합하여 씁니다.
    iom_fpga_itf_write((unsigned int)IOM_FND1_ADDRESS, (value[0] & 0x0F) << 4 | (value[1] & 0x0F));
    iom_fpga_itf_write((unsigned int)IOM_FND2_ADDRESS, (value[2] & 0x0F) << 4 | (value[3] & 0x0F));

    return len;
}

// /dev/fpga_fnd 장치 파일에서 read()를 할 때 호출되는 함수
static ssize_t iom_fnd_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    unsigned char data1, data2;
    unsigned char value[4];

    data1 = iom_fpga_itf_read((unsigned int)IOM_FND1_ADDRESS);
    data2 = iom_fpga_itf_read((unsigned int)IOM_FND2_ADDRESS);

    value[0] = (data1 >> 4) & 0x0F;
    value[1] = data1 & 0x0F;
    value[2] = (data2 >> 4) & 0x0F;
    value[3] = data2 & 0x0F;

    if (copy_to_user(buf, value, 4)) {
        return -EFAULT;
    }

    return 4;
}

// 모듈이 커널에 로드될 때 호출되는 초기화 함수
static int __init iom_fnd_init(void)
{
    int result;

    result = register_chrdev(IOM_FND_MAJOR, IOM_FND_NAME, &iom_fpga_fnd_fops);
    if (result < 0) {
        pr_warn("Can't get major number %d for device %s\n", IOM_FND_MAJOR, IOM_FND_NAME);
        return result;
    }

    pr_info("init module, %s major number : %d\n", IOM_FND_NAME, IOM_FND_MAJOR);
    return 0;
}

// 모듈이 커널에서 제거될 때 호출되는 종료 함수
static void __exit iom_fnd_exit(void)
{
    unregister_chrdev(IOM_FND_MAJOR, IOM_FND_NAME);
    pr_info("exit module, %s\n", IOM_FND_NAME);
}

module_init(iom_fnd_init);
module_exit(iom_fnd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("FPGA FND (7-Segment) device driver");


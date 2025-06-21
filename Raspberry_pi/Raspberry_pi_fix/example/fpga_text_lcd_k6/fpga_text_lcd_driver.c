/*
 * FPGA Text LCD Driver for Linux Kernel 6.x
 *
 * This driver creates a character device (/dev/fpga_text_lcd) to control the
 * 2x16 Text LCD. It relies on 'fpga_interface_driver' for low-level access.
 *
 * Updated for modern kernel conventions and stability.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // For copy_from_user

#define IOM_FPGA_TEXT_LCD_MAJOR 263
#define IOM_FPGA_TEXT_LCD_NAME "fpga_text_lcd"

#define IOM_FPGA_TEXT_LCD_ADDRESS 0x090 // Text LCD의 물리 주소

/*
 * 이 함수는 'fpga_interface_driver.ko' 모듈에 의해 제공됩니다.
 */
extern ssize_t iom_fpga_itf_write(unsigned int addr, unsigned char value);

// 동시 접근 방지를 위한 전역 변수
static int fpga_text_lcd_port_usage = 0;

/* 함수 프로토타입 선언 */
static int iom_fpga_text_lcd_open(struct inode *inode, struct file *file);
static int iom_fpga_text_lcd_release(struct inode *inode, struct file *file);
static ssize_t iom_fpga_text_lcd_write(struct file *file, const char __user *buf, size_t len, loff_t *off);

/* 파일 오퍼레이션 구조체 */
static const struct file_operations iom_fpga_text_lcd_fops = {
    .owner   = THIS_MODULE,
    .open    = iom_fpga_text_lcd_open,
    .write   = iom_fpga_text_lcd_write,
    .release = iom_fpga_text_lcd_release,
};

// /dev/fpga_text_lcd 장치 파일을 열 때 호출
static int iom_fpga_text_lcd_open(struct inode *inode, struct file *file)
{
    if (fpga_text_lcd_port_usage != 0)
        return -EBUSY;
    fpga_text_lcd_port_usage = 1;
    return 0;
}

// /dev/fpga_text_lcd 장치 파일을 닫을 때 호출
static int iom_fpga_text_lcd_release(struct inode *inode, struct file *file)
{
    fpga_text_lcd_port_usage = 0;
    return 0;
}

// /dev/fpga_text_lcd 장치 파일에 write()를 할 때 호출
static ssize_t iom_fpga_text_lcd_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    int i;
    unsigned char value[33]; // 32 chars + null terminator
    size_t length_to_copy = len > (sizeof(value) - 1) ? (sizeof(value) - 1) : len;

    if (copy_from_user(value, buf, length_to_copy)) {
        return -EFAULT;
    }
    value[length_to_copy] = '\0'; // 문자열 끝을 명시

    pr_info("Writing to LCD: %s (size: %zu)\n", value, length_to_copy);

    for (i = 0; i < length_to_copy; i++) {
        iom_fpga_itf_write((unsigned int)IOM_FPGA_TEXT_LCD_ADDRESS + i, value[i]);
    }

    return length_to_copy;
}

// 모듈 초기화 함수
static int __init iom_fpga_text_lcd_init(void)
{
    int result = register_chrdev(IOM_FPGA_TEXT_LCD_MAJOR, IOM_FPGA_TEXT_LCD_NAME, &iom_fpga_text_lcd_fops);
    if (result < 0) {
        pr_warn("Can't get major number %d for device %s\n", IOM_FPGA_TEXT_LCD_MAJOR, IOM_FPGA_TEXT_LCD_NAME);
        return result;
    }
    pr_info("init module, %s major number: %d\n", IOM_FPGA_TEXT_LCD_NAME, IOM_FPGA_TEXT_LCD_MAJOR);
    return 0;
}

// 모듈 종료 함수
static void __exit iom_fpga_text_lcd_exit(void)
{
    unregister_chrdev(IOM_FPGA_TEXT_LCD_MAJOR, IOM_FPGA_TEXT_LCD_NAME);
    pr_info("exit module, %s\n", IOM_FPGA_TEXT_LCD_NAME);
}

module_init(iom_fpga_text_lcd_init);
module_exit(iom_fpga_text_lcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("FPGA Text LCD device driver");


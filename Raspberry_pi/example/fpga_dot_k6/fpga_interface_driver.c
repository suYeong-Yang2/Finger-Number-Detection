/*
 * FPGA Interface Driver for modern Linux Kernels (6.x+)
 *
 * v7 (Final - Direct I/O): Bypasses the gpiod subsystem entirely to resolve
 * persistent -EPROBE_DEFER (-517) errors on Raspberry Pi 4B.
 * This version directly maps and manipulates the GPIO hardware registers.
 *
 * Key Changes:
 * - Uses ioremap() to directly access GPIO controller physical memory.
 * - Replaces all gpiod_* and gpio_* function calls with low-level
 * register reads/writes (readl/writel).
 * - Manually configures pin direction (GPFSEL) and state (GPSET/GPCLR).
 * - This method is independent of the kernel's GPIO driver readiness and
 * bypasses any pin ownership conflicts.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/uaccess.h>

/* RPi 4B BCM2711 GPIO Base Address */
#define GPIO_BASE    0xfe200000
#define GPIO_SIZE    0xB4

/* GPIO Register Offsets */
#define GPFSEL0      0x00
#define GPSET0       0x1C
#define GPCLR0       0x28
#define GPLEV0       0x34

/* 제어 신호 인덱스 정의 */
#define CTRL_nWE    0
#define CTRL_nOE    1
#define CTRL_nCS    2

/* 함수 프로토타입 선언 */
static int __init iom_fpga_itf_init(void);
static void __exit iom_fpga_itf_exit(void);
ssize_t iom_fpga_itf_write(unsigned int addr, unsigned char value);
unsigned char iom_fpga_itf_read(unsigned int addr);

/* GPIO 핀 번호 정의 */
static const int address_gpios[] = { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };
static const int data_gpios[] = { 2, 3, 4, 5, 6, 7, 8, 9 };
static const int control_gpios[] = { 22, 23, 25 }; // nWE, nOE, nCS 순서

/* I/O Memory 포인터 */
static void __iomem *gpio_regs;

/* Low-level GPIO functions */
static void set_gpio_output(int pin) {
    u32 reg_index = pin / 10;
    u32 bit = (pin % 10) * 3;
    u32 old_val, new_val;

    old_val = readl(gpio_regs + GPFSEL0 + (reg_index * 4));
    new_val = old_val & ~(7 << bit); // Clear the 3 bits for the pin
    new_val |= (1 << bit);          // Set to 001 for output
    writel(new_val, gpio_regs + GPFSEL0 + (reg_index * 4));
}

static void set_gpio_input(int pin) {
    u32 reg_index = pin / 10;
    u32 bit = (pin % 10) * 3;
    u32 old_val, new_val;

    old_val = readl(gpio_regs + GPFSEL0 + (reg_index * 4));
    new_val = old_val & ~(7 << bit); // Set to 000 for input
    writel(new_val, gpio_regs + GPFSEL0 + (reg_index * 4));
}

static void set_gpio_value(int pin, int value) {
    u32 reg_index = pin / 32;
    u32 bit = pin % 32;
    if (value)
        writel(1 << bit, gpio_regs + GPSET0 + (reg_index * 4));
    else
        writel(1 << bit, gpio_regs + GPCLR0 + (reg_index * 4));
}

static int get_gpio_value(int pin) {
    u32 reg_index = pin / 32;
    u32 bit = pin % 32;
    return (readl(gpio_regs + GPLEV0 + (reg_index * 4)) >> bit) & 1;
}

ssize_t iom_fpga_itf_write(unsigned int addr, unsigned char value)
{
    int i;
    // A0는 하드웨어 풀다운에 의해 LOW로 간주, 주소 버스는 A1부터 시작
    unsigned int effective_addr = addr << 1;

    pr_info("FPGA WRITE: address = 0x%x, data = 0x%x \n", addr, value);

    for (i = 0; i < ARRAY_SIZE(address_gpios); i++) {
        set_gpio_value(address_gpios[i], (effective_addr >> (i + 1)) & 0x1);
    }
    for (i = 0; i < ARRAY_SIZE(data_gpios); i++) {
        set_gpio_value(data_gpios[i], (value >> i) & 0x1);
    }

    set_gpio_value(control_gpios[CTRL_nCS], 0); udelay(1);
    set_gpio_value(control_gpios[CTRL_nWE], 0); udelay(5);
    set_gpio_value(control_gpios[CTRL_nWE], 1);
    set_gpio_value(control_gpios[CTRL_nCS], 1);

    return 1;
}
EXPORT_SYMBOL(iom_fpga_itf_write);

unsigned char iom_fpga_itf_read(unsigned int addr)
{
    unsigned char value = 0;
    int i;
    unsigned int effective_addr = addr << 1;

    pr_info("FPGA READ: address = 0x%x\n", addr);
    
    for (i = 0; i < ARRAY_SIZE(address_gpios); i++) {
        set_gpio_value(address_gpios[i], (effective_addr >> (i + 1)) & 0x1);
    }

    for (i = 0; i < ARRAY_SIZE(data_gpios); i++) {
        set_gpio_input(data_gpios[i]);
    }

    set_gpio_value(control_gpios[CTRL_nCS], 0); udelay(1);
    set_gpio_value(control_gpios[CTRL_nOE], 0); udelay(1);

    for (i = 0; i < ARRAY_SIZE(data_gpios); i++) {
        value |= (get_gpio_value(data_gpios[i]) << i);
    }

    set_gpio_value(control_gpios[CTRL_nOE], 1);
    set_gpio_value(control_gpios[CTRL_nCS], 1);

    for (i = 0; i < ARRAY_SIZE(data_gpios); i++) {
        set_gpio_output(data_gpios[i]);
    }

    pr_info("FPGA READ value = 0x%x\n", value);
    return value;
}
EXPORT_SYMBOL(iom_fpga_itf_read);

static void __exit iom_fpga_itf_exit(void)
{
    pr_info("exit module: %s\n", __func__);
    if (gpio_regs) {
        iounmap(gpio_regs);
    }
}

static int __init iom_fpga_itf_init(void)
{
    int i;
    pr_info("init module: %s (Direct I/O Mode)\n", __func__);

    gpio_regs = ioremap(GPIO_BASE, GPIO_SIZE);
    if (!gpio_regs) {
        pr_err("Failed to map GPIO memory\n");
        return -ENOMEM;
    }

    // GPIO 10 (A0)는 제어하지 않음. 하드웨어 기본 상태(pull-down)에 의존.
    
    for (i = 0; i < ARRAY_SIZE(address_gpios); i++) {
        set_gpio_output(address_gpios[i]);
        set_gpio_value(address_gpios[i], 0);
    }
    for (i = 0; i < ARRAY_SIZE(data_gpios); i++) {
        set_gpio_output(data_gpios[i]);
        set_gpio_value(data_gpios[i], 0);
    }
    for (i = 0; i < ARRAY_SIZE(control_gpios); i++) {
        set_gpio_output(control_gpios[i]);
        set_gpio_value(control_gpios[i], 1);
    }

    pr_info("FPGA interface GPIOs configured directly.\n");
    return 0;
}

module_init(iom_fpga_itf_init);
module_exit(iom_fpga_itf_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("FPGA Interface Driver (v7 - Direct I/O Final)");


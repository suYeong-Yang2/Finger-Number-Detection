#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd75c6742, "__register_chrdev" },
	{ 0x122c3a7e, "_printk" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0xff9cc569, "iom_fpga_itf_write" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xa6620baa, "iom_fpga_itf_read" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x39ff040a, "module_layout" },
};

MODULE_INFO(depends, "fpga_interface_driver");


MODULE_INFO(srcversion, "914DE8A002DCDF52336F358");

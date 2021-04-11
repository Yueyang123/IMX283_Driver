#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x405fe887, "module_layout" },
	{ 0x98082893, "__copy_to_user" },
	{ 0x564a6735, "device_create" },
	{ 0x582a74ba, "__class_create" },
	{ 0xc81bb072, "cdev_add" },
	{ 0x8e74204, "cdev_init" },
	{ 0xbd68de57, "cdev_alloc" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x40a6f522, "__arm_ioremap" },
	{ 0xea147363, "printk" },
	{ 0x42b26667, "class_destroy" },
	{ 0x891e3a52, "device_del" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x73ff3ad4, "cdev_del" },
	{ 0x45a55ec8, "__iounmap" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


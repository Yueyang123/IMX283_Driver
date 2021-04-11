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
	{ 0xfa2a45e, "__memzero" },
	{ 0x17a142df, "__copy_from_user" },
	{ 0x3ec6c705, "kmalloc_caches" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x564a6735, "device_create" },
	{ 0x582a74ba, "__class_create" },
	{ 0xc81bb072, "cdev_add" },
	{ 0x8e74204, "cdev_init" },
	{ 0x6b1e3028, "kmem_cache_alloc" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xfe990052, "gpio_free" },
	{ 0x42b26667, "class_destroy" },
	{ 0xfc9f28f6, "device_unregister" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x37a0cba, "kfree" },
	{ 0x73ff3ad4, "cdev_del" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0xea147363, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


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
	{ 0x17a142df, "__copy_from_user" },
	{ 0x3ec6c705, "kmalloc_caches" },
	{ 0x75d5680c, "misc_register" },
	{ 0xfa2a45e, "__memzero" },
	{ 0x6b1e3028, "kmem_cache_alloc" },
	{ 0x98082893, "__copy_to_user" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0x65d6d0f0, "gpio_direction_input" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xfe990052, "gpio_free" },
	{ 0xea147363, "printk" },
	{ 0xbf42b7f, "misc_deregister" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


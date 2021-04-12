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
	{ 0x9a6221c5, "mod_timer" },
	{ 0x541ff6bd, "input_event" },
	{ 0x7d11c268, "jiffies" },
	{ 0x8cfc414e, "add_timer" },
	{ 0x74c86cc0, "init_timer_key" },
	{ 0xa003fced, "input_register_device" },
	{ 0x65d6d0f0, "gpio_direction_input" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x713054a0, "input_allocate_device" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0xea147363, "printk" },
	{ 0x9ff5efc3, "input_free_device" },
	{ 0xc7290f0f, "input_unregister_device" },
	{ 0xd8f795ca, "del_timer" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


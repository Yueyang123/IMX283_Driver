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
	{ 0xa003fced, "input_register_device" },
	{ 0x713054a0, "input_allocate_device" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0x541ff6bd, "input_event" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xfda85a7d, "request_threaded_irq" },
	{ 0xde75b689, "set_irq_type" },
	{ 0x65d6d0f0, "gpio_direction_input" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xfe990052, "gpio_free" },
	{ 0xea147363, "printk" },
	{ 0xc7290f0f, "input_unregister_device" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x11f447ce, "__gpio_to_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


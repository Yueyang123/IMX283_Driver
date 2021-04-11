#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x405fe887, "module_layout" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0x65d6d0f0, "gpio_direction_input" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xfe990052, "gpio_free" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0xeae3dfd6, "__const_udelay" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


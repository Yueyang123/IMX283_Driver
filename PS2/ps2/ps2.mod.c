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
	{ 0x4833714b, "struct_module" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0xb6c70a7d, "__wake_up" },
	{ 0x80a8d70d, "kill_fasync" },
	{ 0x77bf8cb, "malloc_sizes" },
	{ 0xbe784e4b, "device_create" },
	{ 0xc9001987, "cdev_add" },
	{ 0xdde8bf5c, "cdev_init" },
	{ 0x7044a99d, "__class_create" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xb0e97286, "dev_driver_string" },
	{ 0x1fc91fb2, "request_irq" },
	{ 0xde75b689, "set_irq_type" },
	{ 0x6cb34e5, "init_waitqueue_head" },
	{ 0xfa2a45e, "__memzero" },
	{ 0xea147363, "printk" },
	{ 0xdc74cc24, "kmem_cache_alloc" },
	{ 0xffd5a395, "default_wake_function" },
	{ 0xbb72d4fe, "__put_user_1" },
	{ 0x36e47222, "remove_wait_queue" },
	{ 0x1000e51, "schedule" },
	{ 0xfd8dba4c, "add_wait_queue" },
	{ 0x5f754e5a, "memset" },
	{ 0x43b0c9c3, "preempt_schedule" },
	{ 0xbed60566, "sub_preempt_count" },
	{ 0x4c6ff041, "add_preempt_count" },
	{ 0x74cc238d, "current_kernel_time" },
	{ 0x28118cb6, "__get_user_1" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xeefe9366, "fasync_helper" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x37a0cba, "kfree" },
	{ 0x97b594ba, "cdev_del" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "30A13D8BF62EBEC504B0EDF");

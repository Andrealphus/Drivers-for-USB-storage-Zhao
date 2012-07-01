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
	{ 0xf6628fc9, "module_layout" },
	{ 0xcd9991a9, "usb_usual_set_present" },
	{ 0x810b3618, "param_ops_string" },
	{ 0x3ac8c215, "usb_deregister" },
	{ 0x27e1a049, "printk" },
	{ 0x3edad223, "usb_usual_clear_present" },
	{ 0xb4390f9a, "mcount" },
	{ 0x63afefa5, "usb_register_driver" },
	{ 0xbe13004, "usb_storage_usb_ids" },
	{ 0xc3fe87c8, "param_ops_uint" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "A8A67BFC67B141BA2ED5540");

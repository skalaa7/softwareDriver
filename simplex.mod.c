#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd9726f80, "module_layout" },
	{ 0xa78af5f3, "ioread32" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x1bcee483, "cdev_del" },
	{ 0x68de7c4, "platform_driver_unregister" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x64b60eb0, "class_destroy" },
	{ 0xd4415d08, "__platform_driver_register" },
	{ 0x46dce43, "device_destroy" },
	{ 0xd5f10699, "cdev_add" },
	{ 0x54dc6ab9, "cdev_alloc" },
	{ 0xb7ad68f3, "device_create" },
	{ 0xa946dcde, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xde80cd09, "ioremap" },
	{ 0x85bd1608, "__request_region" },
	{ 0x69ecc112, "kmem_cache_alloc_trace" },
	{ 0x36c11c94, "kmalloc_caches" },
	{ 0xcedb6db2, "platform_get_resource" },
	{ 0x37a0cba, "kfree" },
	{ 0x1035c7c2, "__release_region" },
	{ 0x77358855, "iomem_resource" },
	{ 0xedc03953, "iounmap" },
	{ 0x4a453f53, "iowrite32" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0x56470118, "__warn_printk" },
	{ 0x96848186, "scnprintf" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cxlnx,fft2");
MODULE_ALIAS("of:N*T*Cxlnx,fft2C*");
MODULE_ALIAS("of:N*T*Cxlnx,bram_re");
MODULE_ALIAS("of:N*T*Cxlnx,bram_reC*");
MODULE_ALIAS("of:N*T*Cxlnx,bram_im");
MODULE_ALIAS("of:N*T*Cxlnx,bram_imC*");

MODULE_INFO(srcversion, "B2B4FFA953FC48C96EC41E9");

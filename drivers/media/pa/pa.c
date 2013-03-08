/*
 * drivers/media/pa/pa.c
 * (C) Copyright 2010-2016
 * reuuimllatech Technology Co., Ltd. <www.reuuimllatech.com>
 * huangxin <huangxin@reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/preempt.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <mach/hardware.h>
#include <asm/system.h>
#include <linux/rmap.h>
#include <linux/string.h>
#include <mach/gpio.h>
#include <linux/gpio.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif
#include <mach/sys_config.h>
#include <mach/system.h>

static bool gpio_pa_count = false;
static struct class *pa_dev_class;
static struct cdev *pa_dev;
static dev_t dev_num ;

typedef enum PA_OPT
{
	PA_OPEN = 200,
	PA_CLOSE,
	PA_DEV_
}__ace_ops_e;


static int pa_dev_open(struct inode *inode, struct file *filp){
    return 0;
}

static int pa_dev_release(struct inode *inode, struct file *filp){
	return 0;
}

static long pa_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int req_status;
	script_item_u item;
	script_item_value_type_e  type;
	switch (cmd) {
		case PA_OPEN:
			gpio_pa_count = true;
			/*get the default pa val(close)*/
		    type = script_get_item("audio_para", "audio_pa_ctrl", &item);
			if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
				printk("script_get_item return type err\n");
				return -EFAULT;
			}
			/*request gpio*/
			req_status = gpio_request(item.gpio.gpio, NULL);
			if (0!=req_status) {
				printk("request gpio failed!\n");
			}
			item.gpio.data = 1;
			/*config gpio info of audio_pa_ctrl*/
			if (0 != sw_gpio_setall_range(&item.gpio, 1)) {
				printk("sw_gpio_setall_range failed\n");
			}
			/*while set the pgio value, release gpio handle*/
			if (0 == req_status) {
				gpio_free(item.gpio.gpio);
			}
			break;
		case PA_CLOSE:
			default:
			gpio_pa_count = false;
			/*get the default pa val(close)*/
		    type = script_get_item("audio_para", "audio_pa_ctrl", &item);
			if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
				printk("script_get_item return type err\n");
				return -EFAULT;
			}
			/*request gpio*/
			req_status = gpio_request(item.gpio.gpio, NULL);
			if (0!=req_status) {
				printk("request gpio failed!\n");
			}
			item.gpio.data = 0;
			/*config gpio info of audio_pa_ctrl*/
			if (0 != sw_gpio_setall_range(&item.gpio, 1)) {
				printk("sw_gpio_setall_range failed\n");
			}
			/*while set the pgio value, release gpio handle*/
			if (0 == req_status) {
				gpio_free(item.gpio.gpio);
			}
			break;
	}
	return 0;
}

static int snd_pa_suspend(struct platform_device *pdev,pm_message_t state)
{
	int req_status;
	script_item_u item;
	script_item_value_type_e  type;
	/*get the default pa val(close)*/
    type = script_get_item("audio_para", "audio_pa_ctrl", &item);
	if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
		printk("script_get_item return type err\n");
		return -EFAULT;
	}
	/*request gpio*/
	req_status = gpio_request(item.gpio.gpio, NULL);
	if (0!=req_status) {
		printk("request gpio failed!\n");
	}
	item.gpio.data = 0;
	/*config gpio info of audio_pa_ctrl*/
	if (0 != sw_gpio_setall_range(&item.gpio, 1)) {
		printk("sw_gpio_setall_range failed\n");
	}
	/*while set the pgio value, release gpio handle*/
	if (0 == req_status) {
		gpio_free(item.gpio.gpio);
	}
	return 0;
}

static int snd_pa_resume(struct platform_device *pdev)
{
	int req_status;
	script_item_u item;
	script_item_value_type_e  type;
	/*get the default pa val(close)*/
    type = script_get_item("audio_para", "audio_pa_ctrl", &item);
	if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
		printk("script_get_item return type err\n");
		return -EFAULT;
	}
	/*request gpio*/
	req_status = gpio_request(item.gpio.gpio, NULL);
	if (0 != req_status) {
		printk("request gpio failed!\n");
	}
	item.gpio.data = 1;
	/*config gpio info of audio_pa_ctrl*/
	if (0 != sw_gpio_setall_range(&item.gpio, 1)) {
		printk("sw_gpio_setall_range failed\n");
	}
	/*while set the pgio value, release gpio handle*/
	if (0 == req_status) {
		gpio_free(item.gpio.gpio);
	}
	return 0;
}

static struct file_operations pa_dev_fops = {
    .owner 			= THIS_MODULE,
    .unlocked_ioctl = pa_dev_ioctl,
    .open           = pa_dev_open,
    .release        = pa_dev_release,
};

/*data relating*/
static struct platform_device device_pa = {
	.name = "pa",
};

/*method relating*/
static struct platform_driver pa_driver = {
#ifdef CONFIG_PM
	.suspend	= snd_pa_suspend,
	.resume		= snd_pa_resume,
#endif
	.driver		= {
		.name	= "pa",
	},
};

static int __init pa_dev_init(void)
{
    int err = 0;
    int req_status;
	script_item_u item;
	script_item_value_type_e  type;
	printk("[pa_drv] start!!!\n");

	if ((platform_device_register(&device_pa)) < 0) {
		return err;
	}
	if ((err = platform_driver_register(&pa_driver)) < 0) {
		return err;
	}

    alloc_chrdev_region(&dev_num, 0, 1, "pa_chrdev");
    pa_dev = cdev_alloc();
    cdev_init(pa_dev, &pa_dev_fops);
    pa_dev->owner = THIS_MODULE;
    err = cdev_add(pa_dev, dev_num, 1);
    if (err) {
	printk(KERN_NOTICE"Error %d adding pa_dev!\n", err);
        return -1;
    }
    pa_dev_class = class_create(THIS_MODULE, "pa_cls");
    device_create(pa_dev_class, NULL, dev_num, NULL, "pa_dev");

	/*get the default pa val(close)*/
    type = script_get_item("audio_para", "audio_pa_ctrl", &item);
	if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
		printk("script_get_item return type err\n");
		return -EFAULT;
	}
	/*request gpio*/
	req_status = gpio_request(item.gpio.gpio, NULL);
	if (0!=req_status) {
		printk("request gpio failed!\n");
	}
	/*config gpio info of audio_pa_ctrl*/
	if (0 != sw_gpio_setall_range(&item.gpio, 1)) {
		printk("sw_gpio_setall_range failed\n");
	}
	/*while set the pgio value, release gpio handle*/
	if (0 == req_status) {
		gpio_free(item.gpio.gpio);
	}

    printk("[pa_drv] init end!!!\n");
    return 0;
}
module_init(pa_dev_init);

static void __exit pa_dev_exit(void)
{
    device_destroy(pa_dev_class,  dev_num);
    class_destroy(pa_dev_class);
    platform_driver_unregister(&pa_driver);
}
module_exit(pa_dev_exit);

MODULE_AUTHOR("huangxin");
MODULE_DESCRIPTION("User mode encrypt device interface");
MODULE_LICENSE("GPL");

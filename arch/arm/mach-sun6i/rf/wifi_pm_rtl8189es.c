/*
 * rtl8189es sdio wifi power management API
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <mach/sys_config.h>
#include <mach/gpio.h>

#include "wifi_pm.h"

#define rtl8189es_msg(...)    do {printk("[rtl8189es]: "__VA_ARGS__);} while(0)

static int rtl8189es_powerup = 0;
static int rtl8189es_suspend = 0;

static int rtl8189es_gpio_ctrl(char* name, int level)
{
	int i = 0, ret = 0;
	struct wifi_pm_ops *ops = &wifi_card_pm_ops;
	char* gpio_name[4] = {"rtl8189es_wakeup", "rtl8189es_shdn",	"rtl8189es_vcc_en",	"rtl8189es_vdd_en"};

	for (i=0; i<4; i++) {
		if (strcmp(name, gpio_name[i])==0)
			break;
	}
	if (i==4) {
		rtl8189es_msg("No gpio %s for rtl8189es-wifi module\n", name);
		return -1;
	}

	ret = sw_gpio_write_one_pin_value(ops->pio_hdle, level, name);
	if (ret) {
		rtl8189es_msg("Failed to set gpio %s to %d !\n", name, level);
		return -1;
	} else
		rtl8189es_msg("Succeed to set gpio %s to %d !\n", name, level);

	if (strcmp(name, "rtl8189es_vdd_en") == 0) {
		rtl8189es_powerup = level;
	}

	return 0;
}

static int rtl8189es_get_io_value(char* name)
{
	int ret = -1;
	struct wifi_pm_ops *ops = &wifi_card_pm_ops;

	if (strcmp(name, "rtl8189es_wakeup")) {
		rtl8189es_msg("No gpio %s for rtl8189es\n", name);
		return -1;
	}
	ret = sw_gpio_read_one_pin_value(ops->pio_hdle, name);
	rtl8189es_msg("Succeed to get gpio %s value: %d !\n", name, ret);

	return ret;
}

static void rtl8189es_standby(int instadby)
{
	if (instadby) {
		if (rtl8189es_powerup) {
			rtl8189es_gpio_ctrl("rtl8189es_shdn", 0);
			rtl8189es_gpio_ctrl("rtl8189es_vcc_en", 0);
			rtl8189es_gpio_ctrl("rtl8189es_vdd_en", 0);
			rtl8189es_suspend = 1;
		}
	} else {
		if (rtl8189es_suspend) {
			rtl8189es_gpio_ctrl("rtl8189es_vdd_en", 1);
			udelay(100);
			rtl8189es_gpio_ctrl("rtl8189es_vcc_en", 1);
			udelay(500);
			rtl8189es_gpio_ctrl("rtl8189es_shdn", 1);
			sw_mci_rescan_card(3, 1);
			rtl8189es_suspend = 0;
		}
	}
	rtl8189es_msg("sdio wifi : %s\n", instadby ? "suspend" : "resume");
}

static void rtl8189es_power(int mode, int *updown)
{
    if (mode) {
        if (*updown) {
			rtl8189es_gpio_ctrl("rtl8189es_vdd_en", 1);
			udelay(100);
			rtl8189es_gpio_ctrl("rtl8189es_vcc_en", 1);
			udelay(500);
			rtl8189es_gpio_ctrl("rtl8189es_shdn", 1);
        } else {
			rtl8189es_gpio_ctrl("rtl8189es_shdn", 0);
			rtl8189es_gpio_ctrl("rtl8189es_vcc_en", 0);
			rtl8189es_gpio_ctrl("rtl8189es_vdd_en", 0);
        }
        rtl8189es_msg("sdio wifi power state: %s\n", *updown ? "on" : "off");
    } else {
        if (rtl8189es_powerup)
            *updown = 1;
        else
            *updown = 0;
		rtl8189es_msg("sdio wifi power state: %s\n", rtl8189es_powerup ? "on" : "off");
    }
    return;
}

void rtl8189es_gpio_init(void)
{
	struct wifi_pm_ops *ops = &wifi_card_pm_ops;

	rtl8189es_msg("exec rtl8189es_wifi_gpio_init\n");
	rtl8189es_powerup = 0;
	rtl8189es_suspend = 0;
	ops->gpio_ctrl 	  = rtl8189es_gpio_ctrl;
	ops->get_io_val   = rtl8189es_get_io_value;
	ops->standby 	  = rtl8189es_standby;
	ops->power 		  = rtl8189es_power;
}

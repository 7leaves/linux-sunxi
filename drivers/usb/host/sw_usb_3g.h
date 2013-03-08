/*
*************************************************************************************
*                         			      Linux
*					                 USB Host Driver
*
*				        (c) Copyright 2006-2012, SoftWinners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sw_usb_3g.h
*
* Author 		: javen
*
* Description 	: USB 3G operations
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*       javen     	  2012-4-10            1.0          create this file
*
*************************************************************************************
*/

#ifndef  __SW_USB_3G_H__
#define  __SW_USB_3G_H__

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#define SLEEP_BY_GPIO_WAKEUP_BY_GPIO    0
#define SLEEP_BY_USB_WAKEUP_BY_GPIO     1
#define SLEEP_BY_USB_WAKEUP_BY_USB      2

/* HUAWEI MU509 HSDPA LGA Module Hardware Guide */
struct sw_usb_3g{
    struct work_struct irq_work;
    spinlock_t lock;

#ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend early_suspend;
#endif

    u32 used;
    u32 usbc_no;                        /* 挂载的USB控制器编号 */
    u32 usbc_type;                      /* 挂载的USB控制器控制器类型 */
    u32 uart_no;                        /* 挂载的uart控制器 */

    u32 vbat_valid;
    user_gpio_set_t vbat_set;           /* vbat pin, 3g总电源 */
    u32 vbat_hd;

    u32 power_on_off_valid;
    user_gpio_set_t power_on_off_set;   /* power_on_off pin */
    u32 power_on_off_hd;

    u32 reset_valid;
    user_gpio_set_t reset_set;          /* reset pin */
    u32 reset_hd;

    u32 wakeup_in_valid;
    user_gpio_set_t wakeup_in_set;     /* wakeup_out pin, A10 wakeup/sleep usb_3g */
    u32 wakeup_in_hd;
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void usb_3g_power(u32 usbc_no, u32 on);
void usb_3g_reset(u32 usbc_no, u32 time);
void usb_3g_wakeup_sleep(u32 usbc_no, u32 sleep);
u32 is_suspport_usb_3g(u32 usbc_no, u32 usbc_type);

int usb_3g_wakeup_irq_init(void);
int usb_3g_wakeup_irq_exit(void);

int usb_3g_init(void);
int usb_3g_exit(void);


#endif   //__SW_USB_3G_H__

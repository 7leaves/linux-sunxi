/*
 * arch/arm/mach-sun6i/include/mach/clock.h
 *
 * Copyright 2012 (c) Allwinner.
 * kevin.z.m (kevin@allwinnertech.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __AW_CLOCK_H__
#define __AW_CLOCK_H__

#include <linux/kernel.h>
#include <linux/string.h>

/* define clock error type      */
typedef enum __AW_CCU_ERR
{
    AW_CCU_ERR_NONE     =  0,
    AW_CCU_ERR_PARA_NUL = -1,
    AW_CCU_ERR_PARA_INV = -2,
} __aw_ccu_err_e;


typedef enum __AW_CCU_CLK_ONOFF
{
    AW_CCU_CLK_OFF          = 0,
    AW_CCU_CLK_ON           = 1,
} __aw_ccu_clk_onff_e;


typedef enum __AW_CCU_CLK_RESET
{
    AW_CCU_CLK_RESET        = 0,
    AW_CCU_CLK_NRESET       = 1,
} __aw_ccu_clk_reset_e;



/* define system clock id       */
typedef enum __AW_CCU_CLK_ID
{
    AW_SYS_CLK_NONE=0,  /* invalid clock id                     */

    /* OSC */
    AW_SYS_CLK_LOSC,    /* "sys_losc"   ,LOSC, 32768 hz clock   */
    AW_SYS_CLK_HOSC,    /* "sys_hosc"   ,HOSC, 24Mhz clock      */

    /* PLL */
    AW_SYS_CLK_PLL1,    /* "sys_pll1"   ,PLL1 clock             */
    AW_SYS_CLK_PLL2,    /* "sys_pll2"   ,PLL2 clock             */
    AW_SYS_CLK_PLL3,    /* "sys_pll3"   ,PLL3 clock             */
    AW_SYS_CLK_PLL4,    /* "sys_pll4"   ,PLL4 clock             */
    AW_SYS_CLK_PLL5,    /* "sys_pll5"   ,PLL5 clock             */
    AW_SYS_CLK_PLL6,    /* "sys_pll6"   ,PLL6 clock,            */
    AW_SYS_CLK_PLL7,    /* "sys_pll7"   ,PLL7 clock             */
    AW_SYS_CLK_PLL8,    /* "sys_pll8"   ,PLL8 clock             */
    AW_SYS_CLK_PLL9,    /* "sys_pll9"   ,PLL9 clock             */
    AW_SYS_CLK_PLL10,   /* "sys_pll10"  ,PLL10 clock            */

    /* related PLL */
    AW_SYS_CLK_PLL2X8,  /* "sys_pll2X8"   ,PLL2 8X clock        */
    AW_SYS_CLK_PLL3X2,  /* "sys_pll3X2"   ,PLL3 X2 clock        */
    AW_SYS_CLK_PLL6x2,  /* "sys_pll6X2"   ,PLL6 X2 clock        */
    AW_SYS_CLK_PLL7X2,  /* "sys_pll7X2"   ,PLL7 X2 clock        */
    AW_SYS_CLK_MIPIPLL, /* "sys_mipi_pll" ,MIPI PLL clock       */

    /* CPU & BUS */
    AW_SYS_CLK_AC327,   /* "sys_ac327"  ,CPU clock              */
    AW_SYS_CLK_AR100,   /* "sys_ar100"  ,CPU clock              */
    AW_SYS_CLK_AXI,     /* "sys_axi"    ,AXI clock              */
    AW_SYS_CLK_AHB0,    /* "sys_ahb0"   ,AHB0 clock             */
    AW_SYS_CLK_AHB1,    /* "sys_ahb1"   ,AHB1 clock             */
    AW_SYS_CLK_APB0,    /* "sys_apb0"   ,APB0 clock             */
    AW_SYS_CLK_APB1,    /* "sys_apb1"   ,APB1 clock             */
    AW_SYS_CLK_APB2,    /* "sys_apb2"   ,APB2 clock             */

    AW_CCU_CLK_NULL,    /* null module clock id                 */

    /* module clock */
    AW_MOD_CLK_NAND0,       /* "mod_nand0"                      */
    AW_MOD_CLK_NAND1,       /* "mod_nand1"                      */
    AW_MOD_CLK_SDC0,        /* "mod_sdc0"                       */
    AW_MOD_CLK_SDC1,        /* "mod_sdc1"                       */
    AW_MOD_CLK_SDC2,        /* "mod_sdc2"                       */
    AW_MOD_CLK_SDC3,        /* "mod_sdc3"                       */
    AW_MOD_CLK_TS,          /* "mod_ts"                         */
    AW_MOD_CLK_SS,          /* "mod_ss"                         */
    AW_MOD_CLK_SPI0,        /* "mod_spi0"                       */
    AW_MOD_CLK_SPI1,        /* "mod_spi1"                       */
    AW_MOD_CLK_SPI2,        /* "mod_spi2"                       */
    AW_MOD_CLK_SPI3,        /* "mod_spi3"                       */
    AW_MOD_CLK_I2S0,        /* "mod_i2s0"                       */
    AW_MOD_CLK_I2S1,        /* "mod_i2s1"                       */
    AW_MOD_CLK_SPDIF,       /* "mod_spdif"                      */
    AW_MOD_CLK_USBPHY0,     /* "mod_usbphy0"                    */
    AW_MOD_CLK_USBPHY1,     /* "mod_usbphy1"                    */
    AW_MOD_CLK_USBPHY2,     /* "mod_usbphy2"                    */
    AW_MOD_CLK_USBEHCI0,    /* "mod_usbehci0"                   */
    AW_MOD_CLK_USBEHCI1,    /* "mod_usbehci1"                   */
    AW_MOD_CLK_USBOHCI0,    /* "mod_usbohci0"                   */
    AW_MOD_CLK_USBOHCI1,    /* "mod_usbohci1"                   */
    AW_MOD_CLK_USBOHCI2,    /* "mod_usbohci2"                   */
    AW_MOD_CLK_USBOTG,      /* "mod_usbotg"                     */
    AW_MOD_CLK_MDFS,        /* "mod_mdfs"                       */
    AW_MOD_CLK_DEBE0,       /* "mod_debe0"                      */
    AW_MOD_CLK_DEBE1,       /* "mod_debe1"                      */
    AW_MOD_CLK_DEFE0,       /* "mod_defe0"                      */
    AW_MOD_CLK_DEFE1,       /* "mod_defe1"                      */
    AW_MOD_CLK_DEMIX,       /* "mod_demp"                       */
    AW_MOD_CLK_LCD0CH0,     /* "mod_lcd0ch0"                    */
    AW_MOD_CLK_LCD0CH1,     /* "mod_lcd0ch1"                    */
    AW_MOD_CLK_LCD1CH0,     /* "mod_lcd1ch0"                    */
    AW_MOD_CLK_LCD1CH1,     /* "mod_lcd1ch1"                    */
    AW_MOD_CLK_CSI0S,       /* "mod_csi0s"                      */
    AW_MOD_CLK_CSI0M,       /* "mod_csi0m"                      */
    AW_MOD_CLK_CSI1S,       /* "mod_csi1s"                      */
    AW_MOD_CLK_CSI1M,       /* "mod_csi1m"                      */
    AW_MOD_CLK_VE,          /* "mod_ve"                         */
    AW_MOD_CLK_ADDA,        /* "mod_adda"                       */
    AW_MOD_CLK_AVS,         /* "mod_avs"                        */
    AW_MOD_CLK_HDMI,        /* "mod_hdmi"                       */
    AW_MOD_CLK_PS,          /* "mod_ps"                         */
    AW_MOD_CLK_MTCACC,      /* "mod_mtcacc"                     */
    AW_MOD_CLK_MBUS0,       /* "mod_mbus0"                      */
    AW_MOD_CLK_MBUS1,       /* "mod_mbus1"                      */
    AW_MOD_CLK_DRAM,        /* "mod_dram"                       */
    AW_MOD_CLK_MIPIDSIS,    /* "mod_mipidsis"                   */
    AW_MOD_CLK_MIPIDSIP,    /* "mod_mipidsip"                   */
    AW_MOD_CLK_MIPICSIS,    /* "mod_mipicsis"                   */
    AW_MOD_CLK_MIPICSIP,    /* "mod_mipicsip"                   */
    AW_MOD_CLK_IEPDRC0,     /* "mod_iepdrc0"                    */
    AW_MOD_CLK_IEPDRC1,     /* "mod_iepdrc1"                    */
    AW_MOD_CLK_IEPDEU0,     /* "mod_iepdeu0"                    */
    AW_MOD_CLK_IEPDEU1,     /* "mod_iepdeu1"                    */
    AW_MOD_CLK_GPUCORE,     /* "mod_gpucore"                    */
    AW_MOD_CLK_GPUMEM,      /* "mod_gpumem"                     */
    AW_MOD_CLK_GPUHYD,      /* "mod_gpuhyd"                     */
    AW_MOD_CLK_TWI0,        /* "mod_twi0"                       */
    AW_MOD_CLK_TWI1,        /* "mod_twi1"                       */
    AW_MOD_CLK_TWI2,        /* "mod_twi2"                       */
    AW_MOD_CLK_TWI3,        /* "mod_twi3"                       */
    AW_MOD_CLK_UART0,       /* "mod_uart0"                      */
    AW_MOD_CLK_UART1,       /* "mod_uart1"                      */
    AW_MOD_CLK_UART2,       /* "mod_uart2"                      */
    AW_MOD_CLK_UART3,       /* "mod_uart3"                      */
    AW_MOD_CLK_UART4,       /* "mod_uart4"                      */
    AW_MOD_CLK_UART5,       /* "mod_uart5"                      */
    AW_MOD_CLK_GMAC,        /* "mod_gmac"                       */
    AW_MOD_CLK_DMA,         /* "mod_dma"                        */
    AW_MOD_CLK_HSTMR,       /* "mod_hstmr"                      */
    AW_MOD_CLK_MSGBOX,      /* "mod_msgbox"                     */
    AW_MOD_CLK_SPINLOCK,    /* "mod_spinlock"                   */
    AW_MOD_CLK_LVDS,        /* "mod_lvds"                       */
    AW_MOD_CLK_SMPTWD,      /* "smp_twd"                        */

    /* axi module gating */
    AW_AXI_CLK_DRAM,        /* "axi_dram"                       */

    /* ahb module gating */
    AW_AHB_CLK_MIPICSI,     /* "ahb_mipicsi"                    */
    AW_AHB_CLK_MIPIDSI,     /* "ahb_mipidsi"                    */
    AW_AHB_CLK_SS,          /* "ahb_ss"                         */
    AW_AHB_CLK_DMA,         /* "ahb_dma"                        */
    AW_AHB_CLK_SDMMC0,      /* "ahb_sdmmc0"                     */
    AW_AHB_CLK_SDMMC1,      /* "ahb_sdmmc1"                     */
    AW_AHB_CLK_SDMMC2,      /* "ahb_sdmmc2"                     */
    AW_AHB_CLK_SDMMC3,      /* "ahb_sdmmc3"                     */
    AW_AHB_CLK_NAND1,       /* "ahb_nand1"                      */
    AW_AHB_CLK_NAND0,       /* "ahb_nand0"                      */
    AW_AHB_CLK_SDRAM,       /* "ahb_sdram"                      */
    AW_AHB_CLK_GMAC,        /* "ahb_gmac"                       */
    AW_AHB_CLK_TS,          /* "ahb_ts"                         */
    AW_AHB_CLK_HSTMR,       /* "ahb_hstmr"                      */
    AW_AHB_CLK_SPI0,        /* "ahb_spi0"                       */
    AW_AHB_CLK_SPI1,        /* "ahb_spi1"                       */
    AW_AHB_CLK_SPI2,        /* "ahb_spi2"                       */
    AW_AHB_CLK_SPI3,        /* "ahb_spi3"                       */
    AW_AHB_CLK_OTG,         /* "ahb_otg"                        */
    AW_AHB_CLK_EHCI0,       /* "ahb_ehci0"                      */
    AW_AHB_CLK_EHCI1,       /* "ahb_ehci1"                      */
    AW_AHB_CLK_OHCI0,       /* "ahb_ohci0"                      */
    AW_AHB_CLK_OHCI1,       /* "ahb_ohci1"                      */
    AW_AHB_CLK_OHCI2,       /* "ahb_ohci2"                      */
    AW_AHB_CLK_VE,          /* "ahb_ve"                         */
    AW_AHB_CLK_LCD0,        /* "ahb_lcd0"                       */
    AW_AHB_CLK_LCD1,        /* "ahb_lcd1"                       */
    AW_AHB_CLK_CSI0,        /* "ahb_csi0"                       */
    AW_AHB_CLK_CSI1,        /* "ahb_csi1"                       */
    AW_AHB_CLK_HDMID,       /* "ahb_hdmid"                      */
    AW_AHB_CLK_DEBE0,       /* "ahb_debe0"                      */
    AW_AHB_CLK_DEBE1,       /* "ahb_debe1"                      */
    AW_AHB_CLK_DEFE0,       /* "ahb_defe0"                      */
    AW_AHB_CLK_DEFE1,       /* "ahb_defe1"                      */
    AW_AHB_CLK_MP,          /* "ahb_mp"                         */
    AW_AHB_CLK_GPU,         /* "ahb_gpu"                        */
    AW_AHB_CLK_MSGBOX,      /* "ahb_msgbox"                     */
    AW_AHB_CLK_SPINLOCK,    /* "ahb_spinlock"                   */
    AW_AHB_CLK_DEU0,        /* "ahb_deu0"                       */
    AW_AHB_CLK_DEU1,        /* "ahb_deu1"                       */
    AW_AHB_CLK_DRC0,        /* "ahb_drc0"                       */
    AW_AHB_CLK_DRC1,        /* "ahb_drc1"                       */
    AW_AHB_CLK_MTCACC,      /* "ahb_mtcacc"                     */

    /* apb module gating */
    AW_APB_CLK_ADDA,        /* "apb_adda"                       */
    AW_APB_CLK_SPDIF,       /* "apb_spdif"                      */
    AW_APB_CLK_PIO,         /* "apb_pio"                        */
    AW_APB_CLK_I2S0,        /* "apb_i2s0"                       */
    AW_APB_CLK_I2S1,        /* "apb_i2s1"                       */
    AW_APB_CLK_TWI0,        /* "apb_twi0"                       */
    AW_APB_CLK_TWI1,        /* "apb_twi1"                       */
    AW_APB_CLK_TWI2,        /* "apb_twi2"                       */
    AW_APB_CLK_TWI3,        /* "apb_twi3"                       */
    AW_APB_CLK_UART0,       /* "apb_uart0"                      */
    AW_APB_CLK_UART1,       /* "apb_uart1"                      */
    AW_APB_CLK_UART2,       /* "apb_uart2"                      */
    AW_APB_CLK_UART3,       /* "apb_uart3"                      */
    AW_APB_CLK_UART4,       /* "apb_uart4"                      */
    AW_APB_CLK_UART5,       /* "apb_uart5"                      */

    /* dram module gating */
    AW_DRAM_CLK_VE,         /* "dram_ve"                        */
    AW_DRAM_CLK_CSI0,       /* "dram_csi0"                      */
    AW_DRAM_CLK_CSI1,       /* "dram_csi1"                      */
    AW_DRAM_CLK_TS,         /* "dram_ts"                        */
    AW_DRAM_CLK_DRC0,       /* "dram_drc0"                      */
    AW_DRAM_CLK_DRC1,       /* "dram_drc1"                      */
    AW_DRAM_CLK_DEU0,       /* "dram_deu0"                      */
    AW_DRAM_CLK_DEU1,       /* "dram_deu1"                      */
    AW_DRAM_CLK_DEFE0,      /* "dram_defe0"                     */
    AW_DRAM_CLK_DEFE1,      /* "dram_defe1"                     */
    AW_DRAM_CLK_DEBE0,      /* "dram_debe0"                     */
    AW_DRAM_CLK_DEBE1,      /* "dram_debe1"                     */
    AW_DRAM_CLK_MP,         /* "dram_mp"                        */

    AW_CCU_CLK_CNT          /* invalid id, for calc count       */

} __aw_ccu_clk_id_e;


/* define handle for moduel clock   */
typedef struct __AW_CCU_CLK
{
    __aw_ccu_clk_id_e       id;     /* clock id         */
    __aw_ccu_clk_id_e       parent; /* parent clock id  */
    char                    *name;  /* clock name       */
    __aw_ccu_clk_onff_e     onoff;  /* on/off status    */
    __aw_ccu_clk_reset_e    reset;  /* reset status     */
    __u64                   rate;   /* clock rate, frequency for system clock, division for module clock */
    __s32                   hash;   /* hash value, for fast search without string compare   */

}__aw_ccu_clk_t;


typedef struct clk_ops
{
    int     (*set_status)(__aw_ccu_clk_id_e id, __aw_ccu_clk_onff_e status);
    __aw_ccu_clk_onff_e (*get_status)(__aw_ccu_clk_id_e id);
    int     (*set_parent)(__aw_ccu_clk_id_e id, __aw_ccu_clk_id_e parent);
    __aw_ccu_clk_id_e (*get_parent)(__aw_ccu_clk_id_e id);
    int     (*set_rate)(__aw_ccu_clk_id_e id, __u64 rate);
    __u64   (*round_rate)(__aw_ccu_clk_id_e id, __u64 rate);
    __u64   (*get_rate)(__aw_ccu_clk_id_e id);
    int     (*set_reset)(__aw_ccu_clk_id_e id, __aw_ccu_clk_reset_e);
    __aw_ccu_clk_reset_e (*get_reset)(__aw_ccu_clk_id_e id);

} __clk_ops_t;


/*
*********************************************************************************************************
*                           mod_clk_calc_hash
*
*Description: calculate hash value of a string;
*
*Arguments  : string    string whose hash value need be calculate;
*
*Return     : hash value
*
*Notes      :
*
*********************************************************************************************************
*/
static inline __s32 ccu_clk_calc_hash(char *string)
{
    __s32   tmpLen, i, tmpHash = 0;

    if(!string) {
        return 0;
    }

    tmpLen = strlen(string);
    for(i=0; i<tmpLen; i++) {
        tmpHash += string[i];
    }

    return tmpHash;
}


typedef struct clk
{
    __aw_ccu_clk_t  *aw_clk;    /* clock handle from ccu csp                            */
    __clk_ops_t     *ops;       /* clock operation handle                               */
    int             enable;     /* enable count, when it down to 0, it will be disalbe  */

} __ccu_clk_t;


extern int clk_reset(struct clk *clk, __aw_ccu_clk_reset_e reset);

#endif  /* #ifndef __AW_CLOCK_H__ */

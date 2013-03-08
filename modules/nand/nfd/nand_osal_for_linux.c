/*
*********************************************************************************************************
*											        eBIOS
*						            the Easy Portable/Player Develop Kits
*									           dma sub system
*
*						        (c) Copyright 2006-2008, David China
*											All	Rights Reserved
*
* File    : clk_for_nand.c
* By      : Richard
* Version : V1.00
*********************************************************************************************************
*/
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/spinlock.h>
#include <linux/hdreg.h>
#include <linux/init.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <mach/clock.h>
#include <mach/platform.h>
#include <mach/hardware.h>
#include <linux/dma-mapping.h>
#include <mach/dma.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/cacheflush.h>
#include "nand_lib.h"
#include "nand_blk.h"

#ifndef __FPGA_TEST__
    #include <mach/sys_config.h>
#endif

static struct clk *ahb_nand_clk = NULL;
static struct clk *mod_nand_clk = NULL;

int seq=0;
int nand_handle=0;

#ifdef __OS_NAND_SUPPORT_INT__
static int nandrb_ready_flag = 1;
static int nanddma_ready_flag = 1;
static DECLARE_WAIT_QUEUE_HEAD(NAND_RB_WAIT);
static DECLARE_WAIT_QUEUE_HEAD(NAND_DMA_WAIT);
#endif

//#define RB_INT_MSG_ON
#ifdef  RB_INT_MSG_ON
#define dbg_rbint(fmt, args...) printk(fmt, ## args)
#else
#define dbg_rbint(fmt, ...)  ({})
#endif

#define RB_INT_WRN_ON
#ifdef  RB_INT_WRN_ON
#define dbg_rbint_wrn(fmt, args...) printk(fmt, ## args)
#else
#define dbg_rbint_wrn(fmt, ...)  ({})
#endif

//#define DMA_INT_MSG_ON
#ifdef  DMA_INT_MSG_ON
#define dbg_dmaint(fmt, args...) printk(fmt, ## args)
#else
#define dbg_dmaint(fmt, ...)  ({})
#endif

#define DMA_INT_WRN_ON
#ifdef  DMA_INT_WRN_ON
#define dbg_dmaint_wrn(fmt, args...) printk(fmt, ## args)
#else
#define dbg_dmaint_wrn(fmt, ...)  ({})
#endif

/*
*********************************************************************************************************
*                                               DMA TRANSFER END ISR
*
* Description: dma transfer end isr.
*
* Arguments  : none;
*
* Returns    : EPDK_TRUE/ EPDK_FALSE
*********************************************************************************************************
*/

int NAND_ClkRequest(void)
{
    printk("[NAND] nand clk request start\n");
	ahb_nand_clk = clk_get(NULL,"ahb_nfc");
	if(!ahb_nand_clk) {
		return -1;
	}
	mod_nand_clk = clk_get(NULL,"nfc");
		if(!mod_nand_clk) {
		return -1;
	}
	printk("[NAND] nand clk request ok!\n");
	return 0;
}

void NAND_ClkRelease(void)
{
	clk_put(ahb_nand_clk);
	clk_put(mod_nand_clk);
}


int NAND_AHBEnable(void)
{
	return clk_enable(ahb_nand_clk);
}

int NAND_ClkEnable(void)
{
	return clk_enable(mod_nand_clk);
}

void NAND_AHBDisable(void)
{
	clk_disable(ahb_nand_clk);
}

void NAND_ClkDisable(void)
{
	clk_disable(mod_nand_clk);
}

int NAND_SetClk(__u32 nand_clk)
{
	return clk_set_rate(mod_nand_clk, nand_clk*2000000);
}

int NAND_GetClk(void)
{
	return (clk_get_rate(mod_nand_clk)/2000000);
}

void eLIBs_CleanFlushDCacheRegion_nand(void *adr, size_t bytes)
{
	__cpuc_flush_dcache_area(adr, bytes + (1 << 5) * 2 - 2);
}


__s32 NAND_CleanFlushDCacheRegion(__u32 buff_addr, __u32 len)
{
	eLIBs_CleanFlushDCacheRegion_nand((void *)buff_addr, (size_t)len);
    return 0;
}

__u32 NAND_DMASingleMap(__u32 rw, __u32 buff_addr, __u32 len)
{
    __u32 mem_addr;

    if (rw == 1)
    {
	    mem_addr = dma_map_single(NULL, buff_addr, len, DMA_TO_DEVICE);
	}
	else
    {
	    mem_addr = dma_map_single(NULL, buff_addr, len, DMA_FROM_DEVICE);
	}

	return mem_addr;
}

__u32 NAND_DMASingleUnmap(__u32 rw, __u32 buff_addr, __u32 len)
{
    __u32 mem_addr;

    mem_addr = virt_to_phys(buff_addr);

	if (rw == 1)
	{
	    dma_unmap_single(NULL, mem_addr, len, DMA_TO_DEVICE);
	}
	else
	{
	    dma_unmap_single(NULL, mem_addr, len, DMA_FROM_DEVICE);
	}

	return mem_addr;
}



#ifdef __OS_NAND_SUPPORT_INT__
void NAND_EnDMAInt(void)
{
	//clear interrupt
	NFC_WRITE_REG(NFC_REG_ST,NFC_DMA_INT_FLAG);
	if(NFC_READ_REG(NFC_REG_ST)&NFC_DMA_INT_FLAG)
	{
		dbg_rbint_wrn("nand clear dma int status error in int enable \n");
		dbg_rbint_wrn("rb status: 0x%x\n", NFC_READ_REG(NFC_REG_ST));
	}

	nanddma_ready_flag = 0;

	//enable interrupt
	NFC_WRITE_REG(NFC_REG_INT, NFC_READ_REG(NFC_REG_INT)|NFC_DMA_INT_ENABLE);

	dbg_rbint("dma int en\n");
}

void NAND_ClearDMAInt(void)
{

	//disable interrupt
	NFC_WRITE_REG(NFC_REG_INT, NFC_READ_REG(NFC_REG_INT)&(~(NFC_DMA_INT_ENABLE)));

	dbg_dmaint("rb int clear\n");

	//clear interrupt
	NFC_WRITE_REG(NFC_REG_ST,NFC_DMA_INT_FLAG);
	if(NFC_READ_REG(NFC_REG_ST)&NFC_DMA_INT_FLAG)
	{
		dbg_dmaint_wrn("nand clear dma int status error in int clear \n");
		dbg_dmaint_wrn("rb status: 0x%x\n", NFC_READ_REG(NFC_REG_ST));
	}

	nanddma_ready_flag = 0;
}

void NAND_DMAInterrupt(void)
{

	dbg_dmaint("dma int occor! \n");
	if(!(NFC_READ_REG(NFC_REG_ST)&NFC_DMA_INT_FLAG))
	{
		dbg_dmaint_wrn("nand rb int late, dma status: 0x%x, dma int en: 0x%x \n",NFC_READ_REG(NFC_REG_ST),NFC_READ_REG(NFC_REG_INT));
	}

    NAND_ClearDMAInt();

    nanddma_ready_flag = 1;
	wake_up( &NAND_DMA_WAIT );

}

__s32 NAND_WaitDmaFinish(void)
{
#ifdef __OS_SUPPORT_DMA_INT__
    NAND_EnDMAInt();

	//wait_event(NAND_RB_WAIT, nandrb_ready_flag);
	dbg_dmaint("dma wait, nfc_ctl: 0x%x, dma status: 0x%x, dma int en: 0x%x\n", NFC_READ_REG(NFC_REG_CTL), NFC_READ_REG(NFC_REG_ST), NFC_READ_REG(NFC_REG_INT));

	if(nanddma_ready_flag)
	{
		dbg_rbint("fast dma int\n");
		NAND_ClearDMAInt();
		return 0;
	}

    if(wait_event_timeout(NAND_DMA_WAIT, nanddma_ready_flag, 1*HZ)==0)
	{
		dbg_dmaint_wrn("nand wait rb ready time out\n");
		dbg_dmaint_wrn("rb wait time out, nfc_ctl: 0x%x, dma status: 0x%x, dma int en: 0x%x\n", NFC_READ_REG(NFC_REG_CTL), NFC_READ_REG(NFC_REG_ST), NFC_READ_REG(NFC_REG_INT));
	    NAND_ClearDMAInt();
	}
	else
	{
		dbg_rbint("nand wait dma ready ok\n");
		NAND_ClearDMAInt();
	}
#endif
    return 0;
}

void NAND_EnRbInt(void)
{
	//clear interrupt
	NFC_RbIntClear();

	if(NFC_RbIntStatus())
	{
		dbg_rbint_wrn("nand clear rb int status error in int enable \n");
		dbg_rbint_wrn("rb status: 0x%x\n", NFC_READ_REG(NFC_REG_ST));
	}

	nandrb_ready_flag = 0;

	//enable interrupt
	NFC_RbIntEnable();

	dbg_rbint("rb int en\n");
}


void NAND_ClearRbInt(void)
{

	//disable interrupt
	NFC_RbIntDisable();;

	dbg_rbint("rb int clear\n");

	//clear interrupt
	NFC_RbIntClear();

	//check rb int status
	if(NFC_RbIntStatus())
	{
		dbg_rbint_wrn("nand clear rb int status error in int clear \n");
		dbg_rbint_wrn("rb status: 0x%x\n", NFC_READ_REG(NFC_REG_ST));
	}

	nandrb_ready_flag = 0;
}


void NAND_RbInterrupt(void)
{

	dbg_rbint("rb int occor! \n");
	if(!(NFC_READ_REG(NFC_REG_ST)&NFC_RB_B2R))
	{
		dbg_rbint_wrn("nand rb int late, rb status: 0x%x, rb int en: 0x%x \n",NFC_READ_REG(NFC_REG_ST),NFC_READ_REG(NFC_REG_INT));
	}

    NAND_ClearRbInt();

    nandrb_ready_flag = 1;
	wake_up( &NAND_RB_WAIT );

}

__s32 NAND_WaitRbReady(void)
{
#ifdef __OS_SUPPORT_RB_INT__
	__u32 rb;

	NAND_EnRbInt();

	//wait_event(NAND_RB_WAIT, nandrb_ready_flag);
	dbg_rbint("rb wait, nfc_ctl: 0x%x, rb status: 0x%x, rb int en: 0x%x\n", NFC_READ_REG(NFC_REG_CTL), NFC_READ_REG(NFC_REG_ST), NFC_READ_REG(NFC_REG_INT));

	if(nandrb_ready_flag)
	{
		dbg_rbint("fast rb int\n");
		NAND_ClearRbInt();
		return 0;
	}

	rb=  NFC_GetRbSelect();
	if(NFC_GetRbStatus(rb))
	{
		dbg_rbint_wrn("rb %u fast ready \n", rb);
		dbg_rbint_wrn("nfc_ctl: 0x%x, rb status: 0x%x, rb int en: 0x%x\n", NFC_READ_REG(NFC_REG_CTL), NFC_READ_REG(NFC_REG_ST), NFC_READ_REG(NFC_REG_INT));
		NAND_ClearRbInt();
		return 0;
	}


	if(wait_event_timeout(NAND_RB_WAIT, nandrb_ready_flag, 1*HZ)==0)
	{
		dbg_rbint_wrn("nand wait rb ready time out\n");
		dbg_rbint_wrn("rb wait time out, nfc_ctl: 0x%x, rb status: 0x%x, rb int en: 0x%x\n", NFC_READ_REG(NFC_REG_CTL), NFC_READ_REG(NFC_REG_ST), NFC_READ_REG(NFC_REG_INT));
		NAND_ClearRbInt();
	}
	else
	{
		dbg_rbint("nand wait rb ready ok\n");
	}
#endif
    return 0;
}
#else
__s32 NAND_WaitDmaFinish(void)
{
    return 0;
}

__s32 NAND_WaitRbReady(void)
{
    return 0;
}
#endif

__u32 NAND_VA_TO_PA(__u32 buff_addr)
{
    return (__u32)(__pa((void *)buff_addr));
}

void NAND_PIORequest(void)
{
	printk("[NAND] nand gpio_request\n");
	#ifndef __FPGA_TEST__
	nand_handle = gpio_request_ex("nand_para",NULL);
	if(!nand_handle)
	{
		printk("[NAND] nand gpio_request ok\n");
	}
	else
	{
	    printk("[NAND] nand gpio_request fail\n");
	}
	#endif


}

void NAND_Interrupt(void)
{
#ifdef __OS_NAND_SUPPORT_INT__
    if((NFC_READ_REG(NFC_REG_ST)&NFC_RB_B2R)&&(NFC_READ_REG(NFC_REG_INT)&NFC_B2R_INT_ENABLE))
    {
        //printk("nand rb int\n");
        NAND_RbInterrupt();
    }
    else if((NFC_READ_REG(NFC_REG_ST)&NFC_DMA_INT_FLAG)&&(NFC_READ_REG(NFC_REG_INT)&NFC_DMA_INT_ENABLE))
    {
        //printk("nand dma int\n");
        NAND_DMAInterrupt();
    }
#endif
}

void NAND_PIORelease(void)
{

	//printk("[NAND] nand gpio_release\n");
	//gpio_release("nand_para",NULL);

}

void NAND_Memset(void* pAddr, unsigned char value, unsigned int len)
{
    memset(pAddr, value, len);
}

void NAND_Memcpy(void* pAddr_dst, void* pAddr_src, unsigned int len)
{
    memcpy(pAddr_dst, pAddr_src, len);
}

void* NAND_Malloc(unsigned int Size)
{
    return kmalloc(Size, GFP_KERNEL);
}

void NAND_Free(void *pAddr, unsigned int Size)
{
    kfree(pAddr);
}

int NAND_Print(const char * str, ...)
{
    printk(str);

    return 0;
}

void *NAND_IORemap(unsigned int base_addr, unsigned int size)
{
    return (void *)base_addr;
}

__u32 NAND_GetIOBaseAddrCH0(void)
{
	return 0xf1c03000;
}

__u32 NAND_GetIOBaseAddrCH1(void)
{
	return 0xf1c05000;
}

#ifdef __STANDBY_MODULE__
#include "pm_types.h"
#include "pm.h"

#elif defined(__KERNEL__)
#include <linux/delay.h>
#include "pm_i.h"
#endif

//#define CHECK_RESTORE_STATUS
static __ccmu_reg_list_t   *CmuReg;


/*
*********************************************************************************************************
*                           mem_clk_init
*
*Description: ccu init for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_save(struct clk_state *pclk_state)
{
	pclk_state->CmuReg = CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);

	/*backup clk src and ldo*/
	pclk_state->ccu_reg_back[0] = *(volatile __u32 *)&CmuReg->SysClkDiv;
	pclk_state->ccu_reg_back[1] = *(volatile __u32 *)&CmuReg->Apb2Div;
	pclk_state->ccu_reg_back[2] = *(volatile __u32 *)&CmuReg->Ahb1Div;

	/* backup pll registers and tuning?*/
	pclk_state->ccu_reg_back[3] = *(volatile __u32 *)&CmuReg->Pll1Ctl;
	pclk_state->ccu_reg_back[4] = *(volatile __u32 *)&CmuReg->Pll2Ctl;
	pclk_state->ccu_reg_back[5] = *(volatile __u32 *)&CmuReg->Pll3Ctl;
	pclk_state->ccu_reg_back[6] = *(volatile __u32 *)&CmuReg->Pll4Ctl;
	pclk_state->ccu_reg_back[7] = *(volatile __u32 *)&CmuReg->Pll5Ctl;
	pclk_state->ccu_reg_back[8] = *(volatile __u32 *)&CmuReg->Pll6Ctl;
	pclk_state->ccu_reg_back[9] = *(volatile __u32 *)&CmuReg->Pll7Ctl;

	/*backup axi, ahb, apb gating*/
	pclk_state->ccu_reg_back[10] = *(volatile __u32 *)&CmuReg->AxiGate;
	pclk_state->ccu_reg_back[11] = *(volatile __u32 *)&CmuReg->AhbGate0;
	pclk_state->ccu_reg_back[12] = *(volatile __u32 *)&CmuReg->AhbGate1;
	pclk_state->ccu_reg_back[13] = *(volatile __u32 *)&CmuReg->Apb1Gate;
	pclk_state->ccu_reg_back[14] = *(volatile __u32 *)&CmuReg->Apb2Gate;

	return 0;
}


/*
*********************************************************************************************************
*                           mem_clk_exit
*
*Description: ccu exit for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_restore(struct clk_state *pclk_state)
{
	/* initialise the CCU io base */
	CmuReg = pclk_state->CmuReg;

	/*restore clk src and ldo*/
	*(volatile __u32 *)&CmuReg->SysClkDiv    = pclk_state->ccu_reg_back[0];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->SysClkDiv = 0x%x, pclk_state->ccu_reg_back[0] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->SysClkDiv, pclk_state->ccu_reg_back[0]);
#endif

	*(volatile __u32 *)&CmuReg->Apb2Div  = pclk_state->ccu_reg_back[1];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->Apb2Div = 0x%x, pclk_state->ccu_reg_back[1] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->Apb2Div, pclk_state->ccu_reg_back[1]);
#endif

	*(volatile __u32 *)&CmuReg->Ahb1Div = pclk_state->ccu_reg_back[2];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->Ahb1Div = 0x%x, pclk_state->ccu_reg_back[2] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->Ahb1Div, pclk_state->ccu_reg_back[2]);
#endif

	/*restore axi, ahb, apb gating*/
	*(volatile __u32 *)&CmuReg->AxiGate    = pclk_state->ccu_reg_back[10];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->AxiGate = 0x%x, pclk_state->ccu_reg_back[10] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->AxiGate, pclk_state->ccu_reg_back[10]);
#endif

	*(volatile __u32 *)&CmuReg->AhbGate0   = pclk_state->ccu_reg_back[11];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->AhbGate0 = 0x%x, pclk_state->ccu_reg_back[11] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->AhbGate0, pclk_state->ccu_reg_back[11]);
#endif

	*(volatile __u32 *)&CmuReg->AhbGate1   = pclk_state->ccu_reg_back[12];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->AhbGate1 = 0x%x, pclk_state->ccu_reg_back[12] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->AhbGate1, pclk_state->ccu_reg_back[12]);
#endif

	*(volatile __u32 *)&CmuReg->Apb1Gate   = pclk_state->ccu_reg_back[13];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->Apb0Gate = 0x%x, pclk_state->ccu_reg_back[13] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->Apb1Gate, pclk_state->ccu_reg_back[13]);
#endif

	*(volatile __u32 *)&CmuReg->Apb2Gate   = pclk_state->ccu_reg_back[14];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->Apb1Gate = 0x%x, pclk_state->ccu_reg_back[14] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->Apb2Gate, pclk_state->ccu_reg_back[14]);
#endif

	/* restore pll registers and tuning? latency?*/
	//notice: do not touch pll1 and pll5
	//*(volatile __u32 *)&CmuReg->Pll1Ctl    = pclk_state->ccu_reg_back[3];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->Pll1Ctl = 0x%x, pclk_state->ccu_reg_back[3] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->Pll1Ctl, pclk_state->ccu_reg_back[3]);
#endif

	*(volatile __u32 *)&CmuReg->Pll2Ctl    = pclk_state->ccu_reg_back[4];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->Pll2Ctl = 0x%x, pclk_state->ccu_reg_back[4] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->Pll2Ctl, pclk_state->ccu_reg_back[4]);
#endif

	*(volatile __u32 *)&CmuReg->Pll3Ctl    = pclk_state->ccu_reg_back[5];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->Pll3Ctl = 0x%x, pclk_state->ccu_reg_back[5] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->Pll3Ctl, pclk_state->ccu_reg_back[5]);
#endif

	*(volatile __u32 *)&CmuReg->Pll4Ctl    = pclk_state->ccu_reg_back[6];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->Pll4Ctl = 0x%x, pclk_state->ccu_reg_back[6] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->Pll4Ctl, pclk_state->ccu_reg_back[6]);
#endif

	//*(volatile __u32 *)&CmuReg->Pll5Ctl    = pclk_state->ccu_reg_back[7];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->Pll5Ctl = 0x%x, pclk_state->ccu_reg_back[7] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->Pll5Ctl, pclk_state->ccu_reg_back[7]);
#endif

	*(volatile __u32 *)&CmuReg->Pll6Ctl    = pclk_state->ccu_reg_back[8];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->Pll6Ctl = 0x%x, pclk_state->ccu_reg_back[8] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->Pll6Ctl, pclk_state->ccu_reg_back[8]);
#endif

	*(volatile __u32 *)&CmuReg->Pll7Ctl    = pclk_state->ccu_reg_back[9];
#ifdef CHECK_RESTORE_STATUS
	printk("*(volatile __u32 *)&CmuReg->Pll7Ctl = 0x%x, pclk_state->ccu_reg_back[9] = 0x%x. \n", \
		*(volatile __u32 *)&CmuReg->Pll7Ctl, pclk_state->ccu_reg_back[9]);
#endif

	//is this neccessary?
	// mdelay(2);

	/* config the CCU to default status */
	//needed?
#if 0
	if(MAGIC_VER_C == sw_get_ic_ver()) {
		/* switch PLL4 to PLL6 */
		#if(USE_PLL6M_REPLACE_PLL4)
		CmuReg->VeClk.PllSwitch = 1;
		#else
		CmuReg->VeClk.PllSwitch = 0;
		#endif
	}
#endif

	return 0;
}


/*
*********************************************************************************************************
*                           mem_clk_init
*
*Description: ccu init for platform mem
*
*Arguments  : none
*
*Return     : result,
*
*Notes      :
*
*********************************************************************************************************
*/
__s32 mem_clk_init(void)
{
    CmuReg = (__ccmu_reg_list_t *)IO_ADDRESS(AW_CCM_BASE);

    return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_setdiv
*
* Description: switch core clock to 32k low osc.
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_setdiv(struct clk_div_t *clk_div)
{
	if(!clk_div)
	{
	return -1;
	}

	CmuReg->SysClkDiv.AXIClkDiv = clk_div->axi_div;
	//CmuReg->SysClkDiv.AHBClkDiv = clk_div->ahb_div;
	//CmuReg->SysClkDiv.APB0ClkDiv = clk_div->apb_div;

	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_getdiv
*
* Description: switch core clock to 32k low osc.
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/
__s32 mem_clk_getdiv(struct clk_div_t  *clk_div)
{
	if(!clk_div)
	{
	return -1;
	}

	clk_div->axi_div = CmuReg->SysClkDiv.AXIClkDiv;
	//clk_div->ahb_div = CmuReg->SysClkDiv.AHBClkDiv;
	//clk_div->apb_div = CmuReg->SysClkDiv.APB0ClkDiv;

	return 0;
}


/*
*********************************************************************************************************
*                                     mem_clk_set_pll_factor
*
* Description: set pll factor, target cpu freq is ?M hz
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_set_pll_factor(struct pll_factor_t *pll_factor)
{

	CmuReg->Pll1Ctl.FactorN = pll_factor->FactorN;
	CmuReg->Pll1Ctl.FactorK = pll_factor->FactorK;
	CmuReg->Pll1Ctl.FactorM = pll_factor->FactorM;
	//CmuReg->Pll1Ctl.PLLDivP = pll_factor->FactorP;

	//busy_waiting();

	return 0;
}

/*
*********************************************************************************************************
*                                     mem_clk_get_pll_factor
*
* Description: get pll factor
*
* Arguments  : none
*
* Returns    : 0;
*********************************************************************************************************
*/

__s32 mem_clk_get_pll_factor(struct pll_factor_t *pll_factor)
{
	pll_factor->FactorN = CmuReg->Pll1Ctl.FactorN;
	pll_factor->FactorK = CmuReg->Pll1Ctl.FactorK;
	pll_factor->FactorM = CmuReg->Pll1Ctl.FactorM;
	//pll_factor->FactorP = CmuReg->Pll1Ctl.PLLDivP;

	//busy_waiting();

	return 0;
}

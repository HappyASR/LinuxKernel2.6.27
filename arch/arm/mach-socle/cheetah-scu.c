/********************************************************************************
* File Name     : arch/arm/mach-socle/cheetah-scu.c 
* Author        : ryan chen
* Description   : Socle System Control Unit Driver (SCU)
* 
* Copyright (C) SQ Tech. Corp.
* This program is free software; you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by the Free Software Foundation; 
* either version 2 of the License, or (at your option) any later version. 
* This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or 
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
* You should have received a copy of the GNU General Public License 
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
    
*   Version      : 2,0,0,1
*   History      : 
*      1. 2007/01/25 ryan chen create this file
*    
********************************************************************************/

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/module.h>

#include <mach/hardware.h>
#include <mach/platform.h>
#include <asm/io.h>

#include <mach/regs-cheetah-scu.h>
#include <mach/cheetah-scu.h>

static struct socle_clock_st {
	unsigned long cpu_clock;
	unsigned long ahb_clock;
	unsigned long apb_clock;
	unsigned long uart_clock;
}socle_clock;

//#define EXT_OSC				16			// Ext OSC (MHz)
#define EXT_OSC				SCU_XIN			// Ext OSC (MHz)
#define UPLL_XIN				12			// UART Ext OSC (MHz)


extern unsigned long apb_clock;
//#define CONFIG_CHEETAH_SCU_DEBUG
#ifdef CONFIG_CHEETAH_SCU_DEBUG
	#define SCU_DBG(fmt, args...) printk(KERN_DEBUG "CHEETAH_SCU: " fmt, ## args)
#else
	#define SCU_DBG(fmt, args...)
#endif

static DEFINE_SPINLOCK(scu_lock);

static inline u32 socle_scu_read(u32 reg)
{
	u32 val;
	val = ioread32(SOCLE_CHEETAH_SCU_BASE + reg);
	return val;
}

static inline void socle_scu_write(u32 val, u32 reg)
{
	iowrite32(val, SOCLE_CHEETAH_SCU_BASE + reg);
}

extern u32 scu_check_amba_mode(void)
{
	if (socle_scu_read(CHEETAH_SCU_DEVCON) & AMBA_MODE)
		return 1;
	else
		return 0;
}

extern unsigned long __init 
get_pll_clock(void)
{
	unsigned long scu_clock, scu_ratio, scu_n, scu_m, scu_od, normal,tmp;

	/*	get PLL clock */
	normal = socle_scu_read(CHEETAH_SCU_PWMCON);
	if(normal & 0x1)
	{
		tmp = socle_scu_read(CHEETAH_SCU_MPLLCON);
		scu_n = (tmp & SCU_MPLLCON_N) >> SCU_MPLLCON_N_S;
		scu_m = (tmp & SCU_MPLLCON_M) >> SCU_MPLLCON_M_S;
		scu_od = (tmp & SCU_MPLLCON_OD) >> SCU_MPLLCON_OD_S;
		if(scu_od == 0)
			scu_od = 1;
		else if (scu_od == 1)
			scu_od = 2;
		else if (scu_od == 2)
			scu_od = 4;		
		else
			scu_od = 8;
		
		scu_clock = SCU_XIN * scu_m / scu_n / scu_od;
	}
	else
	{
		scu_clock = SCU_XIN;
	}
	
	/*	get scu ratio */
	tmp = socle_scu_read(CHEETAH_SCU_MCLKDIV);
	scu_ratio = (tmp & SCU_MCLKDIV_RATIO) >> SCU_MCLKDIV_RATIO_S;
	switch(scu_ratio){
		case 0 :		//[1:1]
			scu_ratio = 1;
			break;
		case 1 :		//[2:1]
			scu_ratio = 2;
			break;
		case 2 :		//[3:1]
			scu_ratio = 3;
			break;
		case 3 :		//[4:1]
			scu_ratio = 4;
			break;
		case 4 :		//[8:1]
			scu_ratio = 8;
			break;
	}

	apb_clock = scu_clock/scu_ratio/2;
	printk("CDK CPU = %ld MHz, HCLCK = %ld MHz \n",scu_clock/1000000,scu_clock/scu_ratio/1000000);
	
	return (apb_clock);
}

static int
select_amba_clock(int en, unsigned long arg)
{
	if (!((99 == arg) || ((23 >= arg) && (0 <= arg))))
		return -EINVAL;

	spin_lock(scu_lock);

	if (en) {
		if (99 == arg) {
			// enable all ip clocks of amba
			socle_scu_write(0xffffffff, CHEETAH_SCU_MCLKEN);
		} else {
			// enable ip clock of amba
			socle_scu_write(socle_scu_read(CHEETAH_SCU_MCLKEN) | (0x1 << arg),CHEETAH_SCU_MCLKEN);
		}
	} else {
		if (99 == arg) {
			// disable all ip clocks of amba
			socle_scu_write(0,CHEETAH_SCU_MCLKEN);
		} else {
			// disable ip clock of amba
			socle_scu_write(socle_scu_read(CHEETAH_SCU_MCLKEN) & ~(0x1 << arg), CHEETAH_SCU_MCLKEN);
		}
	}

	spin_unlock(scu_lock);

	return 0;
}

static int
select_ap_clock(int en, unsigned long arg)
{
	if (!((99 == arg) || ((12 >= arg) && (0 <= arg))))
		return -EINVAL;

	spin_lock(scu_lock);

	if (en) {
		if (99 == arg) {
			// enable all ip clocks of ap
			socle_scu_write(0xffffffff,CHEETAH_SCU_ACLKEN);
		} else {
			// enable ip clock of ap
			socle_scu_write(socle_scu_read(CHEETAH_SCU_ACLKEN) | (0x1 << arg),CHEETAH_SCU_ACLKEN);
		}
	} else {
		if (99 == arg) {
			// disable all ip clocks of ap
			socle_scu_write(0x0,CHEETAH_SCU_ACLKEN);
		} else {
			// disable ip clock of ap
			socle_scu_write(socle_scu_read(CHEETAH_SCU_ACLKEN) & ~(0x1 << arg),CHEETAH_SCU_ACLKEN);
		}
	}

	spin_unlock(scu_lock);

	return 0;
}

static void
select_power_mode(int mode)
{
	spin_lock(scu_lock);

	if (0 == mode) {			// CHEETAH_SCU_IOC_SLOW_MODE
		socle_scu_write(0x0,CHEETAH_SCU_PWMCON);
	} else if (1 == mode) {		// CHEETAH_SCU_IOC_NORMAL_MODE
		socle_scu_write(0x1 << 0,CHEETAH_SCU_PWMCON);
	} else if (2 == mode) {		// CHEETAH_SCU_IOC_IDLE_MODE
		socle_scu_write(CHEETAH_SCU_PWMCON, socle_scu_read(CHEETAH_SCU_PWMCON) | (0x1 << 2));
	} else {
		printk("Error! SCU does not support mode = %d\n", mode);
	}

	spin_unlock(scu_lock);
}

// ahb clock control 
extern void 
socle_scu_ahb_clk_enable(u32 dev)
{
	socle_scu_write(dev | socle_scu_read(CHEETAH_SCU_MCLKEN), CHEETAH_SCU_MCLKEN);
}

extern void 
socle_scu_ahb_clk_disable(u32 dev)
{
	socle_scu_write(~dev & socle_scu_read(CHEETAH_SCU_MCLKEN), CHEETAH_SCU_MCLKEN);
}

// application clock control 
extern void 
socle_scu_app_clk_enable(u32 dev)
{
	socle_scu_write(dev | socle_scu_read(CHEETAH_SCU_ACLKEN), CHEETAH_SCU_ACLKEN);
}

extern void 
socle_scu_app_clk_disable(u32 dev)
{
	socle_scu_write(~dev & socle_scu_read(CHEETAH_SCU_ACLKEN), CHEETAH_SCU_ACLKEN);
}

// device control 
extern void 
socle_scu_dev_enable(u32 dev)
{
	socle_scu_write(dev | socle_scu_read(CHEETAH_SCU_DEVCON), CHEETAH_SCU_DEVCON);
}

extern void 
socle_scu_dev_disable(u32 dev)
{
	socle_scu_write(~dev & socle_scu_read(CHEETAH_SCU_DEVCON), CHEETAH_SCU_DEVCON);
}

//=================================================
extern int 
socle_scu_cpll_power_down_status(void)		//0:power-down , 1:Active
{
	u32 tmp;

	tmp = socle_scu_read(CHEETAH_SCU_MPLLCON) & SCU_MPLLCOM_PLL_ROWER_DOWN;

	if(SCU_MPLLCOM_PLL_ROWER_DOWN == tmp)
		return 0;
	else
		return 1;
}

extern int 
socle_scu_clock_ratio_get(void)
{
	u32 tmp;
	int ratio;

	tmp = (socle_scu_read(CHEETAH_SCU_MCLKDIV) & SCU_MCLKDIV_CLK_RATIO_M) >> SCU_MCLKDIV_CLK_RATIO_S;

	switch(tmp){
		case SCU_MCLKDIV_CLK_RATIO_1_1 :
			ratio = 1;
			break;
		case SCU_MCLKDIV_CLK_RATIO_2_1 :
			ratio = 2;
			break;
		case SCU_MCLKDIV_CLK_RATIO_3_1 :
			ratio = 3;
			break;
		case SCU_MCLKDIV_CLK_RATIO_4_1 :
			ratio = 4;
			break;
		case SCU_MCLKDIV_CLK_RATIO_8_1 :
			ratio = 8;
			break;
		default :
			printk("unknow ratio value\n");
			return -1;
			break;
	}		
	return ratio;
}

static int 
socle_scu_pll_formula (int n, int m, int od, int type)
{
	int clock, no;

	no = 0x1 << od ;

	if(1 == type)		//CPLL
		clock = EXT_OSC * m / n / no ;
	else
		clock = UPLL_XIN * m / n / no ;
	
	return clock;
}

extern int 
socle_scu_cpll_get(void)			//return cpll clock value
{
	int m,n,od;
	u32 clk;

	clk = socle_scu_read(CHEETAH_SCU_MPLLCON);
	
	n = (clk & SCU_E0PLLCON_INPUT_DIVIDER_M) >> SCU_E0PLLCON_INPUT_DIVIDER_S;
	m = (clk & SCU_E0PLLCON_FEEDBACK_DIVIDER_M) >> SCU_E0PLLCON_FEEDBACK_DIVIDER_S;
	od = (clk & SCU_E0PLLCON_OUTPUT_DIVIDER_M) >> SCU_E0PLLCON_OUTPUT_DIVIDER_S;

	clk = socle_scu_pll_formula(n, m, od, 1);

	return clk;
}

extern unsigned long
socle_get_cpu_clock(void)
{	
	/*	get power mode */
	if(0 == socle_scu_cpll_power_down_status())
		socle_clock.cpu_clock = EXT_OSC * 1000 * 1000;		/* power down */
	else
		socle_clock.cpu_clock = socle_scu_cpll_get() * 1000 * 1000;
				
	return socle_clock.cpu_clock ;
}

extern unsigned long
socle_get_ahb_clock(void)
{
	int ratio;
	
	ratio = socle_scu_clock_ratio_get();

	//20080407 leonid fix for ahb clock
	socle_clock.ahb_clock = socle_get_cpu_clock() / ratio;
	
	return socle_clock.ahb_clock;
}

extern unsigned long
socle_get_apb_clock(void)
{
	//20080407 leonid fix for apb clock
	socle_clock.apb_clock = socle_get_ahb_clock() / 2;	
	
	return socle_clock.apb_clock;
}

EXPORT_SYMBOL(socle_get_apb_clock);

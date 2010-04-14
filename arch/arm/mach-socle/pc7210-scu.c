/********************************************************************************
* File Name     : arch/arm/mach-socle/pc7210-scu.c 
* Author        : Leonid
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
*      1. 2007/01/25 Leonid create this file
*    
********************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include <mach/platform.h>

#include <mach/regs-pc7210-scu.h>
#include <mach/pc7210-scu.h>

extern unsigned long apb_clock;
//#define CONFIG_CHEETAH_SCU_DEBUG
#ifdef CONFIG_PC7210_SCU_DEBUG
	#define SCU_DBG(fmt, args...) printk(KERN_DEBUG "PC7210_SCU: " fmt, ## args)
#else
	#define SCU_DBG(fmt, args...)
#endif

//static DEFINE_SPINLOCK(scu_lock);

static struct socle_clock_st {
	unsigned long cpu_clock;
	unsigned long ahb_clock;
	unsigned long apb_clock;
	unsigned long uart_clock;
}socle_clock;

#define SOCLE_SCU_BASE	SOCLE_APB0_SCU

static inline void
socle_scu_write(u32 value, u32 reg) 
{
	iowrite32(value, SOCLE_SCU_BASE+reg);
}

static inline u32
socle_scu_read(u32 reg)
{
	return ioread32(SOCLE_SCU_BASE+reg);
}

static inline void
socle_scu_show(char *info)
{
	printk("%s", info);
}

	/* read chip ID	*/
extern u32 
socle_scu_chip_id (void)
{
	u32 tmp;

	tmp = socle_scu_read(SOCLE_SCU_P7CID);

	return tmp;
}

static unsigned long socle_scu_pll_formula (int m, int n, int od, int type);

static unsigned long 
socle_scu_pll_formula (int m, int n, int od, int type)
{
	int clock, no;

	no = 0x1 <<  (((od >> 1) & 0x1) + (od & 0x1)) ;

	if(1 == type)		//CPLL
		clock = EXT_OSC * (m +2) / (n+2) / (no) ;
	else
		clock = UPLL_XIN * (m +2) / (n+2) / (no) ;
	
	return clock;
}

	/* UPLL configuration */
extern int 
socle_scu_upll_set (int clock)
{
	u32 tmp, upll;

	switch(clock){
		case SOCLE_SCU_UART_CLOCK_88 :			
			tmp = SCU_UART_CLOCK_88;
			break;
		default :		
			socle_scu_show("unknow upll clock !!\n");
			return -1;
			break;
	}

	upll = ((socle_scu_read(SCU_PLLPARAM_A) & ~SCU_PLLPARAM_A_UPLL_M) 
			| (tmp << SCU_PLLPARAM_A_UPLL_S));
	
	socle_scu_write(upll, SCU_PLLPARAM_A);
	
	return 0;
}		

extern int 
socle_scu_upll_get (void)
{
	int m,n,od;
	u32 uclk;

	uclk = socle_scu_read(SCU_PLLPARAM_A);
	
	m = (uclk & SCU_PLLPARAM_A_UPLL_M_M) >> SCU_PLLPARAM_A_UPLL_M_S;
	n = (uclk & SCU_PLLPARAM_A_UPLL_N_M) >> SCU_PLLPARAM_A_UPLL_N_S;
	od = (uclk & SCU_PLLPARAM_A_UPLL_OD_M) >> SCU_PLLPARAM_A_UPLL_OD_S;

	uclk = socle_scu_pll_formula(m, n, od, 0);

	return uclk;
}

	/* CPLL configuration */
extern int 
socle_scu_cpll_set (int clock)
{
	u32 tmp, cpll;
	
	switch(clock){
		case SOCLE_SCU_CPU_CLOCK_33 :			
			tmp = SCU_CPU_CLOCK_33;
			break;
		case SOCLE_SCU_CPU_CLOCK_40 :			
			tmp = SCU_CPU_CLOCK_40;
			break;
		case SOCLE_SCU_CPU_CLOCK_50 :			
			tmp = SCU_CPU_CLOCK_50;
			break;
		case SOCLE_SCU_CPU_CLOCK_66 :			
			tmp = SCU_CPU_CLOCK_66;
			break;
		case SOCLE_SCU_CPU_CLOCK_83 :			
			tmp = SCU_CPU_CLOCK_83;
			break;
		case SOCLE_SCU_CPU_CLOCK_90 :			
			tmp = SCU_CPU_CLOCK_90;
			break;
		case SOCLE_SCU_CPU_CLOCK_100 :			
			tmp = SCU_CPU_CLOCK_100;
			break;
		case SOCLE_SCU_CPU_CLOCK_120 :			
			tmp = SCU_CPU_CLOCK_120;
			break;
		case SOCLE_SCU_CPU_CLOCK_132 :			
			tmp = SCU_CPU_CLOCK_132;
			break;
		case SOCLE_SCU_CPU_CLOCK_133 :			
			tmp = SCU_CPU_CLOCK_133;
			break;
		case SOCLE_SCU_CPU_CLOCK_166 :			
			tmp = SCU_CPU_CLOCK_166;
			break;
		default :		
			socle_scu_show("unknow upll clock !!\n");
			return -1;
			break;
	}

	cpll = ((socle_scu_read(SCU_PLLPARAM_A) & ~SCU_PLLPARAM_A_CPLL_M) 
			| (tmp << SCU_PLLPARAM_A_CPLL_S));

	socle_scu_write(cpll, SCU_PLLPARAM_A);

	socle_get_cpu_clock();	

	return 0;
}
	
extern int 
socle_scu_cpll_get (void)
{
	int m,n,od;
	u32 clk;

	clk = socle_scu_read(SCU_PLLPARAM_A);
	
	m = (clk & SCU_PLLPARAM_A_CPLL_M_M) >> SCU_PLLPARAM_A_CPLL_M_S;
	n = (clk & SCU_PLLPARAM_A_CPLL_N_M) >> SCU_PLLPARAM_A_CPLL_N_S;
	od = (clk & SCU_PLLPARAM_A_CPLL_OD_M) >> SCU_PLLPARAM_A_CPLL_OD_S;

	clk = socle_scu_pll_formula(m, n, od, 1);

	return clk;
}

	/* UPLL power down/normal	*/
extern void 
socle_scu_upll_normal (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLLPARAM_B) & ~SCU_PLLPARAM_B_UPLL_POWER_DOWN;

	socle_scu_write(tmp, SCU_PLLPARAM_B);
	
	return ;
}
	
extern void 
socle_scu_upll_power_down (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLLPARAM_B) | SCU_PLLPARAM_B_UPLL_POWER_DOWN;

	socle_scu_write(tmp, SCU_PLLPARAM_B);
	
	return ;
}

extern int 
socle_scu_upll_status_get (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLLPARAM_B) & SCU_PLLPARAM_B_UPLL_POWER_DOWN;

	if(SCU_PLLPARAM_B_UPLL_POWER_DOWN == tmp)
		return 0;
	else
		return 1;
}

	/* CPLL power down/normal	*/
extern void 
socle_scu_cpll_normal (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLLPARAM_B) & ~SCU_PLLPARAM_B_CPLL_POWER_DOWN;

	socle_scu_write(tmp, SCU_PLLPARAM_B);
	
	return ;
}
	
extern void 
socle_scu_cpll_power_down (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLLPARAM_B) | SCU_PLLPARAM_B_CPLL_POWER_DOWN;

	socle_scu_write(tmp, SCU_PLLPARAM_B);
	
	return ;
}

extern int 
socle_scu_cpll_status_get (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLLPARAM_B) & SCU_PLLPARAM_B_CPLL_POWER_DOWN;

	if(SCU_PLLPARAM_B_CPLL_POWER_DOWN == tmp)
		return 0;
	else
		return 1;
}
	
	/* CPU/AHB clock ratio	*/
extern int 
socle_scu_clock_ratio_set (int ratio)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLLPARAM_B) & ~SCU_PLLPARAM_B_CLK_RATIO_M;

	switch(ratio){
		case SOCLE_SCU_CLOCK_RATIO_1_1 :
			tmp = tmp |SCU_PLLPARAM_B_CLK_RATIO_1_1;
			break;
		case SOCLE_SCU_CLOCK_RATIO_2_1 :
			tmp = tmp |SCU_PLLPARAM_B_CLK_RATIO_2_1;
			break;
		case SOCLE_SCU_CLOCK_RATIO_3_1 :
			tmp = tmp |SCU_PLLPARAM_B_CLK_RATIO_3_1;
			break;
		case SOCLE_SCU_CLOCK_RATIO_4_1 :
			tmp = tmp |SCU_PLLPARAM_B_CLK_RATIO_4_1;
			break;
		default :
			socle_scu_show("unknow ratio value\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_PLLPARAM_B);
	
	return 0;
}
	
extern int 
socle_scu_clock_ratio_get (void)
{
	u32 tmp;
	int ratio;

	tmp = socle_scu_read(SCU_PLLPARAM_B) & SCU_PLLPARAM_B_CLK_RATIO_M;

	switch(tmp){
		case SCU_PLLPARAM_B_CLK_RATIO_1_1 :
			ratio = 1;
			break;
		case SCU_PLLPARAM_B_CLK_RATIO_2_1 :
			ratio = 2;
			break;
		default :
			socle_scu_show("unknow ratio value\n");
			return -1;
			break;
	}	
	
	return ratio;
}
	
	/* PLL lock period	*/
extern void 
socle_scu_pll_lock_period_set (int period)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLLPARAM_B) & ~SCU_PLLPARAM_B_PLL_LOCK_PERIOD_M;
	tmp = tmp | period;
	socle_scu_write(tmp, SCU_PLLPARAM_B);
	
	return ;
}	
	
	/*	Force USB PHY's PLL Powered in Suspend	*/
extern void 
socle_scu_usb_pll_powered (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_USB_PLL_POWER_SAVE;

	socle_scu_write(tmp, SCU_CHIPCFG_A);
	
	return ;
}
	
extern void 
socle_scu_usb_pll_power_save (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) | SCU_CHIPCFG_A_USB_PLL_POWER_SAVE;

	socle_scu_write(tmp, SCU_CHIPCFG_A);
	
	return ;
}
	
	/*	DCFG MODE	*/
extern void 
socle_scu_dcfg_mode_set (int mode)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_DCFG_MODE_M;
	tmp = tmp | (mode << SCU_CHIPCFG_A_DCFG_MODE_S);
	socle_scu_write(tmp, SCU_CHIPCFG_A);
	
	return ;
}

extern int 
socle_scu_dcfg_mode_get (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) & SCU_CHIPCFG_A_DCFG_MODE_M;

	return (tmp >> 16);
}
	
	/*	UCFG MODE	*/
extern void 
socle_scu_ucfg_mode_set (int mode)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_UCFG_MODE_M;
	tmp = tmp | (mode << SCU_CHIPCFG_A_UCFG_MODE_S);
	socle_scu_write(tmp, SCU_CHIPCFG_A);
	
	return ;
}

extern int 
socle_scu_ucfg_mode_get (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) & SCU_CHIPCFG_A_UCFG_MODE_M;

	return (tmp>>13);
}

	/*	POWER SAVE MODE	*/
extern void 
socle_scu_sleep_mode_set (void)
{
	//Enable FIQ
	socle_scu_write(socle_scu_read(SCU_CHIPCFG_A) | SCU_CHIPCFG_A_FIRQ_FUNCTION, SCU_CHIPCFG_A);
	socle_scu_write(0xdeedbabe,SCU_REMAP);
	
	return ;
}

extern void 
socle_scu_stop_mode_set (void)
{
	//Enable FIQ
	socle_scu_write(socle_scu_read(SCU_CHIPCFG_A) | SCU_CHIPCFG_A_FIRQ_FUNCTION, SCU_CHIPCFG_A);
	socle_scu_write(0xdeedcafe,SCU_REMAP);	
	return ;
}
	
	/*	Config UARTx as irDA function	*/
extern int 
socle_scu_irda_enable (int uart)
{
	u32 tmp;

	switch(uart){
		case 3 :
			tmp = socle_scu_read(SCU_CHIPCFG_A) | SCU_CHIPCFG_A_UART3_IRDA;
			break;
		case 2 :
			tmp = socle_scu_read(SCU_CHIPCFG_A) | SCU_CHIPCFG_A_UART2_IRDA;
			break;
		case 1 :
			tmp = socle_scu_read(SCU_CHIPCFG_A) | SCU_CHIPCFG_A_UART1_IRDA;
			break;
		case 0 :
			tmp = socle_scu_read(SCU_CHIPCFG_A) | SCU_CHIPCFG_A_UART0_IRDA;
			break;
		default : 
			socle_scu_show("unknow uart number\n");
			return -1;
			break;
	}

	socle_scu_write(tmp, SCU_CHIPCFG_A);

	return 0;
}
	
	/*	Config UARTx as UART function	*/
extern int 
socle_scu_irda_disable (int uart)
{
	u32 tmp;

	switch(uart){
		case 3 :
			tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_UART3_IRDA;
			break;
		case 2 :
			tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_UART2_IRDA;
			break;
		case 1 :
			tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_UART1_IRDA;
			break;
		case 0 :
			tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_UART0_IRDA;
			break;
		default : 
			socle_scu_show("unknow uart number\n");
			return -1;
			break;
	}

	socle_scu_write(tmp, SCU_CHIPCFG_A);

	return 0;
}
	
	/*	select which UART will occupy HDMA request 2/3 for UART Tx/Rx	*/
extern int 
socle_scu_hdma_req23_uart (int uart)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_HDMA_REQ23_M;
	tmp = tmp | (uart << SCU_CHIPCFG_A_HDMA_REQ23_S);
	socle_scu_write(tmp, SCU_CHIPCFG_A);

	return 0;
}
	
	/*	select which UART will occupy HDMA request 0/1 for UART Tx/Rx	*/
extern int 
socle_scu_hdma_req01_uart (int uart)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_HDMA_REQ01_M;
	tmp = tmp | (uart << SCU_CHIPCFG_A_HDMA_REQ01_S);
	socle_scu_write(tmp, SCU_CHIPCFG_A);

	return 0;
}
	
	/*	select USB Tranceiver play Downstream or Upstream	*/
extern void 
socle_scu_usb_tranceiver_downstream (void)		//UHC
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) | SCU_CHIPCFG_A_USB_TRAN_DOWN_STREAM;
	socle_scu_write(tmp, SCU_CHIPCFG_A);	
	
	return ;
}
	
extern void 
socle_scu_usb_tranceiver_upstream (void)		//UDC
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_USB_TRAN_DOWN_STREAM;
	socle_scu_write(tmp, SCU_CHIPCFG_A);	
	
	return ;
}
	
	/*	select fast IRQ polarity	*/
extern void 
socle_scu_fast_irq_active_high (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) | SCU_CHIPCFG_A_FIRQ_POLARITY_HIGH;
	socle_scu_write(tmp, SCU_CHIPCFG_A);	
	
	return ;
}
	
extern void 
socle_scu_fast_irq_active_low (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_FIRQ_POLARITY_HIGH;
	socle_scu_write(tmp, SCU_CHIPCFG_A);	
	
	return ;
}
	
	/*	select USB port over current function polarity	*/
extern void 
socle_scu_usb_port_over_current_active_high (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) | SCU_CHIPCFG_A_USB_OVER_CURR_POLARITY_HIGH;
	socle_scu_write(tmp, SCU_CHIPCFG_A);	
	
	return ;
}
	
extern void 
socle_scu_usb_port_over_current_active_low (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_USB_OVER_CURR_POLARITY_HIGH;
	socle_scu_write(tmp, SCU_CHIPCFG_A);	
	
	return ;
}
	
	/*	select fast IRQ and USB port over current function	*/
extern void 
socle_scu_fast_irq_enable (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) | SCU_CHIPCFG_A_FIRQ_FUNCTION;
	socle_scu_write(tmp, SCU_CHIPCFG_A);	
	
	return ;
}
	
extern void 
socle_scu_usb_port_over_current_enable (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_A) & ~SCU_CHIPCFG_A_FIRQ_FUNCTION;
	socle_scu_write(tmp, SCU_CHIPCFG_A);	
	
	return ;
}
		
	/*	ADC clock duty period	*/
extern void 
socle_scu_adc_clock_duty_period (int period)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPCFG_B) & ~SCU_CHIPCFG_B_ADC_CLK_DUTY_PERIOD_M;
	tmp = tmp | period;
	socle_scu_write(tmp, SCU_CHIPCFG_B);	

	return;
}	
	/*	pclk enable/disable	*/
extern int 
socle_scu_pclk_enable (int ip)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CLKCFG) ;
	
	switch(ip){
		case SOCLE_SCU_ADCCLK :
			tmp = tmp | SCU_CLKCFG_ADCCLK_EN;
			break;
		case SOCLE_SCU_PCLK_UART3 :
			tmp = tmp | SCU_CLKCFG_PCLK_UART3_EN;
			break;
		case SOCLE_SCU_PCLK_UART2 :
			tmp = tmp | SCU_CLKCFG_PCLK_UART2_EN;
			break;
		case SOCLE_SCU_PCLK_UART1 :
			tmp = tmp | SCU_CLKCFG_PCLK_UART1_EN;
			break;
		case SOCLE_SCU_PCLK_UART0 :
			tmp = tmp | SCU_CLKCFG_PCLK_UART0_EN;
			break;
		case SOCLE_SCU_PCLK_ADC :
			tmp = tmp | SCU_CLKCFG_PCLK_ADC_EN;
			break;
		case SOCLE_SCU_PCLK_PWM :
			tmp = tmp | SCU_CLKCFG_PCLK_PWM_EN;
			break;
		case SOCLE_SCU_PCLK_SDC :
			tmp = tmp | SCU_CLKCFG_PCLK_SDC_EN;
			break;
		case SOCLE_SCU_PCLK_I2S :
			tmp = tmp | SCU_CLKCFG_PCLK_I2S_EN;
			break;
		case SOCLE_SCU_PCLK_I2C :
			tmp = tmp | SCU_CLKCFG_PCLK_I2C_EN;
			break;
		case SOCLE_SCU_PCLK_SPI :
			tmp = tmp | SCU_CLKCFG_PCLK_SPI_EN;
			break;
		case SOCLE_SCU_PCLK_GPIO :
			tmp = tmp | SCU_CLKCFG_PCLK_GPIO_EN;
			break;
		case SOCLE_SCU_PCLK_WDT :
			tmp = tmp | SCU_CLKCFG_PCLK_WDT_EN;
			break;
		case SOCLE_SCU_PCLK_RTC :
			tmp = tmp | SCU_CLKCFG_PCLK_RTC_EN;
			break;
		case SOCLE_SCU_PCLK_TMR :
			tmp = tmp | SCU_CLKCFG_PCLK_TMR_EN;
			break;
		case SOCLE_SCU_PCLK_LCD :
			tmp = tmp | SCU_CLKCFG_PCLK_LCD_EN;
			break;
		case SOCLE_SCU_PCLK_NFC :
			tmp = tmp | SCU_CLKCFG_PCLK_NFC_EN;
			break;
		case SOCLE_SCU_PCLK_UDC :
			tmp = tmp | SCU_CLKCFG_PCLK_UDC_EN;
			break;
		case SOCLE_SCU_PCLK_UHC :
			tmp = tmp | SCU_CLKCFG_PCLK_UHC_EN;
			break;
		case SOCLE_SCU_PCLK_MAC :
			tmp = tmp | SCU_CLKCFG_PCLK_MAC_EN;
			break;
		case SOCLE_SCU_PCLK_HDMA :
			tmp = tmp | SCU_CLKCFG_PCLK_HDMA_EN;
			break;
		case SOCLE_SCU_PCLK_SDRSTMC :
			tmp = tmp | SCU_CLKCFG_SDRSTMC_CLK_EN;
			break;
		default :
			socle_scu_show("unknow IP number\n");
			return -1;
			break;
	}
				
	socle_scu_write(tmp, SCU_CLKCFG);	
	
	return 0;
}
	
extern int 
socle_scu_pclk_disable (int ip)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CLKCFG) ;
	
	switch(ip){
		case SOCLE_SCU_ADCCLK :
			tmp = tmp & ~SCU_CLKCFG_ADCCLK_EN;
			break;
		case SOCLE_SCU_PCLK_UART3 :
			tmp = tmp & ~SCU_CLKCFG_PCLK_UART3_EN;
			break;
		case SOCLE_SCU_PCLK_UART2 :
			tmp = tmp & ~SCU_CLKCFG_PCLK_UART2_EN;
			break;
		case SOCLE_SCU_PCLK_UART1 :
			tmp = tmp & ~SCU_CLKCFG_PCLK_UART1_EN;
			break;
		case SOCLE_SCU_PCLK_UART0 :
			tmp = tmp & ~SCU_CLKCFG_PCLK_UART0_EN;
			break;
		case SOCLE_SCU_PCLK_ADC :
			tmp = tmp & ~SCU_CLKCFG_PCLK_ADC_EN;
			break;
		case SOCLE_SCU_PCLK_PWM :
			tmp = tmp & ~SCU_CLKCFG_PCLK_PWM_EN;
			break;
		case SOCLE_SCU_PCLK_SDC :
			tmp = tmp & ~SCU_CLKCFG_PCLK_SDC_EN;
			break;
		case SOCLE_SCU_PCLK_I2S :
			tmp = tmp & ~SCU_CLKCFG_PCLK_I2S_EN;
			break;
		case SOCLE_SCU_PCLK_I2C :
			tmp = tmp & ~SCU_CLKCFG_PCLK_I2C_EN;
			break;
		case SOCLE_SCU_PCLK_SPI :
			tmp = tmp & ~SCU_CLKCFG_PCLK_SPI_EN;
			break;
		case SOCLE_SCU_PCLK_GPIO :
			tmp = tmp & ~SCU_CLKCFG_PCLK_GPIO_EN;
			break;
		case SOCLE_SCU_PCLK_WDT :
			tmp = tmp & ~SCU_CLKCFG_PCLK_WDT_EN;
			break;
		case SOCLE_SCU_PCLK_RTC :
			tmp = tmp & ~SCU_CLKCFG_PCLK_RTC_EN;
			break;
		case SOCLE_SCU_PCLK_TMR :
			tmp = tmp & ~SCU_CLKCFG_PCLK_TMR_EN;
			break;
		case SOCLE_SCU_PCLK_LCD :
			tmp = tmp & ~SCU_CLKCFG_PCLK_LCD_EN;
			break;
		case SOCLE_SCU_PCLK_NFC :
			tmp = tmp & ~SCU_CLKCFG_PCLK_NFC_EN;
			break;
		case SOCLE_SCU_PCLK_UDC :
			tmp = tmp & ~SCU_CLKCFG_PCLK_UDC_EN;
			break;
		case SOCLE_SCU_PCLK_UHC :
			tmp = tmp & ~SCU_CLKCFG_PCLK_UHC_EN;
			break;
		case SOCLE_SCU_PCLK_MAC :
			tmp = tmp & ~SCU_CLKCFG_PCLK_MAC_EN;
			break;
		case SOCLE_SCU_PCLK_HDMA :
			tmp = tmp & ~SCU_CLKCFG_PCLK_HDMA_EN;
			break;
		case SOCLE_SCU_PCLK_SDRSTMC :
			tmp = tmp & ~SCU_CLKCFG_SDRSTMC_CLK_EN;
			break;
		default :
			socle_scu_show("unknow IP number\n");
			return -1;
			break;
	}
				
	socle_scu_write(tmp, SCU_CLKCFG);	
	
	return 0;
}
	
	/*	SDRAM data bus width status	*/
extern int 
socle_scu_sdram_bus_width_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_SDRAM_BUS_WIDTH_STATUS_M;

	if(tmp == SCU_REMAP_SDRAM_BUS_WIDTH_STATUS_32)
		return SOCLE_SCU_SDRAM_BUS_WIDTH_32;
	else
		return SOCLE_SCU_SDRAM_BUS_WIDTH_16;
	
}
	
	/*	MAC Tx process stop status	*/
extern int 
socle_scu_mac_tx_stop_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_TPS_MAC_STATUS_M;
	tmp = tmp >> SCU_REMAP_TPS_MAC_STATUS_S;

	return tmp;
	
}
	
	/*	MAC Rx process stop status	*/
extern int 
socle_scu_mac_rx_stop_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_RPS_MAC_STATUS_M;
	tmp = tmp >> SCU_REMAP_RPS_MAC_STATUS_S;

	return tmp;
}
	
	/*	UCFG Mode6 status	*/
extern int 
socle_scu_ucfg_mode6_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_UCFG_MODE6_STATUS_M;
	tmp = tmp >> SCU_REMAP_UCFG_MODE6_STATUS_S;

	return tmp;
}
	
	/*	UCFG Mode5 status	*/
extern int 
socle_scu_ucfg_mode5_status(void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_UCFG_MODE5_STATUS_M;
	tmp = tmp >> SCU_REMAP_UCFG_MODE5_STATUS_S;

	return tmp;
}
	
	/*	UCFG Mode4 status	*/
extern int 
socle_scu_ucfg_mode4_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_UCFG_MODE4_STATUS_M;
	tmp = tmp >> SCU_REMAP_UCFG_MODE4_STATUS_S;

	return tmp;
}
	
	/*	UCFG Mode3 status	*/
extern int 
socle_scu_ucfg_mode3_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_UCFG_MODE3_STATUS_M;
	tmp = tmp >> SCU_REMAP_UCFG_MODE3_STATUS_S;

	return tmp;
}
	
	/*	UCFG Mode2 status	*/
extern int 
socle_scu_ucfg_mode2_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_UCFG_MODE2_STATUS_M;
	tmp = tmp >> SCU_REMAP_UCFG_MODE2_STATUS_S;

	return tmp;
}
	
	/*	UCFG Mode1 status	*/
extern int 
socle_scu_ucfg_mode1_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_UCFG_MODE1_STATUS_M;
	tmp = tmp >> SCU_REMAP_UCFG_MODE1_STATUS_S;

	return tmp;
}
	
	/*	Boot source selection status	*/
extern int 
socle_scu_boot_source_status (void)
{
	u32 tmp, status=0;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_BOOT_SRC_STATUS_M;

	switch(tmp){
		case SCU_REMAP_BOOT_SRC_STATUS_NOR_16 :
			status = SOCLE_SCU_BOOT_NOR_16;
			break;
		case SCU_REMAP_BOOT_SRC_STATUS_NOR_8 :
			status = SOCLE_SCU_BOOT_NOR_8;
			break;
		case SCU_REMAP_BOOT_SRC_STATUS_NAND :
			status = SOCLE_SCU_BOOT_NAND;
			break;
		case SCU_REMAP_BOOT_SRC_STATUS_ISP_ROM:
			status = SOCLE_SCU_BOOT_ISP_ROM;
			break;
	}
		return status;
		
}
	
	/*	FIQDIS from ARM7 status	*/
extern int 
socle_scu_fiq_dis_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_FIQ_DIS_SATUS_M;
	tmp = tmp >> SCU_REMAP_FIQ_DIS_SATUS_S;

	return tmp;
}
	
	/*	IRQDIS from ARM7 status	*/
extern int 
socle_scu_irq_dis_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_IRQ_DIS_SATUS_M;
	tmp = tmp >> SCU_REMAP_IRQ_DIS_SATUS_S;

	return tmp;
}
	
	/*	auto boot fail indicator from NFC status	*/
extern int 
socle_scu_nand_boot_fail_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_NFC_AUTO_BOOT_FAIL_SATUS_M;
	tmp = tmp >> SCU_REMAP_NFC_AUTO_BOOT_FAIL_SATUS_S;

	return tmp;
}
	
	/*	pll lock status	*/
extern int 
socle_scu_pll_lock_status (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_REMAP) & SCU_REMAP_PLL_LOCK_SATUS_M;
	tmp = tmp >> SCU_REMAP_PLL_LOCK_SATUS_S;

	return tmp;
}
	
	/*	stop mode -- systen clock	*/
extern void 
socle_scu_stop_mode_enable (void)
{
	socle_scu_write(SCU_REMAP_STOP_MODE_ENABLE, SCU_REMAP);

	return ;
}
	
extern void 
socle_scu_stop_mode_disable (void)
{
	socle_scu_write(SCU_REMAP_STOP_MODE_DISABLE, SCU_REMAP);

	return ;
}
	
	/*	sleep mode -- cpu clock	*/
extern void 
socle_scu_sleep_mode_enable (void)
{
	socle_scu_write(SCU_REMAP_SLEEP_MODE_ENABLE, SCU_REMAP);

	return ;
}
	
extern void 
socle_scu_sleep_mode_disable (void)
{
	socle_scu_write(SCU_REMAP_SLEEP_MODE_DISABLE, SCU_REMAP);

	return ;
}
	
	/*	normal mode -- use PLL clock or Base clock	*/
extern void 
socle_scu_normal_mode_enable (void)
{
	socle_scu_write(SCU_REMAP_NORMAL_MODE_ENABLE, SCU_REMAP);

	return ;
}
	
extern void 
socle_scu_normal_mode_disable (void)
{
	socle_scu_write(SCU_REMAP_NORMAL_MODE_DISABLE, SCU_REMAP);

	return ;
}
	
	/*	decoder remap function	*/
extern void 
socle_scu_remap (void)
{
	socle_scu_write(SCU_REMAP_REMAP_MODE_ENABLE, SCU_REMAP);

	return ;
}

extern unsigned long
socle_get_cpu_clock (void)
{	
	/*	get power mode */
	if(0 == socle_scu_cpll_status_get())
		socle_clock.cpu_clock = EXT_OSC ;				/* power down */
	else
		socle_clock.cpu_clock = socle_scu_cpll_get() ;
				
	return socle_clock.cpu_clock ;
}

extern unsigned long
socle_get_ahb_clock (void)
{
	int ratio;
	
	ratio = socle_scu_clock_ratio_get();

	//20080409 leonid fix for ahb clock
	socle_clock.ahb_clock = socle_get_cpu_clock() / ratio;
	
	return socle_clock.ahb_clock;
}

extern unsigned long
socle_get_apb_clock (void)
{
	//20080409 leonid fix for apb clock
	socle_clock.apb_clock = socle_get_ahb_clock() / 2;
	
	return socle_clock.apb_clock;
}

extern unsigned long
socle_get_uart_clock (void)
{	
	/*	get power mode */
	if(0 == socle_scu_upll_status_get())
		socle_clock.uart_clock = UPLL_XIN ;				/* power down */
	else
		socle_clock.uart_clock = socle_scu_upll_get() ;
				
	return socle_clock.uart_clock ;
}

//
extern unsigned long  
get_pll_clock(void)
{
	unsigned long scu_clock, scu_ratio;
	
	scu_clock = socle_get_cpu_clock();
//	 printk("scu_clock = %x \n",scu_clock);
	/*      get scu ratio */
	scu_ratio = socle_scu_clock_ratio_get();
//	 printk("ratio = %x \n",scu_ratio);
	if(socle_scu_sdram_bus_width_status() == SOCLE_SCU_SDRAM_BUS_WIDTH_32)
		printk("PDK CPU = %ld MHz , HCLCK = %ld MHz (32bit SDR) \n",scu_clock/1000000, (scu_clock/scu_ratio)/1000000);
	else
		printk("PDK CPU = %ld MHz , HCLCK = %ld MHz (16bit SDR) \n",scu_clock/1000000, (scu_clock/scu_ratio)/1000000);

	apb_clock = scu_clock/scu_ratio/2;
	return (apb_clock);
}

static int socle_dcfg_write_proc(struct file *file, const char __user *buffer,unsigned long count, void *data)
{
	char cmd;
//	printk("SQ_cache_write_proc \n");
	copy_from_user(&cmd, buffer, 1);
	switch (cmd) {
		case '1': 
			socle_scu_dcfg_mode_set(1);
		break;
		case '2': 
			socle_scu_dcfg_mode_set(2);
		break;
		case '3': 
			socle_scu_dcfg_mode_set(3);
		break;
	}
	return 1;
}

static int socle_dcfg_read_proc(char *page, char **start,off_t off, int count, int *eof, void *data)
{
	char *buffer = page;
	int copyCount = 0;
//	printk("SQ_cache_read_proc \n");
	copyCount += snprintf(buffer+copyCount, count, "UCFG : %x\n", socle_scu_dcfg_mode_get());

	*eof = 1;

	return copyCount;
}

static int socle_ucfg_write_proc(struct file *file, const char __user *buffer,unsigned long count, void *data)
{
	char cmd;
//	printk("SQ_cache_write_proc \n");
	copy_from_user(&cmd, buffer, 1);
	switch (cmd) {
		case '1': 
			socle_scu_ucfg_mode_set(1);
		break;
		case '2': 
			socle_scu_ucfg_mode_set(2);
		break;
		case '3': 
			socle_scu_ucfg_mode_set(3);
		break;
		case '4': 
			socle_scu_ucfg_mode_set(4);
		break;
		case '5': 
			socle_scu_ucfg_mode_set(5);
		break;
		case '6': 
			socle_scu_ucfg_mode_set(6);
		break;
		
	}
	return 1;
}

static int socle_ucfg_read_proc(char *page, char **start,off_t off, int count, int *eof, void *data)
{
	char *buffer = page;
	int copyCount = 0;
//	printk("SQ_cache_read_proc \n");
	copyCount += snprintf(buffer+copyCount, count, "UCFG : %x\n", socle_scu_ucfg_mode_get());

	*eof = 1;

	return copyCount;
}

extern void stop_mode_test(void);
extern void sleep_mode_test(void);

static int socle_pwr_write_proc(struct file *file, const char __user *buffer,unsigned long count, void *data)
{
	char cmd;
//	printk("SQ_cache_write_proc \n");
	copy_from_user(&cmd, buffer, 1);
	switch (cmd) {
		case 'P': 	//STOP mode
			//socle_scu_stop_mode_set();
			stop_mode_test();
		break;
		case 'S':		//SLEEP Mode 
			//socle_scu_sleep_mode_set();
			sleep_mode_test();
		break;
	}
	return 1;
}

static struct proc_dir_entry *socle_ucfg_proc_entry;
static struct proc_dir_entry *socle_dcfg_proc_entry;
static struct proc_dir_entry *socle_pwr_proc_entry;

static int __init
socle_pc7210_scu_init(void)
{
	int ret = 0;

	/* Install the proc_fs entry */
	socle_ucfg_proc_entry = create_proc_entry("sq_ucfg", S_IRUGO | S_IFREG, &proc_root);
	if (socle_ucfg_proc_entry)
	{
		socle_ucfg_proc_entry->read_proc = socle_ucfg_read_proc;
		socle_ucfg_proc_entry->write_proc = socle_ucfg_write_proc;
	}
	else
		return -ENOMEM;

	socle_dcfg_proc_entry = create_proc_entry("sq_dcfg", S_IRUGO | S_IFREG, &proc_root);
	if (socle_dcfg_proc_entry)
	{
		socle_dcfg_proc_entry->read_proc = socle_dcfg_read_proc;
		socle_dcfg_proc_entry->write_proc = socle_dcfg_write_proc;
	}
	else
		return -ENOMEM;

	socle_pwr_proc_entry = create_proc_entry("sq_pwr", S_IRUGO | S_IFREG, &proc_root);
	if (socle_pwr_proc_entry)
	{
		socle_pwr_proc_entry->write_proc = socle_pwr_write_proc;
	}
	else
		return -ENOMEM;

	printk("PC7210 CFG Mode : UCFG = %x , DCFG = %x \n",socle_scu_ucfg_mode_get(),socle_scu_dcfg_mode_get());
	
	return ret;

}

core_initcall(socle_pc7210_scu_init);

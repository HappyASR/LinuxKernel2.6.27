/* linux/include/asm/arch-socle/regs-pc7210-scu.h
 *
 * Copyright (c) 2006 Socle-tech Corp
 *		      http://www.socle-tech.com.tw/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __PC7210_SCU_REG_H
#define __PC7210_SCU_REG_H

#define MPLL_XIN					24000000			//(Hz)
#define UPLL_XIN					11592000		//(Hz)
#define EXT_OSC					12000000			//(Hz)

/*
 *  Register for SCU
 *  */
#define SOCLE_SCU_P7CID		0x0000		/*	Chip ID register	*/
#define SCU_PLLPARAM_A			0x0004		/*	PLL Configuration Parameter	*/
#define SCU_PLLPARAM_B			0x0008		/*	Clock mode control register	*/
#define SCU_CHIPCFG_A			0x000c		/*	Chip Configuration control register	*/
#define SCU_CHIPCFG_B			0x0010		/*	LCD/ADC clock control register	*/
#define SCU_CLKCFG				0x0014		/*	IP clock control register	*/
#define SCU_REMAP				0x0018		/*	Memory space remap control register	*/

/*	SCU clock define	*/
#define SCU_CLOCK_REG(m,n,od)	((m<<7) + (n << 2) + od)
#define SCU_CPU_CLOCK_33	SCU_CLOCK_REG	(31, 1, 3)			 
#define SCU_CPU_CLOCK_40	SCU_CLOCK_REG	(78, 4, 3)		 
#define SCU_CPU_CLOCK_41_5	SCU_CLOCK_REG	(81, 4, 3)	
#define SCU_CPU_CLOCK_50	SCU_CLOCK_REG	(81, 8, 1)	
#define SCU_CPU_CLOCK_66	SCU_CLOCK_REG	(31, 1, 1)	
#define SCU_CPU_CLOCK_83	SCU_CLOCK_REG	(81, 4, 1)	
#define SCU_CPU_CLOCK_90	SCU_CLOCK_REG	(28, 0, 1)	
#define SCU_CPU_CLOCK_100	SCU_CLOCK_REG	(81, 8, 0)	
#define SCU_CPU_CLOCK_120	SCU_CLOCK_REG	(18, 0, 0)	
#define SCU_CPU_CLOCK_132	SCU_CLOCK_REG	(31, 1, 0)
#define SCU_CPU_CLOCK_133	SCU_CLOCK_REG	(131, 10, 0)
#define SCU_CPU_CLOCK_166	SCU_CLOCK_REG	(164, 10, 0)

#define SCU_UART_CLOCK_88	SCU_CLOCK_REG	(30, 0, 1)	


/*	SCU_PLLPARAM_A	*/
	/* UPLL configuration */
#define SCU_PLLPARAM_A_UPLL_M			(0xffff << 16)		/* cpll divider control mask	*/
#define SCU_PLLPARAM_A_UPLL_S			16					/* cpll divider control shift	*/
#define SCU_PLLPARAM_A_UPLL_M_M		(0x1ff << 23)	/* upll_mul M:9-bit divider control mask	*/
#define SCU_PLLPARAM_A_UPLL_M_S		23				/* upll_mul M:9-bit divider control shift	*/
#define SCU_PLLPARAM_A_UPLL_N_M		(0x1f << 18)		/* upll_mul N:5-bit divider control mask	*/
#define SCU_PLLPARAM_A_UPLL_N_S		18				/* upll_mul N:5-bit divider control shift	*/
#define SCU_PLLPARAM_A_UPLL_OD_M		(0x3 << 16)		/* upll_mul Output divider control mask	*/
#define SCU_PLLPARAM_A_UPLL_OD_S		16				/* upll_mul Output divider control shift		*/
	/* CPLL configuration */
#define SCU_PLLPARAM_A_CPLL_M			(0xffff << 0)		/* cpll divider control mask	*/
#define SCU_PLLPARAM_A_CPLL_S			0					/* cpll divider control shift	*/
#define SCU_PLLPARAM_A_CPLL_M_M		(0x1ff << 7)		/* cpll_mul M:9-bit divider control mask	*/
#define SCU_PLLPARAM_A_CPLL_M_S		7				/* cpll_mul M:9-bit divider control shift	*/
#define SCU_PLLPARAM_A_CPLL_N_M		(0x1f << 2)		/* cpll_mul N:5-bit divider control mask	*/
#define SCU_PLLPARAM_A_CPLL_N_S		2				/* cpll_mul N:5-bit divider control shift	*/
#define SCU_PLLPARAM_A_CPLL_OD_M		(0x3 << 0)		/* cpll_mul Output divider control mask	*/
#define SCU_PLLPARAM_A_CPLL_OD_S		0				/* cpll_mul Output divider control shift		*/

/*	SCU_PLLPARAM_B	*/
	/* UPLL power down/normal	*/
#define SCU_PLLPARAM_B_UPLL_POWER_DOWN		(0x1 << 19)		/*	UPLL power down	*/
#define SCU_PLLPARAM_B_UPLL_NORMAL			0x0				/*	UPLL normal	*/
	/* CPLL power down/normal	*/
#define SCU_PLLPARAM_B_CPLL_POWER_DOWN		(0x1 << 18)		/*	CPLL power down	*/
#define SCU_PLLPARAM_B_CPLL_NORMAL			0x0				/*	CPLL normal	*/
	/* CPU/AHB clock ratio	*/
#define SCU_PLLPARAM_B_CLK_RATIO_M			(0x3 << 16)		/*	CPU/AHB clock ratio mask	*/
#define SCU_PLLPARAM_B_CLK_RATIO_4_1			(0x3 << 16)		/*	CPU/AHB clock ratio shift	*/
#define SCU_PLLPARAM_B_CLK_RATIO_3_1			(0x2 << 16)		/*	CPU/AHB clock ratio shift	*/
#define SCU_PLLPARAM_B_CLK_RATIO_2_1			(0x1 << 16)		/*	CPU/AHB clock ratio shift	*/
#define SCU_PLLPARAM_B_CLK_RATIO_1_1			0				/*	CPU/AHB clock ratio shift	*/
	/* PLL lock period	*/
#define SCU_PLLPARAM_B_PLL_LOCK_PERIOD_M		(0xffff << 0)	/*	PLL lock perio mask	*/
#define SCU_PLLPARAM_B_PLL_LOCK_PERIOD_S		0				/*	PLL lock perio shift	*/
	
/*	SCU_CHIPCFG_A	*/
	/*	Force USB PHY's PLL Powered in Suspend	*/
#define SCU_CHIPCFG_A_USB_PLL_POWER_SAVE	(0x1 << 18)		/*	power saved	*/
#define SCU_CHIPCFG_A_USB_PLL_POWERED		0x0				/*	Force USB PHY's PLL Powered in Suspend	*/
	/*	DCFG MODE	*/
#define SCU_CHIPCFG_A_DCFG_MODE_M			(0x3 << 16)		/*	DCFG MODE mask	*/
#define SCU_CHIPCFG_A_DCFG_MODE_S			16				/*	DCFG MODE shift	*/
	/*	UCFG MODE	*/
#define SCU_CHIPCFG_A_UCFG_MODE_M			(0x7 << 13)		/*	UCFG MODE mask	*/
#define SCU_CHIPCFG_A_UCFG_MODE_S			13				/*	UCFG MODE shift	*/
	/*	Config UARTx as irDA function	*/
#define SCU_CHIPCFG_A_UART3_IRDA				(0x1 << 12)		/*	Config UART3 as irDA function	*/
#define SCU_CHIPCFG_A_UART3_UART				0x0				/*	Config UART3 as UART function	*/
#define SCU_CHIPCFG_A_UART2_IRDA				(0x1 << 11)		/*	Config UART2 as irDA function	*/
#define SCU_CHIPCFG_A_UART2_UART				0x0				/*	Config UART2 as UART function	*/
#define SCU_CHIPCFG_A_UART1_IRDA				(0x1 << 10)		/*	Config UART1 as irDA function	*/
#define SCU_CHIPCFG_A_UART1_UART				0x0				/*	Config UART1 as UART function	*/
#define SCU_CHIPCFG_A_UART0_IRDA				(0x1 << 9)		/*	Config UART0 as irDA function	*/
#define SCU_CHIPCFG_A_UART0_UART				0x0				/*	Config UART0 as UART function	*/
	/*	select which UART will occupy HDMA request 2/3 for UART Tx/Rx	*/
#define SCU_CHIPCFG_A_HDMA_REQ23_M			(0x3 << 6) 		/*	select which UART will occupy HDMA request 2/3 for UART Tx/Rx mask	*/
#define SCU_CHIPCFG_A_HDMA_REQ23_S			6 				/*	select which UART will occupy HDMA request 2/3 for UART Tx/Rx shift	*/
	/*	select which UART will occupy HDMA request 2/3 for UART Tx/Rx	*/
#define SCU_CHIPCFG_A_HDMA_REQ01_M			(0x3 << 4) 		/*	select which UART will occupy HDMA request 0/1 for UART Tx/Rx mask	*/
#define SCU_CHIPCFG_A_HDMA_REQ01_S			4		 		/*	select which UART will occupy HDMA request 0/1 for UART Tx/Rx shift	*/
	/*	select USB Tranceiver play Downstream or Upstream	*/
#define SCU_CHIPCFG_A_USB_TRAN_DOWN_STREAM		(0x1 << 3)	/*	select USB Tranceiver play Downstream	*/
#define SCU_CHIPCFG_A_USB_TRAN_UP_STREAM		0x0			/*	select USB Tranceiver play Upstream	*/
	/*	select fast IRQ polarity	*/
#define SCU_CHIPCFG_A_FIRQ_POLARITY_HIGH		(0x1 << 2)	/*	select fast IRQ polarity active high	*/
#define SCU_CHIPCFG_A_FIRQ_POLARITY_LOW		0x0			/*	select fast IRQ polarity active low		*/
	/*	select USB port over current function polarity	*/
#define SCU_CHIPCFG_A_USB_OVER_CURR_POLARITY_HIGH		(0x1 << 1)	/*	select fast USB port over current function polarity active high	*/
#define SCU_CHIPCFG_A_USB_OVER_CURR_POLARITY_LOW		0x0			/*	elect fast USB port over current function polarity active low	*/
	/*	select fast IRQ and USB port over current function	*/
#define SCU_CHIPCFG_A_FIRQ_FUNCTION				(0x1 << 0)	/*	select fast IRQ function */
#define SCU_CHIPCFG_A_USB_OVER_CURR_FUNCTION	0x0			/*	select USB port over current function	*/
	
/*	SCU_CHIPCFG_B	*/
	/*	LCD clock duty period	*/
#define SCU_CHIPCFG_B_LCD_CLK_DUTY_PERIOD_M		(0xffff << 0)	/*	LCD clock duty period msak	*/
#define SCU_CHIPCFG_B_LCD_CLK_DUTY_PERIOD_S		0x0				/*	LCD clock duty period shift	*/
	/*	ADC clock duty period	*/
#define SCU_CHIPCFG_B_ADC_CLK_DUTY_PERIOD_M		(0xffff << 0)	/*	ADC clock duty period msak	*/
#define SCU_CHIPCFG_B_ADC_CLK_DUTY_PERIOD_S		0x0				/*	ADC clock duty period shift	*/
	
/*	SCU_CLKCFG	*/
	/*	pclk enable/disable	*/
#define SCU_CLKCFG_ADCCLK_EN		(0x1 << 24)
#define SCU_CLKCFG_PCLK_UART3_EN	(0x1 << 21)
#define SCU_CLKCFG_PCLK_UART2_EN	(0x1 << 20)
#define SCU_CLKCFG_PCLK_UART1_EN	(0x1 << 19)
#define SCU_CLKCFG_PCLK_UART0_EN	(0x1 << 18)
#define SCU_CLKCFG_PCLK_ADC_EN	(0x1 << 17)
#define SCU_CLKCFG_PCLK_PWM_EN	(0x1 << 16)
#define SCU_CLKCFG_PCLK_SDC_EN	(0x1 << 15)
#define SCU_CLKCFG_PCLK_I2S_EN		(0x1 << 14)
#define SCU_CLKCFG_PCLK_I2C_EN		(0x1 << 13)
#define SCU_CLKCFG_PCLK_SPI_EN		(0x1 << 12)
#define SCU_CLKCFG_PCLK_GPIO_EN	(0x1 << 11)
#define SCU_CLKCFG_PCLK_WDT_EN	(0x1 << 10)
#define SCU_CLKCFG_PCLK_RTC_EN	(0x1 << 9)
#define SCU_CLKCFG_PCLK_TMR_EN	(0x1 << 8)
#define SCU_CLKCFG_PCLK_LCD_EN	(0x1 << 6)
#define SCU_CLKCFG_PCLK_NFC_EN	(0x1 << 5)
#define SCU_CLKCFG_PCLK_UDC_EN	(0x1 << 4)
#define SCU_CLKCFG_PCLK_UHC_EN	(0x1 << 3)
#define SCU_CLKCFG_PCLK_MAC_EN	(0x1 << 2)
#define SCU_CLKCFG_PCLK_HDMA_EN	(0x1 << 1)
#define SCU_CLKCFG_SDRSTMC_CLK_EN	(0x1 << 0)

/*	SCU_REMAP	*/
	/*	SDRAM data bus width status	*/
#define SCU_REMAP_SDRAM_BUS_WIDTH_STATUS_M	(0x1 << 18)
#define SCU_REMAP_SDRAM_BUS_WIDTH_STATUS_32	(0x1 << 18)
#define SCU_REMAP_SDRAM_BUS_WIDTH_STATUS_16	0
	/*	MAC Tx process stop status	*/
#define SCU_REMAP_TPS_MAC_STATUS_M			(0x1 << 17)
#define SCU_REMAP_TPS_MAC_STATUS_S			17
	/*	MAC Rx process stop status	*/
#define SCU_REMAP_RPS_MAC_STATUS_M			(0x1 << 16)
#define SCU_REMAP_RPS_MAC_STATUS_S			16
	/*	UCFG Mode6 status	*/
#define SCU_REMAP_UCFG_MODE6_STATUS_M		(0x1 << 15)
#define SCU_REMAP_UCFG_MODE6_STATUS_S		15
	/*	UCFG Mode5 status	*/
#define SCU_REMAP_UCFG_MODE5_STATUS_M		(0x1 << 14)
#define SCU_REMAP_UCFG_MODE5_STATUS_S		14
	/*	UCFG Mode4 status	*/
#define SCU_REMAP_UCFG_MODE4_STATUS_M		(0x1 << 13)
#define SCU_REMAP_UCFG_MODE4_STATUS_S		13
	/*	UCFG Mode3 status	*/
#define SCU_REMAP_UCFG_MODE3_STATUS_M		(0x1 << 12)
#define SCU_REMAP_UCFG_MODE3_STATUS_S		12
	/*	UCFG Mode2 status	*/
#define SCU_REMAP_UCFG_MODE2_STATUS_M		(0x1 << 11)
#define SCU_REMAP_UCFG_MODE2_STATUS_S		11
	/*	UCFG Mode1 status	*/
#define SCU_REMAP_UCFG_MODE1_STATUS_M		(0x1 << 10)
#define SCU_REMAP_UCFG_MODE1_STATUS_S		10
	/*	Boot source selection status	*/
#define SCU_REMAP_BOOT_SRC_STATUS_M			(0x3 << 8)
#define SCU_REMAP_BOOT_SRC_STATUS_NOR_16	(0x3 << 8)
#define SCU_REMAP_BOOT_SRC_STATUS_NOR_8		(0x2 << 8)
#define SCU_REMAP_BOOT_SRC_STATUS_NAND		(0x1 << 8)
#define SCU_REMAP_BOOT_SRC_STATUS_ISP_ROM	0
	/*	FIQDIS from ARM7 status	*/
#define SCU_REMAP_FIQ_DIS_SATUS_M			(0x1 << 7)
#define SCU_REMAP_FIQ_DIS_SATUS_S			7
	/*	IRQDIS from ARM7 status	*/
#define SCU_REMAP_IRQ_DIS_SATUS_M			(0x1 << 6)
#define SCU_REMAP_IRQ_DIS_SATUS_S			6
	/*	auto boot fail indicator from NFC status	*/
#define SCU_REMAP_NFC_AUTO_BOOT_FAIL_SATUS_M	(0x1 << 5)
#define SCU_REMAP_NFC_AUTO_BOOT_FAIL_SATUS_S	5
	/*	pll lock status	*/
#define SCU_REMAP_PLL_LOCK_SATUS_M			(0x1 << 4)
#define SCU_REMAP_PLL_LOCK_SATUS_S			4
	/*	stop mode -- systen clock	*/
#define SCU_REMAP_STOP_MODE_ENABLE			0xdeedcafe
#define SCU_REMAP_STOP_MODE_DISABLE			0x00030003
	/*	sleep mode -- cpu clock	*/
#define SCU_REMAP_SLEEP_MODE_ENABLE			0xdeedbabe
#define SCU_REMAP_SLEEP_MODE_DISABLE			0x00020002
	/*	normal mode -- use PLL clock or Base clock	*/
#define SCU_REMAP_NORMAL_MODE_ENABLE		0xdeeddeed
#define SCU_REMAP_NORMAL_MODE_DISABLE		0x00010001
	/*	decoder remap function	*/
#define SCU_REMAP_REMAP_MODE_ENABLE			0xbeefdead
#define SCU_REMAP_REMAP_MODE_DISABLE		0x0

#endif	//__PC7210_SCU_REG_H


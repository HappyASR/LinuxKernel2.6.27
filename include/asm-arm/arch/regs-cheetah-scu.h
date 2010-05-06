/* linux/include/asm/arch-socle/cheetah-scu.h
 *
 * Copyright (c) 2006 Socle-tech Corp
 *		      http://www.socle-tech.com.tw/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __CHEETAH_SCU_REG_H
#define __CHEETAH_SCU_REG_H


#define SOCLE_CHEETAH_SCU_BASE      IO_ADDRESS(SOCLE_APB0_SCU)

/*
 *  Register for SCU
 *  */
#define CHEETAH_SCU_MPLLCON				0x0000		/*	Main PLL control register	*/
#define CHEETAH_SCU_E0PLLCON			0x0004		/*	EXT0 PLL control register	*/
#define CHEETAH_SCU_OSCCON				0x000c		/*	OSC controller register	*/
#define CHEETAH_SCU_MCLKEN				0x0010		/*	Main clock enable register	*/
#define CHEETAH_SCU_ACLKEN				0x0014		/*	Application clock enable register	*/
#define CHEETAH_SCU_MCLKDIV				0x0018		/*	Main clock divide register	*/
#define CHEETAH_SCU_PWMCON				0x001c		/*	Power management register	*/
#define CHEETAH_SCU_SWRST				0x0020		/*	Software reset register	*/
#define CHEETAH_SCU_REMAP				0x0024		/*	Decoder remap control register	*/
#define CHEETAH_SCU_DEVCON				0x0028		/*	System devide mode setting register	*/
#define CHEETAH_SCU_SDICON0				0x002c		/*	ST/SDRAM I/O driving control register	*/
#define CHEETAH_SCU_SDICON1				0x0030		/*	ST/SDRAM I/O driving control register	*/
#define CHEETAH_SCU_INFORM0				0x0034		/*	User defined register	*/
#define CHEETAH_SCU_INFORM1				0x0038		/*	User defined register	*/
#define CHEETAH_SCU_INFORM2				0x003c		/*	User defined register	*/
#define CHEETAH_SCU_INFORM3				0x0040		/*	User defined register	*/
#define CHEETAH_SCU_CID					0x0044		/*	Chip ID register	*/

/*	SCU clock define	*/
#define SCU_CLOCK_REG(n,m,od)	((n << 10) + (m << 2) + od)
#define SCU_CPU_CLOCK_33	SCU_CLOCK_REG	(2, 33, 3)
#define SCU_CPU_CLOCK_50	SCU_CLOCK_REG	(2, 25, 2)
#define SCU_CPU_CLOCK_66	SCU_CLOCK_REG	(2, 33, 2)
#define SCU_CPU_CLOCK_80	SCU_CLOCK_REG	(2, 40, 2)
#define SCU_CPU_CLOCK_100	SCU_CLOCK_REG	(2, 25, 1)
#define SCU_CPU_CLOCK_120  SCU_CLOCK_REG	(2, 30, 1)
#define SCU_CPU_CLOCK_133  SCU_CLOCK_REG	(2, 33, 1)
#define SCU_CPU_CLOCK_150  SCU_CLOCK_REG	(2, 75, 2)
#define SCU_CPU_CLOCK_180  SCU_CLOCK_REG	(2, 45, 1)
#define SCU_CPU_CLOCK_200	SCU_CLOCK_REG	(2, 50, 1)
#define SCU_CPU_CLOCK_240  SCU_CLOCK_REG	(2, 60, 1)
#define SCU_CPU_CLOCK_266	SCU_CLOCK_REG	(2, 66, 1)
#define SCU_CPU_CLOCK_280	SCU_CLOCK_REG	(2, 70, 1)
#define SCU_CPU_CLOCK_300	SCU_CLOCK_REG	(2, 75, 1)
#define SCU_CPU_CLOCK_320	SCU_CLOCK_REG	(2, 80, 1)
#define SCU_CPU_CLOCK_340	SCU_CLOCK_REG	(2, 85, 1)
#define SCU_CPU_CLOCK_350	SCU_CLOCK_REG	(2, 88, 1)
#define SCU_CPU_CLOCK_360	SCU_CLOCK_REG	(2, 90, 1)
#define SCU_CPU_CLOCK_400	SCU_CLOCK_REG	(2, 100, 1)

#define SCU_UART_CLOCK_177	SCU_CLOCK_REG	(2, 59, 1)	


/*	SCU_MPLLCOM	*/
	/* the configuration value of PLL1 for system clock usage */
#define SCU_MPLLCOM_PLL_LOCK			(0x1 << 16)		/* PLL Lock detector bit (RO) 0:Non-locked, 1:Locked	*/
#define SCU_MPLLCOM_PLL_RESET			(0x1 << 15)		/* PLL reset bit 0:Normal, 1:Reset PLL	*/
#define SCU_MPLLCOM_PLL_ROWER_DOWN		(0x1 << 14)		/* PLL power-down bit 0:Active, 1:Power-down	*/
#define SCU_MPLLCOM_CPLL_M				0x3fff 			/* cpll divider control mask	*/
#define SCU_MPLLCOM_CPLL_S				0				/* cpll divider control shift	*/
#define SCU_MPLLCOM_INPUT_DIVIDER_M		(0xf << 10)		/* Input divider value mask (N[3:0]), the minimum value is 2	*/
#define SCU_MPLLCOM_INPUT_DIVIDER_S		10				/* Input divider value shift	*/
#define SCU_MPLLCOM_FEEDBACK_DIVIDER_M	(0xff << 2)		/* Feedback divider value mask (M[7:0]), the minimum value is 2	*/
#define SCU_MPLLCOM_FEEDBACK_DIVIDER_S	2				/* Feedback divider value shift	*/
#define SCU_MPLLCOM_OUTPUT_DIVIDER_M	0x3 			/* Output driver divider value mask (OD[1:0])	*/
#define SCU_MPLLCOM_OUTPUT_DIVIDER_S	0				/* Output driver divider value shift	*/

/*	SCU_E0PLLCON	*/
	/* the configuration value of PLL2 for high-speed UART usage */
#define SCU_E0PLLCON_PLL_LOCK			(0x1 << 16)		/* PLL Lock detector bit (RO) 0:Non-locked, 1:Locked	*/
#define SCU_E0PLLCON_PLL_RESET			(0x1 << 15)		/* PLL reset bit 0:Normal, 1:Reset PLL	*/
#define SCU_E0PLLCON_PLL_ROWER_DOWN		(0x1 << 14)		/* PLL power-down bit 0:Active, 1:Power-down	*/
#define SCU_MPLLCOM_UPLL_M				0x3fff 			/* upll divider control mask	*/
#define SCU_MPLLCOM_UPLL_S				0				/* upll divider control shift	*/
#define SCU_E0PLLCON_INPUT_DIVIDER_M	(0xf << 10)		/* Input divider value mask (N[3:0]), the minimum value is 2	*/
#define SCU_E0PLLCON_INPUT_DIVIDER_S	10				/* Input divider value shift	*/
#define SCU_E0PLLCON_FEEDBACK_DIVIDER_M	(0xff << 2)		/* Feedback divider value mask (M[7:0]), the minimum value is 2	*/
#define SCU_E0PLLCON_FEEDBACK_DIVIDER_S	2				/* Feedback divider value shift	*/
#define SCU_E0PLLCON_OUTPUT_DIVIDER_M	0x3 			/* Output driver divider value mask (OD[1:0])	*/
#define SCU_E0PLLCON_OUTPUT_DIVIDER_S	0				/* Output driver divider value shift	*/

/*	SCU_OSCCON	*/
	/* the configuration value of Oscillators */
#define SCU_OSCCON_RTC_OSC_POWER_DOWN	(0x1 << 3)		/* 32.768K RTC OSC power-down bit	*/
#define SCU_OSCCON_EXT0_OSC_POWER_DOWN	(0x1 << 1)		/* External 0 OSC power-down bit for high-speed UART usage	*/
#define SCU_OSCCON_MAIN_OSC_POWER_DOWN	0x1 			/* Main OSC power-down bit	*/

/*	SCU_MCLKEN	*/
	/* the configuration value of Oscillators */
#define SCU_MCLKEN_HCLK_NFC_EN		(0x1 << 23)		/* HCLK enable/disable for NFC block	*/
#define SCU_MCLKEN_HCLK_2CH_DMA_EN	(0x1 << 22)		/* HCLK enable/disable for 2-ch DMA block	*/
#define SCU_MCLKEN_HCLK_IDE_EN		(0x1 << 21)		/* HCLK enable/disable for IDE block	*/
#define SCU_MCLKEN_HCLK_CLCD_EN		(0x1 << 20)		/* HCLK enable/disable for CLCD block	*/
#define SCU_MCLKEN_HCLK_4CH_DMA_EN	(0x1 << 19)		/* HCLK enable/disable for 4-ch DMA block	*/
#define SCU_MCLKEN_HCLK_UHC_EN		(0x1 << 18)		/* HCLK enable/disable for UHC20 block. HCLK disable automatically for UHC20 when UDC mode	*/
#define SCU_MCLKEN_HCLK_UDC_EN		(0x1 << 17)		/* HCLK enable/disable for UDC20 block.	HCLK disable automatically for UDC20 when UHC mode	*/
#define SCU_MCLKEN_HCLK_PCI_EN		(0x1 << 16)		/* HCLK enable/disable for AHB-to-PCI block	*/
#define SCU_MCLKEN_HCLK_MAC_EN		(0x1 << 15)		/* HCLK enable/disable for MAC block	*/
#define SCU_MCLKEN_HCLK_GPIO1_EN	(0x1 << 14)		/* PCLK enable/disable for GPIOI block	*/
#define SCU_MCLKEN_HCLK_ADC_EN		(0x1 << 13)		/* PCLK enable/disable for ADC block	*/
#define SCU_MCLKEN_HCLK_PWM_EN		(0x1 << 12)		/* PCLK enable/disable for PWM block	*/
#define SCU_MCLKEN_HCLK_WDT_EN		(0x1 << 11)		/* PCLK enable/disable for WDT block	*/
#define SCU_MCLKEN_HCLK_RTC_EN		(0x1 << 10)		/* PCLK enable/disable for RTC block	*/
#define SCU_MCLKEN_HCLK_GPIO_EN		(0x1 << 9)		/* PCLK enable/disable for GPIO block	*/
#define SCU_MCLKEN_HCLK_TIMER_EN	(0x1 << 8)		/* PCLK enable/disable for Timers block	*/
#define SCU_MCLKEN_HCLK_SDMMC_EN	(0x1 << 7)		/* PCLK enable/disable for SD/MMC block	*/
#define SCU_MCLKEN_HCLK_I2S_EN		(0x1 << 6)		/* PCLK enable/disable for I2S block	*/
#define SCU_MCLKEN_HCLK_I2C_EN		(0x1 << 5)		/* PCLK enable/disable for I2C block	*/
#define SCU_MCLKEN_HCLK_SPI1_EN		(0x1 << 4)		/* PCLK enable/disable for SPI1 block	*/
#define SCU_MCLKEN_HCLK_SPI0_EN		(0x1 << 3)		/* PCLK enable/disable for SPI0 block	*/
#define SCU_MCLKEN_HCLK_UART2_EN	(0x1 << 2)		/* PCLK enable/disable for UART2 block	*/
#define SCU_MCLKEN_HCLK_UART1_EN	(0x1 << 1)		/* PCLK enable/disable for UART1 block	*/
#define SCU_MCLKEN_HCLK_UART0_EN	0x1 			/* PCLK enable/disable for UART0 block	*/

/*	SCU_ACLKEN	*/
	/* the enable bit for application clock of individual peripheral */
#define SCU_ACLKEN_UHCCLK_EN		(0x1 << 12)		/* UHCCLK enable/disable for UHC block	*/
#define SCU_ACLKEN_LCDCLK_EN		(0x1 << 11)		/* LCDCLK enable/disable for LCD block	*/
#define SCU_ACLKEN_RMIICLK_EN		(0x1 << 10)		/* RMIICLK enable/disable for MAC block	*/
#define SCU_ACLKEN_PCICLK_EN		(0x1 << 9)		/* PCICLK enable/disable for PCI block	*/
#define SCU_MCLKEN_IDECLK_EN		(0x1 << 8)		/* IDECLK enable/disable for IDE block	*/
#define SCU_MCLKEN_UTMICLK_UHC_EN	(0x1 << 7)		/* UTMICLK enable/disable for UHC block	UTMICLK disable automatically for UHC block when UDC mode	*/
#define SCU_MCLKEN_UTMICLK_UDC_EN	(0x1 << 6)		/* UTMICLK enable/disable for UDC block	UTMICLK disable automatically for UDC block when UHC mode	*/
#define SCU_MCLKEN_RTCCLK_EN		(0x1 << 5)		/* RTCCLK enable/disable for RTC block	*/
#define SCU_MCLKEN_ADCCLK_EN		(0x1 << 4)		/* ADCCLK enable/disable for ADC block	*/
#define SCU_MCLKEN_I2SCLK_EN		(0x1 << 3)		/* I2SCLK enable/disable for I2S block	*/
#define SCU_MCLKEN_UCLK_UART2_EN	(0x1 << 2)		/* UCLK enable/disable for UART2 block	*/
#define SCU_MCLKEN_UCLK_UART1_EN	(0x1 << 1)		/* UCLK enable/disable for UART1 block	*/
#define SCU_MCLKEN_UCLK_UART0_EN	0x1				/* UCLK enable/disable for UART0 block	*/

/*	SCU_MCLKDIV	*/
	/*  the power mode control bit */
#define SCU_MCLKDIV_UART2_CLK_FREQ_M		(0x7 << 9)		/* UART2 clock frequency setting	mask	*/
#define SCU_MCLKDIV_UART2_CLK_FREQ_S		9				/* UART2 clock frequency setting	shift	*/
#define SCU_MCLKDIV_UART1_CLK_FREQ_M		(0x7 << 6)		/* UART1 clock frequency setting	mask	*/
#define SCU_MCLKDIV_UART1_CLK_FREQ_S		6				/* UART1 clock frequency setting	shift	*/
#define SCU_MCLKDIV_UART0_CLK_FREQ_M		(0x7 << 3)		/* UART0 clock frequency setting	mask	*/
#define SCU_MCLKDIV_UART0_CLK_FREQ_S		3				/* UART0 clock frequency setting	shift	*/
#define SCU_MCLKDIV_CLK_RATIO_M				0x7		 		/* CPUCLK/HCLK clock ratio	mask	*/
#define SCU_MCLKDIV_CLK_RATIO_S				0				/* CPUCLK/HCLK clock ratio	shift	*/

#define SCU_MCLKDIV_UART_CLK_FREQ_24MHZ			0
#define SCU_MCLKDIV_UART_CLK_FREQ_E1PLL			0x001
#define SCU_MCLKDIV_UART_CLK_FREQ_E1PLL_DIV_2	0x010
#define SCU_MCLKDIV_UART_CLK_FREQ_E1PLL_DIV_4	0x011
#define SCU_MCLKDIV_UART_CLK_FREQ_E1PLL_DIV_8	0x100

	/* CPU/AHB clock ratio	*/
#define SCU_MCLKDIV_CLK_RATIO_8_1		4
#define SCU_MCLKDIV_CLK_RATIO_4_1		3
#define SCU_MCLKDIV_CLK_RATIO_3_1		2
#define SCU_MCLKDIV_CLK_RATIO_2_1		1
#define SCU_MCLKDIV_CLK_RATIO_1_1		0				

/*	SCU_PWMCON	*/
	/*  the power mode control bit */
#define SCU_PWMCON_STANDBYWFI_EN				(0x1 << 2)		/* STANDBYWFI enable	*/
#define SCU_PWMCON_POWER_MODE_NORMAL			0x1				/* Power mode : Normal	*/
#define SCU_PWMCON_POWER_MODE_SLOW				0				/* Power mode : Slow	*/

/*	SCU_DEVCON	*/
	/*  enter various operation modes for peripherals */
#define SCU_DEVCON_ARM926EJ_RESET_WTD_RESET_EN				(0x1 << 13)		/* ARM926EJ reset enable when Watch-Dog reset happen	*/
#define SCU_DEVCON_ARM926EJ_RESET_SW_RESET_EN				(0x1 << 12)		/* ARM926EJ reset enable when Software reset happen	*/
#define SCU_DEVCON_EXT_DMA_TURN_ON_HMASTER_DEBUG			(0x1 << 11)		/* External DMA interface turn-on NHDREQ_ES/NHDACK_ES mode	*/
#define SCU_DEVCON_EXT_DMA_TURN_ON_NHDREQ_ES_NHDACK_ES		0x0				/* External DMA interface turn-on HMASTER debug mode	*/
#define SCU_DEVCON_GPIO_NAND_IF_NAND						(0x1 << 7)		/* GPIO/NAND-Flash interface setting : GPIOinterface	*/
#define SCU_DEVCON_GPIO_NAND_IF_GPIO						0x0				/* GPIO/NAND-Flash interface setting : NAND-Flash interface	*/
#define SCU_DEVCON_AHB_GPIO_IF_AHB							(0x1 << 6)		/* AHB/GPIO interface setting (RO) : AHB bus interface	*/
#define SCU_DEVCON_AHB_GPIO_IF_GPIO							0x0				/* AHB/GPIO interface setting (RO) : GPIO	bus interface	*/
#define SCU_DEVCON_USB_PHY_BYPASS_MODE						(0x1 << 5)		/* USB2.0 PHY bypass mode setting (RO) : Bypass PHY	*/
#define SCU_DEVCON_USB_PHY_NON_BYPASS_MODE					0x0				/* USB2.0 PHY bypass mode setting (RO) : Non-Bypass PHY	*/
#define SCU_DEVCON_BYPASS_PLL_MODE							(0x1 << 4)		/* All PLL bypass mode setting (RO) : Bypass PLL	*/
#define SCU_DEVCON_NON_BYPASS_PLL_MODE						0x0				/* All PLL bypass mode setting (RO) : Non-bypass PLL	*/
#define SCU_DEVCON_DCM_TEST_MODE							(0x1 << 3)		/* DCM mode setting (RO) : DCM test mode	*/
#define SCU_DEVCON_DCM_NOR_MODE								0x0				/* DCM mode setting (RO) : Normal mode	*/
#define SCU_DEVCON_UDC_MODE									(0x1 << 2)		/* UDC/UHC mode setting (RO) : UDC active	*/
#define SCU_DEVCON_UHC_MODE									0x0				/* UDC/UHC mode setting (RO) : UHC active	*/
#define SCU_DEVCON_USB_PHY_SRC_CRYSTAL						(0x1 << 1)		/* USB PHY 48MHz clock source setting : From Crystal	*/
#define SCU_DEVCON_USB_PHY_SRC_OSC							0x0				/* USB PHY 48MHz clock source setting : From OSC	*/
#define SCU_DEVCON_APB_BRI_RESPON_RETRY						(0x1 << 1)		/* APB Bridge response mode setting : RETRY response	*/
#define SCU_DEVCON_APB_BRI_RESPON_WAIT						0x0				/* APB Bridge response mode setting : wait state response	*/

/*	SCU_SDICON0	*/
	/* the driving control of SDRAM interface */
#define SCU_SDICON0_STD_ADDR26_DRIVING_16MA		(1 << 29)		/* STD_ ADDR [26] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR25_DRIVING_16MA		(1 << 28)		/* STD_ ADDR [25] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR24_DRIVING_16MA		(1 << 27)		/* STD_ ADDR [24] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR23_DRIVING_16MA		(1 << 26)		/* STD_ ADDR [23] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR22_DRIVING_16MA		(1 << 25)		/* STD_ ADDR [22] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR21_DRIVING_16MA		(1 << 24)		/* STD_ ADDR [21] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR20_DRIVING_16MA		(1 << 23)		/* STD_ ADDR [20] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR19_DRIVING_16MA		(1 << 22)		/* STD_ ADDR [19] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR18_DRIVING_16MA		(1 << 21)		/* STD_ ADDR [18] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR17_DRIVING_16MA		(1 << 20)		/* STD_ ADDR [17] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR16_DRIVING_16MA		(1 << 19)		/* STD_ ADDR [16] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR15_DRIVING_16MA		(1 << 18)		/* STD_ ADDR [15] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR14_DRIVING_16MA		(1 << 17)		/* STD_ ADDR [14] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR13_DRIVING_16MA		(1 << 16)		/* STD_ ADDR [13] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR12_DRIVING_16MA		(1 << 15)		/* STD_ ADDR [12] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR11_DRIVING_16MA		(1 << 14)		/* STD_ ADDR [11] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR10_DRIVING_16MA		(1 << 13)		/* STD_ ADDR [10] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR9_DRIVING_16MA		(1 << 12)		/* STD_ ADDR [9] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR8_DRIVING_16MA		(1 << 11)		/* STD_ ADDR [8] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR7_DRIVING_16MA		(1 << 10)		/* STD_ ADDR [7] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR6_DRIVING_16MA		(1 << 9)		/* STD_ ADDR [6] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR5_DRIVING_16MA		(1 << 8)		/* STD_ ADDR [5] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR4_DRIVING_16MA		(1 << 7)		/* STD_ ADDR [4] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR3_DRIVING_16MA		(1 << 6)		/* STD_ ADDR [3] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR2_DRIVING_16MA		(1 << 5)		/* STD_ ADDR [2] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR1_DRIVING_16MA		(1 << 4)		/* STD_ ADDR [1] driving setting 16ma */
#define SCU_SDICON0_STD_ADDR0_DRIVING_16MA		(1 << 3)		/* STD_ ADDR [0] driving setting 16ma */
#define SCU_SDICON0_STD_DATA31_16_DRIVING_16MA	(1 << 2)		/* STD_ DATA [31:16] driving setting 16ma */
#define SCU_SDICON0_STD_DATA15_8_DRIVING_16MA	(1 << 1)		/* STD_ DATA [15:8] driving setting 16ma */
#define SCU_SDICON0_STD_DATA7_0_DRIVING_16MA	(1 << 0)		/* STD_ DATA [7:0] driving setting 16ma */

/*	SCU_SDICON1	*/
	/* the driving control of SDRAM and AMBA interface */
#define SCU_SDICON1_AHB_BUS_DRIVING_8MA			(1 << 17)		/* AHB bus interface driving setting 8ma (reserve for test mode)	*/
#define SCU_SDICON1_ST_OE_DRIVING_8MA			(1 << 16)		/* ST_OE driving setting 8ma	*/
#define SCU_SDICON1_ST_WE_DRIVING_8MA			(1 << 15)		/* ST_WE driving setting 8ma	*/
#define SCU_SDICON1_SD_CLKOUT_DRIVING_16MA		(1 << 14)		/* SD_CLKOUT driving setting 16ma	*/
#define SCU_SDICON1_SD_CKE_DRIVING_16MA			(1 << 13)		/* SD_CKE driving setting 16ma	*/
#define SCU_SDICON1_SD_BA1_DRIVING_16MA			(1 << 12)		/* SD_BA [1] driving setting 16ma	*/
#define SCU_SDICON1_SD_BA0_DRIVING_16MA			(1 << 11)		/* SD_BA [0] driving setting 16ma	*/
#define SCU_SDICON1_SD_DQM3_DRIVING_16MA		(1 << 10)		/* SD_DQM [3] driving setting 16ma	*/
#define SCU_SDICON1_SD_DQM2_DRIVING_16MA		(1 << 9)		/* SD_DQM [2] driving setting 16ma	*/
#define SCU_SDICON1_SD_DQM1_DRIVING_16MA		(1 << 8)		/* SD_DQM [1] driving setting 16ma	*/
#define SCU_SDICON1_SD_DQM0_DRIVING_16MA		(1 << 7)		/* SD_DQM [0] driving setting 16ma	*/
#define SCU_SDICON1_SD_WE_DRIVING_16MA			(1 << 6)		/* SD_WE driving setting 16ma	*/
#define SCU_SDICON1_SD_CAS_DRIVING_16MA			(1 << 5)		/* SD_CAS driving setting 16ma	*/
#define SCU_SDICON1_SD_RAS_DRIVING_16MA			(1 << 4)		/* SD_RAS driving setting 16ma	*/
#define SCU_SDICON1_SD_CS3_DRIVING_16MA			(1 << 3)		/* SD_CS [3] driving setting 16ma	*/
#define SCU_SDICON1_SD_CS2_DRIVING_16MA			(1 << 2)		/* SD_CS [2] driving setting 16ma	*/
#define SCU_SDICON1_SD_CS1_DRIVING_16MA			(1 << 1)		/* SD_CS [1] driving setting 16ma	*/
#define SCU_SDICON1_SD_CS0_DRIVING_16MA			(1 << 0)		/* SD_CS [0] driving setting 16ma	*/

/* SCU_MPLLCON */
#define SCU_MPLLCON_CLK_M 0x3fff		//N:M:OD mask -> SCU_MPLLCOM[13:0]
#define SCU_MPLLCON_N     0x00003C00	//input divider value mask, N[3:0] -> SCU_MPLLCOM[13:10]
#define SCU_MPLLCON_N_S   10			//input divider value shift
#define SCU_MPLLCON_M     0x000003FC	//feedback divider value mask, M[7:0] -> SCU_MPLLCOM[9:2]
#define SCU_MPLLCON_M_S   2				//feedback divider value shift
#define SCU_MPLLCON_OD    0x3			//output driver drivider value mask, OD[1:0] -> SCU_MPLLCOM[1:0]
#define SCU_MPLLCON_OD_S  0				//output driver drivider value shift
/* SCU_MCLKDIV */
#define SCU_MCLKDIV_RATIO    0x7		//CPUCLK/HCLK clock ratio mask
#define SCU_MCLKDIV_RATIO_S  0			//CPUCLK/HCLK clock ratio shift -> SCU_MCLKDIV[0:2]
/* SCU_SWRST */
/* SCU_XIN */
#define SCU_XIN 16000000 
//SCU_DEVCON
#define GPIO_NAND_SW				(1<<7)
#define AMBA_MODE					(1<<6)
#define SCU_WDT_RESET				(1<<13)

#endif	//__CHEETAH_SCU_REG_H


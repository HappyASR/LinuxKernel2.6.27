#ifndef __SCU_REGS_H_INCLUDED
#define __SCU_REGS_H_INCLUDED


#define EXT_OSC					48000000			//(Hz)

/*
 *  Register for SCU
 *  */
#define SCU_MPLLCON				0x0000		/*	Main PLL control register	*/
#define SCU_PLLLOCK				0x0004		/*	PLL lock time value register	*/
#define SCU_CLKSRC				0x0008		/*	Clock source control register	*/
#define SCU_CLKDIV				0x000c		/*	Clock divisor register	*/
#define SCU_CLKOUT				0x0010		/*	Output debug clock pin configure register	*/
#define SCU_HCLKEN				0x0014		/*	HCLK gating control register	*/
#define SCU_PCLKEN				0x0018		/*	PCLK gating control register	*/
#define SCU_SCLKEN				0x001c		/*	Special clock gating control register	*/
#define SCU_SWRST				0x0020		/*	Software reset control register	*/
#define SCU_REMAP				0x0024		/*	Decoder remap control register	*/
#define SCU_PWRMODE			0x0028		/*	Power mode control register	*/
#define SCU_PWRCFG				0x002c		/*	Power manager configuration register	*/
#define SCU_PWREN				0x0030		/*	Internal core power gating control register	*/
#define SCU_RSTCNT				0x0034		/*	Reset count register	*/
#define SCU_RSTSTAT				0x0038		/*	Reset status register	*/
#define SCU_WKUPSTAT			0x003c		/*	Wake-up status register	*/
#define SCU_MEMISLP				0x0040		/*	Memory interface SLEEP mode control register	*/
#define SCU_NFISLP				0x0044		/*	NAND-Flash interface SLEEP mode control register	*/
#define SCU_USBISLP				0x0048		/*	USB interface SLEEP mode control register	*/
#define SCU_LCDISLP				0x004c		/*	LCD interface SLEEP mode control register	*/
#define SCU_PERI0SLP			0x0050		/*	Low-speed peripherals interface SLEEP mode control register	*/
#define SCU_PERI1SLP			0x0054		/*	Low-speed peripherals interface SLEEP mode control register	*/
#define SCU_SLPEN				0x0058		/*	SLEEP mode pad state enable register	*/
#define SCU_IOMODE				0x005c		/*	IO mode selection register	*/
#define SCU_CHIPID				0x0060		/*	System ID register	*/
#define SCU_INFORM0				0x0064		/*	SLEEP mode information register	*/
#define SCU_INFORM1				0x0068		/*	SLEEP mode information register	*/
#define SCU_INFORM2				0x006c		/*	SLEEP mode information register	*/
#define SCU_INFORM3				0x0070		/*	SLEEP mode information register	*/
#define SCU_CHIPMD				0x0100		/*	Chip operation mode	*/
#define SCU_PLL					0x0104		/*	Test register for PLL	*/
#define SCU_DBCT				0x0108		/*	Debounce time for noise filter	*/

/*	SCU clock define	*/
#define SCU_CLOCK_REG(n,m,od)	((n << 15) + (m << 3) + od)
#define SCU_CPU_CLOCK_33	SCU_CLOCK_REG	(15, 10, 0)
#define SCU_CPU_CLOCK_66	SCU_CLOCK_REG	(7, 10, 0)
#define SCU_CPU_CLOCK_80	SCU_CLOCK_REG	(2, 4, 0)
#define SCU_CPU_CLOCK_100	SCU_CLOCK_REG	(11, 24, 0)
#define SCU_CPU_CLOCK_132	SCU_CLOCK_REG	(3, 10, 0)
#define SCU_CPU_CLOCK_133	SCU_CLOCK_REG	(23, 132, 1)
#define SCU_CPU_CLOCK_200	SCU_CLOCK_REG	(5, 24, 0)
#define SCU_CPU_CLOCK_264	SCU_CLOCK_REG	(3, 21, 0)
#define SCU_CPU_CLOCK_266	SCU_CLOCK_REG	(23, 132, 0)
#define SCU_CPU_CLOCK_280	SCU_CLOCK_REG	(5, 34, 0)
#define SCU_CPU_CLOCK_300	SCU_CLOCK_REG	(3, 24, 0)
#define SCU_CPU_CLOCK_320	SCU_CLOCK_REG	(2, 19, 0)
#define SCU_CPU_CLOCK_340	SCU_CLOCK_REG	(11, 84, 0)
#define SCU_CPU_CLOCK_350	SCU_CLOCK_REG	(23, 174, 0)
#define SCU_CPU_CLOCK_360	SCU_CLOCK_REG	(1, 14, 0)
#define SCU_CPU_CLOCK_400	SCU_CLOCK_REG	(2, 24, 0)
#define SCU_CPU_CLOCK_252	SCU_CLOCK_REG	(3, 20, 0)
#define SCU_CPU_CLOCK_240	SCU_CLOCK_REG	(1, 9, 0)
#define SCU_CPU_CLOCK_212	SCU_CLOCK_REG	(11, 52, 0)
#define SCU_CPU_CLOCK_292	SCU_CLOCK_REG	(11, 72, 0)
#define SCU_CPU_CLOCK_160	SCU_CLOCK_REG	(2, 9, 0)


/*	SCU_MPLLCOM	*/
	/* the configuration value of MPLL for system clock usage */
#define SCU_MPLLCOM_SATUR_BEHAV_EN		(0x1 << 24)		/* Enable saturation behavior 1: Enable	*/
#define SCU_MPLLCOM_SATUR_BEHAV_DIS		(0x0)			/* Enable saturation behavior 0: Disable	*/
#define SCU_MPLLCOM_FAST_LOCK_EN			(0x1 << 23)		/* Enable fast locking circuit. 1: Enable	*/
#define SCU_MPLLCOM_FAST_LOCK_DIS		(0x0)			/* Enable fast locking circuit. 0: Disable	*/
#define SCU_MPLLCOM_PLL_RESET				(0x1 << 22)		/* PLL reset bit 0:Normal, 1:Reset PLL	*/
#define SCU_MPLLCOM_PLL_ROWER_DOWN		(0x1 << 21)		/* PLL power-down bit 0:Active, 1:Power-down	*/
#define SCU_MPLLCOM_CPLL_M				0x1fffff 		/* cpll divider control mask	*/
#define SCU_MPLLCOM_CPLL_S					0				/* cpll divider control shift	*/
#define SCU_MPLLCOM_REF_DIVIDER_M		(0x3f << 15)		/* Reference divider value (CLKR[5:0]) mask	*/
#define SCU_MPLLCOM_REF_DIVIDER_S			15				/* Reference divider value (CLKR[5:0]) shift	*/
#define SCU_MPLLCOM_MUL_FACTOR_M			(0xfff << 3)		/* Multiplication factor value (CLKF[11:0]) mask	*/
#define SCU_MPLLCOM_MUL_FACTOR_S			3				/* Multiplication factor value (CLKF[11:0]) shift	*/
#define SCU_MPLLCOM_OUTPUT_DIVIDER_M		0x3 				/* Output driver divider value. (CLKOD[2:0]) mask	*/
#define SCU_MPLLCOM_OUTPUT_DIVIDER_S		0				/* Output driver divider value. (CLKOD[2:0]) shift	*/

/*	SCU_PLLLOCK		*/
	/* the lock time value of MPLL */
#define SCU_PLLLOCK_MPLL_LOCK_TIME_CNT_M	(0xffff)			/* PLL lock time counter for MPLL	mask */

/*	SCU_CLKSRC	*/
	/* the clock source control */
#define SCU_CLKSRC_CLK_SRC_MPLL_OUTPUT	(0x1)			/* CPUCLK/HCLK/PCLK clock source selection : MPLL output clock	*/
#define SCU_CLKSRC_CLK_SRC_EXT_INPUT		(0x0)			/* CPUCLK/HCLK/PCLK clock source selection : External input clock	*/

/*	SCU_CLKDIV	*/
	/* the clock divisor control */
#define SCU_CLKDIV_RATIO_CCLK_HCLK_2		(0x1)			/* CPUCLK/HCLK clock ratio	:HCLK has the clock same as CPUCLK/2	*/
#define SCU_CLKDIV_RATIO_CCLK_HCLK_4		(0x2)			/* CPUCLK/HCLK clock ratio: HCLK has the clock same as CPUCLK/4	*/
#define SCU_CLKDIV_RATIO_CCLK_HCLK_8		(0x3)			/* CPUCLK/HCLK clock ratio: HCLK has the clock same as CPUCLK/8	*/
#define SCU_CLKDIV_RATIO_CCLK_HCLK_M		(0x3)			/* CPUCLK/HCLK clock ratio mask	*/

/*	SCU_CLKOUT	*/
	/* the clock output configuration */
#define SCU_CLKOUT_CLK_SRC_MPLL_OUTPUT		(0x0)			/* CLKOUT clock source selection : MPLL output clock	*/
#define SCU_CLKOUT_CLK_SRC_CPUCLK				(0x1 << 3)		/* CLKOUT clock source selection : CPUCLK clock	*/
#define SCU_CLKOUT_CLKOUT_DIV_CLKOUT_M		(0x7)			/* CLKOUT divide value	mask	*/
#define SCU_CLKOUT_CLKOUT_DIV_CLKOUT			(0x0)			/* CLKOUT divide value	:CLKOUT	*/
#define SCU_CLKOUT_CLKOUT_DIV_CLKOUT_2		(0x1)			/* CLKOUT divide value	:CLKOUT/2	*/
#define SCU_CLKOUT_CLKOUT_DIV_CLKOUT_4		(0x2)			/* CLKOUT divide value	:CLKOUT/4	*/
#define SCU_CLKOUT_CLKOUT_DIV_CLKOUT_8		(0x3)			/* CLKOUT divide value	:CLKOUT/8	*/
#define SCU_CLKOUT_CLKOUT_DIV_CLKOUT_16		(0x4)			/* CLKOUT divide value	:CLKOUT/16	*/
#define SCU_CLKOUT_CLKOUT_DIV_CLKOUT_32		(0x5)			/* CLKOUT divide value	:CLKOUT/32	*/

/*	SCU_HCLKEN	*/
	/*  the enable bit for HCLK clock of individual peripheral */
#define SCU_HCLKEN_EMB_SRAM_EN		(0x1 << 4)		/* HCLK enable/disable for embedded SRAM controller block : 1: Enable	*/
#define SCU_HCLKEN_EMB_SRAM_DIS		(0x0)			/* HCLK enable/disable for embedded SRAM controller block : 0: Disable	*/
#define SCU_HCLKEN_NFC_EN			(0x1 << 3)		/* HCLK enable/disable for NFC block : 1: Enable	*/
#define SCU_HCLKEN_NFC_DIS		(0x0)			/* HCLK enable/disable for NFC block : 0: Disable	*/
#define SCU_HCLKEN_CLCD_EN		(0x1 << 2)		/* HCLK enable/disable for CLCD block : 1: Enable	*/
#define SCU_HCLKEN_CLCD_DIS		(0x0)			/* HCLK enable/disable for CLCD block : 0: Disable	*/
#define SCU_HCLKEN_DMA_EN		(0x1 << 1)		/* HCLK enable/disable for DMA block : 1: Enable	*/
#define SCU_HCLKEN_DMA_DIS		(0x0)			/* HCLK enable/disable for DMA block : 0: Disable	*/
#define SCU_HCLKEN_UHC_EN			(0x1 << 0)		/* HCLK enable/disable for UHC block : 1: Enable	*/
#define SCU_HCLKEN_UHC_DIS		(0x0)			/* HCLK enable/disable for UHC block : 0: Disable	*/


/*	SCU_PCLKEN	*/
	/*  the enable bit for PCLK clock of individual peripheral*/
#define SCU_PCLKEN_PWM_EN					(0x1 << 12)		/* PCLK enable/disable for PWM block. 1: Enable	*/
#define SCU_PCLKEN_PWM_DIS				(0x0)			/* PCLK enable/disable for PWM block. 0: Disable	*/
#define SCU_PCLKEN_WDT_EN					(0x1 << 11)		/* PCLK enable/disable for WDT block. 1: Enable	*/
#define SCU_PCLKEN_WDT_DIS				(0x0)			/* PCLK enable/disable for WDT block. 0: Disable	*/
#define SCU_PCLKEN_RTC_EN					(0x1 << 10)		/* PCLK enable/disable for RTC block. 1: Enable	*/
#define SCU_PCLKEN_RTC_DIS					(0x0)			/* PCLK enable/disable for RTC block. 0: Disable	*/
#define SCU_PCLKEN_GPIO_EN					(0x1 << 9)		/* PCLK enable/disable for GPIO block. 1: Enable	*/
#define SCU_PCLKEN_GPIO_DIS				(0x0)			/* PCLK enable/disable for GPIO block. 0: Disable	*/
#define SCU_PCLKEN_TIMER_EN				(0x1 << 8)		/* PCLK enable/disable for Timers block. 1: Enable	*/
#define SCU_PCLKEN_TIMER_DIS				(0x0)			/* PCLK enable/disable for Timers block. 0: Disable	*/
#define SCU_PCLKEN_SDMMC_EN				(0x1 << 7)		/* PCLK enable/disable for SD/MMC block. 1: Enable	*/
#define SCU_PCLKEN_SDMMC_DIS				(0x0)			/* PCLK enable/disable for SD/MMC block. 0: Disable	*/
#define SCU_PCLKEN_I2S_EN					(0x1 << 6)		/* PCLK enable/disable for I2S block. 1: Enable		*/
#define SCU_PCLKEN_I2S_DIS					(0x0)			/* PCLK enable/disable for I2S block. 0: Disable	*/
#define SCU_PCLKEN_I2C2_EN					(0x1 << 5)		/* PCLK enable/disable for I2C2 block. 1: Enable	*/
#define SCU_PCLKEN_I2C2_DIS				(0x0)			/* PCLK enable/disable for I2C2 block. 0: Disable	*/
#define SCU_PCLKEN_I2C1_EN					(0x1 << 4)		/* PCLK enable/disable for I2C1 block. 1: Enable	*/
#define SCU_PCLKEN_I2C1_DIS				(0x0)			/* PCLK enable/disable for I2C1 block. 0: Disable	*/
#define SCU_PCLKEN_I2C0_EN					(0x1 << 3)		/* PCLK enable/disable for I2C0 block. 1: Enable	*/
#define SCU_PCLKEN_I2C0_DIS				(0x0)			/* PCLK enable/disable for I2C0 block. 0: Disable	*/
#define SCU_PCLKEN_SPI_EN					(0x1 << 2)		/* PCLK enable/disable for SPI block. 1: Enable		*/
#define SCU_PCLKEN_SPI_DIS					(0x0)			/* PCLK enable/disable for SPI block. 0: Disable	*/
#define SCU_PCLKEN_UART1_EN				(0x1 << 1)		/* PCLK enable/disable for UART1 block. 1: Enable	*/
#define SCU_PCLKEN_UART1_DIS				(0x0)			/* PCLK enable/disable for UART1 block. 0: Disable	*/
#define SCU_PCLKEN_UART0_EN				(0x1 << 0)		/* PCLK enable/disable for UART0 block. 1: Enable	*/
#define SCU_PCLKEN_UART0_DIS				(0x0)			/* PCLK enable/disable for UART0 block. 0: Disable	*/

/*	SCU_SCLKEN	*/
	/*  the enable bit for special clock of individual peripheral */
#define SCU_SCLKEN_RTCCLK_RTC_EN			(0x1 << 5)		/*	RTCCLK enable/disable for RTC block. 1: Enable	*/
#define SCU_SCLKEN_RTCCLK_RTC_DIS			(0x0)			/*	RTCCLK enable/disable for RTC block. 0: Disable	*/
#define SCU_SCLKEN_LCDCLK_LCD_EN			(0x1 << 4)		/*	LCDCLK enable/disable for LCD block. 1: Enable	*/
#define SCU_SCLKEN_LCDCLK_LCD_DIS			(0x0)			/*	LCDCLK enable/disable for LCD block. 0: Disable	*/
#define SCU_SCLKEN_UHCCLK_UHC_EN			(0x1 << 3)		/*	UHCCLK enable/disable for UHC block. 1: Enable	*/
#define SCU_SCLKEN_UHCCLK_UHC_DIS			(0x0)			/*	UHCCLK enable/disable for UHC block. 0: Disable	*/
#define SCU_SCLKEN_I2SCLK_I2S_EN			(0x1 << 2)		/*	I2SCLK enable/disable for I2S block. 1: Enable		*/
#define SCU_SCLKEN_I2SCLK_I2S_DIS			(0x0)			/*	I2SCLK enable/disable for I2S block. 0: Disable		*/
#define SCU_SCLKEN_UCLK_UART1_EN			(0x1 << 1)		/*	UCLK enable/disable for UART1 block. 1: Enable		*/
#define SCU_SCLKEN_UCLK_UART1_DIS			(0x01)			/*	UCLK enable/disable for UART1 block. 0: Disable	*/
#define SCU_SCLKEN_UCLK_UART0_EN			(0x1 << 0)		/*	UCLK enable/disable for UART0 block. 1: Enable		*/
#define SCU_SCLKEN_UCLK_UART0_DIS			(0x0)			/*	UCLK enable/disable for UART0 block. 0: Disable	*/

/*	SCU_PWRMODE	*/
	/*	the power mode control bit	*/
#define SCU_PWRMODE_IDLE_EN				(0x1 << 17)		/*	IDLE Mode, When set this bit to "1", then system enters into IDLE mode	*/
#define SCU_PWRMODE_STOP_EN				(0x1 << 16)		/*	STOP Mode, When set this bit to "1", then system enters into STOP mode	*/
#define SCU_PWRMODE_SLEEP_M				(0xffff)			/*	SLEEP Mode mask	*/
#define SCU_PWRMODE_SLEEP_EN				(0x2bed)			/*	SLEEP Mode, When write "0x2bed" to this bits, then the system enters into SLEEP mode	*/

/*	SCU_PWRCFG	*/
	/*	the power management configuration bit	*/
#define SCU_PWRCFG_FILTER_GPIO7_EN				(0x1 << 21)		/*	Filter enable for GPIO[7]. 1: Enable	*/
#define SCU_PWRCFG_FILTER_GPIO7_DIS				(0x0)			/*	Filter enable for GPIO[7]. 0: Disable	*/
#define SCU_PWRCFG_FILTER_GPIO6_EN				(0x1 << 20)		/*	Filter enable for GPIO[6]. 1: Enable	*/
#define SCU_PWRCFG_FILTER_GPIO6_DIS				(0x0)			/*	Filter enable for GPIO[6]. 0: Disable	*/
#define SCU_PWRCFG_FILTER_GPIO5_EN				(0x1 << 19)		/*	Filter enable for GPIO[5]. 1: Enable	*/
#define SCU_PWRCFG_FILTER_GPIO5_DIS				(0x0)			/*	Filter enable for GPIO[5]. 0: Disable	*/
#define SCU_PWRCFG_FILTER_GPIO4_EN				(0x1 << 18)		/*	Filter enable for GPIO[4]. 1: Enable	*/
#define SCU_PWRCFG_FILTER_GPIO4_DIS				(0x08)			/*	Filter enable for GPIO[4]. 0: Disable	*/
#define SCU_PWRCFG_FILTER_GPIO3_EN				(0x1 << 17)		/*	Filter enable for GPIO[3]. 1: Enable	*/
#define SCU_PWRCFG_FILTER_GPIO3_DIS				(0x0)			/*	Filter enable for GPIO[3]. 0: Disable	*/
#define SCU_PWRCFG_FILTER_GPIO2_EN				(0x1 << 16)		/*	Filter enable for GPIO[2]. 1: Enable	*/
#define SCU_PWRCFG_FILTER_GPIO2_DIS				(0x0)			/*	Filter enable for GPIO[2]. 0: Disable	*/
#define SCU_PWRCFG_FILTER_GPIO1_EN				(0x1 << 15)		/*	Filter enable for GPIO[1]. 1: Enable	*/
#define SCU_PWRCFG_FILTER_GPIO1_DIS				(0x0)			/*	Filter enable for GPIO[1]. 0: Disable	*/
#define SCU_PWRCFG_FILTER_GPIO0_EN				(0x1 << 14)		/*	Filter enable for GPIO[0]. 1: Enable	*/
#define SCU_PWRCFG_FILTER_GPIO0_DIS				(0x0)			/*	Filter enable for GPIO[0]. 0: Disable	*/
#define SCU_PWRCFG_FILTER_EXT_INT1_EN			(0x1 << 13)		/*	Filter enable for EXT_INT[1]. 1: Enable	*/
#define SCU_PWRCFG_FILTER_EXT_INT1_DIS			(0x0)			/*	Filter enable for EXT_INT[1]. 0: Disable	*/
#define SCU_PWRCFG_FILTER_EXT_INT0_EN			(0x1 << 12)		/*	Filter enable for EXT_INT[0]. 1: Enable	*/
#define SCU_PWRCFG_FILTER_EXT_INT0_DIS			(0x0)			/*	Filter enable for EXT_INT[0]. 0: Disable	*/
#define SCU_PWRCFG_EXT_INT1_TRIG_TYPE_M			(0x3 << 9)		/*	External interrupt 1 trigger type mask	*/
#define SCU_PWRCFG_EXT_INT1_TRIG_TYPE_HI_LV		(0x0)			/*	External interrupt 1 trigger type : High-level	*/
#define SCU_PWRCFG_EXT_INT1_TRIG_TYPE_LO_LV		(0x1 << 9)		/*	External interrupt 1 trigger type : Low-level		*/
#define SCU_PWRCFG_EXT_INT1_TRIG_TYPE_RIS_EG	(0x2 << 9)		/*	External interrupt 1 trigger type : Rising-edge	*/
#define SCU_PWRCFG_EXT_INT1_TRIG_TYPE_FAL_EG	(0x3 << 9)		/*	External interrupt 1 trigger type : Falling-edge	*/
#define SCU_PWRCFG_EXT_INT0_TRIG_TYPE_M			(0x3 << 7)		/*	External interrupt 0 trigger type mask	*/
#define SCU_PWRCFG_EXT_INT0_TRIG_TYPE_HI_LV		(0x0)			/*	External interrupt 0 trigger type : High-level	*/
#define SCU_PWRCFG_EXT_INT0_TRIG_TYPE_LO_LV		(0x1 << 7)		/*	External interrupt 0 trigger type : Low-level		*/
#define SCU_PWRCFG_EXT_INT0_TRIG_TYPE_RIS_EG	(0x2 << 7)		/*	External interrupt 0 trigger type : Rising-edge	*/
#define SCU_PWRCFG_EXT_INT0_TRIG_TYPE_FAL_EG	(0x3 << 7)		/*	External interrupt 0 trigger type : Falling-edge	*/
#define SCU_PWRCFG_STANDBYWIFI_M					(0x3 << 4)		/*	STANDBYWFI configuration mask	*/
#define SCU_PWRCFG_STANDBYWIFI_IGNORE			(0x0)			/*	STANDBYWFI configuration : Ignore STANDBYWFI	*/
#define SCU_PWRCFG_STANDBYWIFI_IDLE				(0x1 << 4)		/*	STANDBYWFI configuration: Use STANDBYWFI as IDLE mode entry command	*/
#define SCU_PWRCFG_STANDBYWIFI_STOP				(0x2 << 4)		/*	STANDBYWFI configuration: Use STANDBYWFI as STOP mode entry command	*/
#define SCU_PWRCFG_MASK_WAKE_SRC_RTC_MASK		(0x1 << 3)		/*	Mask RTC alarm interrupt wake-up source. 0: No mask	*/
#define SCU_PWRCFG_MASK_WAKE_SRC_RTC_UNMASK	(0x0)			/*	Mask RTC alarm interrupt wake-up source. 1: Masked	*/
#define SCU_PWRCFG_MASK_WAKE_SRC_GPIO_MASK	(0x1 << 2)		/*	Mask GPIO interface wake-up source. 0: No mask	*/
#define SCU_PWRCFG_MASK_WAKE_SRC_GPIO_UNMASK	(0x0)			/*	Mask GPIO interface wake-up source. 1: Masked	*/
#define SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT_M		(0x3)			/*	Mask external interrupt wake-up source mask	*/
#define SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT_NO	(0x0)			/*	Mask external interrupt wake-up source 00 : No mask	*/
#define SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT0		(0x1)			/*	Mask external interrupt wake-up source 01 : Mask external interrupt 0	*/
#define SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT1		(0x2)			/*	Mask external interrupt wake-up source 01 : Mask external interrupt 1	*/
#define SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT01		(0x3)			/*	Mask external interrupt wake-up source 01 : Mask external interrupt 0 and 1	*/

/*	SCU_PWREN	*/
	/*	the internal core power gating control	*/	
#define SCU_PWREN_EMB_SRAM1_EN					(0x1 << 7)		/*	Retention control for embedded SRAM block1. 1: Enable	*/
#define SCU_PWREN_EMB_SRAM1_DIS					(0x0)			/*	Retention control for embedded SRAM block1. 0: Disable	*/
#define SCU_PWREN_EMB_SRAM1_RESET				(0x1 << 6)		/*	Reset embedded SRAM block1. 1: Reset	*/
#define SCU_PWREN_EMB_SRAM1_NOACT				(0x0)			/*	Reset embedded SRAM block1. 0: No action	*/
#define SCU_PWREN_EMB_SRAM0_RESET				(0x1 << 5)		/*	Reset embedded SRAM block0. 1: Reset	*/
#define SCU_PWREN_EMB_SRAM0_NOACT				(0x0)			/*	Reset embedded SRAM block0. 0: No action	*/
#define SCU_PWREN_UHC_RESET						(0x1 << 3)		/*	Reset UHC blocks. 1: Reset	*/
#define SCU_PWREN_UHC_NOACT						(0x0)			/*	Reset UHC blocks. 0: No action	*/
#define SCU_PWREN_EMB_SRAM_CORR_PWR_EN			(0x1 << 1)		/*	Core power enable/disable for embedded SRAM block. 1: Enable	*/
#define SCU_PWREN_EMB_SRAM_CORR_PWR_DIS		(0x0)			/*	Core power enable/disable for embedded SRAM block. 0: Disable	*/
#define SCU_PWREN_UHC_CORR_PWR_EN				(0x1 << 0)		/*	Core power enable/disable for UHC block. 1: Enable	*/
#define SCU_PWREN_UHC_CORR_PWR_DIS				(0x0)			/*	Core power enable/disable for UHC block. 0: Disable	*/

/*	SCU_RSTCNT	*/
	/*	reset status bits	*/
#define SCU_RSTCNT_PCLK_CNT_M				(0xffff << 16)	/*	Force reset signal active when internal reset requests are invoked until internal reset counter reaches this bit value mask	*/
#define SCU_RSTCNT_PCLK_CNT_S				16				/*	Force reset signal active when internal reset requests are invoked until internal reset counter reaches this bit value shift	*/
#define SCU_RSTCNT_EXT_CNT_M				(0xffff)			/*	Force reset signal active during the external power source settle down when a wake-up procedure from SLEEP mode is being started mask	*/
#define SCU_RSTCNT_EXT_CNT_S				0				/*	Force reset signal active during the external power source settle down when a wake-up procedure from SLEEP mode is being started shift	*/

/*	SCU_RSTSTAT	*/
	/*	the power mode control bit	*/
#define SCU_RSTSTAT_SW_RESET				(0x1 << 3)		/*	Software reset happened by writing SCU_SWRST register, if this bit is 1. This bit is cleared by the other reset	*/
#define SCU_RSTSTAT_WAKEUP_RESET			(0x1 << 2)		/*	Wake-up reset happened from SLEEP mode, if this bit is 1. This bit is cleared by other reset	*/
#define SCU_RSTSTAT_WDT_RESET				(0x1 << 1)		/*	Watch-dog reset happened, if this bit is 1. This bit is cleared by the other reset	*/
#define SCU_RSTSTAT_HW_RESET				(0x1 << 0)		/*	Hardware reset happened (from POR_N pin), if this bit is 1. This bit is cleared by the other reset	*/

/*	SCU_WKUPSTAT	*/
	/*	the trigger source for exiting SLEEP mode	*/
#define SCU_WKUPSTAT_RTC					(0x1 << 2)		/*	Wake-up by RTC alarm. This bit is cleared by write "1"	*/
#define SCU_WKUPSTAT_GPIO					(0x1 << 1)		/*	Wake-up by GPIO toggling. This bit is cleared by write "1"	*/
#define SCU_WKUPSTAT_EXT_INT				(0x1 << 0)			/*	Wake-up by external interrupts[1:0]. This bit is cleared by write "1"	*/

/*	SCU_MEMISLP		*/
	/*	the output pin value of ST/SDR memory interface in SLEEP mode	*/
#define SCU_MEMISLP_SD_ADDR_M				(0x3 << 24)		/*	SDRAM memory SD_ADDR pin configuration. mask		*/
#define SCU_MEMISLP_SD_ADDR_S					24				/*	SDRAM memory SD_ADDR pin configuration. shift		*/
#define SCU_MEMISLP_SD_ADDR_OUTPUT0			(0x0)			/*	SDRAM memory SD_ADDR pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_SD_ADDR_OUTPUT1			(0x1 << 24)		/*	SDRAM memory SD_ADDR pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_SD_ADDR_OUTPUT_DIS		(0x2 << 24)		/*	SDRAM memory SD_ADDR pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_MEMISLP_SD_DQ_M					(0x3 << 22)		/*	SDRAM memory SD_DQ pin configuration. mask		*/
#define SCU_MEMISLP_SD_DQ_S					22				/*	SDRAM memory SD_DQ pin configuration. shift		*/
#define SCU_MEMISLP_SD_DQ_OUTPUT0			(0x0)			/*	SDRAM memory SD_DQ pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_SD_DQ_OUTPUT1			(0x1 << 22)		/*	SDRAM memory SD_DQ pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_SD_DQ_INPUT				(0x2 << 22)		/*	SDRAM memory SD_DQ pin configuration. 10: Input (Hi-Z)	*/
#define SCU_MEMISLP_SD_CEN_M					(0x3 << 20)		/*	SDRAM memory SD_CEN pin configuration. mask		*/
#define SCU_MEMISLP_SD_CEN_S					20				/*	SDRAM memory SD_CEN pin configuration. shift		*/
#define SCU_MEMISLP_SD_CEN_OUTPUT0			(0x0)			/*	SDRAM memory SD_CEN pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_SD_CEN_OUTPUT1			(0x1 << 20)		/*	SDRAM memory SD_CEN pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_SD_CEN_OUTPUT_DIS		(0x2 << 20)		/*	SDRAM memory SD_CEN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_MEMISLP_SD_CASN_M					(0x3 << 18)		/*	SDRAM memory SD_CASN pin configuration. mask		*/
#define SCU_MEMISLP_SD_CASN_S					18				/*	SDRAM memory SD_CASN pin configuration. shift		*/
#define SCU_MEMISLP_SD_CASN_OUTPUT0			(0x0)			/*	SDRAM memory SD_CASN pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_SD_CASN_OUTPUT1			(0x1 << 18)		/*	SDRAM memory SD_CASN pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_SD_CASN_OUTPUT_DIS		(0x2 << 18)		/*	SDRAM memory SD_CASN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_MEMISLP_SD_RASN_M					(0x3 << 16)		/*	SDRAM memory SD_RASN pin configuration. mask		*/
#define SCU_MEMISLP_SD_RASN_S					16				/*	SDRAM memory SD_RASN pin configuration. shift		*/
#define SCU_MEMISLP_SD_RASN_OUTPUT0			(0x0)			/*	SDRAM memory SD_RASN pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_SD_RASN_OUTPUT1			(0x1 << 16)		/*	SDRAM memory SD_RASN pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_SD_RASN_OUTPUT_DIS		(0x2 << 16)		/*	SDRAM memory SD_RASN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_MEMISLP_SD_WEN_M					(0x3 << 14)		/*	SDRAM memory SD_WEN pin configuration. mask		*/
#define SCU_MEMISLP_SD_WEN_S					14				/*	SDRAM memory SD_WEN pin configuration. shift		*/
#define SCU_MEMISLP_SD_WEN_OUTPUT0			(0x0)			/*	SDRAM memory SD_WEN pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_SD_WEN_OUTPUT1			(0x1 << 14)		/*	SDRAM memory SD_WEN pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_SD_WEN_OUTPUT_DIS		(0x2 << 14)		/*	SDRAM memory SD_WEN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_MEMISLP_SD_DQM_M					(0x3 << 12)		/*	SDRAM memory SD_DQM pin configuration. mask		*/
#define SCU_MEMISLP_SD_DQM_S					12				/*	SDRAM memory SD_DQM pin configuration. shift		*/
#define SCU_MEMISLP_SD_DQM_OUTPUT0			(0x0)			/*	SDRAM memory SD_DQM pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_SD_DQM_OUTPUT1			(0x1 << 12)		/*	SDRAM memory SD_DQM pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_SD_DQM_OUTPUT_DIS		(0x2 << 12)		/*	SDRAM memory SD_DQM pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_MEMISLP_SD_BA_M					(0x3 << 10)		/*	SDRAM memory SD_BA pin configuration. mask		*/
#define SCU_MEMISLP_SD_BA_S					10				/*	SDRAM memory SD_BA pin configuration. shift		*/
#define SCU_MEMISLP_SD_BA_OUTPUT0			(0x0)			/*	SDRAM memory SD_BA pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_SD_BA_OUTPUT1			(0x1 << 10)		/*	SDRAM memory SD_BA pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_SD_BA_OUTPUT_DIS			(0x2 << 10)		/*	SDRAM memory SD_BA pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_MEMISLP_SD_CKE_M					(0x3 << 8)		/*	SDRAM memory SD_CKE pin configuration. msk		*/
#define SCU_MEMISLP_SD_CKE_S					8				/*	SDRAM memory SD_CKE pin configuration. shift		*/
#define SCU_MEMISLP_SD_CKE_OUTPUT0			(0x0)			/*	SDRAM memory SD_CKE pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_SD_CKE_OUTPUT1			(0x1 << 8)		/*	SDRAM memory SD_CKE pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_SD_CKE_OUTPUT_DIS		(0x2 << 8)		/*	SDRAM memory SD_CKE pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_MEMISLP_SD_CLKOUT_M				(0x3 << 6)		/*	SDRAM memory SD_CLKOUT pin configuration. mask		*/
#define SCU_MEMISLP_SD_CLKOUT_S				6				/*	SDRAM memory SD_CLKOUT pin configuration. shift		*/
#define SCU_MEMISLP_SD_CLKOUT_OUTPUT0		(0x0)			/*	SDRAM memory SD_CLKOUT pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_SD_CLKOUT_OUTPUT1		(0x1 << 6)		/*	SDRAM memory SD_CLKOUT pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_SD_CLKOUT_OUTPUT_DIS	(0x2 << 6)		/*	SDRAM memory SD_CLKOUT pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_MEMISLP_ST_CSN_M					(0x3 << 4)		/*	Static  memory ST_CSN pin configuration. mask		*/
#define SCU_MEMISLP_ST_CSN_S					4				/*	Static  memory ST_CSN pin configuration. 00: shift		*/
#define SCU_MEMISLP_ST_CSN_OUTPUT0			(0x0)			/*	Static  memory ST_CSN pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_ST_CSN_OUTPUT1			(0x1 << 4)		/*	Static  memory ST_CSN pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_ST_CSN_OUTPUT_DIS		(0x2 << 4)		/*	Static  memory ST_CSN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_MEMISLP_ST_OEN_M					(0x3 << 2)		/*	Static  memory ST_OEN pin configuration. mask		*/
#define SCU_MEMISLP_ST_OEN_S					2				/*	Static  memory ST_OEN pin configuration. shift		*/
#define SCU_MEMISLP_ST_OEN_OUTPUT0			(0x0)			/*	Static  memory ST_OEN pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_ST_OEN_OUTPUT1			(0x1 << 2)		/*	Static  memory ST_OEN pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_ST_OEN_OUTPUT_DIS		(0x2 << 2)		/*	Static  memory ST_OEN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_MEMISLP_ST_WEN_M					(0x3)			/*	Static  memory ST_WEN pin configuration. mask		*/
#define SCU_MEMISLP_ST_WEN_S					0				/*	Static  memory ST_WEN pin configuration. shift		*/
#define SCU_MEMISLP_ST_WEN_OUTPUT0			(0x0)			/*	Static  memory ST_WEN pin configuration. 00: Output 0		*/
#define SCU_MEMISLP_ST_WEN_OUTPUT1			(0x1)			/*	Static  memory ST_WEN pin configuration. 01: Output 1		*/
#define SCU_MEMISLP_ST_WEN_OUTPUT_DIS		(0x2)			/*	Static  memory ST_WEN pin configuration. 10: Output disable (Hi-Z)	*/

/*	SCU_NFISLP	*/
	/*	the output pin value of NAND-Flash memory interface in SLEEP mode	*/
#define SCU_NFISLP_NF_WPN_M					(0x3 << 12)		/*	NAND-Flash NF_WPN pin configuration. mask		*/
#define SCU_NFISLP_NF_WPN_S					12				/*	NAND-Flash NF_WPN pin configuration. shift		*/
#define SCU_NFISLP_NF_WPN_OUTPUT0			(0x0)			/*	NAND-Flash NF_WPN pin configuration. 00: Output 0		*/
#define SCU_NFISLP_NF_WPN_OUTPUT1			(0x1 << 12)		/*	NAND-Flash NF_WPN pin configuration. 01: Output 1		*/
#define SCU_NFISLP_NF_WPN_OUTPUT_DIS			(0x2 << 12)		/*	NAND-Flash NF_WPN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_NFISLP_NF_WEN_M					(0x3 << 10)		/*	NAND-Flash NF_WEN pin configuration. mask		*/
#define SCU_NFISLP_NF_WEN_S					10				/*	NAND-Flash NF_WEN pin configuration. shift		*/
#define SCU_NFISLP_NF_WEN_OUTPUT0			(0x0)			/*	NAND-Flash NF_WEN pin configuration. 00: Output 0		*/
#define SCU_NFISLP_NF_WEN_OUTPUT1			(0x1 << 10)		/*	NAND-Flash NF_WEN pin configuration. 01: Output 1		*/
#define SCU_NFISLP_NF_WEN_OUTPUT_DIS			(0x2 << 10)		/*	NAND-Flash NF_WEN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_NFISLP_NF_REN_M					(0x3 << 8)		/*	NAND-Flash NF_REN pin configuration. mask		*/
#define SCU_NFISLP_NF_REN_S					8				/*	NAND-Flash NF_REN pin configuration. shift		*/
#define SCU_NFISLP_NF_REN_OUTPUT0			(0x0)			/*	NAND-Flash NF_REN pin configuration. 00: Output 0		*/
#define SCU_NFISLP_NF_REN_OUTPUT1			(0x1 << 8)		/*	NAND-Flash NF_REN pin configuration. 01: Output 1		*/
#define SCU_NFISLP_NF_REN_OUTPUT_DIS			(0x2 << 8)		/*	NAND-Flash NF_REN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_NFISLP_NF_CEN_M					(0x3 << 6)		/*	NAND-Flash NF_CEN pin configuration. mask		*/
#define SCU_NFISLP_NF_CEN_S					6				/*	NAND-Flash NF_CEN pin configuration. shift		*/
#define SCU_NFISLP_NF_CEN_OUTPUT0			(0x0)			/*	NAND-Flash NF_CEN pin configuration. 00: Output 0		*/
#define SCU_NFISLP_NF_CEN_OUTPUT1			(0x1 << 6)		/*	NAND-Flash NF_CEN pin configuration. 01: Output 1		*/
#define SCU_NFISLP_NF_CEN_OUTPUT_DIS			(0x2 << 6)		/*	NAND-Flash NF_CEN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_NFISLP_NF_ALE_M					(0x3 << 4)		/*	NAND-Flash NF_ALE pin configuration. mask		*/
#define SCU_NFISLP_NF_ALE_S					4				/*	NAND-Flash NF_ALE pin configuration. shift		*/
#define SCU_NFISLP_NF_ALE_OUTPUT0				(0x0)			/*	NAND-Flash NF_ALE pin configuration. 00: Output 0		*/
#define SCU_NFISLP_NF_ALE_OUTPUT1				(0x1 << 4)		/*	NAND-Flash NF_ALE pin configuration. 01: Output 1		*/
#define SCU_NFISLP_NF_ALE_OUTPUT_DIS			(0x2 << 4)		/*	NAND-Flash NF_ALE pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_NFISLP_NF_CLE_M					(0x3 << 2)		/*	NAND-Flash NF_CLE pin configuration. mask		*/
#define SCU_NFISLP_NF_CLE_S					2				/*	NAND-Flash NF_CLE pin configuration. shift		*/
#define SCU_NFISLP_NF_CLE_OUTPUT0				(0x0)			/*	NAND-Flash NF_CLE pin configuration. 00: Output 0		*/
#define SCU_NFISLP_NF_CLE_OUTPUT1				(0x1 << 2)		/*	NAND-Flash NF_CLE pin configuration. 01: Output 1		*/
#define SCU_NFISLP_NF_CLE_OUTPUT_DIS			(0x2 << 2)		/*	NAND-Flash NF_CLE pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_NFISLP_NF_IO_M						(0x3)			/*	NAND-Flash NF_IO pin configuration. mask		*/
#define SCU_NFISLP_NF_IO_S						0				/*	NAND-Flash NF_IO pin configuration. shift		*/
#define SCU_NFISLP_NF_IO_OUTPUT0				(0x0)			/*	NAND-Flash NF_IO pin configuration. 00: Output 0		*/
#define SCU_NFISLP_NF_IO_OUTPUT1				(0x1)			/*	NAND-Flash NF_IO pin configuration. 01: Output 1		*/
#define SCU_NFISLP_NF_IO_OUTPUT_DIS			(0x2)			/*	NAND-Flash NF_IO pin configuration. 10: Output disable (Hi-Z)	*/

/*	SCU_USBISLP	*/
	/*	the output pin value of USB interface in SLEEP mode	*/
#define SCU_USBISLP_USB_OPMODE_M					(0x3 << 28)		/*	USB USB_OPMODE pin configuration. mask		*/
#define SCU_USBISLP_USB_OPMODE_S					28				/*	USB USB_OPMODE pin configuration. shift		*/
#define SCU_USBISLP_USB_OPMODE_OUTPUT0			(0x0)			/*	USB USB_OPMODE pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_OPMODE_OUTPUT1			(0x1 << 28)		/*	USB USB_OPMODE pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_OPMODE_OUTPUT_DIS		(0x2 << 28)		/*	USB USB_OPMODE pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_TERM_SEL_M				(0x3 << 26)		/*	USB USB_TERM_SEL pin configuration. mask		*/
#define SCU_USBISLP_USB_TERM_SEL_S				26				/*	USB USB_TERM_SEL pin configuration. shift		*/
#define SCU_USBISLP_USB_TERM_SEL_OUTPUT0		(0x0)			/*	USB USB_TERM_SEL pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_TERM_SEL_OUTPUT1		(0x1 << 26)		/*	USB USB_TERM_SEL pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_TERM_SEL_OUTPUT_DIS		(0x2 << 26)		/*	USB USB_TERM_SEL pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_XCVR_SEL_M				(0x3 << 24)		/*	USB USB_XCVR_SEL pin configuration. mask		*/
#define SCU_USBISLP_USB_XCVR_SEL_S				24				/*	USB USB_XCVR_SEL pin configuration. shift		*/
#define SCU_USBISLP_USB_XCVR_SEL_OUTPUT0			(0x0)			/*	USB USB_XCVR_SEL pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_XCVR_SEL_OUTPUT1			(0x1 << 24)		/*	USB USB_XCVR_SEL pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_XCVR_SEL_OUTPUT_DIS		(0x2 << 24)		/*	USB USB_XCVR_SEL pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_DP_PULLDOWN_M			(0x3 << 22)		/*	USB USB_DP_PULLDOWNpin configuration. mask		*/
#define SCU_USBISLP_USB_DP_PULLDOWN_S			22				/*	USB USB_DP_PULLDOWNpin configuration. shift		*/
#define SCU_USBISLP_USB_DP_PULLDOWN_OUTPUT0	(0x0)			/*	USB USB_DP_PULLDOWN pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_DP_PULLDOWN_OUTPUT1	(0x1 << 22)		/*	USB USB_DP_PULLDOWN pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_DP_PULLDOWN_OUTPUT_DIS	(0x2 << 22)		/*	USB USB_DP_PULLDOWN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_DM_PULLDOWN_M			(0x3 << 20)		/*	USB USB_DM_PULLDOWN pin configuration. mask		*/
#define SCU_USBISLP_USB_DM_PULLDOWN_S			20				/*	USB USB_DM_PULLDOWN pin configuration. shift		*/
#define SCU_USBISLP_USB_DM_PULLDOWN_OUTPUT0	(0x0)			/*	USB USB_DM_PULLDOWN pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_DM_PULLDOWN_OUTPUT1	(0x1 << 20)		/*	USB USB_DM_PULLDOWN pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_DM_PULLDOWN_OUTPUT_DIS	(0x2 << 20)		/*	USB USB_DM_PULLDOWN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_DATAI_M					(0x3 << 18)		/*	USB USB_DATAI pin configuration. mask		*/
#define SCU_USBISLP_USB_DATAI_S					18				/*	USB USB_DATAI pin configuration. shift		*/
#define SCU_USBISLP_USB_DATAI_OUTPUT0			(0x0)			/*	USB USB_DATAI pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_DATAI_OUTPUT1			(0x1 << 18)		/*	USB USB_DATAI pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_DATAI_OUTPUT_DIS			(0x2 << 18)		/*	USB USB_DATAI pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_TX_VALID_M				(0x3 << 16)		/*	USB USB_TX_VALID pin configuration. mask		*/
#define SCU_USBISLP_USB_TX_VALID_S				16				/*	USB USB_TX_VALID pin configuration. shift		*/
#define SCU_USBISLP_USB_TX_VALID_OUTPUT0			(0x0)			/*	USB USB_TX_VALID pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_TX_VALID_OUTPUT1			(0x1 << 16)		/*	USB USB_TX_VALID pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_TX_VALID_OUTPUT_DIS		(0x2 << 16)		/*	USB USB_TX_VALID pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_TX_VALIDH_M				(0x3 << 14)		/*	USB USB_TX_VALIDH pin configuration. mask		*/
#define SCU_USBISLP_USB_TX_VALIDH_S				14				/*	USB USB_TX_VALIDH pin configuration. shift		*/
#define SCU_USBISLP_USB_TX_VALIDH_OUTPUT0		(0x0)			/*	USB USB_TX_VALIDH pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_TX_VALIDH_OUTPUT1		(0x1 << 14)		/*	USB USB_TX_VALIDH pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_TX_VALIDH_OUTPUT_DIS	(0x2 << 14)		/*	USB USB_TX_VALIDH pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_FS_DATA_EXT_M			(0x3 << 12)		/*	USB USB_FS_DATA_EXT pin configuration. mask		*/
#define SCU_USBISLP_USB_FS_DATA_EXT_S			12				/*	USB USB_FS_DATA_EXT pin configuration. shift		*/
#define SCU_USBISLP_USB_FS_DATA_EXT_OUTPUT0		(0x0)			/*	USB USB_FS_DATA_EXT pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_FS_DATA_EXT_OUTPUT1		(0x1 << 12)		/*	USB USB_FS_DATA_EXT pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_FS_DATA_EXT_OUTPUT_DIS	(0x2 << 12)		/*	USB USB_FS_DATA_EXT pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_FS_SE0_EXT_M				(0x3 << 10)		/*	USB USB_FS_SE0_EXT pin configuration. mask		*/
#define SCU_USBISLP_USB_FS_SE0_EXT_S				10				/*	USB USB_FS_SE0_EXT pin configuration. shift		*/
#define SCU_USBISLP_USB_FS_SE0_EXT_OUTPUT0		(0x0)			/*	USB USB_FS_SE0_EXT pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_FS_SE0_EXT_OUTPUT1		(0x1 << 10)		/*	USB USB_FS_SE0_EXT pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_FS_SE0_EXT_OUTPUT_DIS	(0x2 << 10)		/*	USB USB_FS_SE0_EXT pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_TX_ENB_M					(0x3 << 8)		/*	USB USB_TX_ENB pin configuration. mask		*/
#define SCU_USBISLP_USB_TX_ENB_S					8				/*	USB USB_TX_ENB pin configuration. shift		*/
#define SCU_USBISLP_USB_TX_ENB_OUTPUT0			(0x0)			/*	USB USB_TX_ENB pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_TX_ENB_OUTPUT1			(0x1 << 8)		/*	USB USB_TX_ENB pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_TX_ENB_OUTPUT_DIS		(0x2 << 8)		/*	USB USB_TX_ENB pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_FS_XCVR_OWN_M			(0x3 << 6)		/*	USB USB_FS_XCVR_OWN pin configuration. mask		*/
#define SCU_USBISLP_USB_FS_XCVR_OWN_S			6				/*	USB USB_FS_XCVR_OWN pin configuration. shift		*/
#define SCU_USBISLP_USB_FS_XCVR_OWN_OUTPUT0	(0x0)			/*	USB USB_FS_XCVR_OWN pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_FS_XCVR_OWN_OUTPUT1	(0x1 << 6)		/*	USB USB_FS_XCVR_OWN pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_FS_XCVR_OWN_OUTPUT_DIS	(0x2 << 6)		/*	USB USB_FS_XCVR_OWN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_POR_M						(0x3 << 4)		/*	USB USB_POR pin configuration. mask		*/
#define SCU_USBISLP_USB_POR_S						4				/*	USB USB_POR pin configuration. shift		*/
#define SCU_USBISLP_USB_POR_OUTPUT0				(0x0)			/*	USB USB_POR pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_POR_OUTPUT1				(0x1 << 4)		/*	USB USB_POR pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_POR_OUTPUT_DIS			(0x2 << 4)		/*	USB USB_POR pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_USB_SUSPENDM_M				(0x3 << 2)		/*	USB USB_SUSPENDM pin configuration. mask		*/
#define SCU_USBISLP_USB_SUSPENDM_S				2				/*	USB USB_SUSPENDM pin configuration. shift		*/
#define SCU_USBISLP_USB_SUSPENDM_OUTPUT0		(0x0)			/*	USB USB_SUSPENDM pin configuration. 00: Output 0		*/
#define SCU_USBISLP_USB_SUSPENDM_OUTPUT1		(0x1 << 2)		/*	USB USB_SUSPENDM pin configuration. 01: Output 1		*/
#define SCU_USBISLP_USB_SUSPENDM_OUTPUT_DIS		(0x2 << 2)		/*	USB USB_SUSPENDM pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_USBISLP_VBUS_UHC_M					(0x3)			/*	USB VBUS_UHC pin configuration. mask		*/
#define SCU_USBISLP_VBUS_UHC_S					0				/*	USB VBUS_UHC pin configuration. shift		*/
#define SCU_USBISLP_VBUS_UHC_OUTPUT0				(0x0)			/*	USB VBUS_UHC pin configuration. 00: Output 0		*/
#define SCU_USBISLP_VBUS_UHC_OUTPUT1				(0x1)			/*	USB VBUS_UHC pin configuration. 01: Output 1		*/
#define SCU_USBISLP_VBUS_UHC_OUTPUT_DIS			(0x2)			/*	USB VBUS_UHC pin configuration. 10: Output disable (Hi-Z)	*/

/*	SCU_LCDISLP	*/
	/*	the output pin value of LCD interface in SLEEP mode	*/
#define SCU_LCDISLP_LCD_AC_M						(0x3 << 12)		/*	LCD LCD_AC pin configuration. mask		*/
#define SCU_LCDISLP_LCD_AC_S						12				/*	LCD LCD_AC pin configuration. shift		*/
#define SCU_LCDISLP_LCD_AC_OUTPUT0				(0x0)			/*	LCD LCD_AC pin configuration. 00: Output 0		*/
#define SCU_LCDISLP_LCD_AC_OUTPUT1				(0x1 << 12)		/*	LCD LCD_AC pin configuration. 01: Output 1		*/
#define SCU_LCDISLP_LCD_AC_OUTPUT_DIS			(0x2 << 12)		/*	LCD LCD_AC pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_LCDISLP_LCD_CP_M						(0x3 << 10)		/*	LCD LCD_CP pin configuration. mask		*/
#define SCU_LCDISLP_LCD_CP_S						10				/*	LCD LCD_CP pin configuration. shift		*/
#define SCU_LCDISLP_LCD_CP_OUTPUT0				(0x0)			/*	LCD LCD_CP pin configuration. 00: Output 0		*/
#define SCU_LCDISLP_LCD_CP_OUTPUT1				(0x1 << 10)		/*	LCD LCD_CP pin configuration. 01: Output 1		*/
#define SCU_LCDISLP_LCD_CP_OUTPUT_DIS			(0x2 << 10)		/*	LCD LCD_CP pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_LCDISLP_LCD_DATA_M					(0x3 << 8)		/*	LCD LCD_DATA pin configuration. mask		*/
#define SCU_LCDISLP_LCD_DATA_S					8				/*	LCD LCD_DATA pin configuration. shift		*/
#define SCU_LCDISLP_LCD_DATA_OUTPUT0				(0x0)			/*	LCD LCD_DATA pin configuration. 00: Output 0		*/
#define SCU_LCDISLP_LCD_DATA_OUTPUT1				(0x1 << 8)		/*	LCD LCD_DATA pin configuration. 01: Output 1		*/
#define SCU_LCDISLP_LCD_DATA_OUTPUT_DIS			(0x2 << 8)		/*	LCD LCD_DATA pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_LCDISLP_LCD_FP_M						(0x3 << 6)		/*	LCD LCD_FP pin configuration. mask		*/
#define SCU_LCDISLP_LCD_FP_S						6				/*	LCD LCD_FP pin configuration. shift		*/
#define SCU_LCDISLP_LCD_FP_OUTPUT0				(0x0)			/*	LCD LCD_FP pin configuration. 00: Output 0		*/
#define SCU_LCDISLP_LCD_FP_OUTPUT1				(0x1 << 6)		/*	LCD LCD_FP pin configuration. 01: Output 1		*/
#define SCU_LCDISLP_LCD_FP_OUTPUT_DIS			(0x2 << 6)		/*	LCD LCD_FP pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_LCDISLP_LCD_LE_M						(0x3 << 4)		/*	LCD LCD_LE pin configuration. mask		*/
#define SCU_LCDISLP_LCD_LE_S						4				/*	LCD LCD_LE pin configuration. shift		*/
#define SCU_LCDISLP_LCD_LE_OUTPUT0				(0x0)			/*	LCD LCD_LE pin configuration. 00: Output 0		*/
#define SCU_LCDISLP_LCD_LE_OUTPUT1				(0x1 << 4)		/*	LCD LCD_LE pin configuration. 01: Output 1		*/
#define SCU_LCDISLP_LCD_LE_OUTPUT_DIS				(0x2 << 4)		/*	LCD LCD_LE pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_LCDISLP_LCD_LP_M						(0x3 << 2)		/*	LCD LCD_LP pin configuration. mask		*/
#define SCU_LCDISLP_LCD_LP_S						2				/*	LCD LCD_LP pin configuration. shift		*/
#define SCU_LCDISLP_LCD_LP_OUTPUT0				(0x0)			/*	LCD LCD_LP pin configuration. 00: Output 0		*/
#define SCU_LCDISLP_LCD_LP_OUTPUT1				(0x1 << 2)		/*	LCD LCD_LP pin configuration. 01: Output 1		*/
#define SCU_LCDISLP_LCD_LP_OUTPUT_DIS				(0x2 << 2)		/*	LCD LCD_LP pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_LCDISLP_LCD_POWER_M					(0x3)			/*	LCD LCD_POWER pin configuration. mask		*/
#define SCU_LCDISLP_LCD_POWER_S					0				/*	LCD LCD_POWER pin configuration. shift		*/
#define SCU_LCDISLP_LCD_POWER_OUTPUT0			(0x0)			/*	LCD LCD_POWER pin configuration. 00: Output 0		*/
#define SCU_LCDISLP_LCD_POWER_OUTPUT1			(0x1)			/*	LCD LCD_POWER pin configuration. 01: Output 1		*/
#define SCU_LCDISLP_LCD_POWER_OUTPUT_DIS		(0x2)			/*	LCD LCD_POWER pin configuration. 10: Output disable (Hi-Z)	*/

/*	SCU_PERI0SLP	*/
	/*	the output pin value of peripherals interface including UART, SPI, I2C, I2S, and PWM in SLEEP mode	*/
#define SCU_PERI0SLP_GPIO_M						(0x3 << 30)		/*	GPIO GPIO pin configuration. mask		*/
#define SCU_PERI0SLP_GPIO_S						30				/*	GPIO GPIO pin configuration. shift		*/
#define SCU_PERI0SLP_GPIO_OUTPUT0					(0x0)			/*	GPIO GPIO pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_GPIO_OUTPUT1					(0x1 << 30)		/*	GPIO GPIO pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_GPIO_OUTPUT_DIS				(0x2 << 30)		/*	GPIO GPIO pin configuration. 10: Input (Hi-Z)	*/
#define SCU_PERI0SLP_UART0_TXD_M					(0x3 << 28)		/*	UART UART0_TXD pin configuration. mask		*/
#define SCU_PERI0SLP_UART0_TXD_S					28				/*	UART UART0_TXD pin configuration. shift		*/
#define SCU_PERI0SLP_UART0_TXD_OUTPUT0			(0x0)			/*	UART UART0_TXD pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_UART0_TXD_OUTPUT1			(0x1 << 28)		/*	UART UART0_TXD pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_UART0_TXD_OUTPUT_DIS		(0x2 << 28)		/*	UART UART0_TXD pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_PERI0SLP_UART1_TXD_M					(0x3 << 26)		/*	UART UART1_TXD pin configuration. mask		*/
#define SCU_PERI0SLP_UART1_TXD_S					26				/*	UART UART1_TXD pin configuration. shift		*/
#define SCU_PERI0SLP_UART1_TXD_OUTPUT0			(0x0)			/*	UART UART1_TXD pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_UART1_TXD_OUTPUT1			(0x1 << 26)		/*	UART UART1_TXD pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_UART1_TXD_OUTPUT_DIS		(0x2 << 26)		/*	UART UART1_TXD pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_PERI0SLP_SPI_SCK_M						(0x3 << 24)		/*	SPI SPI_SCK pin configuration. mask		*/
#define SCU_PERI0SLP_SPI_SCK_S						24				/*	SPI SPI_SCK pin configuration. shift		*/
#define SCU_PERI0SLP_SPI_SCK_OUTPUT0				(0x0)			/*	SPI SPI_SCK pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_SPI_SCK_OUTPUT1				(0x1 << 24)		/*	SPI SPI_SCK pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_SPI_SCK_OUTPUT_DIS			(0x2 << 24)		/*	SPI SPI_SCK pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_PERI0SLP_SPI_SSN_M						(0x3 << 22)		/*	SPI SPI_SSN pin configuration. mask		*/
#define SCU_PERI0SLP_SPI_SSN_S						22				/*	SPI SPI_SSN pin configuration. shift		*/
#define SCU_PERI0SLP_SPI_SSN_OUTPUT0				(0x0)			/*	SPI SPI_SSN pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_SPI_SSN_OUTPUT1				(0x1 << 22)		/*	SPI SPI_SSN pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_SPI_SSN_OUTPUT_DIS			(0x2 << 22)		/*	SPI SPI_SSN pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_PERI0SLP_SPI_MOSI_M					(0x3 << 20)		/*	SPI SPI_MOSI pin configuration. mask		*/
#define SCU_PERI0SLP_SPI_MOSI_S					20				/*	SPI SPI_MOSI pin configuration. shift		*/
#define SCU_PERI0SLP_SPI_MOSI_OUTPUT0			(0x0)			/*	SPI SPI_MOSI pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_SPI_MOSI_OUTPUT1			(0x1 << 20)		/*	SPI SPI_MOSI pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_SPI_MOSI_OUTPUT_DIS			(0x2 << 20)		/*	SPI SPI_MOSI pin configuration. 10: Input (Hi-Z)	*/
#define SCU_PERI0SLP_I2C0_SCL_M					(0x3 << 18)		/*	SPI I2C0_SCL pin configuration. mask		*/
#define SCU_PERI0SLP_I2C0_SCL_S					18				/*	SPI I2C0_SCL pin configuration. shift		*/
#define SCU_PERI0SLP_I2C0_SCL_OUTPUT0				(0x0)			/*	SPI I2C0_SCL pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_I2C0_SCL_OUTPUT1				(0x1 << 18)		/*	SPI I2C0_SCL pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_I2C0_SCL_OUTPUT_DIS			(0x2 << 18)		/*	SPI I2C0_SCL pin configuration. 10: Input (Hi-Z)	*/
#define SCU_PERI0SLP_I2C0_SDA_M					(0x3 << 16)		/*	SPI I2C0_SDA pin configuration. mask		*/
#define SCU_PERI0SLP_I2C0_SDA_S					16				/*	SPI I2C0_SDA pin configuration. shift		*/
#define SCU_PERI0SLP_I2C0_SDA_OUTPUT0			(0x0)			/*	SPI I2C0_SDA pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_I2C0_SDA_OUTPUT1			(0x1 << 16)		/*	SPI I2C0_SDA pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_I2C0_SDA_OUTPUT_DIS			(0x2 << 16)		/*	SPI I2C0_SDA pin configuration. 10: Input (Hi-Z)	*/
#define SCU_PERI0SLP_I2C1_SCL_M					(0x3 << 14)		/*	SPI I2C1_SCL pin configuration. mask		*/
#define SCU_PERI0SLP_I2C1_SCL_S					14				/*	SPI I2C1_SCL pin configuration. shift		*/
#define SCU_PERI0SLP_I2C1_SCL_OUTPUT0				(0x0)			/*	SPI I2C1_SCL pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_I2C1_SCL_OUTPUT1				(0x1 << 14)		/*	SPI I2C1_SCL pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_I2C1_SCL_OUTPUT_DIS			(0x2 << 14)		/*	SPI I2C1_SCL pin configuration. 10: Input (Hi-Z)	*/
#define SCU_PERI0SLP_I2C1_SDA_M					(0x3 << 12)		/*	SPI I2C1_SDA pin configuration. mask		*/
#define SCU_PERI0SLP_I2C1_SDA_S					12				/*	SPI I2C1_SDA pin configuration. shift		*/
#define SCU_PERI0SLP_I2C1_SDA_OUTPUT0			(0x0)			/*	SPI I2C1_SDA pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_I2C1_SDA_OUTPUT1			(0x1 << 12)		/*	SPI I2C1_SDA pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_I2C1_SDA_OUTPUT_DIS			(0x2 << 12)		/*	SPI I2C1_SDA pin configuration. 10: Input (Hi-Z)	*/
#define SCU_PERI0SLP_I2C2_SCL_M					(0x3 << 10)		/*	SPI I2C2_SCL pin configuration. mask		*/
#define SCU_PERI0SLP_I2C2_SCL_S					10				/*	SPI I2C2_SCL pin configuration. shift		*/
#define SCU_PERI0SLP_I2C2_SCL_OUTPUT0				(0x0)			/*	SPI I2C2_SCL pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_I2C2_SCL_OUTPUT1				(0x1 << 10)		/*	SPI I2C2_SCL pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_I2C2_SCL_OUTPUT_DIS			(0x2 << 10)		/*	SPI I2C2_SCL pin configuration. 10: Input (Hi-Z)	*/
#define SCU_PERI0SLP_I2C2_SDA_M					(0x3 << 8)		/*	SPI I2C2_SDA pin configuration. mask		*/
#define SCU_PERI0SLP_I2C2_SDA_S					8				/*	SPI I2C2_SDA pin configuration. shift		*/
#define SCU_PERI0SLP_I2C2_SDA_OUTPUT0			(0x0)			/*	SPI I2C2_SDA pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_I2C2_SDA_OUTPUT1			(0x1 << 8)		/*	SPI I2C2_SDA pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_I2C2_SDA_OUTPUT_DIS			(0x2 << 8)		/*	SPI I2C2_SDA pin configuration. 10: Input (Hi-Z)	*/
#define SCU_PERI0SLP_I2S_TXSCLK_M					(0x3 << 6)		/*	SPI I2S_TXSCLK pin configuration. mask		*/
#define SCU_PERI0SLP_I2S_TXSCLK_S					6				/*	SPI I2S_TXSCLK pin configuration. shift		*/
#define SCU_PERI0SLP_I2S_TXSCLK_OUTPUT0			(0x0)			/*	SPI I2S_TXSCLK pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_I2S_TXSCLK_OUTPUT1			(0x1 << 6)		/*	SPI I2S_TXSCLK pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_I2S_TXSCLK_OUTPUT_DIS		(0x2 << 6)		/*	SPI I2S_TXSCLK pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_PERI0SLP_I2S_TXLRCK_M					(0x3 << 4)		/*	SPI I2S_TXLRCK pin configuration. mask		*/
#define SCU_PERI0SLP_I2S_TXLRCK_S					4				/*	SPI I2S_TXLRCK pin configuration. shift		*/
#define SCU_PERI0SLP_I2S_TXLRCK_OUTPUT0			(0x0)			/*	SPI I2S_TXLRCK pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_I2S_TXLRCK_OUTPUT1			(0x1 << 4)		/*	SPI I2S_TXLRCK pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_I2S_TXLRCK_OUTPUT_DIS		(0x2 << 4)		/*	SPI I2S_TXLRCK pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_PERI0SLP_I2S_SDO_M					(0x3 << 2)		/*	SPI I2S_SDO pin configuration. mask		*/
#define SCU_PERI0SLP_I2S_SDO_S						2				/*	SPI I2S_SDO pin configuration. shift		*/
#define SCU_PERI0SLP_I2S_SDO_OUTPUT0				(0x0)			/*	SPI I2S_SDO pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_I2S_SDO_OUTPUT1				(0x1 << 2)		/*	SPI I2S_SDO pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_I2S_SDO_OUTPUT_DIS			(0x2 << 2)		/*	SPI I2S_SDO pin configuration. 10: Output disable (Hi-Z)	*/
#define SCU_PERI0SLP_PWM_M						(0x3)			/*	PWM PWM pin configuration. mask		*/
#define SCU_PERI0SLP_PWM_S						0				/*	PWM PWM pin configuration. shift		*/
#define SCU_PERI0SLP_PWM_OUTPUT0					(0x0)			/*	PWM PWM pin configuration. 00: Output 0		*/
#define SCU_PERI0SLP_PWM_OUTPUT1					(0x1)			/*	PWM PWM pin configuration. 01: Output 1		*/
#define SCU_PERI0SLP_PWM_OUTPUT_DIS				(0x2)			/*	PWM PWM pin configuration. 10: Input (Hi-Z)	*/

/*	SCU_PERI1SLP	*/
	/*	the output pin value of peripherals interface including SD/MMC in SLEEP mode	*/
#define SCU_PERI1SLP_SDC_CMD_M					(0x3 << 6)		/*	SD/MMC SDC_CMD configuration. mask		*/
#define SCU_PERI1SLP_SDC_CMD_S					6				/*	SD/MMC SDC_CMD configuration. shift		*/
#define SCU_PERI1SLP_SDC_CMD_OUTPUT0				(0x0)			/*	SD/MMC SDC_CMD configuration. 00: Output 0		*/
#define SCU_PERI1SLP_SDC_CMD_OUTPUT1				(0x1 << 6)		/*	SD/MMC SDC_CMD configuration. 01: Output 1		*/
#define SCU_PERI1SLP_SDC_CMD_OUTPUT_DIS			(0x2 << 6)		/*	SD/MMC SDC_CMD configuration. 10: Input (Hi-Z)	*/
#define SCU_PERI1SLP_SDC_DATA_M					(0x3 << 4)		/*	SD/MMC SDC_DATA configuration. mask		*/
#define SCU_PERI1SLP_SDC_DATA_S					4				/*	SD/MMC SDC_DATA configuration. shift		*/
#define SCU_PERI1SLP_SDC_DATA_OUTPUT0			(0x0)			/*	SD/MMC SDC_DATA configuration. 00: Output 0		*/
#define SCU_PERI1SLP_SDC_DATA_OUTPUT1			(0x1 << 4)		/*	SD/MMC SDC_DATA configuration. 01: Output 1		*/
#define SCU_PERI1SLP_SDC_DATA_OUTPUT_DIS		(0x2 << 4)		/*	SD/MMC SDC_DATA configuration. 10: Input (Hi-Z)	*/
#define SCU_PERI1SLP_SDC_PWR_M					(0x3 << 2)		/*	SD/MMC SDC_PWR configuration. mask		*/
#define SCU_PERI1SLP_SDC_PWR_S					2				/*	SD/MMC SDC_PWR configuration. shift		*/
#define SCU_PERI1SLP_SDC_PWR_OUTPUT0				(0x0)			/*	SD/MMC SDC_PWR configuration. 00: Output 0		*/
#define SCU_PERI1SLP_SDC_PWR_OUTPUT1				(0x1 << 2)		/*	SD/MMC SDC_PWR configuration. 01: Output 1		*/
#define SCU_PERI1SLP_SDC_PWR_OUTPUT_DIS			(0x2 << 2)		/*	SD/MMC SDC_PWR configuration. 10: Output disable (Hi-Z)	*/
#define SCU_PERI1SLP_SDC_CLK_M					(0x3)			/*	SD/MMC SDC_CLK configuration. mask		*/
#define SCU_PERI1SLP_SDC_CLK_S						0				/*	SD/MMC SDC_CLK configuration. shift		*/
#define SCU_PERI1SLP_SDC_CLK_OUTPUT0				(0x0)			/*	SD/MMC SDC_CLK configuration. 00: Output 0		*/
#define SCU_PERI1SLP_SDC_CLK_OUTPUT1				(0x1)			/*	SD/MMC SDC_CLK configuration. 01: Output 1		*/
#define SCU_PERI1SLP_SDC_CLK_OUTPUT_DIS			(0x2)			/*	SD/MMC SDC_CLK configuration. 10: Output disable (Hi-Z)	*/

/*	SCU_SLPEN	*/
	/*	SLEEP mode IO configuration register	*/
#define SCU_SLPEN_IO_STAT_CONF_SCU_SLPEN0		(0x0)			/*	IO state configure register in SLEEP mode 0: Set by SCU_SLPEN[0] bit	*/
#define SCU_SLPEN_IO_STAT_CONF_AUTO				(0x1 << 1)		/*	IO state configure register in SLEEP mode Automatically by SLEEP mode		*/
#define SCU_SLPEN_IO_STAT_EN_SCU_REGS			(0x1)			/*	IO state enable register in SLEEP mode. 1: External output pins are controlled by 
																		SCU_MEMISLP, SCU_NFISLP, SCU_LCDISLP, SCU_USBISLP, SCU_PERI0SLP, and SCU_PERI1SLP registers. 
																		This bit is set to "1" automatically when system entry into SLEEP mode	*/
#define SCU_SLPEN_IO_STAT_EN_NOR					(0x0)			/*	IO state enable register in SLEEP mode. 0: Change output into normal function	*/

/*	SCU_IOMODE	*/
	/*	GPIO function register	*/
#define SCU_IOMODE_GPIO7_GPIO7					(0x0)			/*	GPIO[7] I/O function selection. 0: GPIO[7] function	*/
#define SCU_IOMODE_GPIO7_I2C2_SDA				(0x1 << 4)		/*	GPIO[7] I/O function selection. 1: I2C2_SDA function	*/
#define SCU_IOMODE_GPIO6_GPIO6					(0x0)			/*	GPIO[6] I/O function selection. 0: GPIO[6] function	*/
#define SCU_IOMODE_GPIO6_I2C2_SCL					(0x1 << 3)		/*	GPIO[6] I/O function selection. 1: I2C2_SCL function	*/
#define SCU_IOMODE_GPIO5_GPIO5					(0x0)			/*	GPIO[5] I/O function selection. 0: GPIO[5] function	*/
#define SCU_IOMODE_GPIO5_I2C1_SDA				(0x1 << 2)		/*	GPIO[5] I/O function selection. 1: I2C1_SDA function	*/
#define SCU_IOMODE_GPIO4_GPIO4					(0x0)			/*	GPIO[4] I/O function selection. 0: GPIO[4] function	*/
#define SCU_IOMODE_GPIO4_I2C1_SCL					(0x1 << 1)		/*	GPIO[4] I/O function selection. 1: I2C1_SCL function	*/
#define SCU_IOMODE_GPIO0_GPIO0					(0x0)			/*	GPIO[0] I/O function selection. 0: GPIO[0] function	*/
#define SCU_IOMODE_GPIO0_I2C1_PLLOUT				(0x1)			/*	GPIO[0] I/O function selection. 1: PLLOUT function	*/

/*	SCU_CHIPMD	*/
	/*	PLL related setting	*/
#define SCU_CHIPMD_DCM_MODE_NOR				(0x0)			/*	DCM mode setting. This mode setting is from HW pin. 0: Normal mode	*/
#define SCU_CHIPMD_DCM_MODE_DCM				(0x1 << 1)		/*	DCM mode setting. This mode setting is from HW pin. 1: DCM test mode	*/
#define SCU_CHIPMD_PMU_DEBUG_MODE_NOR			(0x0)			/*	PMU Debug mode. All SCUT_xxx register will be active when this bit is HIGH. 0: Normal function modee	*/
#define SCU_CHIPMD_PMU_DEBUG_MODE_DEBUG		(0x1)			/*	PMU Debug mode. All SCUT_xxx register will be active when this bit is HIGH. 1: Debug and test mode	*/

/*	SCU_PLL	*/
	/*	PLL related setting	*/
#define SCU_PLL_RELOCK_STAT_M						(0x3 << 1)		/*	State of PLL re-lock process mask	*/
#define SCU_PLL_RELOCK_STAT_S						1				/*	State of PLL re-lock process shift	*/
#define SCU_PLL_RELOCK_STAT_LOCK					(0x0)			/*	State of PLL re-lock process 00: PLL is locked	*/
#define SCU_PLL_RELOCK_STAT_START_RECONF			(0x1 << 1)		/*	State of PLL re-lock process 01: PLL is start to re-configure	*/
#define SCU_PLL_RELOCK_STAT_ON_RECONF_UNLOCK	(0x2 << 1)		/*	State of PLL re-lock process 02: PLL is on re-configuring and unlocked	*/
#define SCU_PLL_LOCK_COUNTER_EN					(0x0)			/*	Disable PLL lock counter mask 0: Enable PLL lock counter	*/
#define SCU_PLL_LOCK_COUNTER_DIS					(0x1)			/*	Disable PLL lock counter mask 1: Disable PLL lock counter	*/

/*	SCU_DBCT	*/
	/*	PLL related setting	*/
#define SCU_DBCT_DEBOUNCE_TIME_M				(0xffff)			/*	Debounce time for GPIO and EXT_INT noise filter mask	*/
#define SCU_DBCT_DEBOUNCE_TIME_S				0				/*	Debounce time for GPIO and EXT_INT noise filter shift	*/

#endif


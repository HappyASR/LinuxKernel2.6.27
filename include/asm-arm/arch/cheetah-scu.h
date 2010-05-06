/********************************************************************************
* File Name     : include/asm-arm/arch-socle/cheetah-scu.h
* Author        : Ryan Chen
* Description   : Socle PC9220 SCU Service Header
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
*      1. 2008/02/22 ryanchen create this file
*
********************************************************************************/

#ifndef __SOCLE_CHEETAH_SCU_H_INCLUDED
#define __SOCLE_CHEETAH_SCU_H_INCLUDED

#define SOCLE_SCU_CPU_CLOCK_33	0
#define SOCLE_SCU_CPU_CLOCK_40	1
#define SOCLE_SCU_CPU_CLOCK_50	2
#define SOCLE_SCU_CPU_CLOCK_66	3
#define SOCLE_SCU_CPU_CLOCK_83	4
#define SOCLE_SCU_CPU_CLOCK_90	5
#define SOCLE_SCU_CPU_CLOCK_100	6
#define SOCLE_SCU_CPU_CLOCK_120	7
#define SOCLE_SCU_CPU_CLOCK_132	8
#define SOCLE_SCU_CPU_CLOCK_133	9
#define SOCLE_SCU_CPU_CLOCK_166	10

	/* UART clock */
#define SOCLE_SCU_UART_CLOCK_24	0
#define SOCLE_SCU_UART_CLOCK_88	1

	/* CPU/AHB clock ratio	*/
#define SOCLE_SCU_CLOCK_RATIO_1_1		0
#define SOCLE_SCU_CLOCK_RATIO_2_1		1
#define SOCLE_SCU_CLOCK_RATIO_3_1		2
#define SOCLE_SCU_CLOCK_RATIO_4_1		3

	/* select USB Tranceiver play Downstream or Upstream	*/
#define SOCLE_SCU_USB_TRAN_UPSTREAM		0
#define SOCLE_SCU_USB_TRAN_DOWNSTREAM	1

	/*	pclk enable 	*/
#define SOCLE_SCU_ADCCLK			24
#define SOCLE_SCU_PCLK_UART3		21
#define SOCLE_SCU_PCLK_UART2		20
#define SOCLE_SCU_PCLK_UART1		19
#define SOCLE_SCU_PCLK_UART0		18
#define SOCLE_SCU_PCLK_ADC			17
#define SOCLE_SCU_PCLK_PWM		16
#define SOCLE_SCU_PCLK_SDC			15
#define SOCLE_SCU_PCLK_I2S			14
#define SOCLE_SCU_PCLK_I2C			13
#define SOCLE_SCU_PCLK_SPI			12
#define SOCLE_SCU_PCLK_GPIO		11
#define SOCLE_SCU_PCLK_WDT		10
#define SOCLE_SCU_PCLK_RTC			9
#define SOCLE_SCU_PCLK_TMR			8
#define SOCLE_SCU_PCLK_LCD			6
#define SOCLE_SCU_PCLK_NFC			5
#define SOCLE_SCU_PCLK_UDC			4
#define SOCLE_SCU_PCLK_UHC			3
#define SOCLE_SCU_PCLK_MAC			2
#define SOCLE_SCU_PCLK_HDMA		1
#define SOCLE_SCU_PCLK_SDRSTMC	0

	/*	SDRAM data bus width status	*/
#define SOCLE_SCU_SDRAM_BUS_WIDTH_32	 1
#define SOCLE_SCU_SDRAM_BUS_WIDTH_16	 0

	/*	Boot source selection status	*/
#define SOCLE_SCU_BOOT_NOR_16	 	3
#define SOCLE_SCU_BOOT_NOR_8	 	2
#define SOCLE_SCU_BOOT_NAND	 	1
#define SOCLE_SCU_BOOT_ISP_ROM	0


extern unsigned long __init get_pll_clock(void);

extern void socle_scu_ahb_clk_enable(u32 dev);
extern void socle_scu_ahb_clk_disable(u32 dev);

extern void socle_scu_app_clk_enable(u32 dev);
extern void socle_scu_app_clk_disable(u32 dev);

extern void socle_scu_dev_enable(u32 dev);
extern void socle_scu_dev_disable(u32 dev);

extern unsigned long socle_get_cpu_clock(void);		//return CPU clock (Hz)
extern unsigned long socle_get_ahb_clock(void);		//return AHB clock (Hz)
extern unsigned long socle_get_apb_clock(void);		//return APB clock (Hz)

#endif

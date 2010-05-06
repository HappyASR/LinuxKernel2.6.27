/* linux/include/asm/arch-socle/pdk-scu.h
 *
 * Copyright (c) 2006 Socle-tech Corp
 *		      http://www.socle-tech.com.tw/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * PDK SCU configuration
*/

#ifndef __SOCLE_SCUREG_H
#define __SOCLE_SCUREG_H                     1

#include <mach/platform.h>

// offset of regisgers

#define SCU_REG_BASE 			IO_ADDRESS(SOCLE_APB0_SCU)
#define SCU_PLLPARAM_A	        	(SCU_REG_BASE + 0x04)
#define SCU_PLLPARAM_B	        	(SCU_REG_BASE + 0x08)

//SCU_PLLPARAM_A
#define SCU_PLLPARAMA_CPLL_M 		(0x0000ff80)
#define SCU_PLLPARAMA_CPLL_N 		(0x0000007c)
#define SCU_PLLPARAMA_CPLL_OD 		(0x00000003)
#define SCU_PLLPARAMA_CPLL_M_S		7	
#define SCU_PLLPARAMA_CPLL_N_S		2
#define SCU_PLLPARAMA_CPLL_OD_S		0

//SCU_PLLPARAM_B
#define SCU_PLLPARAMB_CPLL_DOWN 		(0x00040000)
#define SCU_PLLPARAMB_CLK_RATIO 		(0x00030000)
#define SCU_PLLPARAMB_CLK_RATIO_S		16	

#endif


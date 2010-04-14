/* linux/include/asm/arch-socle/regs-gpio.h
 *
 * Copyright (c) 2006 Socle-tech Corp
 *		      http://www.socle-tech.com.tw/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __CDK_GPIO_REG_H
#define __CDK_GPIO_REG_H

#include <mach/platform.h>
#include <mach/irqs.h>

#define GPIO_PORT_NUM				17
#define GPIO_PIN_NUM				134
#define GPIO_PIN_NUM_OF_PORT		8

#define CDK_GPIO_REG_BASE			IO_ADDRESS(SOCLE_APB0_MP)
#define CDK_GPIO_INT				IRQ_MPS2

#define CDK_GPIO_OFFSET				0X40

#define CDK_GPIO_DR					0X0000
#define CDK_GPIO_DIR				0X0004
#define CDK_GPIO_PD					0X0008
#define CDK_GPIO_IS					0X000C
#define CDK_GPIO_IBE				0X0010
#define CDK_GPIO_IEV				0X0014
#define CDK_GPIO_INTE				0X0018
#define CDK_GPIO_INTS				0X001C
#define CDK_GPIO_INTC				0X0020
#define CDK_GPIO_TMODE				0X0024

/* CDK_GPIO_TMODE (0X0024) */
#define TMODE_CTRL_MSK				0X6
#define TMODE_CTRL_OFF				0X1
#define TMODE_EN					0X1

#define BIT_MASK(nbits)			((0x1 << (nbits)) - 1)

#endif


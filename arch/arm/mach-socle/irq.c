/*
 *  linux/arch/arm/mach-socle/irq.c
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
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/ptrace.h>
#include <linux/sysdev.h>
#include <linux/irq.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <mach/regs-intr.h>

#include "generic.h"

void socle_mask_irq(unsigned int irq)
{
	INT0_DISABLE(irq);
	INT0_CLR_MASK(irq);
	writel(1 << (irq), INTC0_ICCR);
}

void socle_unmask_irq(unsigned int irq)
{
	INT0_SET_MASK(irq);
	INT0_ENABLE(irq);
}

static struct irq_chip socle_irq_chip = {
	.ack	= socle_mask_irq,
	.mask	= socle_mask_irq,
	.unmask	= socle_unmask_irq,
};

void __init socle_init_irq(void)
{
	unsigned int i;
	// disable all interrupt
	writel(0, INTC0_IECR);
	// clear all interrupt
	writel(0xFFFFFFFF, INTC0_ICCR);	
	/* Disable all interrupts initially. */

	for (i = 0; i < NR_IRQS; i++) {
#if defined(CONFIG_ARCH_PDK_PC9002) || defined(CONFIG_ARCH_CDK) || defined(CONFIG_ARCH_SCDK)
		if((i >= 16) && (i <= 19)){
			INT0_SET_TYPE( i, LO_LEVEL);
		}else if ((i == 29)||(i == 30)) {                       //for ads7846 irq : 30
			INT0_SET_TYPE( i, LO_LEVEL);		//for touch screen tsc2000
		}else if ((i == 22) || (i == 23)) {		//for hdma
			INT0_SET_TYPE( i, POSITIVE_EDGE);
		}else{
			INT0_SET_TYPE( i, HI_LEVEL);
		}
#elif (defined(CONFIG_ARCH_PDK_PC7210) || defined(CONFIG_ARCH_P7DK))
		INT0_SET_TYPE( i, HI_LEVEL);
#elif defined(CONFIG_ARCH_LDK3V21)
		if((i >= 11) && (i <= 14)){
			INT0_SET_TYPE( i, LO_LEVEL);
		}else{
			INT0_SET_TYPE( i, HI_LEVEL);
		}
#elif defined(CONFIG_ARCH_PDK_PC9220)
		if (i == 29 || i==30) {			
			INT0_SET_TYPE( i, LO_LEVEL);		//for touch screen tsc2000 and spi wifi
		}else if (i == 27) {		//for hdma
			INT0_SET_TYPE( i, POSITIVE_EDGE);
		}else{
			INT0_SET_TYPE( i, HI_LEVEL);
		}
#else
		INT0_SET_TYPE( i, HI_LEVEL);
#endif		
		set_irq_chip(i, &socle_irq_chip);
		set_irq_handler(i, handle_level_irq);	// 20090209 cyli fix from do_level_IRQ to handle_level_irq
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);	

	}
}


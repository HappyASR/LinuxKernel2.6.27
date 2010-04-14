/*
 * include/asm-arm/arch-socle/entry-macro.S
 *
 * Low-level IRQ helper macros for SOCLE platforms
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <mach/regs-intr.h>

		.macro	disable_fiq
		.endm

		@ 20090210 cyli add
		.macro  get_irqnr_preamble, base, tmp
		.endm

		@ 20090210 cyli add
		.macro  arch_ret_to_user, tmp1, tmp2
		.endm

		.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
/* FIXME: should not be using soo many LDRs here */
		ldr	\base, =IO_ADDRESS(INTC0_PHY_ADDR)
        ldr     \irqnr,[\base,#260]   @ INTC0_PHY_ADDR + 0x104 ISR
     
        teq    \irqnr, #31
        .endm

		.macro	irq_prio_table
		.endm

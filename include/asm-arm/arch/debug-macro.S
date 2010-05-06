/*
 * include/asm-arm/arch-socle/debug-macro.S
 *
 * Debugging macro include header
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <mach/platform.h>
#include <mach/hardware.h>

		.macro	addruart,rx
		mrc	p15, 0, \rx, c1, c0
		tst	\rx, #1					@ mmu enabled?
                ldreq   \rx, =SOCLE_APB0_UART0                  @ physical
                ldrne   \rx, =IO_ADDRESS(SOCLE_APB0_UART0)      @ virtual
#ifdef __ARMEB__
		orr	\rx, \rx, #0x00000003
#endif
		.endm

#define UART_SHIFT	2
#include <asm/hardware/debug-8250.S>

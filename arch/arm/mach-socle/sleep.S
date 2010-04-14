/* linux/arch/arm/mach-socle/sleep.S
 *
 * Copyright (c) 2006 Socle-Tech. Corp.
 *	Leonid Cheng <leonid.c@socle-tech.com.tw>
 *
 * SOCLE MSMV Power Manager (Suspend-To-RAM) support
 *
 * Based on PXA/SA1100 sleep code by:
 *	Nicolas Pitre, (c) 2002 Monta Vista Software Inc
 *	Cliff Brake, (c) 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <linux/linkage.h>

	/* msmv_cpu_suspend
	 *
	 * put the cpu into sleep mode
	 *
	 * entry:
	 *	r0 = sleep save block
	*/

ENTRY(socle_cpu_suspend)
	@@save register state and return address on the stack
        mov     r3, r0                                          /*      base of Sleep mode storage (virt)       */

	@@save MMU & CPU Registers to RAM
        adr     r2, msmv_cpu_resume
        sub r2, r2, #0x80000000
        str     r2, [r3],#4                                             /*      save resume function address(virtual)   */

        mrc     p15,0,r2,c1,c0,0
        #ldr    r0, MMU_CTL_MASK
        #bic    r2,r2,r0
        str     r2,[r3],#4                                              /*      save MMU control data   */

        mrc     p15,0,r2,c2,c0,0
        ldr     r0, MMU_TTB_MASK
        bic     r2,r2,r0
        str     r2,[r3],#4                                              /*      save TLB address        */

        mrc     p15,0,r2,c3,c0,0
        str     r2,[r3],#4                                              /*      save domain access control      */

        stmia   r3!, {r4-r12,sp,lr}                                     /*      save SVC mode register  */
        add r3, r3, #4
        
	mov     r1,#0xD1                                        /*      enter FIQ mode, no interrupts   */
        msr     cpsr,r1
        mrs     r2,spsr
        stmia   r3!, {r2,r8-r12,sp,lr}                  /*      save the FIQ mode registers     */

        mov r1,#0xd7                                    /*      enter ABT mode, no interrupts   */
        msr     cpsr,r1
        mrs     r0,spsr
        stmia   r3!, {r0,sp,lr}                         /*      save the ABT mode registers     */

        mov r1,#0x92                                    /*      enter IRQ mode, no interrupts   */
        msr     cpsr,r1
        mrs     r0,spsr
        stmia   r3!, {r0,sp,lr}                         /*      save the IRQ mode registers     */

        mov r1,#0xDB                                    /*      enter UND mode, no interrupts   */
        msr     cpsr,r1
        mrs     r0,spsr
        stmia   r3!, {r0,sp,lr}                         /*      save the UND mode registers     */

        mov r1,#0xDF                                    /*      enter SYS mode, no interrupts   */
        msr     cpsr,r1
        #stmia  r3!, {r0,sp,lr}                         /*      save the SYS mode registers     */
        stmia   r3!, {sp,lr}                            /*      save the SYS mode registers     */

        mov r1,#0xD3                                    /*      back to SVC mode, no interrupts */
        msr     cpsr,r1
	
	/*	sleep mode	*/
	ldr	r1,	_sleep_addr
	ldr	r2, _sleep
	sub	r5, r1, #0x20		/*	prepare to disable PLL	*/
	mov	r6, #0 

		
	/*	test clean and invalidate	*/
tci_loop:			
	MRC p15, 0, r15, c7, c14, 3						
	BNE tci_loop	

	mcr     p15, 0, r3, c8, c7, 0           @ invalidate I & D TLBs
	mcr     p15, 0, r3, c7, c7, 0           @ invalidate I & D caches, BTB

	b	msmv_do_sleep

        mov     pc,     lr

        @@ align next bit of code to cache line
        .align  8

msmv_do_sleep:
	str r6, [r5]		/*      disable PLL     */
	str r2, [r1]		/*	enter to sleep mode	*/
        nop
        nop
        nop
						
	mov	pc,	lr	
		
	
	

ENTRY(msmv_cpu_resume)
	#Recover CPU Registers
        ldr     r3, scu_info_p
        ldr     r3, [r3, #4]            /*      base of Sleep mode storage (phy)        */

        add     r2, r3,#0x40
        mov r1, #0xD1                   /*      enter FIQ mode, no interrupts   */
        msr     cpsr, r1
        ldr     r0, [r2],#4
        msr     spsr_cxsf, r0
        ldr     r8, [r2],#4
        ldr     r9, [r2],#4
        ldr     r10, [r2],#4
        ldr     r11, [r2],#4
        ldr     r12, [r2],#4
        ldr     sp, [r2],#4
        ldr     lr, [r2],#4

        mov r1,#0x97                    /*      enter ABT mode, no interrupts   */
        msr     cpsr,r1
        ldr     r0, [r2],#4
        msr     spsr_cxsf, r0
        ldr     sp, [r2],#4
        ldr     lr, [r2],#4

        mov r1,#0x92                    /*      enter IRQ mode, no interrupts   */
        msr     cpsr,r1
        ldr     r0, [r2],#4
        msr     spsr_cxsf, r0
        ldr     sp, [r2],#4
        ldr     lr, [r2],#4

        mov r1,#0x9B                    /*      enter UND mode, no interrupts   */
        msr     cpsr,r1
        ldr     r0, [r2],#4
        msr     spsr_cxsf, r0
        ldr     sp, [r2],#4
        ldr     lr, [r2],#4

        mov r1,#0x9F                    /*      enter SYS mode, no interrupts   */
        msr     cpsr,r1
        ldr     sp, [r2],#4
	ldr	lr, [r2]
				
	mov r1,#0x93			/*	enter SVC mode, no interrupts	*/
        msr     cpsr,r1
        ldr     r0, [r3, #0x3c]
        orr r0, r0, #0xc0
        msr     cpsr_cxsf, r0

        ldr     r3, scu_info_p
	mov r2, r3			/*	test	*/
        ldr     r3, [r3, #4]            /*      base of Sleep mode storage (phy)        */

#Recover Last mode's REG's, & go back to caller of CPUPowerOFF
        add     r0, r3, #0x10
        ldmia   r0!, {r4-r12,sp,lr}

        nop
        nop
        nop
        nop
        nop

#******************************

        mov     r1, #0
	mcr     p15, 0, r1, c8, c7, 0           @ invalidate I & D TLBs
	mcr     p15, 0, r1, c7, c7, 0           @ invalidate I & D caches, BTB


        ldr     r2, [r2, #0x4]

        ldr     r0, [r2, #0x8]
        mcr     p15,0,r0,c2,c0,0                @ set TLB address 

        ldr     r0, [r2, #0xc]
        mcr     p15,0,r0,c3,c0,0                @ set domain access control       

        ldr     r0, [r2, #0x4]

        mcr     p15,0,r0,c1,c0,0                @ set MMU control data    @ turn on MMU, etc	

#*******************************
	nop					@ second-to-last before mmu

        mov     pc, lr                          @ go back to virtual address  (our sponsors)

        nop
        nop
        nop
        nop

.globl msmv_cpu_irq_disable	
msmv_cpu_irq_disable:
	mrs	r2, cpsr
	str	r2, [r0]
	orr	r2, r2, #0xc0
	msr	cpsr,r2
	mov	pc, lr
	
.globl msmv_cpu_irq_enable	
msmv_cpu_irq_enable:
	ldr	r3, scu_info_v
	ldr	r3, [r3, #8]
	ldr	r0, [r3, #0x3c]
	msr	cpsr_cxsf, r0
	mov	pc, lr
	


MMU_CTL_MASK:
        .word   0xFFFF0000

MMU_TTB_MASK:
        .word   0x3FFF

scu_info_v:
        .word   0xfc1a0064              /*      virt info addr  */

scu_info_p:
        .word   0x1d1a0064              /*      phy info addr   */

_sleep_addr:
        .word   0xfc1a0028

_sleep:
        .word   0x2bed







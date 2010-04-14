/* linux/arch/arm/mach-socle/pm.c
 *
 * Copyright (c) 2004 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * Socle Power Manager (Suspend-To-RAM) support
 *
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
 *
 * Parts based on arch/arm/mach-pxa/pm.c
 *
 * Thanks to Dimitry Andric for debugging
 *
 * Modifications:
 *     
*/

#ifdef LEONID_PM_TASK

#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/crc32.h>
#include <linux/ioport.h>
#include <linux/delay.h>

#include <mach/hardware.h>
#include <asm/io.h>
#include <linux/types.h>

#include <mach/regs-serial.h>
#include <mach/regs-clock.h>
#include <mach/regs-mem.h>
#include <mach/regs-irq.h>

#include <asm/mach/time.h>

#include "pm.h"

/* cache functions from arch/arm/mm/proc-arm926.S */

#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
extern void arm926_flush_kern_cache_all(void);
#else
static void arm926_flush_kern_cache_all(void) { }
#endif

#define PFX "socle -pm: "

static struct sleep_save core_save[] = {
	//SAVE_ITEM(S3C2410_LOCKTIME),
	//SAVE_ITEM(S3C2410_CLKCON),

	/* we restore the timings here, with the proviso that the board
	 * brings the system up in an slower, or equal frequency setting
	 * to the original system.
	 *
	 * if we cannot guarantee this, then things are going to go very
	 * wrong here, as we modify the refresh and both pll settings.
	 */

};

/* this lot should be really saved by the IRQ code */
static struct sleep_save irq_save[] = {
/*	SAVE_ITEM(S3C2410_EXTINT0),
	SAVE_ITEM(S3C2410_EXTINT1),
	SAVE_ITEM(S3C2410_EXTINT2),
	SAVE_ITEM(S3C2410_EINFLT0),
	SAVE_ITEM(S3C2410_EINFLT1),
	SAVE_ITEM(S3C2410_EINFLT2),
	SAVE_ITEM(S3C2410_EINFLT3),
	SAVE_ITEM(S3C2410_EINTMASK),
	SAVE_ITEM(S3C2410_INTMSK)*/
};

/* helper functions to save and restore register state */

void socle_pm_do_save(struct sleep_save *ptr, int count)
{
	for (; count > 0; count--, ptr++) {
		ptr->val = __raw_readl(ptr->reg);
		printk("saved %p value %08lx\n", ptr->reg, ptr->val);
	}
}

/* s3c2410_pm_do_restore
 *
 * restore the system from the given list of saved registers
 *
 * Note, we do not use DBG() in here, as the system may not have
 * restore the UARTs state yet
*/

void socle_pm_do_restore(struct sleep_save *ptr, int count)
{
	for (; count > 0; count--, ptr++) {
		printk(KERN_DEBUG "restore %p (restore %08lx, was %08x)\n",
		       ptr->reg, ptr->val, __raw_readl(ptr->reg));

		__raw_writel(ptr->val, ptr->reg);
	}
}

/* s3c2410_pm_do_restore_core
 *
 * similar to s3c2410_pm_do_restore_core
 *
 * WARNING: Do not put any debug in here that may effect memory or use
 * peripherals, as things may be changing!
*/

static void socle_pm_do_restore_core(struct sleep_save *ptr, int count)
{
	for (; count > 0; count--, ptr++) {
		__raw_writel(ptr->val, ptr->reg);
	}
}

/* s3c2410_pm_show_resume_irqs
 *
 * print any IRQs asserted at resume time (ie, we woke from)
*/

static void socle_pm_show_resume_irqs(int start, unsigned long which,
					unsigned long mask)
{
	int i;

	which &= ~mask;

	for (i = 0; i <= 31; i++) {
		if ((which) & (1L<<i)) {
			printk("IRQ %d asserted at resume\n", start+i);
		}
	}
}

/* s3c2410_pm_configure_extint
 *
 * configure all external interrupt pins
*/

static void socle_pm_configure_extint(void)
{
	int pin;

	/* for each of the external interrupts (EINT0..EINT15) we
	 * need to check wether it is an external interrupt source,
	 * and then configure it as an input if it is not
	*/
/*
	for (pin = S3C2410_GPF0; pin <= S3C2410_GPF7; pin++) {
		s3c2410_pm_check_resume_pin(pin, pin - S3C2410_GPF0);
	}

	for (pin = S3C2410_GPG0; pin <= S3C2410_GPG7; pin++) {
		s3c2410_pm_check_resume_pin(pin, (pin - S3C2410_GPG0)+8);
	}
	*/
}

#define any_allowed(mask, allow) (((mask) & (allow)) != (allow))

extern void msmv_cpu_irq_enable(void);
extern void msmv_cpu_irq_disable(u32 addr);

static void msmv_irq_backup(u32 addr);
static void msmv_irq_resume(u32 addr);

static void msmv_timer_backup(u32 addr);
static void msmv_timer_resume(u32 addr);

#define MSMV_INTC_SAVE		0x100	
#define MSMV_TIMER_SAVE	0x230

extern void socle_cpu_suspend(u32 addr);

extern void socle_i2c_master_initialize(struct socle_i2c_host *host);


static u32 *addr_virt;

extern void 
msmv_sleep_mode_enter()
{
	u32 tmp;

	//printk("msmv_sleep_mode_enter\n");	
	
	//printk("addr_virt = 0x%08x\n", (u32)addr_virt);


	socle_scu_info2_set((u32)addr_virt);
	tmp = virt_to_phys((u32 *)addr_virt);
	socle_scu_info1_set(tmp);
	
	/*	irq backup	*/
	msmv_cpu_irq_disable((u32)(addr_virt+(0x3c>>2)));
	msmv_irq_backup((u32)(addr_virt+(MSMV_INTC_SAVE>>2)));		

	/*	un remap	*/
	iowrite32(0x0, 0xfc1a0024);


	/*	timer backup	*/
//	msmv_timer_backup((u32)(addr_virt+(MSMV_TIMER_SAVE>>2)));
	
	socle_cpu_suspend((u32)addr_virt);
		
	//continue when sleep wake up

	
	mdelay(1000);
		
		/*	UART resume	*/
		//msmv_serial_resume((u32)((addr_virt)+(MSMV_SERIAL_SAVE>>2)));
		printk("uart ok!!\n");

//		msmv_timer_resume((u32)(addr_virt+(MSMV_TIMER_SAVE>>2)));
		printk("timer wake up from sleep mode\n");
		
		/*	remap	*/
		iowrite32(0xCAD20712, 0xfc1a0024);
		printk("remap after sleep mode\n");
		
		/*	irq resume	*/
		msmv_irq_resume((u32)(addr_virt+(MSMV_INTC_SAVE>>2)));
		printk("irq resume from sleep mode\n");
				
		msmv_cpu_irq_enable();	
		printk("msmv_cpu_irq_enable\n");

		printk("\nwake up from sleep mode\n");


	return;
}

//EXPORT_SYMBOL(msmv_sleep_mode_enter);


static void
msmv_irq_backup(u32 addr)
{
	int tmp;
	u32 addr_int = 0xf7040000;
	
	iowrite32(ioread32(addr_int+0x10c), (addr+0x10c));
	iowrite32(0, addr_int+0x10c);

	for(tmp=0;tmp<0x10c;tmp+=4){
		iowrite32(ioread32(addr_int+tmp), (addr+tmp));
	}
	for(tmp=0x110;tmp<0x124;tmp+=4){
		iowrite32(ioread32(addr_int+tmp), (addr+tmp));
	}

	return ;
}

static void
msmv_irq_resume(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xf7040000;

	for(tmp=0;tmp<0x10c;tmp+=4){
		iowrite32(ioread32(addr+tmp), (addr_int+tmp));
	}
	for(tmp=0x110;tmp<0x124;tmp+=4){
		iowrite32(ioread32(addr+tmp), (addr_int+tmp));
	}
	iowrite32(ioread32(addr+0x10c), (addr_int+0x10c));
	
	return ;
}

static void 
msmv_timer_backup(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xfc100000;
	
	for(tmp=0;tmp<0x30;tmp+=0x10){
		iowrite32(ioread32(addr_int+tmp), (addr+tmp));
		iowrite32(ioread32(addr_int+tmp+0x8), (addr+tmp+0x8));
	}
	
	return ;
}

static void
msmv_timer_resume(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xfc100000;

	for(tmp=0;tmp<0x30;tmp+=0x10){
		iowrite32(ioread32(addr+tmp), (addr_int+tmp));
		iowrite32(ioread32(addr+tmp+0x8), (addr_int+tmp+0x8));
	}
	
	return ;
}

/* socle_pm_enter
 *
 * central control for sleep/resume process
*/

static int socle_pm_enter(suspend_state_t state)
{
	unsigned long regs_save[16];
	unsigned long tmp;
#if 1
	/* ensure the debug is initialised (if enabled) */

//	socle_pm_debug_init();

	pr_debug("sq_pm_enter(%s)\n", state);

	switch (state) {
		/*
		 * Suspend-to-RAM is like STANDBY plus slow clock mode, so
		 * drivers must suspend more deeply:  only the master clock
		 * controller may be using the main oscillator.
		 */
		case PM_SUSPEND_MEM: 
			/*
			 * Ensure that clocks are in a valid state.
			 */
			//if (!at91_pm_verify_clocks())

			/* flush cache back to ram */
			pr_debug("PM_SUSPEND_MEM\n");
			arm926_flush_kern_cache_all();
			msmv_sleep_mode_enter();
//			socle_cpu_suspend(regs_save);
			/* restore the cpu state */
			//cpu_init();
			//asm("mcr     p15, 0, r0, c7, c0, 4");	//@ Wait for interrupt

			/*
			 * Enter slow clock mode by switching over to clk32k and
			 * turning off the main oscillator; reverse on wakeup.
			 */
			#if 0
			if (slow_clock) {
				slow_clock();
				break;
			} else {
				/* DEVELOPMENT ONLY */
				pr_info("AT91: PM - no slow clock mode yet ...\n");
				/* FALLTHROUGH leaving master clock alone */
			}
			#endif
			break;

		/*
		 * STANDBY mode has *all* drivers suspended; ignores irqs not
		 * marked as 'wakeup' event sources; and reduces DRAM power.
		 * But otherwise it's identical to PM_SUSPEND_ON:  cpu idle, and
		 * nothing fancy done with main or cpu clocks.
		 */
		case PM_SUSPEND_STANDBY:	//IDLE Mode
			/*
			 * NOTE: the Wait-for-Interrupt instruction needs to be
			 * in icache so the SDRAM stays in self-refresh mode until
			 * the wakeup IRQ occurs.
			 */
			pr_debug("PM_SUSPEND_STANDBY\n");
			asm("b 1f; .align 5; 1:");
			asm("mcr p15, 0, r0, c7, c10, 4");	/* drain write buffer */
			//at91_sys_write(AT91_SDRAMC_SRR, 1);	/* self-refresh mode */
			/* fall though to next state */
			break;

		case PM_SUSPEND_ON:
			pr_debug("PM_SUSPEND_ON \n");
			asm("mcr p15, 0, r0, c7, c0, 4");	/* wait for interrupt */
			break;

		default:
			printk("SQ: PM - bogus suspend state %d\n", state);
	}



	pr_debug("SQ PM Exit \n");
#else
//	socle_pm_debug_init();

	printk("SQ_pm_enter(%d)\n", state);

	if (state != PM_SUSPEND_MEM) {
		printk(KERN_ERR PFX "error: only PM_SUSPEND_MEM supported\n");
		return -EINVAL;
	}

	/* check if we have anything to wake-up with... bad things seem
	 * to happen if you suspend with no wakeup (system will often
	 * require a full power-cycle)
	*/


	/* prepare check area if configured */

//	socle_pm_check_prepare();

	/* store the physical address of the register recovery block */



	/* ensure at least GESTATUS3 has the resume address */


	/* set the irq configuration for wake */

	socle_pm_configure_extint();


	/* flush cache back to ram */

	arm926_flush_kern_cache_all();

//	s3c2410_pm_check_store();

	/* send the cpu to sleep... */

//	__raw_writel(0x00, S3C2410_CLKCON);  /* turn off clocks over sleep */

//	s3c2410_cpu_suspend(regs_save);

	/* restore the cpu state */

//	cpu_init();

	/* unset the return-from-sleep flag, to ensure reset */


	/* restore the system state */


	/* check what irq (if any) restored the system */


	printk("post sleep, preparing to return\n");


	/* ok, let's return from sleep */

	printk("SQ PM Resume (post-restore)\n");

#endif
	return 0;
}

/*
 * Called after processes are frozen, but before we shut down devices.
 */
static int socle_pm_prepare(suspend_state_t state)
{
	pr_debug("sq_pm_prepare \n");
	return 0;
}

/*
 * Called after devices are re-setup, but before processes are thawed.
 */
static int socle_pm_finish(suspend_state_t state)
{
	pr_debug("sq_pm_finish \n");
	return 0;
}

/*
 * Set to PM_DISK_FIRMWARE so we can quickly veto suspend-to-disk.
 */
static struct pm_ops socle_pm_ops = {
	.pm_disk_mode	= PM_DISK_FIRMWARE,
	.prepare	= socle_pm_prepare,
	.enter		= socle_pm_enter,
	.finish		= socle_pm_finish,
};

/* socle_pm_init
 *
 * Attach the power management functions. This should be called
 * from the board specific initialisation if the board supports
 * it.
*/

int __init socle_pm_init(void)
{
	printk("SQ - Power Management, (c) 2004 Socle-tech Corp. Electronics\n");
	
	addr_virt = kmalloc(0x200, GFP_KERNEL);

	pm_set_ops(&socle_pm_ops);
	return 0;
}


arch_initcall(socle_pm_init);


#endif

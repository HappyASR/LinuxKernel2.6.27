/* linux/arch/arm/mach-socle/pm.h
 *
 * Copyright (c) 2004 Simtec Electronics
 *	Written by Ben Dooks, <ben@simtec.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

/* socle_pm_init
 *
 * called from board at initialisation time to setup the power
 * management
*/

#ifdef CONFIG_PM

extern __init int socle_pm_init(void);

#else

static inline int socle_pm_init(void)
{
	return 0;
}
#endif

/* configuration for the IRQ mask over sleep */
//extern unsigned long ldk_irqwake_intmask;
//extern unsigned long ldk_irqwake_eintmask;

/* IRQ masks for IRQs allowed to go to sleep (see irq.c) */
//extern unsigned long s3c_irqwake_intallow;
//extern unsigned long s3c_irqwake_eintallow;

/* from sleep.S */

extern void socle_cpu_suspend(u32 addr);
extern void socle_cpu_resume(void);

//extern unsigned long s3c2410_sleep_save_phys;

/* sleep save info */

struct sleep_save {
	void __iomem	*reg;
	unsigned long	val;
};

#define SAVE_ITEM(x) \
	{ .reg = (x) }

extern void socle_pm_do_save(struct sleep_save *ptr, int count);
extern void socle_pm_do_restore(struct sleep_save *ptr, int count);

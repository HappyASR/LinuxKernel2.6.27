/*
 * linux/arch/arm/mach-socle/clock.c
 *
 * Copyright (C) 2007 Socle Tech. Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include <linux/semaphore.h>
#include <asm/io.h>
#include <asm/mach-types.h>

#include <mach/hardware.h>

//#include "regs-scu.h"


int __init socle_clock_init(unsigned long main_clock)
{

	printk("SQ_clock_init \n");
	return 0;
}


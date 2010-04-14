/*
 *  linux/arch/arm/mach-socle/panther.c
 *
 * Copyright (C) SQ Tech. Corp.
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by the Free Software Foundation; 
 * either version 2 of the License, or (at your option) any later version. 
 * This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY; 
 *without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysdev.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/serial_8250.h>

#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <asm/io.h>

#include <mach/platform.h>
#include <mach/hardware.h>
#include <mach/regs-intr.h>

#include "devs.h"
#include "generic.h"

/* --------------------------------------------------------------------
 *  UART
 * -------------------------------------------------------------------- */
 /* SOCLE-PDK UART */

#define PDK_UART_CLOCK (11059200*8)

static struct plat_serial8250_port pdk_uart_data[] = {
	{
		.mapbase	= SOCLE_APB0_UART0,
		.membase	= (char*)(IO_ADDRESS(SOCLE_APB0_UART0)),
		.irq			= IRQ_UART0,
		.uartclk		= PDK_UART_CLOCK,
		.regshift		= 2,
		.iotype		= UPIO_MEM,
		.flags		= UPF_SKIP_TEST,
	},
	{
		.mapbase	= SOCLE_APB0_UART1,
		.membase	= (char*)(IO_ADDRESS(SOCLE_APB0_UART1)),
		.irq			= IRQ_UART1,
		.uartclk		= PDK_UART_CLOCK,
		.regshift		= 2,
		.iotype		= UPIO_MEM,
		.flags		= UPF_SKIP_TEST,
	},
	{ },
};

struct platform_device pdk_uart_device = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev.platform_data	= pdk_uart_data,
};


static void __init pdk_init(void)
{
	platform_device_register(&pdk_uart_device);
	
	//add device register
	socle_add_device_flash();
	socle_add_device_rtc();
	socle_add_device_watchdog();
	socle_add_device_eth();
	socle_add_device_ehci();
	socle_add_device_ohci();
	socle_add_device_i2c();
	socle_add_device_snd_i2s();
	socle_add_device_hdma_pseudo();
	socle_add_device_udc();
	socle_sdmmc_add_device_mmc();
	socle_spi_add_device();
	socle_add_device_pwmt();
	pdk_add_device_kpd();
	socle_add_device_nand();	
#ifdef CONFIG_ARCH_PDK_PC7210
	socle_add_lcd_device();
#else //for P7DK
	socle_add_device_clcd();
#endif
#ifdef CONFIG_PM
	socle_pm_init();
#endif
	socle_add_device_inr();
}

extern struct socle_dma panther7_hdma_channel_0;
extern struct socle_dma panther7_hdma_channel_1;

extern void
socle_platform_dma_init(struct socle_dma **dma)
{
	dma[0] = &panther7_hdma_channel_0;
	dma[1] = &panther7_hdma_channel_1;
}

#if defined(CONFIG_ARCH_PDK_PC7210)
MACHINE_START(SOCLE, "PDK-PC7210")
#elif defined(CONFIG_ARCH_P7DK)
MACHINE_START(SOCLE, "Socle-P7DK")
#else
MACHINE_START(SOCLE, "Panther 7")
#endif
	.phys_io	= 0x1802c000,
	.boot_params	= 0x80000100,
	.init_irq	= socle_init_irq,
	.timer		= &socle_timer,
	.init_machine	= pdk_init,
MACHINE_END

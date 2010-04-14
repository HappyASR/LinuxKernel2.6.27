/*
 *  linux/arch/arm/mach-socle/ldk3v21.c
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

#include "devs.h"
#include "generic.h"

/* --------------------------------------------------------------------
 *  UART
 * -------------------------------------------------------------------- */
 /* SOCLE-LDK UART */

#define LDK_UART_CLOCK (24000000)

static struct plat_serial8250_port ldk_uart_data[] = {
	{
		.mapbase	= SOCLE_APB0_UART0,
		.membase	= (char*)(IO_ADDRESS(SOCLE_APB0_UART0)),
		.irq		= IRQ_UART0,
		.uartclk	= LDK_UART_CLOCK,
		.regshift	= 2,
		.iotype		= UPIO_MEM,
		.flags		= UPF_BOOT_AUTOCONF,
	},
	{
		.mapbase	= SOCLE_APB0_UART1,
		.membase	= (char*)(IO_ADDRESS(SOCLE_APB0_UART1)),
		.irq		= IRQ_UART1,
		.uartclk	= LDK_UART_CLOCK,
		.regshift	= 2,
		.iotype		= UPIO_MEM,
		.flags		= UPF_BOOT_AUTOCONF,
	},
	{ },
};

struct platform_device ldk_uart_device = {
	.name			= "serial8250",
	.id				= PLAT8250_DEV_PLATFORM,
	.dev.platform_data	= ldk_uart_data,
//	.num_resources	= ARRAY_SIZE(ldk_uart_resources),
//	.resource			= ldk_uart_resources,
};

static struct map_desc ldk_io_desc[] __initdata = {
	{
		.virtual	= IO_ADDRESS(SOCLE_AHB0_INTC),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_INTC),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_NOR_FLASH0),
		.pfn		= __phys_to_pfn(SOCLE_NOR_FLASH0),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_TIMER),
		.pfn		= __phys_to_pfn(SOCLE_APB0_TIMER),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_UART0),
		.pfn		= __phys_to_pfn(SOCLE_APB0_UART0),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_UART1),
		.pfn		= __phys_to_pfn(SOCLE_APB0_UART1),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_GPIO0),
		.pfn		= __phys_to_pfn(SOCLE_APB0_GPIO0),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_MAC0),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_MAC0),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_MAC1),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_MAC1),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_RTC),
		.pfn		= __phys_to_pfn(SOCLE_APB0_RTC),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_WDT),
		.pfn		= __phys_to_pfn(SOCLE_APB0_WDT),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_BUS0_PCI),
		.pfn		= __phys_to_pfn(SOCLE_BUS0_PCI),
		.length		= (SZ_2M+SZ_8M+SZ_64M),
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_UDC),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_UDC),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
 		.virtual	= IO_ADDRESS(SOCLE_APB0_I2C),
		.pfn		= __phys_to_pfn(SOCLE_APB0_I2C),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_SPI0),
		.pfn		= __phys_to_pfn(SOCLE_APB0_SPI0),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_SDMMC),
		.pfn		= __phys_to_pfn(SOCLE_APB0_SDMMC),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB1_MPEG4_EN),
		.pfn		= __phys_to_pfn(SOCLE_AHB1_MPEG4_EN),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB1_MPEG4_DE),
		.pfn		= __phys_to_pfn(SOCLE_AHB1_MPEG4_DE),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_A2A_DMA),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_A2A_DMA),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_ES0),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_ES0),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_ES3),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_ES3),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB1_VOP),
		.pfn		= __phys_to_pfn(SOCLE_AHB1_VOP),
		.length		= SZ_128K,
		.type		= MT_DEVICE,
	},
};

#ifdef CONFIG_MMU
static void __init ldk_map_io(void)
{
	iotable_init(ldk_io_desc, ARRAY_SIZE(ldk_io_desc));
	
}
#endif

static void __init ldk_init(void)
{
	platform_device_register(&ldk_uart_device);
	socle_add_lcd_device();
	//add device register
	socle_add_device_flash();
	socle_add_device_rtc();
	socle_add_device_watchdog();
	socle_add_device_eth();
	socle_add_device_ehci();
	socle_add_device_ohci();
	socle_add_device_udc();
	socle_sdmmc_add_device_mmc();
	socle_spi_add_device();
	socle_add_device_i2c();
	socle_add_device_vop();
#ifdef CONFIG_PM
	socle_pm_init();
#endif

}


extern struct socle_dma socle_a2a_channel_0;
extern struct socle_dma socle_a2a_channel_1;

extern void
socle_platform_dma_init(struct socle_dma **dma)
{
	dma[0] = &socle_a2a_channel_0;
	dma[1] = &socle_a2a_channel_1;
}

MACHINE_START(SOCLE, "Socle-LDK3")
	.phys_io	= 0x1e840000,
#ifdef CONFIG_MMU
	.io_pg_offst	= ((0xfd840000) >> 18) & 0xfffc,
#endif
	.boot_params	= 0x40000100,
#ifdef CONFIG_MMU
	.map_io		= ldk_map_io,
#endif
	.init_irq		= socle_init_irq,
	.timer		= &socle_timer,
	.init_machine	= ldk_init,
MACHINE_END

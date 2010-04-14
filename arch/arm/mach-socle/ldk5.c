/*
 *  linux/arch/arm/mach-socle/ldk5.c
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

/* --------------------------------------------------------------------
 *  UART
 * -------------------------------------------------------------------- */
 /* SOCLE-LDK UART */

#define LDK_UART_CLOCK (24000000)
//#define UART_TX_FIFO_SIZE 16

static struct plat_serial8250_port ldk_uart0_data[] = {
	[0] = {
		.membase	= (char*)(IO_ADDRESS(SOCLE_APB0_UART0)),
		.mapbase	= SOCLE_APB0_UART0,
		.irq		= IRQ_UART0,
		.uartclk	= LDK_UART_CLOCK,
		.regshift	= 2,
		.iotype		= UPIO_MEM,
		.flags		= UPF_SKIP_TEST,
	},
	{ },
};
/*
static struct resource ldk_uart0_resources[] = {
	[0] = {
		.start	= LDK_APB0_UART0,
		.end	= LDK_APB0_UART0 + SZ_128K,
		.flags	= IORESOURCE_MEM,
	},
	{},
};
*/
struct platform_device ldk_uart0_device = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev		= {
		.platform_data		= ldk_uart0_data,
	},
	//.num_resources	= 1,
	//.resource	= ldk_uart0_resources,
};

static struct map_desc ldk_io_desc[] __initdata = {
	{
		.virtual	= IO_ADDRESS(SOCLE_AHB0_INTC),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_INTC),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB1_INTC),
		.pfn		= __phys_to_pfn(SOCLE_AHB1_INTC),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_TIMER),
		.pfn		= __phys_to_pfn(SOCLE_APB0_TIMER),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_UART0),
		.pfn		= __phys_to_pfn(SOCLE_APB0_UART0),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_MAC0),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_MAC0),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_RTC),
		.pfn		= __phys_to_pfn(SOCLE_APB0_RTC),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_IDE),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_IDE),
		.length		= SZ_16K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_BUS0_PCI),
		.pfn		= __phys_to_pfn(SOCLE_BUS0_PCI),
		.length		= (SZ_2M+SZ_8M+SZ_64M),
		.type		= MT_DEVICE,
	}, {
                .virtual        = IO_ADDRESS(SOCLE_AHB0_UHC0),
                .pfn            = __phys_to_pfn(SOCLE_AHB0_UHC0),
                .length         = SZ_16K,
                .type           = MT_DEVICE,
        }, {
	     	.virtual    = IO_ADDRESS(SOCLE_AHB0_NAND),
	    	.pfn        = __phys_to_pfn(SOCLE_AHB0_NAND),
	    	.length     = SZ_128K,
	    	.type       = MT_DEVICE,
	}, {
              .virtual        = IO_ADDRESS(SOCLE_APB0_SCU),
              .pfn            = __phys_to_pfn(SOCLE_APB0_SCU),
              .length         = SZ_16K,
              .type           = MT_DEVICE,
        },
};

static void __init ldk_map_io(void)
{
	iotable_init(ldk_io_desc, ARRAY_SIZE(ldk_io_desc));
}

void vic0_mask_irq(unsigned int irq)
{
	INT0_DISABLE(irq);
	INT0_CLR_MASK(irq);
	__raw_writel(1 << (irq),INTC0_ICCR);
}

void vic0_unmask_irq(unsigned int irq)
{
	INT0_SET_MASK(irq);
	INT0_ENABLE(irq);
}

static struct irqchip ldk_irq_intc0_chip = {
	.ack	= vic0_mask_irq,
	.mask	= vic0_mask_irq,
	.unmask	= vic0_unmask_irq,
};

void vic1_mask_irq(unsigned int irq)
{
	INT1_DISABLE(irq);
	INT1_CLR_MASK(irq);
	__raw_writel(1 << (irq),INTC1_ICCR);
}

void vic1_unmask_irq(unsigned int irq)
{
	INT1_SET_MASK(irq);
	INT1_ENABLE(irq);
}
static struct irqchip ldk_irq_intc1_chip = {
	.ack	= vic1_mask_irq,
	.mask	= vic1_mask_irq,
	.unmask	= vic1_unmask_irq,
};

static void __init ldk_init_irq(void)
{
	unsigned int i;
	// disable all interrupt
	__raw_writel(0, INTC0_IECR);
	// clear all interrupt
	__raw_writel(0xFFFFFFFF, INTC0_ICCR);	
	/* Disable all interrupts initially. */

	for (i = 0; i < 32; i++) {

		INT0_SET_TYPE( i, HI_LEVEL);

		set_irq_chip(i, &ldk_irq_intc0_chip);
		set_irq_handler(i, do_level_IRQ);
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);			
	}

	for (i = 32; i < NR_IRQS; i++) {

		INT0_SET_TYPE( i-32, HI_LEVEL);

		set_irq_chip(i, &ldk_irq_intc1_chip);
		set_irq_handler(i, do_level_IRQ);
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);			
	}
	
}

static void __init ldk_init(void)
{
	platform_device_register(&ldk_uart0_device);
	//add device register
	socle_add_device_rtc();
	socle_add_device_watchdog();
	socle_add_device_eth();
	socle_add_device_ehci();
	socle_add_device_ohci();
#ifdef CONFIG_PM
	socle_pm_init();
#endif

}

MACHINE_START(SOCLE, "Socle-LDK5")
	.phys_io	= 0x18004000,
	.io_pg_offst	= ((0xf7004000) >> 18) & 0xfffc,
	.boot_params	= 0x40000100,
	.map_io		= ldk_map_io,
	.init_irq	= ldk_init_irq,
	.timer		= &socle_timer,
	.init_machine	= ldk_init,
MACHINE_END

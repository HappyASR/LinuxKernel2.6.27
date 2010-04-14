/*
 *  linux/arch/arm/mach-socle/cheetah.c
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

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <asm/mach/flash.h>


#include <mach/platform.h>
#include <mach/hardware.h>

#include "devs.h"
#include "generic.h"

/* --------------------------------------------------------------------
 *  UART
 * -------------------------------------------------------------------- */
 /* SOCLE-PDK UART */

#define PDK_UART_CLOCK (24000000)

static struct plat_serial8250_port pdk_uart_data[] = {
	{
		.mapbase	= SOCLE_APB0_UART0,
		.membase	= (char*)(IO_ADDRESS(SOCLE_APB0_UART0)),
		.irq		= IRQ_UART0,
		.uartclk	= PDK_UART_CLOCK,
		.regshift	= 2,
		.iotype		= UPIO_MEM,
		.flags		= UPF_SKIP_TEST,
	},{
		.mapbase	= SOCLE_APB0_UART1,
		.membase	= (char*)(IO_ADDRESS(SOCLE_APB0_UART1)),
		.irq		= IRQ_UART1,
		.uartclk	= PDK_UART_CLOCK,
		.regshift	= 2,
		.iotype		= UPIO_MEM,
		.flags		= UPF_SKIP_TEST,
	},{
		.mapbase	= SOCLE_APB0_UART2,
		.membase	= (char*)(IO_ADDRESS(SOCLE_APB0_UART2)),
		.irq		= IRQ_UART2,
		.uartclk	= PDK_UART_CLOCK,
		.regshift	= 2,
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

static struct map_desc pdk_io_desc[] __initdata = {
	{
		.virtual	= IO_ADDRESS(SOCLE_AHB0_INTC),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_INTC),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_NOR_FLASH0),
		.pfn		= __phys_to_pfn(SOCLE_NOR_FLASH0),
		.length		= SZ_4M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_UART0),
		.pfn		= __phys_to_pfn(SOCLE_APB0_UART0),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_UART1),
		.pfn		= __phys_to_pfn(SOCLE_APB0_UART1),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_UART2),
		.pfn		= __phys_to_pfn(SOCLE_APB0_UART2),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_SPI0),
		.pfn		= __phys_to_pfn(SOCLE_APB0_SPI0),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_SPI1),
		.pfn		= __phys_to_pfn(SOCLE_APB0_SPI1),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_I2C),
		.pfn		= __phys_to_pfn(SOCLE_APB0_I2C),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_I2S),
		.pfn		= __phys_to_pfn(SOCLE_APB0_I2S),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_SDMMC),
		.pfn		= __phys_to_pfn(SOCLE_APB0_SDMMC),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_TIMER),
		.pfn		= __phys_to_pfn(SOCLE_APB0_TIMER),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_PWM),
		.pfn		= __phys_to_pfn(SOCLE_APB0_PWM),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_GPIO0),
		.pfn		= __phys_to_pfn(SOCLE_APB0_GPIO0),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_RTC),
		.pfn		= __phys_to_pfn(SOCLE_APB0_RTC),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_WDT),
		.pfn		= __phys_to_pfn(SOCLE_APB0_WDT),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_ADC),
		.pfn		= __phys_to_pfn(SOCLE_APB0_ADC),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_SCU),
		.pfn		= __phys_to_pfn(SOCLE_APB0_SCU),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_MAC0),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_MAC0),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_UDC),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_UDC),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_UHC0),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_UHC0),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_HDMA),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_HDMA),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_BUS0_PCI),
		.pfn		= __phys_to_pfn(SOCLE_BUS0_PCI),
		.length		= (SZ_2M+SZ_8M+SZ_64M),
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_IDE),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_IDE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_LCD),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_LCD),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(PANTHER7_AHB0_HDMA),
		.pfn		= __phys_to_pfn(PANTHER7_AHB0_HDMA),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_AHB0_NAND),
		.pfn		= __phys_to_pfn(SOCLE_AHB0_NAND),
		.length		= SZ_64K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= IO_ADDRESS(SOCLE_APB0_MP),
		.pfn		= __phys_to_pfn(SOCLE_APB0_MP),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	},
};

#ifdef CONFIG_MMU
static void __init pdk_map_io(void)
{
	iotable_init(pdk_io_desc, ARRAY_SIZE(pdk_io_desc));
	
}
#endif

extern struct platform_device socle_i2s_0_device;
static void __init pdk_init(void)
{
	socle_add_device_clcd();
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
	socle_add_device_adc();
	socle_add_device_pwmt();
	socle_add_device_udc();
	socle_add_device_otg_udc();

#if defined(CONFIG_ARCH_SCDK) || defined(CONFIG_ARCH_PDK_PC9002)
	//20080704 ryan add for check amba mode
//	if(scu_check_amba_mode() == 0) {
	socle_add_device_nand();
	pdk_add_device_kpd();
//	}
#endif

	socle_sdmmc_add_device_mmc();
	socle_spi_add_device();
	cdk_add_device_kpd();
	socle_add_device_fake_battery();
	socle_add_device_fake_rfkill();
#ifdef CONFIG_ANDROID_SYSTEM
	socle_add_device_android_adb();		//20090527 leonid+ for adb
#endif
#ifdef LEONID_PM_TASK
#ifdef CONFIG_PM
	socle_pm_init();
#endif
#endif

}
extern struct socle_dma socle_hdma_channel_0;
extern struct socle_dma socle_hdma_channel_1;
extern struct socle_dma socle_hdma_channel_2;
extern struct socle_dma socle_hdma_channel_3;
extern struct socle_dma panther7_hdma_channel_0;
extern struct socle_dma panther7_hdma_channel_1;

extern void
socle_platform_dma_init(struct socle_dma **dma)
{
	dma[0] = &socle_hdma_channel_0;
	dma[1] = &socle_hdma_channel_1;
	dma[2] = &socle_hdma_channel_2;
	dma[3] = &socle_hdma_channel_3;
	dma[4] = &panther7_hdma_channel_0;
	dma[5] = &panther7_hdma_channel_1;
}
#if defined(CONFIG_ARCH_CDK)
MACHINE_START(SOCLE, "Socle-CDK")
#elif defined(CONFIG_ARCH_SCDK)
MACHINE_START(SOCLE, "Socle-SCDK")
#elif defined(CONFIG_ARCH_PDK_PC9002)
MACHINE_START(SOCLE, "PDK-PC9002")
#else
MACHINE_START(SOCLE, "Cheetah C1")
#endif
	.phys_io	= 0x1d000000,
#ifdef CONFIG_MMU
	.io_pg_offst	= ((0xfc000000) >> 18) & 0xfffc,
#endif
	.boot_params	= 0x40000100,
#ifdef CONFIG_MMU
	.map_io		= pdk_map_io,
#endif
	.init_irq	= socle_init_irq,
	.timer		= &socle_timer,
	.init_machine	= pdk_init,
MACHINE_END

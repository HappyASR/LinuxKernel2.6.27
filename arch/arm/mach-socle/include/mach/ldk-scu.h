/* linux/include/asm/arch-ldk/ldk-scu.h
 *
 * Copyright (c) 2006 Socle-tech Corp
 *		      http://www.socle-tech.com.tw/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * LDK SCU configuration
*/

#ifndef __LDK_SCUREG_H
#define __LDK_SCUREG_H                     1

#include <mach/platform.h>
#define LDK_SCU_MAJOR 256

// offset of regisgers

#define SCU_REG_BASE 			IO_ADDRESS(SOCLE_APB0_SCU)
#define LDK_SCU_REMAP0	        	SCU_REG_BASE + 0x00)
#define LDK_SCU_LPCID	       	(SCU_REG_BASE + 0x08)
#define LDK_SCU_REMAP1	       	(SCU_REG_BASE + 0x04)
#define LDK_SCU_CLKCFG	       	(SCU_REG_BASE + 0x0c)
#define LDK_SCU_PWM		       (SCU_REG_BASE + 0x10)
#define LDK_SCU_PLLPD	       	(SCU_REG_BASE + 0x14)
#define LDK_SCU_STATUS	       	(SCU_REG_BASE + 0x18)


//SCU_CLKCFG 	0x08
#define LDK_SCU_CLKCFG_SD		(1<<28)
#define LDK_SCU_CLKCFG_I2S		(1<<27)
#define LDK_SCU_CLKCFG_I2C		(1<<26)
#define LDK_SCU_CLKCFG_WDT1		(1<<25)
#define LDK_SCU_CLKCFG_WDT0		(1<<24)
#define LDK_SCU_CLKCFG_TIMER1	(1<<23)
#define LDK_SCU_CLKCFG_TIMER0	(1<<22)
#define LDK_SCU_CLKCFG_RTC		(1<<21)
#define LDK_SCU_CLKCFG_GPIO1	(1<<20)
#define LDK_SCU_CLKCFG_GPIO0	(1<<19)
#define LDK_SCU_CLKCFG_UART2	(1<<18)
#define LDK_SCU_CLKCFG_UART01	(1<<17)
#define LDK_SCU_CLKCFG_SPI		(1<<16)
#define LDK_SCU_CLKCFG_MP4D		(1<<13)
#define LDK_SCU_CLKCFG_MP4E		(1<<12)
#define LDK_SCU_CLKCFG_NAND		(1<<11)
#define LDK_SCU_CLKCFG_MAC1		(1<<10)
#define LDK_SCU_CLKCFG_MAC0		(1<<9)
#define LDK_SCU_CLKCFG_A2PCI	(1<<8)
#define LDK_SCU_CLKCFG_IDE		(1<<7)
#define LDK_SCU_CLKCFG_UDC		(1<<6)
#define LDK_SCU_CLKCFG_UHC		(1<<5)
#define LDK_SCU_CLKCFG_HDMA		(1<<4)
#define LDK_SCU_CLKCFG_ARM7		(1<<2)
#define LDK_SCU_CLKCFG_SDRSTMC	(1<<1)
#define LDK_SCU_CLKCFG_DDRMC	(1<<0)

#define IDLE_DIS_ALLIP	(~(LDK_SCU_CLKCFG_DDRMC | LDK_SCU_CLKCFG_SDRSTMC | LDK_SCU_CLKCFG_UART01))

//SCU_PWM	0x0C
#define LDK_SCU_PWM_EXT_PIN		(1<<6)
#define LDK_SCU_PWM_RTC_WAK		(1<<5)
#define LDK_SCU_PWM_EXT_WAK		(1<<4)
#define LDK_SCU_PWM_INT_CLR		(1<<3)
#define LDK_SCU_PWM_MODE_MASK	0x07	//bit 0 ~ 2
#define SCU_RTC_WAK				(LDK_SCU_PWM_EXT_WAK & (~LDK_SCU_PWM_RTC_WAK))
#define SCU_EXT_WAK				(LDK_SCU_PWM_RTC_WAK & (~LDK_SCU_PWM_EXT_WAK) & (~LDK_SCU_PWM_EXT_PIN))

//PWM Mode
#define PWM_NORMAL				0x0
#define PWM_SLOW				0x1
#define PWM_IDLE				0x2
#define PWM_STOP				0x4

//SCU_PLLPD		0x10
#define LDK_SCU_PLLPD_PWR_DOWN	0x1

//SCU_STATUS	0x14

#define LDK_SCU_STATUS_SYSCLK		(1<<1)
#define LDK_SCU_STATUS_PLLCLK		(1<<0)

//SCU_LPCID

#define SCU_IP_DISABLE(x) 		(writew( readw(LDK_SCU_CLKCFG) |x,LDK_SCU_CLKCFG))
#define SCU_IP_ENABLE(x) 		(writew( (~x) & readw(LDK_SCU_CLKCFG),LDK_SCU_CLKCFG))

#define SCU_NORMAL				0
#define SCU_SLOW				1
#define SCU_IDLE				2
#define SCU_STOP				3
#define SCU_PW_OFF				4
/*
* ioctl commands
*/
/* soft reset, master enable, and run bit is ignored, driver handles it itself. */
#define SCU_IOC_NORMAL_EN	_IO('s', 0)
#define SCU_IOC_NORMAL_DIS	_IO('s', 1)
#define SCU_IOC_SLOW		_IO('s', 2)
#define SCU_IOC_IDLE		_IO('s', 3)
#define SCU_IOC_STOP		_IO('s', 4)
#define SCU_IOC_POWER_OFF	_IO('s', 5)


#endif


/* linux/include/asm/arch-socle/socle-ide.h
 *
 * Copyright (c) 2006 Socle-tech Corp
 *		      http://www.socle-tech.com.tw/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * LDK Timer configuration
*/

#ifndef __LDK_IDEREG_H
#define __LDK_IDEREG_H                     1

#include <mach/platform.h>

#define SOCLE_IDE_BASE IO_ADDRESS(SOCLE_AHB0_IDE)
#define ACSIDE_IDE_CLK 0x1
#define ACSIDE_IDE0_BASE SOCLE_IDE_BASE
#define ACSIDE_IDE1_BASE SOCLE_IDE_BASE + 0x20
#define ACSIDE_IDE0_DMA ACSIDE_IDE0_BASE + 0x40
#define ACSIDE_IDE1_DMA ACSIDE_IDE1_BASE + 0x48
#define ACSIDE_IRQ 0x0a
#define IO_DATA 0x00
#define IO_ERROR 0x01
#define IO_FEATURE 0x01
#define IO_SECCNT 0x02
#define IO_SECNUM 0x03
#define IO_CYLLOW 0x04
#define IO_CYLHIGH 0x05
#define IO_DEVHEAD 0x06
#define IO_CMD 0x07
#define IO_STATUS 0x07
#define IO_DEVCTL 0x0E
#define IO_ALTSTATUS 0x0E
#define IDE_TIME_CTRL 0x10
#define IDE_CLK_DIV 0x14
#define IDE_IP_INFO 0x18
#define ACSIDE_BUS_MA_CMD 0x40
#define ACSIDE_BUS_MA_STATUS 0x42
#define ACSIDE_BUS_MA_SG_BASE_ADDR 0x44
#define PIO_TIMING_REG (SOCLE_IDE_BASE + IDE_TIME_CTRL + 1)
#define MDMA_TIMING_REG (SOCLE_IDE_BASE + IDE_TIME_CTRL + 1)
#define UDMA_TIMING0_REG (SOCLE_IDE_BASE + IDE_TIME_CTRL)
#define UDMA_TIMING1_REG (SOCLE_IDE_BASE + IDE_TIME_CTRL)
#define CLK_MOD_REG (SOCLE_IDE_BASE + IDE_CLK_DIV)
#define TIMING_CTRL_REG (SOCLE_IDE_BASE + IDE_TIME_CTRL)

#endif


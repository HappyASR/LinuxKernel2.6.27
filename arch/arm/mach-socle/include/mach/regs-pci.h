/* linux/include/asm/arch-socle/socle-pci.h
 *
 * Copyright (c) 2006 Socle-tech Corp
 *		      http://www.socle-tech.com.tw/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SQ Timer configuration
*/

#ifndef __ASM_ARCH_SOCLE_PCI_H
#define __ASM_ARCH_SOCLE_PCI_H	1

#include <mach/platform.h>

//============================= PCI =================================
#define PCI_VENDOR_ID_SOCLE				(0xabcd)
#define PCI_DEVICE_ID_SOCLE				(0x1234)

#define SOCLE_PCI_MEM_BASE         		SOCLE_BUS0_PCI /*0x19CE0000 	64M PCI MEMORY*/
#define SOCLE_PCI_MEM_SIZE		 		0x04000000	/* mem size, 64MB */

#define SOCLE_PCI_IO_BASE          		(SOCLE_BUS0_PCI+SOCLE_PCI_MEM_SIZE) /*0x1DCE0000	 8M PCI IO */
#define SOCLE_PCI_IO_SIZE			 		0x00800000	/* I/O size, 8MB */

#define SOCLE_PCI_BR_BASE          		(SOCLE_BUS0_PCI+SOCLE_PCI_MEM_SIZE+SOCLE_PCI_IO_SIZE)	/*0x1e4e0000	 2M PCI CONFIGURATION*/

//#define BR	0x00
//#define	MEM	0x400
#define SOCLE_PCI_BR_VADDR						IO_ADDRESS(SOCLE_PCI_BR_BASE)
#define SOCLE_PCI_MEM_VADDR        					IO_ADDRESS(SOCLE_PCI_MEM_BASE)
#define SOCLE_PCI_IO_VADDR            				IO_ADDRESS(SOCLE_PCI_IO_BASE)

#define MSK(n)                    					((1 << (n)) - 1)

#define SOCLE_PCI_CFGADDR_REGNUM_SHF		2
#define SOCLE_PCI_CFGADDR_REGNUM_MSK		(MSK(6) << SOCLE_PCI_CFGADDR_REGNUM_SHF)
#define SOCLE_PCI_CFGADDR_FUNCTNUM_SHF		8
#define SOCLE_PCI_CFGADDR_FUNCTNUM_MSK		(MSK(3) << SOCLE_PCI_CFGADDR_FUNCTNUM_SHF)
#define SOCLE_PCI_CFGADDR_DEVNUM_SHF		11
#define SOCLE_PCI_CFGADDR_DEVNUM_MSK		(MSK(5) << SOCLE_PCI_CFGADDR_DEVNUM_SHF)
#define SOCLE_PCI_CFGADDR_BUSNUM_SHF		16
#define SOCLE_PCI_CFGADDR_BUSNUM_MSK		(MSK(8) << SOCLE_PCI_CFGADDR_BUSNUM_SHF)
#define SOCLE_PCI_CFGADDR_CONFIGEN_SHF		31
#define SOCLE_PCI_CFGADDR_CONFIGEN_MSK		(MSK(1) << SOCLE_PCI_CFGADDR_CONFIGEN_SHF)
#define SOCLE_PCI_CFGADDR_CONFIGEN_BIT		SOCLE_PCI_CFGADDR_CONFIGEN_MSK

#define SOCLE_PCI_CFGHOST_OFS		(0x00)
#define SOCLE_PCI_CFGBAR0_OFS		(0x04)
#define SOCLE_PCI_CFGUPATU_OFS		(0x14)
#define SOCLE_PCI_CFGADDR_OFS		(0x20)
#define SOCLE_PCI_CFGDATA_OFS		(0x24)
//============================= PCI END ============================

#endif /*  __ASM_ARCH_SOCLE_PCI_H */


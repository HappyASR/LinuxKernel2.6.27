/********************************************************************************
* File Name     : include/asm-arm/arch-socle/sdhc_slave.h
* Author        : CY Li
* Description   : Socle SDHC Slave Header
*
* Copyright (C) SQ Tech. Corp.
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by the Free Software Foundation;
* either version 2 of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
*   Version      : 2,0,0,1
*   History      :
*      1. 2009/05/05 cyli create this file
*
********************************************************************************/

#ifndef __SDHC_SLAVE_H
#define __SDHC_SLAVE_H

#define CID_MID			0xab	// 8-bit binary number
#define CID_OID			"SD"		// 2-character ASCII string
#define CID_PNM			"Socle"	// 5-character ASCII string

/* The product revision is composed of two Binary Coded Decimal (BCD) digits,
     four bits each, representing an "n.m" revision number.
     The "n" is the most significant nibble and "m" is the least significant nibble. */
#define CID_PRV_N		0
#define CID_PRV_M		0

#define CID_PSN			0xabcd1234	// The serial number is 32 bits of binary number.

/* The manufacturing date is composed of two hexadecimal digits, one is 8 bits representing the year(y)
     and the other is 4 bits representing the month (m). */
#define CID_MDT_Y			9	// year, 0 => 2000
#define CID_MDT_M			5	// month, 1 => January


#define DATA_STAT_AFTER_ERASE		0x0		// 0x0 or 0x1


#define C_SIZE_L						0x1f		// n = 0x0 ~ 0xffff, (n + 1) * 512KB => SDHC card size

#endif //__SDHC_SLAVE_H

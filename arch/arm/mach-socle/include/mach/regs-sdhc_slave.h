/********************************************************************************
* File Name     : include/asm-arm/arch-socle/regs-sdhc_slave.h
* Author        : CY Li
* Description   : Socle SDHC Slave Register Header
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

#ifndef __SDHC_SLAVE_REG_H
#define __SDHC_SLAVE_REG_H

/* Function 0 Programmable Registers */
#define SOCLE_SDHC_SLV_CIS_FUNC0_ADDR			0x0000
#define SOCLE_SDHC_SLV_ESW_CCCR					0x0004
#define SOCLE_SDHC_SLV_ESW_IO_OCR				0x0008
#define SOCLE_SDHC_SLV_ESW_CRD_RDY				0x000c
#define SOCLE_SDHC_SLV_INT_STA					0x0010
#define SOCLE_SDHC_SLV_INT_EN						0x0014
#define SOCLE_SDHC_SLV_INT_CLR						0x0018
#define SOCLE_SDHC_SLV_INT_RAW_STA				0x001c

/* Function 1 Programmable Registers */
#define SOCLE_SDHC_SLV_AHB_BST_SZ_REG			0x012c

/* SD Memory Programmable Registers */
#define SOCLE_SDHC_SLV_MEM_OCR					0x0200
//#define SOCLE_SDHC_SLV_MEM_WR_ADDR				0x0204
#define SOCLE_SDHC_SLV_CID0						0x0208
#define SOCLE_SDHC_SLV_CID1						0x020c
#define SOCLE_SDHC_SLV_CID2						0x0210
#define SOCLE_SDHC_SLV_CID3						0x0214
#define SOCLE_SDHC_SLV_CSD0						0x0218
#define SOCLE_SDHC_SLV_CSD1						0x021c
#define SOCLE_SDHC_SLV_CSD2						0x0220
#define SOCLE_SDHC_SLV_CSD3						0x0224
#define SOCLE_SDHC_SLV_SCR0						0x0228
#define SOCLE_SDHC_SLV_SCR1						0x022c
#define SOCLE_SDHC_SLV_PWD_LEN					0x0230
#define SOCLE_SDHC_SLV_PWD0						0x0234
#define SOCLE_SDHC_SLV_PWD1						0x0238
#define SOCLE_SDHC_SLV_PWD2						0x023c
#define SOCLE_SDHC_SLV_PWD3						0x0240
#define SOCLE_SDHC_SLV_BLK_SZ						0x0244
#define SOCLE_SDHC_SLV_ARG							0x0248
#define SOCLE_SDHC_SLV_NUM_OF_WR_BLKS			0x024c
#define SOCLE_SDHC_SLV_ERA_STR_ADDR				0x0250
#define SOCLE_SDHC_SLV_ERA_END_ADDR				0x0254
#define SOCLE_SDHC_SLV_PRE_ERA_BLK_CNT			0x0258
#define SOCLE_SDHC_SLV_SD_MEM_PRG				0x025c
//#define SOCLE_SDHC_SLV_MEM_RD_ADDR				0x0260
#define SOCLE_SDHC_SLV_UNI_CNT_REG				0x0264
#define SOCLE_SDHC_SLV_DMA_SYS_ADDR				0x0268
#define SOCLE_SDHC_SLV_DMA_BUF_SZ				0x026c
#define SOCLE_SDHC_SLV_SDIO_BLK_CNT_REG			0x0270
#define SOCLE_SDHC_SLV_MEM_BLK_CNT_REG			0x0274


// SOCLE_SDHC_SLV_ESW_CCCR (0x0004)
#define ESW_CCCR_SD_REV(x)						(((x) & 0xf) << 8)
#define ESW_CCCR_SHS(x)							(((x) & 0x1) << 21)
#define ESW_CCCR_MEM_PRE(x)					(((x) & 0x3) << 22)

// SOCLE_SDHC_SLV_ESW_CRD_RDY (0x000c)
#define ESW_CRD_RDY_CAD_RDY(x)					(x & 0x1)

// SOCLE_SDHC_SLV_INT_STA (0x0010)
// SOCLE_SDHC_SLV_INT_EN (0x0014)
// SOCLE_SDHC_SLV_INT_CLR (0x0018)
// SOCLE_SDHC_SLV_INT_RAW_STA (0x001c)
#define INT_RES									(1 << 4)
#define INT_CSD_PRG								(1 << 5)
#define INT_PWD_UPD							(1 << 6)
#define INT_ERA									(1 << 7)
#define INT_PRE_ERA								(1 << 8)
#define INT_MEM_RD								(1 << 9)
#define INT_MEM_WR								(1 << 10)
#define INT_MEM_WR_OV							(1 << 11)
#define INT_MEM_RD_OV							(1 << 12)
#define INT_DMA_INT								(1 << 26)

// SOCLE_SDHC_SLV_AHB_BST_SZ_REG (0x012c)
#define AHB_BST_SZ_REG_BST_SZ(x)				((x) & 0xff)
#define AHB_BST_SZ_REG_BST_SZ_1				0x0
#define AHB_BST_SZ_REG_BST_SZ_4				0x3
#define AHB_BST_SZ_REG_BST_SZ_8				0x5
#define AHB_BST_SZ_REG_BST_SZ_16				0x7

// SOCLE_SDHC_SLV_MEM_OCR (0x0200)
#define MEM_OCR_MEM_OCR(x)					((x) & 0x00ffffff)
#define MEM_OCR_CCS(x)							(((x) & 0x1) << 30)

// SOCLE_SDHC_SLV_CID0 (0x0208)
#define CID0_AWS_1								1
#define CID0_CRC(x)								(((x) & 0x7f) << 1)
#define CID0_MDT(x)								(((x) & 0xfff) << 8)
#define CID0_PSN_L(x)							(((x) & 0xff) << 24)

// SOCLE_SDHC_SLV_CID1 (0x020c)
#define CID1_PSN_H(x)							((x) & 0xffffff)
#define CID1_PRV(x)								(((x) & 0xff) << 24)

// SOCLE_SDHC_SLV_CID2 (0x0210)
#define CID2_PNM_L(x)							(x)

// SOCLE_SDHC_SLV_CID3 (0x0214)
#define CID3_PNM_H(x)							((x) & 0xff)
#define CID3_OID(x)								(((x) & 0xffff) << 8)
#define CID3_MID(x)								(((x) & 0xff) << 24)

// SOCLE_SDHC_SLV_CSD0 (0x0218)
#define CSD0_AWS_1								1
#define CSD0_CRC(x)								(((x) & 0x7f) << 1)
#define CSD0_FILE_FORMAT(x)					(((x) & 0x3) << 10)
#define CSD0_TMP_WRITE_PROTECT(x)				(((x) & 0x1) << 12)
#define CSD0_PERM_WRITE_PROTECT(x)			(((x) & 0x1) << 13)
#define CSD0_COPY(x)							(((x) & 0x1) << 14)
#define CSD0_FILE_FORMAT_GRP(x)				(((x) & 0x1) << 15)
#define CSD0_WRITE_BL_PARTIAL(x)				(((x) & 0x1) << 21)
#define CSD0_WRITE_BL_LEN(x)					(((x) & 0xf) << 22)
#define CSD0_R2W_FACTOR(x)						(((x) & 0x7) << 26)
#define CSD0_WP_GRP_ENABLE(x)					(((x) & 0x1) << 31)

// SOCLE_SDHC_SLV_CSD1 (0x021c)
#define CSD1_WP_GRP_SIZE(x)					((x) & 0x7f)
#define CSD1_SECTOR_SIZE(x)						(((x) & 0x7f) << 7)
#define CSD1_ERASE_BLK_EN(x)					(((x) & 0x1) << 14)
#define CSD1_C_SIZE_L(x)							(((x) & 0xffff) << 16)

// SOCLE_SDHC_SLV_CSD2 (0x0220)
#define CSD2_C_SIZE_H(x)						((x) & 0x3f)
#define CSD2_DSR_IMP(x)							(((x) & 0x1) << 12)
#define CSD2_READ_BLK_MISALIGN(x)				(((x) & 0x1) << 13)
#define CSD2_WRITE_BLK_MISALIGN(x)			(((x) & 0x1) << 14)
#define CSD2_READ_BL_PARTIAL(x)				(((x) & 0x1) << 15)
#define CSD2_READ_BL_LEN(x)					(((x) & 0xf) << 16)
#define CSD2_CCC(x)								(((x) & 0xfff) << 20)

// SOCLE_SDHC_SLV_CSD3 (0x0224)
#define CSD3_TRAN_SPEED(x)						((x) & 0xff)
#define CSD3_NSAC(x)							(((x) & 0xff) << 8)
#define CSD3_TAAC(x)							(((x) & 0xff) << 16)
#define CSD3_CSD_STRUCTURE(x)					(((x) & 0x3) << 30)


// SOCLE_SDHC_SLV_SCR0 (0x0228)
#define SCR0_RSV_4_MFT_USG(x)					(x)

// SOCLE_SDHC_SLV_SCR1 (0x022c)
#define SCR1_SD_BUS_WIDTHS(x)					(((x) & 0xf) << 16)
#define SCR1_SD_SECURITY(x)						(((x) & 0x7) << 20)
#define SCR1_DATA_STAT_AFTER_ERASE(x)		(((x) & 0x1) << 23)
#define SCR1_SD_SPEC(x)							(((x) & 0xf) << 24)
#define SCR1_SCR_STRUCTURE(x)					(((x) & 0xf) << 28)

// SOCLE_SDHC_SLV_SD_MEM_PRG (0x025c)
#define SD_MEM_PRG_MEM_PRG_DN				(1 << 0)
//#define SD_MEM_PRG_MEM_RD_AV					(1 << 1)
//#define SD_MEM_PRG_MEM_WR_AV					(1 << 2)
//#define SD_MEM_PRG_MEM_RD_AV_CTRL			(1 << 3)
//#define SD_MEM_PRG_MEM_WR_AV_CTRL			(1 << 4)

// SOCLE_SDHC_SLV_DMA_BUF_SZ (0x026c)
#define DMA_BUF_SZ_512B						(0x0)
#define DMA_BUF_SZ_1KB							(0x1)
#define DMA_BUF_SZ_2KB							(0x2)
#define DMA_BUF_SZ_4KB							(0x3)
#define DMA_BUF_SZ_8KB							(0x4)
#define DMA_BUF_SZ_16KB						(0x5)
#define DMA_BUF_SZ_32KB						(0x6)
#define DMA_BUF_SZ_64KB						(0x7)
#define DMA_BUF_SZ_SYS_ADDR_VLD				(1 << 3)

#endif //__SDHC_SLAVE_REG_H

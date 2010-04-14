/* linux/include/asm/arch-ldk/ldk-nand.h
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
#ifndef __ASM_ARCH_SOCLE_NAND_H
#define __ASM_ARCH_SOCLE_NAND_H

#include <mach/platform.h>

/*
 *  Registers for NAND flash controller
 *  */
#define NAND_VA_BASE IO_ADDRESS(SOCLE_AHB0_NAND)
#define NAND_FLSH_BUFFER(x) ((x) + 0x8000) /* data buffer inside NAND flash controller */
#define NAND_FLSH_CONF(x) ((x) + 0xC000)	/* flash type configuration register */
#define NAND_FLSH_COMM_1(x) ((x) + 0xC004)	/* first Command input register */
#define NAND_FLSH_COMM_2(x) ((x) + 0xC008)/* second Command input register */
#define NAND_FLSH_STATE_COMM(x) ((x) + 0xC00C)/* second Command input register */
#define NAND_FLSH_ADDRESS_1(x) ((x) + 0xC010)/* 1st Cycle address input register */
#define NAND_FLSH_ADDRESS_2(x) ((x) + 0xC014)/* 2nd Cycle address input register */
#define NAND_FLSH_ADDRESS_3(x) ((x) + 0xC018)/* 3rd Cycle address input regiater */
#define NAND_FLSH_ADDRESS_4(x) ((x) + 0xC01C)/* 4th Cycle address input register */
#define NAND_FLSH_ADDRESS_5(x) ((x) + 0xC020)/* 5th Cycle address input register */
#define NAND_FLSH_DATA(x) ((x) + 0xC024)	/* access Data register */
#define NAND_FLSH_BUFF_STADDR(x) ((x) + 0xC028)/* internal buffer start address register */
#define NAND_FLSH_BUFF_CNT(x) ((x) + 0xC02C)/* internal bufer data count register */
#define NAND_FLSH_BUFF_STATE(x) ((x) + 0xC030) /* internal buffer data count register */
#define NAND_FLSH_DMA_SET(x) ((x) + 0xC034) /* dma operation setting register */
#define NAND_FLSH_CE_WP(x) ((x) + 0xC038) 	/* chip enable/Write protect register */
#define NAND_FLSH_CONTROL(x) ((x) + 0xC03C) /* NAND flash controller control register */
#define NAND_FLSH_RESET(x) ((x) + 0xC040)	/* NAND flash controller control register */
#define NAND_FLSH_STATE(x) ((x) + 0xC044)	/* NAND flash controller status register */
#define NAND_FLSH_INT_MASK(x) ((x) + 0xC048)	/* NAND flash controller interrupt mask register */
#define NAND_FLSH_INT_STATE(x) ((x) + 0xC04C)	/* NAND flash controller interrupt status register */
#define NAND_FLSH_GPIO(x) ((x) + 0xC050)	/* GPIO control register */
#define NAND_FLSH_S_NUM(x) ((x) + 0xC054)	/* flash data serial number register */
#define NAND_FLSH_1ST_ECC_1(x) ((x) + 0xC058) /* 1st 512 byte ECC_1 code register */
#define NAND_FLSH_1ST_ECC_2(x) ((x) + 0xC05C) /* 1st 512 byte ECC_2 code register */
#define NAND_FLSH_1ST_ECC_3(x) ((x) + 0xC060) /* 1st 512 byte ECC_3 code register */
#define NAND_FLSH_1ST_ECC_4(x) ((x) + 0xC064) /* 1st 512 byte ECC_4 code register */
#define NAND_FLSH_2ND_ECC_1(x) ((x) + 0xC068) /* 2nd 512 byte ECC_1 code register */
#define NAND_FLSH_2ND_ECC_2(x) ((x) + 0xC06C) /* 2nd 512 byte ECC_2 code register */
#define NAND_FLSH_2ND_ECC_3(x) ((x) + 0xC070) /* 2nd 512 byte ECC_3 code register */
#define NAND_FLSH_2ND_ECC_4(x) ((x) + 0xC074) /* 2nd 512 byte ECC_4 code register */
#define NAND_FLSH_3RD_ECC_1(x) ((x) + 0xC078) /* 3rd 512 byte ECC_1 code register */
#define NAND_FLSH_3RD_ECC_2(x) ((x) + 0xC07C) /* 3rd 512 byte ECC_2 code register */
#define NAND_FLSH_3RD_ECC_3(x) ((x) + 0xC080) /* 3rd 512 byte ECC_3 code register */
#define NAND_FLSH_3RD_ECC_4(x) ((x) + 0xC084) /* 3rd 512 byte ECC_4 code register */
#define NAND_FLSH_4TH_ECC_1(x) ((x) + 0xC088) /* 4th 512 byte ECC_1 code register */
#define NAND_FLSH_4TH_ECC_2(x) ((x) + 0xC08C) /* 4th 512 byte ECC_2 code register */
#define NAND_FLSH_4TH_ECC_3(x) ((x) + 0xC090) /* 4th 512 byte ECC_3 code register */
#define NAND_FLSH_4TH_ECC_4(x) ((x) + 0xC094) /* 4th 512 byte ECC_4 code register */
#define NAND_FLSH_1ST_SYNDR_1(x) ((x) + 0xC098)	/* 1st 512 byte Syndrome_1 code register. (Big page only) */
#define NAND_FLSH_1ST_SYNDR_2(x) ((x) + 0xC09C)	/* 1st 512 byte Syndrome_2 code register */
#define NAND_FLSH_1ST_SYNDR_3(x) ((x) + 0xC0A0)	/* 1st 512 byte Syndrome_3 code register */
#define NAND_FLSH_1ST_SYNDR_4(x) ((x) + 0xC0A4)	/* 1st 512 byte Syndrom3_4 code register */
#define NAND_FLSH_2ND_SYNDR_1(x) ((x) + 0xC0A8)	/* 2nd 512 byte Syndrome_1 code register. (Big page only) */
#define NAND_FLSH_2ND_SYNDR_2(x) ((x) + 0xC0AC)	/* 2nd 512 byte Syndrome_2 code register */
#define NAND_FLSH_2ND_SYNDR_3(x) ((x) + 0xC0B0)	/* 2ne 512 byte Syndrome_3 code register */
#define NAND_FLSH_2ND_SYNDR_4(x) ((x) + 0xC0B4)	/* 2nd 512 byte Syndrom3_4 code register */
#define NAND_FLSH_3RD_SYNDR_1(x) ((x) + 0xC0B8)	/* 3rd 512 byte Syndrome_1 code register. (Big page only) */
#define NAND_FLSH_3RD_SYNDR_2(x) ((x) + 0xC0BC)	/* 3rd 512 byte Syndrome_2 code register */
#define NAND_FLSH_3RD_SYNDR_3(x) ((x) + 0xC0C0)	/* 3rd 512 byte Syndrome_3 code register */
#define NAND_FLSH_3RD_SYNDR_4(x) ((x) + 0xC0C4)	/* 3rd 512 byte Syndrom3_4 code register */
#define NAND_FLSH_4TH_SYNDR_1(x) ((x) + 0xC0C8)	/* 4th 512 byte Syndrome_1 code register. (Big page only) */
#define NAND_FLSH_4TH_SYNDR_2(x) ((x) + 0xC0CC)	/* 4th 512 byte Syndrome_2 code register */
#define NAND_FLSH_4TH_SYNDR_3(x) ((x) + 0xC0D0)	/* 4th 512 byte Syndrome_3 code register */
#define NAND_FLSH_4TH_SYNDR_4(x) ((x) + 0xC0D4)	/* 4th 512 byte Syndrom3_4 code register */

/*
 *  Macros
 *  */
#define NAND_DISABLE_FLSH_RESET(x)		\
     iowrite32(0x00000000 |			\
	       NAND_RESET_DISABLE,		\
	       NAND_FLSH_RESET(x))

#define NAND_RESET_FLSH_INT_MASK(x)		\
     iowrite32(0x00000000, NAND_FLSH_INT_MASK(x))

#define NAND_RESET_FLSH_INT_STATE(x)		\
     iowrite32(0x00000000, NAND_FLSH_INT_STATE(x))

#define NAND_RESET_FLSH_DATA(x)			\
     iowrite32(0x00000000, NAND_FLSH_DATA(x))

/*
 *  NAND_FLSH_CONF
 *  */
/* tWC: Write Cycle time */
#define NAND_tWC_0CYCLE (0 << 17) /* 0cycle */
#define NAND_tWC_1CYCLE (1 << 17) /* 1cycle */
#define NAND_tWC_2CYCLE (2 << 17) /* 2cycle */
#define NAND_tWC_3CYCLE (3 << 17) /* 3cycle */
#define NAND_tWC_4CYCLE (4 << 17) /* 4cycle */
#define NAND_tWC_5CYCLE (5 << 17) /* 5cycle */
#define NAND_tWC_6CYCLE (6 << 17) /* 6cycle */
#define NAND_tWC_7CYCLE (7 << 17) /* 7cycle */
#define NAND_tWC_8CYCLE (8 << 17) /* 8cycle */
#define NAND_tWC_9CYCLE (9 << 17) /* 9cycle */
#define NAND_tWC_10CYCLE (10 << 17) /* 10cycle */
#define NAND_tWC_11CYCLE (11 << 17) /* 11cycle */
#define NAND_tWC_12CYCLE (12 << 17) /* 12cycle */
#define NAND_tWC_13CYCLE (13 << 17) /* 13cycle */
#define NAND_tWC_14CYCLE (14 << 17) /* 14cycle */
#define NAND_tWC_15CYCLE (15 << 17) /* 15cycle (default) */

/* tWP: Write Pulse time, duration of write pulse */
#define NAND_tWP_0CYCLE (0 << 14) /* 0cycle */
#define NAND_tWP_1CYCLE (1 << 14) /* 1cycle */
#define NAND_tWP_2CYCLE (2 << 14) /* 2cycle */
#define NAND_tWP_3CYCLE (3 << 14) /* 3cycle */
#define NAND_tWP_4CYCLE (4 << 14) /* 4cycle */
#define NAND_tWP_5CYCLE (5 << 14) /* 5cycle */
#define NAND_tWP_6CYCLE (6 << 14) /* 6cycle */
#define NAND_tWP_7CYCLE (7 << 14) /* 7cycle (default) */

/* tRC: Read Cycle time */
#define NAND_tRC_0CYCLE (0 << 10) /* 0cycle */
#define NAND_tRC_1CYCLE (1 << 10) /* 1cycle */
#define NAND_tRC_2CYCLE (2 << 10) /* 2cycle */
#define NAND_tRC_3CYCLE (3 << 10) /* 3cycle */
#define NAND_tRC_4CYCLE (4 << 10) /* 4cycle */
#define NAND_tRC_5CYCLE (5 << 10) /* 5cycle */
#define NAND_tRC_6CYCLE (6 << 10) /* 6cycle */
#define NAND_tRC_7CYCLE (7 << 10) /* 7cycle */
#define NAND_tRC_8CYCLE (8 << 10) /* 8cycle */
#define NAND_tRC_9CYCLE (9 << 10) /* 9cycle */
#define NAND_tRC_10CYCLE (10 << 10) /* 10cycle */
#define NAND_tRC_11CYCLE (11 << 10) /* 11cycle */
#define NAND_tRC_12CYCLE (12 << 10) /* 12cycle */
#define NAND_tRC_13CYCLE (13 << 10) /* 13cycle */
#define NAND_tRC_14CYCLE (14 << 10) /* 14cycle */
#define NAND_tRC_15CYCLE (15 << 10) /* 15cycle (default) */

/* tRP: Read Pulse time, duration of read pulse */
#define NAND_tRP_0CYCLE (0 << 7) /* 0cycle */
#define NAND_tRP_1CYCLE (1 << 7) /* 1cycle */
#define NAND_tRP_2CYCLE (2 << 7) /* 2cycle */
#define NAND_tRP_3CYCLE (3 << 7) /* 3cycle */
#define NAND_tRP_4CYCLE (4 << 7) /* 4cycle */
#define NAND_tRP_5CYCLE (5 << 7) /* 5cycle */
#define NAND_tRP_6CYCLE (6 << 7) /* 6cycle */
#define NAND_tRP_7CYCLE (7 << 7) /* 7cycle (default) */


/* NAND-Flash memory page size */
#define NAND_PAGE_512BYTES (0 << 3)	/* 512 bytes (default) */
#define NAND_PAGE_2KBYTES (1 << 3) /* 2K bytes */

/*
 *  NAND_FLSH_COMM
 *  */
/* Command valid bit (set by SW, clear by H/W) */
#define NAND_CMD_NOTVALID 0	/* command not valid */
#define NAND_CMD_VALID (1 << 8)	/* command is valid */
#define NAND_CMD_INPUT(x) ((x) & 0x0000000FF) /* NAND flash memory commmand input port */

/*
 *  NAND_FLSH_ADDRESS
 *  */
/* Address valid Bit (set by S/W, clear by H/W) */
#define NAND_ADDR_NOTVALID 0 /* Address not valid */
#define NAND_ADDR_VALID (1 << 8) /* Address is valid */
#define NAND_ADDR_INPUT(x) ((x) & 0x000000FF) /* NAND Flash memory address input port */

/*
 *  NAND_FLSH_BUFF_STADDR
 *  */
#define NAND_InterBuff_START_ADDR(x) ((x) & 0x0000FFFF) /* internal buffer start address */

/*
 *  NAND_FLSH_BUFF_CNT
 *  */
/* Data access with/without ECC code */
#define NAND_RW_WITHOUT_ECC 0 	/* access without ecc code */
#define NAND_RW_WITH_ECC (1 << 16) /* access with ecc code */
#define NAND_InterBuff_DATA_NUM(x) ((x) & 0x0000FFFF) /* internal buffer read/write data number */
//leonid+ for support 8/15 bytes parity length
#define NAND_NPAR_LEN_8B	0x0			/* ECC Parity length 8 bytes	*/
#define NAND_NPAR_LEN_15B	(0x1 << 17)	/* ECC Parity length 15 bytes	*/

/*
 *  NAND_FLSH_DMA_SET
 *  */
/* Endan mode set */
#define NAND_LITTLE_ENDING 0	/* little ending */
#define NAND_BIG_ENDING (1 << 11) /* big ending */

/* ECC test mode */
#define NAND_ECC_TESTMODE_DISABLE 0 /* disable ecc codec test mode, data path is normal */
#define NAND_ECC_TESTMODE_ENABLE (1 << 10) /* enable ecc codec test mode, data path not to NAND Flash */

/* Buffer Read/Write Enable */
#define NAND_BUFFER_RW_DISABLE 0	/* disable buffer read/write */
#define NAND_BUFFER_RW_ENABLE (1 << 9) /* enable buffer read/write */

/* Second Command Input */
#define NAND_SECOND_CMD_NO 0 /* no second command input */
#define NAND_SECOND_CMD_YES (1 << 8) /* second command input */

/* Auto Status Check Enable */
#define NAND_AUTO_STATUS_CHECK_DISABLE 0 /* disable auto status check */
#define NAND_AUTO_STATUS_CHECK_ENABLE (1 << 7) /* enable auto status check */

/* Access Data Path Select */
#define NAND_ACCESS_DATA_INTER_BUF 0 /* access data from/to internal buffer */
#define NAND_ACCESS_DATA_REG_PORT (1 << 6) /* access data from/to register data port */

/* DMA Operation Type */
#define NAND_DMA_NOP 0		/* no data access */
#define NAND_DMA_READ (1 << 4)	/* read operation */
#define NAND_DMA_WRITE (2 << 4)	/* write operation */


/* Device Wait R/B Enable */
#define NAND_DEVICE_WAIT_RB 1 /* wait R/B */
#define NAND_DEVICE_NOT_WAIT_RB 0 /* not wait R/B */

/*
 *  NAND_FLSH_CE_WP
 *  */
     /* NAND Flash Device 3 Write Protect */
#define NAND_DEVICE3_WP_DISABLE 0 /* disable write protect */
#define NAND_DEVICE3_WP_ENABLE (1 << 7) /* enable write protect */

     /* NAND Flash Device 2 Write Protect */
#define NAND_DEVICE2_WP_DISABLE 0 /* disable write protect */
#define NAND_DEVICE2_WP_ENABLE (1 << 6) /* enable write protect */

     /* NAND Flash Device 1 Write Protect */
#define NAND_DEVICE1_WP_DISABLE 0 /* disable write protect */
#define NAND_DEVICE1_WP_ENABLE (1 << 5) /* enable write protect */

     /* NAND Flash Device 0 Write Protect */
#define NAND_DEVICE0_WP_DISABLE 0 /* disable write protect */
#define NAND_DEVICE0_WP_ENABLE (1 << 4) /* enable write protect */

     /* NAND Flash Device 3 Chip Enable */
#define NAND_DEVICE3_CHIP_DISABLE 0 /* chip disable */
#define NAND_DEVICE3_CHIP_ENABLE (1 << 3) /* chip enable */

     /* NAND Flash Device 2 Chip Enable */
#define NAND_DEVICE2_CHIP_DISABLE 0 /* chip disable */
#define NAND_DEVICE2_CHIP_ENABLE (1 << 2) /* chip enable */

     /* NAND Flash Device 1 Chip Enable */
#define NAND_DEVICE1_CHIP_DISABLE 0 /* chip disable */
#define NAND_DEVICE1_CHIP_ENABLE (1 << 1) /* chip enable */

     /* NAND Flash Device 0 Chip Enable */
#define NAND_DEVICE0_CHIP_DISABLE 0 /* chip disable */
#define NAND_DEVICE0_CHIP_ENABLE 1 /* chip enable */

/*
 *  NAND_FLSH_CONTROL
 *  */
     /* Flash DMA Enable */
#define NAND_DMA_DISABLE 0 /* disable flash DMA */
#define NAND_DMA_ENABLE (1 << 2) /* enable flash DMA */

     /* Access Data Enable */
#define NAND_DATA_RW_DISABLE 0 /* disable to Read/Write Data */
#define NAND_DATA_RW_ENABLE (1 << 1) /* enable to Read/Write Data */

     /* Flash Command/Address Input Enable */
#define NAND_SEND_CMD_ADDR_DISABLE 0 /* disable flash send command and address */
#define NAND_SEND_CMD_ADDR_ENABLE 1 /* enable flash send command and address */

/*
 *  NAND_FLSH_RESET
 *  */
     /* NAND Flash Controller Reset Enable */
#define NAND_RESET_DISABLE 0	/* disable reset */
#define NAND_RESET_ENABLE 1	/* enable reset */

/*
 *  NAND_FLSH_STATE
 *  */
     /* NAND Flash Device 3 Ready/Busy */
#define NAND_DEVICE3_RB_LOW 0 /* device 3 R/B = 1'b0 */
#define NAND_DEVICE3_RB_HIGH (1 << 3) /* device 3 R/B = 1'b1 */

     /* NAND Flash Device 2 Ready/Busy */
#define NAND_DEVICE2_RB_LOW 0 /* device 2 R/B = 1'b0 */
#define NAND_DEVICE2_RB_HIGH (1 << 2) /* device 2 R/B = 1'b1 */

     /* NAND Flash Device 1 Ready/Busy */
#define NAND_DEVICE1_RB_LOW 0 /* device 1 R/B = 1'b0 */
#define NAND_DEVICE1_RB_HIGH (1 << 1) /* device 1 R/B = 1'b1 */

     /* NAND Flash Device 0 Ready/Busy */
#define NAND_DEVICE0_RB_LOW 3 /* device 0 R/B = 1'b0 */
#define NAND_DEVICE0_RB_HIGH 1 /* device 0 R/B = 1'b1 */

/*
 *  NAND_FLSH_INT_MASK
 *  */
#define NAND_MASK_SYNDROME_ERROR_LOCATE4 (1 << 11) /* syndrome bit for 4th 512 data */
#define NAND_MASK_SYNDROME_ERROR_LOCATE3 (1 << 10) /* syndrome bit for 3rd 512 data */
#define NAND_MASK_SYNDROME_ERROR_LOCATE2 (1 << 9) /* syndrome bit for 2nd 512 data */
#define NAND_MASK_SYNDROME_ERROR_LOCATE1 (1 << 8) /* syndrome bit for 1st 512 data */
#define NAND_MASK_BLOCK_IS_ERASE (1 << 7) /* blcok data is erase */
#define NAND_MASK_BLOCK_ERASE_DONE (1 << 6) /* block erase done */
#define NAND_MASK_BLOCK_ERASE_FAIL (1 << 5) /* block erase fail */
#define NAND_MASK_READ_DATA_DONE (1 << 4) /* read data done */
#define NAND_MASK_SYNDROME_ERROR (1 << 3) /* syndrome error */
#define NAND_MASK_TIME_OUT (1 << 2)	/* time out */
#define NAND_MASK_WRITE_DATA_DONE (1 << 1) /* write data done */
#define NAND_MASK_WRITE_DATA_ERROR 1 /* write data error */

/*
 *  NAND_FLSH_INT_STATE
 *  */
 #define NAND_SYNDROME_ERROR_LOCATE4_MASK (0xf << 8) /* syndrome bit mask */	//20080326 leonid+ for  check syn error status
#define NAND_SYNDROME_ERROR_LOCATE4 (1 << 11) /* syndrome bit for 4th 512 data */
#define NAND_SYNDROME_ERROR_LOCATE3 (1 << 10) /* syndrome bit for 3rd 512 data */
#define NAND_SYNDROME_ERROR_LOCATE2 (1 << 9) /* syndrome bit for 2nd 512 data */
#define NAND_SYNDROME_ERROR_LOCATE1 (1 << 8) /* syndrome bit for 1st 512 data */
#define NAND_BLOCK_IS_ERASE (1 << 7) /* blcok data is erase */
#define NAND_BLOCK_ERASE_DONE (1 << 6) /* block erase done */
#define NAND_BLOCK_ERASE_FAIL (1 << 5) /* block erase fail */
#define NAND_READ_DATA_DONE (1 << 4) /* read data done */
#define NAND_SYNDROME_ERROR (1 << 3) /* syndrome error */
#define NAND_TIME_OUT (1 << 2)	/* time out */
#define NAND_WRITE_DATA_DONE (1 << 1) /* write data done */
#define NAND_WRITE_DATA_ERROR 1 /* write data error */

/*
 *  NAND_FLSH_ECC
 *  */
#define NAND_ECC_CODE_1(x) (((x) & (0x3ff << 20)) >> 20)
#define NAND_ECC_CODE_2(x) (((x) & (0x3ff << 10)) >> 10)
#define NAND_ECC_CODE_3(x) ((x) & 0x3ff)

/*
 *  NAND_FLSH_SYNDR
 *  */
#define NAND_SYNDROME_CODE_1(x) (((x) & (0x3ff << 20)) >> 20)
#define NAND_SYNDROME_CODE_2(x) (((x) & (0x3ff << 10)) >> 10)
#define NAND_SYNDROME_CODE_3(x) ((x) & 0x3ff)

#endif



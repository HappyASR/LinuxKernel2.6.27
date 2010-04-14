#ifndef __NAND_REGS_H_INCLUDED
#define __NAND_REGS_H_INCLUDED

#include <config.h>
#include <linux/io.h>


/*
 *  Registers for NAND flash controller
 *  */
#define NAND_FLSH_CONF(x) ((x) + 0x0000)	/* Flash type configuration register */
#define NAND_FLSH_COMM_1(x) ((x) + 0x0004)	/* First Command input register */
#define NAND_FLSH_COMM_2(x) ((x) + 0x0008)/* Second Command input register */
#define NAND_FLSH_STATE_COMM(x) ((x) + 0x000C)/* Second Command input register */
#define NAND_FLSH_ADDRESS_1(x) ((x) + 0x0010)/* 1st Cycle address input register */
#define NAND_FLSH_ADDRESS_2(x) ((x) + 0x0014)/* 2nd Cycle address input register */
#define NAND_FLSH_ADDRESS_3(x) ((x) + 0x0018)/* 3rd Cycle address input regiater */
#define NAND_FLSH_ADDRESS_4(x) ((x) + 0x001C)/* 4th Cycle address input register */
#define NAND_FLSH_ADDRESS_5(x) ((x) + 0x0020)/* 5th Cycle address input register */
#define NAND_FLSH_DATA(x) ((x) + 0x0024)	/* Access Data register */
#define NAND_FLSH_BUFF_STADDR(x) ((x) + 0x0028)/* Internal buffer start address register */
#define NAND_FLSH_BUFF_CNT(x) ((x) + 0x002C)/* Internal bufer data count register */
#define NAND_FLSH_BUFF_STATE(x) ((x) + 0x0030) /* Internal buffer data count register */
#define NAND_FLSH_DMA_SET(x) ((x) + 0x0034) /* DMA operation setting register */
#define NAND_FLSH_CE_WP(x) ((x) + 0x0038) 	/* Chip enable/Write protect register */
#define NAND_FLSH_CONTROL(x) ((x) + 0x003C) /* NAND Flash Controller control register */
#define NAND_FLSH_RESET(x) ((x) + 0x0040)	/* NAND Flash Controller control register */
#define NAND_FLSH_STATE(x) ((x) + 0x0044)	/* NAND Flash Controller status register */
#define NAND_FLSH_INT_MASK(x) ((x) + 0x0048)	/* NAND Flash Controller interrupt mask register */
#define NAND_FLSH_INT_STATE(x) ((x) + 0x004C)	/* NAND Flash Controller interrupt status register */
#define NAND_FLSH_GPIO(x) ((x) + 0x0050)	/* GPIO control register */
#define NAND_FLSH_S_NUM(x) ((x) + 0x0054)	/* Flash data serial number register */
#define NAND_FLSH_1ST_ECC_1(x) ((x) + 0x0058) /* 1st 512 byte ECC_1 code register */
#define NAND_FLSH_1ST_ECC_2(x) ((x) + 0x005C) /* 1st 512 byte ECC_2 code register */
#define NAND_FLSH_1ST_ECC_3(x) ((x) + 0x0060) /* 1st 512 byte ECC_3 code register */
#define NAND_FLSH_1ST_ECC_4(x) ((x) + 0x0064) /* 1st 512 byte ECC_4 code register */
#define NAND_FLSH_2ND_ECC_1(x) ((x) + 0x0068) /* 2ND 512 byte ECC_1 code register */
#define NAND_FLSH_2ND_ECC_2(x) ((x) + 0x006C) /* 2ND 512 byte ECC_2 code register */
#define NAND_FLSH_2ND_ECC_3(x) ((x) + 0x0070) /* 2ND 512 byte ECC_3 code register */
#define NAND_FLSH_2ND_ECC_4(x) ((x) + 0x0074) /* 2ND 512 byte ECC_4 code register */
#define NAND_FLSH_3RD_ECC_1(x) ((x) + 0x0078) /* 3RD 512 byte ECC_1 code register */
#define NAND_FLSH_3RD_ECC_2(x) ((x) + 0x007C) /* 3RD 512 byte ECC_2 code register */
#define NAND_FLSH_3RD_ECC_3(x) ((x) + 0x0080) /* 3RD 512 byte ECC_3 code register */
#define NAND_FLSH_3RD_ECC_4(x) ((x) + 0x0084) /* 3RD 512 byte ECC_4 code register */
#define NAND_FLSH_4TH_ECC_1(x) ((x) + 0x0088) /* 4TH 512 byte ECC_1 code register */
#define NAND_FLSH_4TH_ECC_2(x) ((x) + 0x008C) /* 4TH 512 byte ECC_2 code register */
#define NAND_FLSH_4TH_ECC_3(x) ((x) + 0x0090) /* 4TH 512 byte ECC_3 code register */
#define NAND_FLSH_4TH_ECC_4(x) ((x) + 0x0094) /* 4TH 512 byte ECC_4 code register */
#define NAND_FLSH_1ST_SYNDR_1(x) ((x) + 0x0098)	/* 1st 512 byte Syndrome_1 code register. (Big page only */
#define NAND_FLSH_1ST_SYNDR_2(x) ((x) + 0x009C)	/* 1st 512 byte Syndrome_2 code register */
#define NAND_FLSH_1ST_SYNDR_3(x) ((x) + 0x00A0)	/* 1st 512 byte Syndrome_3 code register */
#define NAND_FLSH_1ST_SYNDR_4(x) ((x) + 0x00A4)	/* 1st 512 byte Syndrom3_4 code register */
#define NAND_FLSH_2ND_SYNDR_1(x) ((x) + 0x00A8)	/* 1st 512 byte Syndrome_1 code register. (Big page only */
#define NAND_FLSH_2ND_SYNDR_2(x) ((x) + 0x00AC)	/* 1st 512 byte Syndrome_2 code register */
#define NAND_FLSH_2ND_SYNDR_3(x) ((x) + 0x00B0)	/* 1st 512 byte Syndrome_3 code register */
#define NAND_FLSH_2ND_SYNDR_4(x) ((x) + 0x00B4)	/* 1st 512 byte Syndrom3_4 code register */
#define NAND_FLSH_3RD_SYNDR_1(x) ((x) + 0x00B8)	/* 1st 512 byte Syndrome_1 code register. (Big page only */
#define NAND_FLSH_3RD_SYNDR_2(x) ((x) + 0x00BC)	/* 1st 512 byte Syndrome_2 code register */
#define NAND_FLSH_3RD_SYNDR_3(x) ((x) + 0x00C0)	/* 1st 512 byte Syndrome_3 code register */
#define NAND_FLSH_3RD_SYNDR_4(x) ((x) + 0x00C4)	/* 1st 512 byte Syndrom3_4 code register */
#define NAND_FLSH_4TH_SYNDR_1(x) ((x) + 0x00C8)	/* 1st 512 byte Syndrome_1 code register. (Big page only */
#define NAND_FLSH_4TH_SYNDR_2(x) ((x) + 0x00CC)	/* 1st 512 byte Syndrome_2 code register */
#define NAND_FLSH_4TH_SYNDR_3(x) ((x) + 0x00D0)	/* 1st 512 byte Syndrome_3 code register */
#define NAND_FLSH_4TH_SYNDR_4(x) ((x) + 0x00D4)	/* 1st 512 byte Syndrom3_4 code register */

/* NAND buffer */
#define NAND_BUFFER_ADDR CFG_NAND_BUFFER_ADDR

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
#define NAND_tWC_1CYCLE (0 << 17) /* 1cycle */
#define NAND_tWC_2CYCLE (1 << 17) /* 2cycle */
#define NAND_tWC_3CYCLE (2 << 17) /* 3cycle */
#define NAND_tWC_4CYCLE (3 << 17) /* 4cycle */
#define NAND_tWC_5CYCLE (4 << 17) /* 5cycle */
#define NAND_tWC_6CYCLE (5 << 17) /* 6cycle */
#define NAND_tWC_7CYCLE (6 << 17) /* 7cycle */
#define NAND_tWC_8CYCLE (7 << 17) /* 8cycle */
#define NAND_tWC_9CYCLE (8 << 17) /* 9cycle */
#define NAND_tWC_10CYCLE (9 << 17) /* 10cycle */
#define NAND_tWC_11CYCLE (10 << 17) /* 11cycle */
#define NAND_tWC_12CYCLE (11 << 17) /* 12cycle */
#define NAND_tWC_13CYCLE (12 << 17) /* 13cycle */
#define NAND_tWC_14CYCLE (13 << 17) /* 14cycle */
#define NAND_tWC_15CYCLE (14 << 17) /* 15cycle */
#define NAND_tWC_16CYCLE (15 << 17) /* 16cycle (default) */

/* tWP: Write Pulse time, duration of write pulse */
#define NAND_tWP_1CYCLE (0 << 14) /* 1cycle */
#define NAND_tWP_2CYCLE (1 << 14) /* 2cycle */
#define NAND_tWP_3CYCLE (2 << 14) /* 3cycle */
#define NAND_tWP_4CYCLE (3 << 14) /* 4cycle */
#define NAND_tWP_5CYCLE (4 << 14) /* 5cycle */
#define NAND_tWP_6CYCLE (5 << 14) /* 6cycle */
#define NAND_tWP_7CYCLE (6 << 14) /* 7cycle */
#define NAND_tWP_8CYCLE (7 << 14) /* 8cycle (default) */

/* tRC: Read Cycle time */
#define NAND_tRC_1CYCLE (0 << 10) /* 1cycle */
#define NAND_tRC_2CYCLE (1 << 10) /* 2cycle */
#define NAND_tRC_3CYCLE (2 << 10) /* 3cycle */
#define NAND_tRC_4CYCLE (3 << 10) /* 4cycle */
#define NAND_tRC_5CYCLE (4 << 10) /* 5cycle */
#define NAND_tRC_6CYCLE (5 << 10) /* 6cycle */
#define NAND_tRC_7CYCLE (6 << 10) /* 7cycle */
#define NAND_tRC_8CYCLE (7 << 10) /* 8cycle */
#define NAND_tRC_9CYCLE (8 << 10) /* 9cycle */
#define NAND_tRC_10CYCLE (9 << 10) /* 10cycle */
#define NAND_tRC_11CYCLE (10 << 10) /* 11cycle */
#define NAND_tRC_12CYCLE (11 << 10) /* 12cycle */
#define NAND_tRC_13CYCLE (12 << 10) /* 13cycle */
#define NAND_tRC_14CYCLE (13 << 10) /* 14cycle */
#define NAND_tRC_15CYCLE (14 << 10) /* 15cycle */
#define NAND_tRC_16CYCLE (15 << 10) /* 16cycle (default) */

/* tRP: Read Pulse time, duration of read pulse */
#define NAND_tRP_1CYCLE (0 << 7) /* 1cycle */
#define NAND_tRP_2CYCLE (1 << 7) /* 2cycle */
#define NAND_tRP_3CYCLE (2 << 7) /* 3cycle */
#define NAND_tRP_4CYCLE (3 << 7) /* 4cycle */
#define NAND_tRP_5CYCLE (4 << 7) /* 5cycle */
#define NAND_tRP_6CYCLE (5 << 7) /* 6cycle */
#define NAND_tRP_7CYCLE (6 << 7) /* 7cycle */
#define NAND_tRP_8CYCLE (7 << 7) /* 8cycle (default) */


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
#define NAND_NPAR_LEN_15B	0x0			/* ECC Parity length 8 bytes	*/
#define NAND_NPAR_LEN_8B	(0x1 << 17)	/* ECC Parity length 15 bytes	*/

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



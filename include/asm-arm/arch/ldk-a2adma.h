/* linux/include/asm/arch-ldk/ldk-a2adma.h
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

#ifndef __LDK_A2ADMAREG_H
#define __LDK_A2ADMAREG_H                     1

#include <mach/platform.h>
// offset of regisgers
#define LDK_VA_A2A_BRIDGE_DMA_BASE IO_ADDRESS(LDK_AHB0_A2A_DMA)
#define LDK_A2A_BRIDGE_DMA_NR_PORTS 0x54
#define A2A_DMA_REG_BASE   (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0000)
#define A2A_DMA_CON        (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0000)
#define A2A_DMA_ISRC       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0004)
#define A2A_DMA_IDST       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0008)
#define A2A_DMA_ICNT       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x000c)
#define A2A_DMA_CSRC       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0010)
#define A2A_DMA_CDST       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0014)
#define A2A_DMA_CCNT       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0018)

#define ISRC_REG_OFFSET 0x0004
#define IDST_REG_OFFSET 0x0008
#define ICNT_REG_OFFSET 0x000c
#define CSRC_REG_OFFSET 0x0010
#define CDST_REG_OFFSET 0x0014
#define CCNT_REG_OFFSET 0x0018

#define A2A_DMA_CON0        (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0000)
#define A2A_DMA_ISRC0       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0004)
#define A2A_DMA_IDST0       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0008)
#define A2A_DMA_ICNT0       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x000c)
#define A2A_DMA_CSRC0       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0010)
#define A2A_DMA_CDST0       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0014)
#define A2A_DMA_CCNT0       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0018)
#define ch_OFFSET          0x1C
#define A2A_DMA_CON1        (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x001c)
#define A2A_DMA_ISRC1       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0020)
#define A2A_DMA_IDST1       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0024)
#define A2A_DMA_ICNT1       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0028)
#define A2A_DMA_CSRC1       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x002c)
#define A2A_DMA_CDST1       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0030)
#define A2A_DMA_CCNT1       (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0034)
#define A2A_DMA_INT_STATUS  (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x0038)
#define A2A_DMA_DMA_STATUS  (LDK_VA_A2A_BRIDGE_DMA_BASE + 0x003c)

// shifter for fields
#define A2A_EMR_S   	    0
#define A2A_DST_S   	    1
#define A2A_CMD_S           3
#define A2A_DDA_S   	    5
#define A2A_DSA_S   	    6
#define A2A_EDS_S           7
#define A2A_TM_S            9
#define A2A_ON_THE_FLY_S    11
#define A2A_INTM_S          12
#define A2A_EN_S            13
#define A2A_AR_S            14

// mask for fields
#define A2A_EMR_M   	    (1<<A2A_EMR_S)
/*-------------------------------------------------*/
#define A2A_EMR_HW_MODE	    0
#define A2A_EMR_SW_MODE	    1
/*-------------------------------------------------*/

#define A2A_DST_M   	    (3<<A2A_DST_S)
/*-------------------------------------------------*/
#define A2A_DST_BYTE   	    0
#define A2A_DST_HALFWORD    1
#define A2A_DST_WORD        2
/*-------------------------------------------------*/

#define A2A_CMD_M           (3<<A2A_CMD_S)
/*-------------------------------------------------*/
#define A2A_CMD_NOP		0
#define A2A_CMD_START		1
#define A2A_CMD_PAUSE		2
#define A2A_CMD_CANCEL		3
/*-------------------------------------------------*/

#define A2A_DDA_M   	    (1<<A2A_DDA_S)
/*-------------------------------------------------*/
#define A2A_DDA_INC		0
#define A2A_DDA_FIXED		1
/*-------------------------------------------------*/

#define A2A_DSA_M   	    (1<<A2A_DSA_S)
/*-------------------------------------------------*/
#define A2A_DSA_INC		0
#define A2A_DSA_FIXED		1
/*-------------------------------------------------*/

#define A2A_EDS_M           (3<<A2A_EDS_S)
/*-------------------------------------------------*/
#define A2A_EDS_SOURCE0		0
#define A2A_EDS_SOURCE1		1
#define A2A_EDS_SOURCE2		2
#define A2A_EDS_SOURCE3		3
/*-------------------------------------------------*/

#define A2A_TM_M            (3<<A2A_TM_S)
/*-------------------------------------------------*/
#define A2A_TM_SINGLE		0
#define A2A_TM_INCR4		1
#define A2A_TM_INCR8		2
#define A2A_TM_INCR16		3
/*-------------------------------------------------*/

#define A2A_ON_THE_FLY_M    (1<<A2A_ON_THE_FLY_S)

#define A2A_INTM_M          (1<<A2A_INTM_S)
/*-------------------------------------------------*/
#define A2A_INTM_OFF        0
#define A2A_INTM_ON         1
/*-------------------------------------------------*/
#define A2A_EN_M            (1<<A2A_EN_S)
#define A2A_AR_M            (1<<A2A_AR_S)

// DMA status check macro
#define A2A_DMA0_STAT_RDY()          (((ioread32(A2A_DMA_DMA_STATUS)>>0) & 0x01)==0)
#define A2A_DMA1_STAT_RDY()          (((ioread32(A2A_DMA_DMA_STATUS))>>1 & 0x01)==0)
#define A2A_DMA_CH_INT(ch)          (((ioread32(A2A_DMA_INT_STATUS)>>ch) & 0x01)==1)
#define A2A_GET_INT_STATUS()          (ioread32(A2A_DMA_INT_STATUS))

// DMA enable / disable control macro
#define A2A_DMA_SW_ENABLE(ch)		(iowrite32(((ioread32(CON_ch(ch)) & ~A2A_CMD_M)|1<<A2A_CMD_S), CON_ch(ch)))
#define A2A_DMA_SW_DISABLE(ch)		(iowrite32((ioread32(CON_ch(ch)) & (~A2A_CMD_M)),   CON_ch(ch)))

#define CON_ch(ch)			(LDK_VA_A2A_BRIDGE_DMA_BASE+ch*ch_OFFSET)
#define A2A_SET_EMR(ch,val)          (iowrite32(((ioread32(CON_ch(ch))) & (~A2A_EMR_M))  |(A2A_DMA_EMR(val)) ,CON_ch(ch)))
#define A2A_SET_DST(ch,val)         (iowrite32(((ioread32(CON_ch(ch))) & (~A2A_DST_M)) |(A2A_DMA_DST(val)),CON_ch(ch)))
#define A2A_SET_DDA(ch,val)         (iowrite32(((ioread32(CON_ch(ch))) & (~A2A_DDA_M)) |(A2A_DMA_DDA(val)),CON_ch(ch)))
#define A2A_SET_DSA(ch,val)         (iowrite32(((ioread32(CON_ch(ch))) & (~A2A_DSA_M)) |(A2A_DMA_DSA(val)),CON_ch(ch)))
#define A2A_SET_SW_CMD(ch,val)         (iowrite32(((ioread32(CON_ch(ch))) & (~A2A_CMD_M)) |(A2A_DMA_CMD(val)),CON_ch(ch)))
#define A2A_SET_EDS(ch,val)         (iowrite32(((ioread32(CON_ch(ch))) & (~A2A_EDS_M)) |(A2A_DMA_EDS(val)),CON_ch(ch)))
#define A2A_SET_TM(ch,val)          (iowrite32(((ioread32(CON_ch(ch))) & (~A2A_TM_M))  |(A2A_DMA_TM(val)) ,CON_ch(ch)))
#define A2A_DMA_FLY_ENABLE(ch)		(iowrite32((ioread32(CON_ch(ch)) |  A2A_ON_THE_FLY_M), CON_ch(ch)))
#define A2A_DMA_FLY_DISABLE(ch)	(iowrite32((ioread32(CON_ch(ch)) &  ~A2A_ON_THE_FLY_M), CON_ch(ch)))
#define A2A_SET_INTM(ch,val)          (iowrite32(((ioread32(CON_ch(ch))) & (~A2A_INTM_M))  |(A2A_DMA_INTM(val)) ,CON_ch(ch)))
#define A2A_DMA_HW_ENABLE(ch)		(iowrite32((ioread32(CON_ch(ch)) | A2A_EN_M), CON_ch(ch)))
#define A2A_DMA_HW_DISABLE(ch)		(iowrite32((ioread32(CON_ch(ch)) &  ~A2A_EN_M), CON_ch(ch)))
#define A2A_DMA_AR_ENABLE(ch)		(iowrite32((ioread32(CON_ch(ch)) | A2A_AR_M), CON_ch(ch)))
#define A2A_DMA_AR_DISABLE(ch)		(iowrite32((ioread32(CON_ch(ch)) &  ~A2A_AR_M), CON_ch(ch)))

#define ISRC_ch(ch)  (LDK_VA_A2A_BRIDGE_DMA_BASE+ch*ch_OFFSET+ISRC_REG_OFFSET)
#define IDST_ch(ch)  (LDK_VA_A2A_BRIDGE_DMA_BASE+ch*ch_OFFSET+IDST_REG_OFFSET)
#define ICNT_ch(ch)  (LDK_VA_A2A_BRIDGE_DMA_BASE+ch*ch_OFFSET+ICNT_REG_OFFSET)
#define CSRC_ch(ch)  (LDK_VA_A2A_BRIDGE_DMA_BASE+ch*ch_OFFSET+CSRC_REG_OFFSET)
#define CDST_ch(ch)  (LDK_VA_A2A_BRIDGE_DMA_BASE+ch*ch_OFFSET+CDST_REG_OFFSET)
#define CCNT_ch(ch)  (LDK_VA_A2A_BRIDGE_DMA_BASE+ch*ch_OFFSET+CCNT_REG_OFFSET)
#define A2A_SET_ISRC(ch,val)         (iowrite32(val,ISRC_ch(ch)))
#define A2A_GET_ISRC(ch)             (ioread32(ISRC_ch(ch)))
#define A2A_SET_IDST(ch,val)         (iowrite32(val,IDST_ch(ch)))
#define A2A_SET_ICNT(ch,val)         (iowrite32(val,ICNT_ch(ch)))
#define A2A_GET_CSRC(ch)             (ioread32(CSRC_ch(ch)))
#define A2A_SET_CDST(ch,val)         (iowrite32(val,CDST_ch(ch)))
#define A2A_GET_CCNT(ch)             (ioread32(CCNT_ch(ch)))

// oh my...
#define ISRC_ch0  (LDK_VA_A2A_BRIDGE_DMA_BASE+ISRC_REG_OFFSET)
#define IDST_ch0  (LDK_VA_A2A_BRIDGE_DMA_BASE+0*ch_OFFSET+IDST_REG_OFFSET)
#define ICNT_ch0  (LDK_VA_A2A_BRIDGE_DMA_BASE+0*ch_OFFSET+ICNT_REG_OFFSET)
#define CSRC_ch0  (LDK_VA_A2A_BRIDGE_DMA_BASE+0*ch_OFFSET+CSRC_REG_OFFSET)
#define CDST_ch0  (LDK_VA_A2A_BRIDGE_DMA_BASE+0*ch_OFFSET+CDST_REG_OFFSET)
#define CCNT_ch0  (LDK_VA_A2A_BRIDGE_DMA_BASE+0*ch_OFFSET+CCNT_REG_OFFSET)
#define A2A_GET_ISRC0             (ioread32(ISRC_ch0))
#define A2A_SET_IDST0(val)         (iowrite32(val,IDST_ch0))
#define A2A_SET_ICNT0(val)         (iowrite32(val,ICNT_ch0))
#define A2A_GET_CSRC0             (ioread32(CSRC_ch0))
#define A2A_SET_CDST0(val)         (iowrite32(val,CDST_ch0))
#define A2A_GET_CCNT0             (ioread32(CCNT_ch0))

#define A2A_DMA0_ENABLE()            iowrite32((ioread32(A2A_DMA_CON0) |  A2A_EN_M), A2A_DMA_CON0)
#define A2A_DMA0_DISABLE()           iowrite32((ioread32(A2A_DMA_CON0) & ~A2A_EN_M), A2A_DMA_CON0)
#define A2A_DMA0_FLY_ENABLE()        iowrite32((ioread32(A2A_DMA_CON0) |  A2A_ON_THE_FLY_M), A2A_DMA_CON0)
#define A2A_DMA0_FLY_DISABLE()       iowrite32((ioread32(A2A_DMA_CON0) |  A2A_ON_THE_FLY_M), A2A_DMA_CON0)
#define A2A_DMA1_ENABLE()            iowrite32((ioread32(A2A_DMA_CON1) |  A2A_EN_M), A2A_DMA_CON1)
#define A2A_DMA1_DISABLE()           iowrite32((ioread32(A2A_DMA_CON1) & ~A2A_EN_M), A2A_DMA_CON1)
#define A2A_DMA1_FLY_ENABLE()        iowrite32((ioread32(A2A_DMA_CON1) |  A2A_ON_THE_FLY_M), A2A_DMA_CON1)
#define A2A_DMA1_FLY_DISABLE()       iowrite32((ioread32(A2A_DMA_CON1) |  A2A_ON_THE_FLY_M), A2A_DMA_CON1)

// DMA software mode control macro
#define A2A_DMA0_SOFTWARE_ENABLE()   iowrite32((ioread32(A2A_DMA_CON0) & (~A2A_CMD_M|1<<A2A_CMD_S)), A2A_DMA_CON0)
#define A2A_DMA0_SOFTWARE_DISABLE()  iowrite32((ioread32(A2A_DMA_CON0) & ~A2A_CMD_M),   A2A_DMA_CON0)
#define A2A_DMA1_SOFTWARE_ENABLE()   iowrite32((ioread32(A2A_DMA_CON1) & (~A2A_CMD_M|1<<A2A_CMD_S)), A2A_DMA_CON1)
#define A2A_DMA1_SOFTWARE_DISABLE()  iowrite32((ioread32(A2A_DMA_CON1) & ~A2A_CMD_M),   A2A_DMA_CON1)

// DMA control value macro
#define A2A_DMA_EMR(val)    ((val & 0x01) << A2A_EMR_S  )
#define A2A_DMA_DST(val)    ((val & 0x03) << A2A_DST_S  )
#define A2A_DMA_CMD(val)    ((val & 0x03) << A2A_CMD_S  )
#define A2A_DMA_DDA(val)    ((val & 0x01) << A2A_DDA_S  )
#define A2A_DMA_DSA(val)    ((val & 0x01) << A2A_DSA_S  )
#define A2A_DMA_EDS(val)    ((val & 0x03) << A2A_EDS_S  )
#define A2A_DMA_TM(val)     ((val & 0x03) << A2A_TM_S   )
#define A2A_DMA_INTM(val)   ((val & 0x01) << A2A_INTM_S )
#define A2A_DMA_EN(val)     ((val & 0x01) << A2A_EN_S   )
#define A2A_DMA_AR(val)     ((val & 0x01) << A2A_AR_S   )

// DMA channel define
#define A2A_DMA_CH0                  0
#define A2A_DMA_CH1                  1
// external DMA request define
#define A2A_DMA_EN_XDREQ             0
#define A2A_DMA_DIS_XDREQ            1
// data type define
#define A2A_DMA_SIZE_BYTE            0
#define A2A_DMA_SIZE_HWORD           1
#define A2A_DMA_SIZE_WORD            2
// addressing mode define
#define A2A_DMA_INCR                 0
#define A2A_DMA_FIX                  1

// data transfer direction
#define A2A_DMA_M2IO                 0
#define A2A_DMA_IO2M                 1
// external DMA reauest channel
#define A2A_DMA_XDREQ0               0
#define A2A_DMA_XDREQ1               1
#define A2A_DMA_XDREQ2               2
#define A2A_DMA_XDREQ3               3
#define A2A_DMA_XDREQ4               4
#define A2A_DMA_XDREQ5               5
#define A2A_DMA_XDREQ6               6
#define A2A_DMA_XDREQ7               7
// DMA protocol type
#define A2A_DMA_XDREQ_HSHAKE         0
#define A2A_DMA_XDREQ_BLOCK          1
#define A2A_DMA_XDREQ_DEMAND         2
// bus data transfer type
#define A2A_DMA_MODE_SINGLE          0
#define A2A_DMA_MODE_4BEAT_WRAP      2
#define A2A_DMA_MODE_4BEAT_INCR      3
#define A2A_DMA_MODE_8BEAT_WRAP      4
#define A2A_DMA_MODE_8BEAT_INCR      5
#define A2A_DMA_MODE_16BEAT_WRAP     6
#define A2A_DMA_MODE_16BEAT_INCR     7
// internal DREQ source define
#define A2A_DMA_XDREQ_SRC_UART0      0
#define A2A_DMA_XDREQ_SRC_UART1      1
#define A2A_DMA_XDREQ_SRC_OTHER      0  // if no external source, set the same as UART0

// AHB protocol type define               //poll or interrupt
#define A2A_DMA_INTM_SW_MODE         0      //Peter:??? is the same as EMR??? Only Block? INTM
#define A2A_DMA_INTM_INT_MODE        1

// flag for auto reload enable / diable
#define A2A_DMA_DIS_AUTO             0
#define A2A_DMA_EN_AUTO              1
// flag for DMA enable / disable
#define A2A_DMA_DIS_DMA              0
#define A2A_DMA_EN_DMA               1
// DMA command mode define     Peter:EMR:DMA MODE
#define A2A_DMA_NO_CMD               0
#define A2A_DMA_SW_DMA               1
// flag for using interrupt to check complete
#define A2A_DMA_DONT_USE_INT         0
#define A2A_DMA_USE_INT              1
// special block mode define( block mode with wait state)
#define A2A_DMA_NORMAL_BLOCK_MODE    0
#define A2A_DMA_BLOCK_WAIT_MODE      1



#endif


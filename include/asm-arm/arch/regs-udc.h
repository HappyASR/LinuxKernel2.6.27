#ifndef	_SOCLE_REG_UDC_H_
#define	_SOCLE_REG_UDC_H_


#define FULL_SPEED_CTRL_PACKET_SIZE	8
#define FULL_SPEED_BULK_PACKET_SIZE	64
#define FULL_SPEED_INTR_PACKET_SIZE	8

#define HI_SPEED_CTRL_PACKET_SIZE		64
#define HI_SPEED_BULK_PACKET_SIZE		512
#define HI_SPEED_INTR_PACKET_SIZE		32


/* UDC Registers Offset Define */

#define	SOCLE_UDC_PHY_TEST_EN			0x0000		/* PHY. Enable Register (option) */
#define	SOCLE_UDC_PHY_TEST				0x0004		/* USB  PHY. Test Register (option) */
#define	SOCLE_UDC_DEV_CTRL				0x0008		/* Device Control Register */
#define	SOCLE_UDC_DEV_INFO				0x0010		/* Device Info. Register */
#define	SOCLE_UDC_EN_INT					0x0014		/* UDC Interrupt Enable Register */
#define	SOCLE_UDC_INTFLG					0x0018		/* UDC Interrupt Flag Register */
#define	SOCLE_UDC_INTCON					0x001c		/* UDC Interrupt Control Register */
#define	SOCLE_UDC_SETUP1					0x0020		/* UDC Setup Status Register 1 (Read-Only) */
#define	SOCLE_UDC_SETUP2					0x0024		/* UDC Setup Status Register 2 (Read-Only) */
#define	SOCLE_UDC_AHBCON					0x0028		/* UDC AHB Control Register */
/* ----------------------------------------- */
/* End-Point 0, Ctrl_OUT */
#define	SOCLE_UDC_RX0STAT					0x0030		/* UDC ENP 0 Ctrl_OUT Receive Status Register */
#define	SOCLE_UDC_RX0CON					0x0034		/* UDC ENP 0 Ctrl_OUT Receive Control Register */
#define	SOCLE_UDC_DMA0CTLO				0x0038		/* UDC ENP 0 Ctrl_OUT DMA Control Register */
#define	SOCLE_UDC_DMA0LM_OADDR			0x003c		/* UDC ENP 0 Ctrl_OUT DMA Local Memory Address Register */
/* End-Point 0, Ctrl_IN */
#define	SOCLE_UDC_TX0STAT					0x0040		/* UDC ENP 0 Ctrl_IN Transmit Status Register */
#define	SOCLE_UDC_TX0CON					0x0044		/* UDC ENP 0 Ctrl_IN Transmit Control Register */
#define	SOCLE_UDC_TX0BUF					0x0048		/* UDC ENP 0 Ctrl_IN Buffer Status Register */
#define	SOCLE_UDC_DMA0CTLI				0x004c		/* UDC ENP 0 Ctrl_IN DMA Control Register */
#define	SOCLE_UDC_DMA0LM_IADDR			0x0050		/* UDC ENP 0 Ctrl_IN DMA Local Memory Address Register */
/* ----------------------------------------- */
/* Bullk OUT Enp */
#define	SOCLE_UDC_BO_BASE(X)				0x54+((X-1)/3)*0x38			// X , 1 : 0x54 , 4: 0x8c, 7: 0xc4
/* ----------------------------------------- */
/* Bullk IN Enp */
#define	SOCLE_UDC_BI_BASE(X)				0x64+((X-1)/3)*0x38			// X , 0 : 0x54 , 1:
/* ----------------------------------------- */
/* Interrupt In Enp */
#define	SOCLE_UDC_II_BASE(X)				0x78+((X-1)/3)*0x38
/* ----------------------------------------- */

/* Bulk_OUT, Receive End Point */
#define	UDC_RXSTAT						0x00				/* UDC Bulk_OUT Receive Status Register */
#define	UDC_RXCON						0x04				/* UDC Bulk_OUT Receive Control Register */
#define	UDC_DMACTRLO					0x08				/* UDC Bulk_OUT DMA Control Register */
#define	UDC_DMALM_OADDR				0x0c				/* UDC Bulk_OUT DMA Local Memory Address Register */

/* Bulk_IN, Intr_IN,  Transfer End Point */
#define	UDC_TXSTAT						0x00				/* Bulk_IN/Intr_IN Transmit Status Register */
#define	UDC_TXCON						0x04				/* Bulk_IN/Intr_IN Transmit Control Register */
#define	UDC_TXBUF						0x08				/* Bulk_IN/Intr_IN Buffer Status Register */
#define	UDC_DMACTRLI					0x0c				/* Bulk_IN/Intr_IN DMA Control Register */
#define	UDC_DMALM_IADDR				0x10				/* Bulk_IN/Intr_IN DMA Local Memory Address Register */

#define 	UDC_DMA_START						0x1

/* SOCLE_UDC_PHY_TEST_EN 0x00 */
#define	PHY_TEST_CLK_EN					(1<<0)	/* For Socle's PHY Test	clock enable */
#define	PHY_TEST_CLK						(1<<1)	/* Enable Socle's PHY analog_test pin  */

/* SOCLE_UDC_PHY_TEST 0x04 */
/* For Socle's PHY Test	mode use */
#define	PHY_TEST_ADDR			0xf
#define	PHY_TEST_DATA_IN		(0xff<4)

/* SOCLE_UDC_DEV_CTL 0x08 */
#define	DEV_CTL_TEST_MODE			(1<<9)
#define	DEV_CTL_CSR_DONE				(1<<8)
#define	DEV_CTL_SOFT_POR				(1<<7)
#define	DEV_CTL_PHY16BIT_MASK		(1<<6)
#define	DEV_CTL_PHY16BIT				6
#define	PHY_8BIT						(0x0)
#define	PHY_16BIT						(0x2)

#define	DEV_CTL_RESUME				(1<<5)
#define	DEV_CTL_SOFT_CN				(1<<4)
#define	DEV_CTL_SELF_PWR				(1<<3)
#define	DEV_CTL_RMTWKP				(1<<2)
#define	DEV_CTL_SPEED_MASK			0x3
#define	FULL_SPEED						(0x3)	// Full speed (USB 1.1) define
#define	HIGH_SPEED						(0x0)

/* SOCLE_UDC_DEV_INFO 0x10 */
#define 	DEV_INFO_EP_NUM_MASK		(0x3<<23)	// Num of EP
#define 	DEV_INFO_EP_NUM				(23)	// Num of EP
#define 	EP_NUM_10						(1<<23)
#define 	EP_NUM_4						(1<<24)
#define 	EP_NUM_16						0

#define	DEV_INFO_SPEED_MASK			(0x3<<21)	// Enum speed bits mask
#define	DEV_INFO_SPEED				(21)			// Enum speed bits
#define	DEV_INFO_VBUS_STS				(0x1<<20)	// VBUS status bit mask
#define	DEV_INFO_ALT_NUM				(0xf<<16)	// Alternate setting number bits mask
#define	DEV_INFO_IF_NUM				(0xf<<12)	// Interface number bits mask
#define	DEV_INFO_CFG_NUM				(0xf<<8)	// Configuration number bits mask
#define	DEV_INFO_DEV_EN				(1<<7)	// Device enable bit mask (set configuration ok)
#define	DEV_INFO_DEV_ADDR			(0x7f)	// Device address's bits mask

/* UDC_ENINT 0x14*/
/* UDC_INTFLG 0x18*/
#define	INT_FLAG_TEST_PKT				(1<<26)		/* Test Packet Interrupt */
#define	INT_FLAG_TEST_K				(1<<25)		/* Test K Packet Interrupt */
#define	INT_FLAG_TEST_J				(1<<24)		/* Test J Packet Interrupt */
#define	INT_FLAG_TEST_SE0_NAK		(1<<23)		/* SE0 NAK Packet Interrupt */
#define	INT_FLAG_EP15_INTR			(1<<22)		/* ENP 15 Intr_IN  Transmit Interrupt */
#define	INT_FLAG_EP14_INTR			(1<<21)		/* ENP 14 Bulk_IN  Transmit Interrupt */
#define	INT_FLAG_EP13_INTR			(1<<20)		/* ENP 13 Bulk_OUT Receive Interrupt  */
#define	INT_FLAG_EP12_INTR			(1<<19)		/* ENP 12 Intr_IN  Transmit Interrupt */
#define	INT_FLAG_EP11_INTR			(1<<18)		/* ENP 11 Bulk_IN  Transmit Interrupt */
#define	INT_FLAG_EP10_INTR			(1<<17)		/* ENP 10 Bulk_OUT Receive Interrupt  */
#define	INT_FLAG_EP9_INTR				(1<<16)		/* ENP 9  Intr_IN  Transmit Interrupt */
#define	INT_FLAG_EP8_INTR				(1<<15)		/* ENP 8  Bulk_IN  Transmit Interrupt */
#define	INT_FLAG_EP7_INTR				(1<<14)		/* ENP 7  Bulk_OUT Receive Interrupt  */
#define	INT_FLAG_EP6_INTR				(1<<13)		/* ENP 6  Intr_IN  Transmit Interrupt */
#define	INT_FLAG_EP5_INTR				(1<<12)		/* ENP 5  Bulk_IN  Transmit Interrupt */
#define	INT_FLAG_EP4_INTR				(1<<11)		/* ENP 4  Bulk_OUT Receive Interrupt  */
#define	INT_FLAG_EP3_INTR				(1<<10)		/* ENP 3  Intr_IN  Transmit Interrupt */
#define	INT_FLAG_EP2_INTR				(1<<9)		/* ENP 2  Bulk_IN  Transmit Interrupt */
#define	INT_FLAG_EP1_INTR				(1<<8)		/* ENP 1  Bulk_OUT Receive Interrupt  */
#define	INT_FLAG_VBUS_INTR			(1<<7)		/* USB VBUS    Interrupt */
#define	INT_FLAG_SUSP_INTR			(1<<6)		/* USB Suspend Interrupt */
#define	INT_FLAG_RSUME_INTR			(1<<5)		/* USB Resume  Interrupt */
#define	INT_FLAG_USBRST_INTR			(1<<4)		/* USB Reset   Interrupt */
#define	INT_FLAG_OUT0_INTR			(1<<3)		/* ENP 0  Ctrl_OUT Receive interrupt  */
#define	INT_FLAG_IN0_INTR				(1<<2)		/* ENP 0  Ctrl_IN  Transmit interrupt */
#define	INT_FLAG_SETUP_INTR			(1<<1)		/* Receive SETUP package */
#define	INT_FLAG_SOF_INTR				(1<<0)		/* Receive Start-Of-Frame Interrupt */

#define	SOCLE_EN_BOUTALL_INTR		(INT_FLAG_EP1_INTR | INT_FLAG_EP4_INTR | INT_FLAG_EP7_INTR | INT_FLAG_EP10_INTR | INT_FLAG_EP13_INTR)
#define	SOCLE_EN_BINALL_INTR			(INT_FLAG_EP2_INTR | INT_FLAG_EP5_INTR | INT_FLAG_EP8_INTR | INT_FLAG_EP11_INTR | INT_FLAG_EP14_INTR)
#define	SOCLE_EN_IINALL_INTR			(INT_FLAG_EP3_INTR | INT_FLAG_EP6_INTR | INT_FLAG_EP9_INTR | INT_FLAG_EP12_INTR | INT_FLAG_EP15_INTR)


/* UDC_INTCON 0x1c */
#define	INTCON_INTEN					(1<<0)		/* UDC Interrupt Enable bit */
#define	INTCON_INTEDGE_TRIG			(1<<1)		/* UDC Interrupt Edge Trig bit */
#define	INTCON_INTHIGH_ACT			(1<<2)		/* UDC Interrupt Active High Trig bit */

/* UDC_SETUP1 */
#define 	SETUP1_WVALUE					(0xffff<16)
#define 	SETUP1_BREQUEST				(0xff<8)

/* UDC_SETUP2 */



/* Bulk-OUT End Point Define */

/* RXSTAT (Ctrl/Bulk Out) 0x54,0x8c,0xc4,0xfc,0x134*/
#define	RXSTAT_OVF			(1<<25)
#define	RXSTAT_FULL		(1<<24)
#define	RXSTAT_CF_INT		(1<<19)   //Bulk Out only
#define	RXSTAT_ACK			(1<<18)
#define	RXSTAT_ERR			(1<<17)
#define	RXSTAT_VOID		(1<<16)
#define	RXSTAT_LEN			(0x7ff)

/* RXCON (Ctrl/Bulk Out) */
/* 0x34, 0x58, 0x90,0xC8, 0x100, 0x138 RX1,4,7,10,13*/
#define	RXCON_STALL_AUTO_CLR			(1<<13)		/* Bulk_OUT STALL auto clear */
#define	RXCON_CF_EN					(1<<12)		/* Bulk_OUT CLR Feature EN */
#define	RXCON_ENPNUM					(0xf<<8)	/* Bulk_OUT ENP Number Mask */
#define	RXCON_ACK_EN					(1<<7)
#define	RXCON_ERR_EN					(1<<6)
#define	RXCON_VOID_EN					(1<<5)
#define	RXCON_EP_EN					(1<<4)
#define	RXCON_NAK						(1<<3)
#define	RXCON_STALL					(1<<2)
#define	RXCON_CLR						(1<<1) /* Flush FIFO */
#define	RXCON_FFRC						(1<<0)

/* DMA0CTRLO (Ctrl Out) 0x38*/
#define	RX0DMA_STA			(1<<0)

/* TXSTAT (Ctrl/Bulk Out) 0x40, */
#define	TXSTAT_OVF			(1<<25)
#define	TXSTAT_FULL		(1<<24)
#define	TXSTAT_TC_CF_INT	(1<<20) //Bulk out only
#define	TXSTAT_DMA_DN		(1<<19) //Bulk out only
#define	TXSTAT_ACK			(1<<18)
#define	TXSTAT_ERR			(1<<17)
#define	TXSTAT_VOID		(1<<16)
#define	TXSTAT_LEN			(0x7ff)

/* TXCON (Ctrl In , Bulk IN)*/
#define	TXCON_STALL_AUTO_CLR			(1<<13) //bulk only 
#define	TXCON_CF_INT_EN				(1<<12) //bulk only 
#define	TXCON_EP_NUM_MASK			(0xff<<8) //bulk only 
#define	TXCON_DMA_DN_EN				(1<<7)  //bulk only 
#define	TXCON_ACK_EN					(1<<6)
#define	TXCON_ERR_EN					(1<<5)
#define	TXCON_VOID_EN					(1<<4)
#define	TXCON_EP_EN					(1<<3) //bulk only 
#define	TXCON_NAK						(1<<2)
#define	TXCON_STALL					(1<<1)
#define	TXCON_CLR						(1<<0)

/* TXBUF (Ctrl In, Bulk IN )*/
#define	TXBUF_DATA_SET1_FULL		(1<<3)	/* Bulk_IN Data Set 0 Status bit, 1 -> buffer full */
#define	TXBUF_DATA_SET0_FULL		(1<<2)	/* Bulk_IN Data Set 1 Status bit, 1 -> buffer full */
#define	TXBUF_UNRUN		(1<<1)		/* Bulk_IN Transmit Underflow */
#define	TXBUF_FULL			(1<<0)		/* Bulk_IN Data Buffer Status, 1 -> buffer full */

/* DMA0INCTL (Ctrl In )*/
#define	DMA0INCTRL_STA	(1<<0)


#define CTL_MAX_PKT            0x40
#define BULK_PKT_LSB           0x00
#define BULK_PKT_MSB           0x02
#define INTRIN_PKT_LSB         0x20
#define INTRIN_PKT_MSB         0x00

#define FS_BULK_PKT_LSB        0x40
#define FS_BULK_PKT_MSB        0x00

/* Endpoint number Config SET 1*/
#define BLKOUT_ENDP_NUM_SET1	    0x01
#define BLKIN_ENDP_NUM_SET1		    0x82
#define INTRIN_ENDP_NUM_SET1	    0x83

/* Endpoint number Config SET 2*/
#define BLKOUT_ENDP_NUM_SET2	    0x04
#define BLKIN_ENDP_NUM_SET2		    0x85
#define INTRIN_ENDP_NUM_SET2	    0x86

/* Endpoint number Config SET 3*/
#define BLKOUT_ENDP_NUM_SET3	    0x07
#define BLKIN_ENDP_NUM_SET3		    0x88
#define INTRIN_ENDP_NUM_SET3	    0x89

/* Endpoint number Config SET 4*/
#define BLKOUT_ENDP_NUM_SET4	    0x0a
#define BLKIN_ENDP_NUM_SET4		    0x8b
#define INTRIN_ENDP_NUM_SET4	    0x8c

/* Endpoint number Config SET 5*/
#define BLKOUT_ENDP_NUM_SET5	    0x0d
#define BLKIN_ENDP_NUM_SET5		    0x8e
#define INTRIN_ENDP_NUM_SET5	    0x8f
/* end of ivan 040803 */

#endif /* _SOCLE_REG_UDC_H_ */

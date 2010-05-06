/*
 *  linux/include/asm-arm/arch-ldk/irqs.h
 *
 *  Copyright (C) 2006 Socle-tech Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <mach/platform.h>

#if defined(CONFIG_MMC_SOCLE_SDHC) || defined(CONFIG_MMC_SOCLE_SDHC_SLAVE )
#warning "IRQ for SDHC need to be re-defined!!!"
#define IRQ_SDCH0			27
#define	IRQ_SDCH1			6
#define IRQ_SDCS			26
#endif

#ifdef CONFIG_ARCH_LDK3V21

#define NR_IRQS				32

#define IRQ_PCI1_A  			(IRQ_EXT3)
#define IRQ_PCI1_B  			(IRQ_EXT1)
#define IRQ_PCI1_C  			(IRQ_EXT2)
#define IRQ_PCI1_D  			(IRQ_EXT4)

#define IRQ_PCI2_A  			(IRQ_EXT1)
#define IRQ_PCI2_B  			(IRQ_EXT2)
#define IRQ_PCI2_C  			(IRQ_EXT4)
#define IRQ_PCI2_D  			(IRQ_EXT3)

#define IRQ_IDE				IRQ_EXT0
#define IRQ_UHC0				IRQ_EXT0
#define IRQ_I2S				IRQ_EXT0

/* 
 *  IRQ interrupts definitions are the same the INT definitions
 *  held within platform.h
 */

#define IRQ_SPI0				0
#define IRQ_UART0			1
#define IRQ_UART1			2
#define IRQ_TIMER0			3
#define IRQ_TIMER1			4
#define IRQ_TIMER2			5
#define IRQ_RTC				6
#define IRQ_MAC0			7
#define IRQ_MAC1			8
#define IRQ_UDC				9
#define IRQ_EXT0				10
#define IRQ_EXT1				11
#define IRQ_EXT2				12
#define IRQ_EXT3				13
#define IRQ_EXT4				14
#define IRQ_EXT5				15
#define IRQ_EXT6				16
#define IRQ_EXT7				17
#define IRQ_EXT8				18
#define IRQ_EXT9				19
#define IRQ_EXT10			20
#define IRQ_EXT11			21
#define IRQ_I2C1				22
#define IRQ_A2A_DMA		23
#define IRQ_MPEG4_DEC		24
#define IRQ_MPEG4_ENC		25
#define IRQ_VIP				26
#define IRQ_VOP				27
#define IRQ_I2C				28
#define IRQ_A2P_DMA		29
#define IRQ_SDMMC				30
#define IRQ_EXT12			31
#define IRQ_NAND			11

#endif

#ifdef CONFIG_ARCH_LDK5

#define NR_IRQS				32

#define IRQ_PCI1_A  			(IRQ_EXT2)
#define IRQ_PCI1_B  			(IRQ_EXT0)
#define IRQ_PCI1_C  			(IRQ_EXT1)
#define IRQ_PCI1_D  			(IRQ_EXT3)

#define IRQ_PCI2_A  			(IRQ_EXT0)
#define IRQ_PCI2_B  			(IRQ_EXT1)
#define IRQ_PCI2_C  			(IRQ_EXT3)
#define IRQ_PCI2_D  			(IRQ_EXT2)

/* 
 *  INTC0
 */
#define IRQ_UART0			0
#define IRQ_UART1			1
#define IRQ_TIMER0_0		2
#define IRQ_TIMER0_1		3
#define IRQ_TIMER0_2		4
#define IRQ_GPIO0			5
#define IRQ_INTC1			6	//INTC1
#define IRQ_AHB0_MAIL		7
#define IRQ_RTC				8
#define IRQ_SCU				9
#define IRQ_SDMMC				10
#define IRQ_SPI0				11
#define IRQ_HDMA			12
#define IRQ_A2ADMA			13
#define IRQ_MAC0			14
#define IRQ_MAC1			15
#define IRQ_UDC				16
#define IRQ_UHC0				17
#define IRQ_IDE				18
#define IRQ_MP4ENC			19
#define IRQ_MP4DEC			20
#define IRQ_VIP				21
#define IRQ_VOP				22
#define IRQ_A2PCI			23
#define IRQ_NAND			24
#define IRQ_EXT0				25
#define IRQ_EXT1				26
#define IRQ_EXT2				27
#define IRQ_EXT3				28
#define IRQ_EXT4				29
#define IRQ_EXT5				30
#define IRQ_EXT6				31

/* 
 *  INTC1
 */
 
#define IRQ_UART2			0
#define IRQ_I2C				1
#define IRQ_TIMER1_0		2
#define IRQ_TIMER1_1		3
#define IRQ_TIMER1_2		4
#define IRQ_GPIO1			5
#define IRQ_I2S				6	
#define IRQ_AHB1_MAIL		7
#define IRQ_RTC				8
#define IRQ_SCU				9
#define IRQ_SDMMC				10
#define IRQ_SPI0				11
#define IRQ_HDMA			12
#define IRQ_A2ADMA			13
#define IRQ_MAC0			14
#define IRQ_MAC1			15
#define IRQ_UDC				16
#define IRQ_UHC0			17
#define IRQ_IDE				18
#define IRQ_MP4ENC			19
#define IRQ_MP4DEC			20
#define IRQ_VIP				21
#define IRQ_VOP				22
#define IRQ_A2PCI			23
#define IRQ_NAND			24
#define IRQ_EXT0				25
#define IRQ_EXT1				26
#define IRQ_EXT2				27
#define IRQ_EXT3				28
#define IRQ_EXT4				29
#define IRQ_EXT5				30
#define IRQ_EXT6				31

#endif

#if (defined (CONFIG_ARCH_CDK) || defined (CONFIG_ARCH_PDK_PC9002) || defined(CONFIG_ARCH_SCDK))

#define IRQ_PCI1_A  			(IRQ_PCI_A)
#define IRQ_PCI1_B  			(IRQ_PCI_B)
#define IRQ_PCI1_C  			(IRQ_PCI_C)
#define IRQ_PCI1_D  			(IRQ_PCI_D)

#define IRQ_PCI2_A  			(IRQ_PCI_C)
#define IRQ_PCI2_B  			(IRQ_PCI_D)
#define IRQ_PCI2_C  			(IRQ_PCI_A)
#define IRQ_PCI2_D  			(IRQ_PCI_B)

#define NR_IRQS				32

#define IRQ_UART0			0
#define IRQ_UART1			1
#define IRQ_UART2			2
#define IRQ_SPI0				3
#define IRQ_SPI1				4
#define IRQ_I2C				5
#define IRQ_I2S				6	
#define IRQ_SDMMC				7
#define IRQ_TIMER0			8
#define IRQ_TIMER1			9
#define IRQ_TIMER2			10
#define IRQ_PWM0			11
#define IRQ_PWM1			12
#define IRQ_RTC				13
#define IRQ_ADC				14
#define IRQ_MAC0			15
#define IRQ_PCI_A			16
#define IRQ_PCI_B			17
#define IRQ_PCI_C			18
#define IRQ_PCI_D			19
#ifdef  CONFIG_SOCLE_EHCI_OTG
#define IRQ_UHC0			IRQ_OTG
#else
#define IRQ_UHC0			20
#endif
#define IRQ_UDC				21
#define IRQ_HDMA0			22
#define IRQ_HDMA1			23
#define IRQ_IDE				24
#define IRQ_CLCD				25
#define IRQ_MPS0			26
#define IRQ_MPS1			27
#define IRQ_MPS2			28
#define IRQ_EXT0				29
#define IRQ_EXT1				30
#define IRQ_EXT2				31
#define IRQ_PANTHER7_HDMA 26
#define IRQ_OTG				26
#endif

#ifdef CONFIG_ARCH_PDK_PC9220

#define	IRQ_EXT1			30
#define	IRQ_EXT0			29
#define	IRQ_GPIO3			28
#define	IRQ_NAND			27
#define	IRQ_GPIO2			26
#define	IRQ_CLCD			25
#define	IRQ_GPIO1			24
#define	IRQ_GPIO0			23
#define	IRQ_HDMA			22
#define	IRQ_UDC			21
//#define	IRQ_UHC1			20
//#define	IRQ_UHC0			19
#define	IRQ_UHC1			19
#define	IRQ_UHC0			20

#define	IRQ_VIP      			18
#define	IRQ_VOP          		17
#define	IRQ_PCIA      	  	16
#define	IRQ_MAC0     	   	15
#define	IRQ_ADC        		14
#define	IRQ_RTC				13
#define	IRQ_PWM1         		12
#define	IRQ_PWM0      	  	11
#define	IRQ_TIMER2			10
#define	IRQ_TIMER1     		9
#define	IRQ_TIMER0        		8
#define	IRQ_SDMMC			7
#define	IRQ_I2S        	 	6
#define	IRQ_I2C         	 	5
#define	IRQ_SPI1       	 	4
#define	IRQ_SPI0      	 	3
#define	IRQ_UART2    	   	2
#define	IRQ_UART1        		1
#define	IRQ_UART0        		0
#define	IRQ_NAND			27
#define 	IRQ_PANTHER7_HDMA IRQ_HDMA

#endif

#if defined(CONFIG_ARCH_P7DK) || defined(CONFIG_ARCH_PDK_PC7210)

#define NR_IRQS				25

//#define IRQ_UART0			0
//#define IRQ_UART1			1
#define IRQ_UART0                     1
#define IRQ_UART1                     0
#define IRQ_TIMER0			2
#define IRQ_TIMER1			3
#define IRQ_TIMER2			4
#define IRQ_GPIO0			5
#define IRQ_UART2                       6
#define IRQ_UART3                       7
#define IRQ_RTC		                8
#define IRQ_I2S		                9
#define IRQ_SDMMC		                10
#define IRQ_SPI0				11
#define IRQ_HDMA			12
#define IRQ_GPIO1			13
#define IRQ_I2C				14
#define IRQ_MAC0			15
#define IRQ_UDC				16
#define IRQ_UHC0				17
#define IRQ_CLCD				18
#define IRQ_PWM0			19
#define IRQ_PWM1			20
#define IRQ_PWM2			21
#define IRQ_PWM3			22
#define IRQ_ADC				23
#define IRQ_NAND			24
#define IRQ_PANTHER7_HDMA 12

#endif


#if (defined (CONFIG_ARCH_MSMV))

#define NR_IRQS				32

#define IRQ_UART0			0
#define IRQ_UART1			1
#define IRQ_I2C1			2
#define IRQ_SPI0			3
#define IRQ_I2C2			4
#define IRQ_I2C				5
#define IRQ_I2S				6	
#define IRQ_TIMER0			8
#define IRQ_TIMER1			9
#define IRQ_TIMER2			10
#define IRQ_PWM0			11
#define IRQ_PWM1			12
#define IRQ_RTC				13
#define IRQ_UHC0				20
#define IRQ_PANTHER7_HDMA	22
#define IRQ_CLCD			25
#define IRQ_NAND			27
#define IRQ_GPIO			28
#define IRQ_EXT0			29
#define IRQ_EXT1			30
#define IRQ_MPS2			28

#endif

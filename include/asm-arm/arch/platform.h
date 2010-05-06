/*
 * linux/include/asm-arm/arch-ldk/platform.h
 *
 * Copyright (c) 2006 Socle-tech Corp.  All rights reserved.
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

#ifndef __SOCLE_MEM_MAPIO_H
#define __SOCLE_MEM_MAPIO_H                     1

#include <mach/hardware.h>

/*
 * SQ System Memory Map
 */

#if defined(CONFIG_MMC_SOCLE_SDHC) || defined(CONFIG_MMC_SOCLE_SDHC_SLAVE )
#warning "base address for SDHC need to be re-defined!!!"
#define SOCLE_AHB0_SDCH0			0x1CD20000
#define SOCLE_AHB0_SDCH1				0x18080000
#define SOCLE_AHB0_SDCS				0x1CB20000
#endif

#ifdef CONFIG_ARCH_LDK3V21

//FPGA Verification IP
#define SOCLE_AHB0_IDE 				SOCLE_AHB0_ES0 	/* IDE */
#define SOCLE_AHB0_UHC0				SOCLE_AHB0_ES0 	/*UHC */
#define SOCLE_AHB0_LCD				SOCLE_AHB0_ES3	/*LCD */
#define SOCLE_AHB0NAND 				SOCLE_AHB0_ES0	/*NAND */
#define SOCLE_APB0_I2S				SOCLE_AHB0_ES0	/*I2S */

#define SOCLE_CC_BASE              		0xEFFF0000	/* Cache Controller */
#define SOCLE_CC_ENABLE            		0x80000000
#define SOCLE_CC_DISABLE           		0x00000000

#define TOTAL_MEMORY_BANKS		4
#define SOCLE_MM_DDR_SDR_BANK0         0x00000000
#define SOCLE_MM_DDR_SDR_BANK1         0x40000000	// in kuseg, use TLB to mapping
#define SOCLE_MM_DDR_SDR_BANK2         0x50000000	// in kuseg, use TLB to mapping
#define SOCLE_MM_DDR_SDR_BANK3         0x17000000	// leonid	

#define SOCLE_AHB0_ES7        			0x20000000	/*256MB */ 

#define SOCLE_NOR_FLASH_BANK0            0x1F000000	// in kseg1, unmapped, uncached space
#define SOCLE_NOR_FLASH_BANK1            0x1F800000	// in kseg1, unmapped, uncached space

#define SOCLE_NOR_FLASH_SIZE                    SZ_8M


// APB device base address define
#define SOCLE_NOR_FLASH0                		0x1f000000
#define SOCLE_APB0_SDMMC                		0x1E920000
#define SOCLE_APB0_I2C               		0x1E900000
#define SOCLE_APB0_SCU               		0x1E8E0000
#define SOCLE_APB0_WDT               		0x1E8C0000
#define SOCLE_APB0_GPIO0              		0x1E8A0000
#define SOCLE_APB0_SPI0               		0x1E880000
#define SOCLE_APB0_UART1             		0x1E860000
#define SOCLE_APB0_UART0             		0x1E840000
#define SOCLE_APB0_RTC               		0x1E820000
#define SOCLE_APB0_TIMER               		0x1E800000

#define SOCLE_AHB0_A2A_DMA            	0x1E7E0000

// External AHB S0 ~ S6 1M
#define SOCLE_AHB0_ES6             		0x1E7A0000
#define SOCLE_AHB0_ES5             		0x1E780000
#define SOCLE_AHB0_ES4             		0x1E760000
#define SOCLE_AHB0_ES3             		0x1E740000
#define SOCLE_AHB0_ES2             		0x1E720000
#define SOCLE_AHB0_ES1             		0x1E700000		/*128K */
#define SOCLE_AHB0_ES0             		0x1E6E0000		/*256K */

#define SOCLE_BUS0_PCI	           		0x19CE0000
#define SOCLE_AHB0_UDC               		0x19CC0000
#define SOCLE_AHB0_MAC1              		0x19CA0000
#define SOCLE_AHB0_MAC0              		0x19C80000
#define SOCLE_AHB0_INTC		       	0x19C40000
#define SOCLE_AHB0_SDRSTMC	       	0x19C20000		// for static SDR
#define SOCLE_AHB0_DDRMC             		0x19C00000		// for DDR
#define SOCLE_AHB0_NAND	            		0x1e6e0000
//DDR START

//AHB1 Device base address define
#define SOCLE_AHB1_MPEG4_DE	        	0x18020000
#define SOCLE_AHB1_MPEG4_EN	        	0x18040000
#define SOCLE_AHB1_VOP                0x18080000
#endif


#ifdef CONFIG_ARCH_LDK5
//=========================================================================
/*
 * SQ LDK5 System Memory Map
 */

//FPGA VERIFICATION IP

// memory mapped address
#define	TOTAL_MEMORY_BANKS	8
#define SOCLE_MM_DDR_SDR_BANK0              0x00000000
#define SOCLE_MM_DDR_SDR_BANK1              0x40000000	
#define SOCLE_MM_DDR_SDR_BANK2              0x50000000	
#define SOCLE_MM_DDR_SDR_BANK3              0xF0000000	
#define SOCLE_MM_DDR_SDR_BANK4              0x60000000	
#define SOCLE_MM_DDR_SDR_BANK5              0x68000000	
#define SOCLE_MM_DDR_SDR_BANK6              0x70000000	
#define SOCLE_MM_DDR_SDR_BANK7              0x78000000	

#define SOCLE_NOR_FLASH_BANK0           		0x10000000	
#define SOCLE_NOR_FLASH_BANK1           		0x11000000
#define SOCLE_NOR_FLASH_BANK2           		0x12000000	
#define SOCLE_NOR_FLASH_BANK3           		0x13000000	

#define SOCLE_NOR_FLASH_SIZE                    SZ_4M

#define SOCLE_BUS0_PCI	              		0x1B600000


// AHB0 device base address define
#define SOCLE_AHB0_ES6    	        		0x18100000
#define SOCLE_AHB0_ES5    	        		0x180FC000
#define SOCLE_AHB0_ES4    	        		0x180F8000
#define SOCLE_AHB0_ES3    	        		0x180F4000
#define SOCLE_AHB0_ES2        	    		0x180F0000
#define SOCLE_AHB0_ES1            			0x180EC000
#define SOCLE_AHB0_ES0				0x180E8000


#define SOCLE_AHB0_DDRMC			    	0x180B4000	
#define SOCLE_AHB0_SDRSTMC            	0x180B0000	
#define SOCLE_AHB0_NAND	            		0x180D0000
#define SOCLE_AHB0_IDE	            		0x180A8000
#define SOCLE_AHB0_UHC0	            		0x180A4000
#define SOCLE_AHB0_UDC	            		0x180A0000
#define SOCLE_AHB0_MAC1               		0x1809C000
#define SOCLE_AHB0_MAC0               		0x18098000
#define SOCLE_AHB0_A2A_DMA            	0x18094000
#define SOCLE_AHB0_HDMA	            		0x18090000
#define SOCLE_AHB0_CPU_DebugIF        	0x1808C000
#define SOCLE_AHB0_CPU_Mailbox        	0x18088000
#define SOCLE_AHB0_Arbiter            		0x18084000
#define SOCLE_AHB0_INTC		        	0x18080000

// APB0 device base address define
#define SOCLE_APB0_I2S       			0x18028000
#define SOCLE_APB0_SDMMC	    	        		0x18024000
#define SOCLE_APB0_I2C 	            		0x18020000
#define SOCLE_APB0_SCU                		0x1801C000
#define SOCLE_APB0_SPI0	            		0x18018000
#define SOCLE_APB0_RTC	            		0x18014000
#define SOCLE_APB0_WDT	            		0x18010000
#define SOCLE_APB0_GPIO0            		0x1800C000
#define SOCLE_APB0_UART1              		0x18008000
#define SOCLE_APB0_UART0              		0x18004000
#define SOCLE_APB0_TIMER	            		0x18000000


// AHB1 device base address define
#define SOCLE_AHB1_ES6    	        		0x18700000
#define SOCLE_AHB1_ES5    	        		0x186FC000
#define SOCLE_AHB1_ES4    	       		0x186F8000
#define SOCLE_AHB1_ES3    	        		0x186F4000
#define SOCLE_AHB1_ES2        	    		0x186F0000
#define SOCLE_AHB1_ES1            			0x186EC000
#define SOCLE_AHB1_ES0				0x186E8000

#define SOCLE_AHB1_VOP			    	0x184C4000	
#define SOCLE_AHB1_VIP            			0x184C0000	
#define SOCLE_AHB1_MPEG4_DE	        	0x184BC000
#define SOCLE_AHB1_MPEG4_EN	        	0x184B8000
#define SOCLE_AHB1_Arbiter            		0x18484000
#define SOCLE_AHB1_INTC		        	0x18480000

// APB1 device base address define
#define SOCLE_APB1_WDT	            		0x18410000
#define SOCLE_APB1_GPIO0            		0x1840C000
#define SOCLE_APB1_UART0              		0x18404000
#define SOCLE_APB1_TIMER	            		0x18400000

#endif

#ifdef CONFIG_ARCH_PDK_PC9220
// memory mapped address
#define	SOCLE_MEMORY_ADDR_START			0x40000000
#define	SOCLE_MEMORY_ADDR_SIZE			0x04000000

#define SOCLE_MM_DDR_SDR_BANK0			0x00000000
#define SOCLE_MM_DDR_SDR_BANK1			0x40000000


#define	SOCLE_NORFLASH_BANKS		1

#define SOCLE_NOR_FLASH_BANK0        0x10000000	
#define SOCLE_STMEM_BANK1			0x11000000

#define SOCLE_NOR_FLASH0                		0x10000000
#define SOCLE_NOR_FLASH_SIZE                    SZ_4M

// APB device base address define

#define SOCLE_APB0_ADC			0x19120000
#define SOCLE_AHB0_LCD			0x19110000
#define SOCLE_APB0_SCU			0x19100000
#define SOCLE_APB0_WDT			0x190F0000
#define SOCLE_APB0_RTC			0x190E0000
#define SOCLE_APB0_GPIO3		0x190D0000
#define SOCLE_APB0_GPIO2		0x190C0000
#define SOCLE_APB0_GPIO1		0x190B0000
#define SOCLE_APB0_GPIO0		0x190A0000
#define SOCLE_APB0_PWM			0x19090000
#define SOCLE_APB0_TIMER		0x19080000
#define SOCLE_APB0_SDMMC		0x19070000
#define SOCLE_APB0_I2S			0x19060000
#define SOCLE_APB0_I2C			0x19050000
#define SOCLE_APB0_SPI1			0x19040000
#define SOCLE_APB0_SPI0			0x19030000
#define SOCLE_APB0_UART2		0x19020000
#define SOCLE_APB0_UART1		0x19010000
#define SOCLE_APB0_UART0		0x19000000

// AHB device base address define

#define SOCLE_AHB0_APB_BRI		0x19000000	
#define SOCLE_AHB0_NAND		0x18140000	
#define SOCLE_AHB0_VIP			0x18120000
#define SOCLE_AHB0_VOP			0x18100000
#define PANTHER7_AHB0_HDMA	0x180C0000
#define SOCLE_AHB0_HDMA		0x180C0000
#define SOCLE_AHB0_UDC			0x180A0000
//#define SOCLE_AHB0_UHC0		0x18080000
//#define SOCLE_AHB0_UHC1		0x180E0000
#define SOCLE_AHB0_UHC0		0x180E0000
#define SOCLE_AHB0_UHC1		0x18080000

#define SOCLE_AHB0_MAC0		0x18060000
#define SOCLE_AHB0_INTC		0x18040000
#define SOCLE_AHB0_ARBITER		0x18020000
#define SOCLE_AHB0_SDRSTMC	0x18000000	

//#define PANTHER7_AHB_0_HDMA_0	     SOCLE_AHB0_HDMA
//#define SOCLE_AHB0_UHC			SOCLE_AHB0_UHC0

#endif

#if (defined (CONFIG_ARCH_CDK) || defined (CONFIG_ARCH_PDK_PC9002) || defined(CONFIG_ARCH_SCDK))
// memory mapped address
#define	TOTAL_MEMORY_BANKS	5
#define	TOTAL_MEMORY_DDR_BANKS	0

#define SOCLE_MM_DDR_SDR_BANK0			0x00000000
#define SOCLE_MM_DDR_SDR_BANK1			0x40000000	
#define SOCLE_MM_DDR_SDR_BANK2			0x48000000	
#define SOCLE_MM_DDR_SDR_BANK3			0x50000000	
#define SOCLE_MM_DDR_SDR_BANK4			0x58000000	

#define SOCLE_MM_DDR_SDR_BANK0_SIZE		0x10000000
#define SOCLE_MM_DDR_SDR_BANK1_SIZE		0x08000000	
#define SOCLE_MM_DDR_SDR_BANK2_SIZE		0x08000000	
#define SOCLE_MM_DDR_SDR_BANK3_SIZE		0x08000000	
#define SOCLE_MM_DDR_SDR_BANK4_SIZE		0x08000000

#define	TOTAL_NORFLASH_BANKS	4

#define SOCLE_NOR_FLASH_BANK0               0x10000000	
#define SOCLE_NOR_FLASH_BANK1               0x12000000
#define SOCLE_NOR_FLASH_BANK2               0x14000000	
#define SOCLE_NOR_FLASH_BANK3               0x16000000	

#define SOCLE_NOR_FLASH0                		0x16000000
#ifdef CONFIG_ARCH_PDK_PC9002
#define SOCLE_NOR_FLASH_SIZE                    SZ_4M
#else //CDK
#define SOCLE_NOR_FLASH_SIZE                    SZ_16M
#endif 

// APB device base address define
#define SOCLE_APB0_MP                			0x1D1E0000		//MP_GPIO
#define SOCLE_APB0_ADC               			0x1D1C0000
#define SOCLE_APB0_SCU               			0x1D1A0000
#define SOCLE_APB0_WDT               			0x1D180000
#define SOCLE_APB0_RTC               			0x1D160000
#define SOCLE_APB0_GPIO0              			0x1D140000
#define SOCLE_APB0_PWM         				0x1D120000
#define SOCLE_APB0_TIMER             			0x1D100000
#define SOCLE_APB0_SDMMC                			0x1D0E0000
#define SOCLE_APB0_I2S               			0x1D0C0000
#define SOCLE_APB0_I2C               			0x1D0A0000
#define SOCLE_APB0_SPI1              			0x1D080000
#define SOCLE_APB0_SPI0              			0x1D060000
#define SOCLE_APB0_UART2             			0x1D040000
#define SOCLE_APB0_UART1             			0x1D020000
#define SOCLE_APB0_UART0             			0x1D000000

// AHB device base address define
#define SOCLE_AHB0_NAND               			0x1CD20000
#define SOCLE_AHB0_HDMA1              			0x1CB20000
//#define SOCLE_AHB0_MP	               		0x1CB20000
#define SOCLE_AHB0_LCD	           			0x1CB00000
#define SOCLE_AHB0_IDE	           			0x1CAE0000
#define SOCLE_AHB0_PCI_BRIDGE        		0x1C8E0000
#define SOCLE_AHB0_PCI_IO            			0x1C0E0000
#define SOCLE_AHB0_PCI_MEMORY        		0x180E0000
#define SOCLE_BUS0_PCI	 		   		0x180E0000
#define SOCLE_AHB0_HDMA	           			0x180C0000
#define SOCLE_AHB0_UDC	           			0x180A0000
#ifdef  CONFIG_SOCLE_EHCI_OTG
#define SOCLE_AHB0_UHC0               			SOCLE_AHB0_OTG
#else
#define SOCLE_AHB0_UHC0               			0x18080000
#endif
#define SOCLE_AHB0_MAC0              			0x18060000
#define SOCLE_AHB0_INTC		       		0x18040000
#define SOCLE_AHB0_ARBITER	       		0x18020000
#define SOCLE_AHB0_SDRSTMC	       		0x18000000	
#define PANTHER7_AHB0_HDMA 0x1CB20000
#define SOCLE_AHB0_OTG	           			0x1cb20000

#endif

#if defined(CONFIG_ARCH_P7DK) || defined(CONFIG_ARCH_PDK_PC7210)

#define TOTAL_MEMORY_BANKS		4
#define SOCLE_MM_DDR_SDR_BANK0         		0x00000000	//16MB
#define SOCLE_MM_DDR_SDR_BANK1         		0x80000000	

#define SOCLE_NOR_FLASH_BANK0           		0x10000000	//16MB
#define SOCLE_NOR_FLASH_BANK1            		0x11000000	//16MB

/*	leonid+	*/
#define SOCLE_NOR_FLASH0 		           		SOCLE_NOR_FLASH_BANK0	
#define SOCLE_NOR_FLASH1 		           		SOCLE_NOR_FLASH_BANK1	
#define SOCLE_NOR_FLASH_SIZE 		    		SZ_4M//0x400000			//4MB
/* Start of Cache Controller */
#define SOCLE_CC_BASE              				0xEFFF0000	/* Cache Controller */
#define SOCLE_CC_ENABLE            				0x80000000
#define SOCLE_CC_DISABLE           				0x00000000
/* End of Cache Controller */


// APB device base address define
#define SOCLE_APB0_EXT               				0x18040000
#define SOCLE_APB0_SCU               				0x1803C000
#define SOCLE_APB0_UART3               			0x18038000
#define SOCLE_APB0_UART2               			0x18034000
//#define SOCLE_APB0_UART1               			0x18030000
//#define SOCLE_APB0_UART0                			0x1802C000
#define SOCLE_APB0_UART0                                0x18030000
#define SOCLE_APB0_UART1                                      0x1802C000
#define SOCLE_APB0_ADC               				0x18028000
#define SOCLE_APB0_PWM               			0x18024000
#define SOCLE_APB0_SDMMC               			0x18020000
#define SOCLE_APB0_I2S               				0x1801C000
#define SOCLE_APB0_I2C               				0x18018000
#define SOCLE_APB0_SPI0	             				0x18014000
#define SOCLE_APB0_GPIO1              			0x18010000
#define SOCLE_APB0_GPIO0              			0x1800C000
#define SOCLE_APB0_WDT             				0x18008000
#define SOCLE_APB0_RTC               				0x18004000
#define SOCLE_APB0_TIMER               			0x18000000

// AHB Device base address devine
#define SOCLE_AHB0_NAND               			0x18070000
#define SOCLE_AHB0_ARBIT              			0x1806C000
#ifdef CONFIG_ARCH_PDK_PC7210
#define SOCLE_AHB0_LCD               				0x18040000
#else
#define SOCLE_AHB0_LCD               				0x18068000
#endif
#define SOCLE_AHB0_UHC0               			0x18064000
#define SOCLE_AHB0_UDC               			0x18060000
#define SOCLE_AHB0_MAC0               			0x1805C000
#define SOCLE_AHB0_SDRSTMC             			0x18058000
#define SOCLE_AHB0_HDMA              			0x18054000
#define PANTHER7_AHB0_HDMA 						SOCLE_AHB0_HDMA
#define SOCLE_AHB0_INTC		       		0x18050000

#endif


#if (defined (CONFIG_ARCH_MSMV))
// memory mapped address
#define	TOTAL_MEMORY_BANKS	1
#define	TOTAL_MEMORY_DDR_BANKS	0

#define SOCLE_MM_DDR_SDR_BANK0			0x00000000
#define SOCLE_MM_DDR_SDR_BANK1			0x40000000	

#define SOCLE_MM_DDR_SDR_BANK0_SIZE		0x02000000
#define SOCLE_MM_DDR_SDR_BANK1_SIZE		0x02000000

#define	TOTAL_NORFLASH_BANKS	1

#define SOCLE_NOR_FLASH_BANK0               0x16000000	

#define SOCLE_NOR_FLASH0                		0x16000000
#define SOCLE_NOR_FLASH_SIZE                    SZ_16M

// APB device base address define
#define SOCLE_APB0_SCU               			0x1D1A0000
#define SOCLE_APB0_WDT               		0x1D180000
#define SOCLE_APB0_RTC               			0x1D160000
#define SOCLE_APB0_GPIO0              		0x1D140000
#define SOCLE_APB0_PWM         			0x1D120000
#define SOCLE_APB0_TIMER             		0x1D100000
#define SOCLE_APB0_I2S               			0x1D0C0000
#define SOCLE_APB0_I2C              			0x1D0A0000
#define SOCLE_APB0_I2C2              			0x1D080000
#define SOCLE_APB0_SPI0              			0x1D060000
#define SOCLE_APB0_I2C1             			0x1D040000
#define SOCLE_APB0_UART1             		0x1D020000
#define SOCLE_APB0_UART0             		0x1D000000

// AHB device base address define
#define SOCLE_AHB0_EBD_SRAM1			0x1CD20000
#define SOCLE_AHB0_EBD_SRAM2			0x1CD30000
#define SOCLE_AHB0_NAND               		0x1CB20000
#define SOCLE_AHB0_LCD	           			0x1CB00000
#define SOCLE_AHB0_HDMA	           		0x180C0000
#define SOCLE_AHB0_UHC0               		0x18080000
#define SOCLE_AHB0_INTC		       	0x18040000
#define SOCLE_AHB0_ARBITER	       		0x18020000
#define SOCLE_AHB0_SDRSTMC	       	0x18000000	

#define SOCLE_APB0_MP					SOCLE_APB0_GPIO0
#define PANTHER7_AHB0_HDMA 			SOCLE_AHB0_HDMA

#define TPS62353_NUM	3

#endif

#endif /* 	END */

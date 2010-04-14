/*
 * linux/include/asm-arm/hardware/socle_clcd.h -- Integrator LCD panel.
 *
 * David A Rusling
 *
 * Copyright (C) 2001 ARM Limited
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */
#include <linux/fb.h>

/*
 * CLCD Controller Internal Register addresses
 */
#define CLCD_TIM0		0x00000000
#define CLCD_TIM1 		0x00000004
#define CLCD_TIM2 		0x00000008
#define CLCD_TIM3 		0x0000000c
#define CLCD_UBAS 		0x00000010
#define CLCD_LBAS 		0x00000014

#if !defined(CONFIG_ARCH_VERSATILE) && !defined(CONFIG_ARCH_REALVIEW)
#define CLCD_IENB 		0x00000018
#define CLCD_CNTL 		0x0000001c
#else
/*
 * Someone rearranged these two registers on the Versatile
 * platform...
 */
#define CLCD_IENB 		0x0000001c
#define CLCD_CNTL 		0x00000018
#endif

#define CLCD_STAT 		0x00000020
#define CLCD_INTR 		0x00000024
#define CLCD_UCUR 		0x00000028
#define CLCD_LCUR 		0x0000002C
#define CLCD_PALL 		0x00000200
#define CLCD_PALETTE		0x00000200

#define TIM2_CLKSEL		(1 << 5)
#define TIM2_IVS		(1 << 11)
#define TIM2_IHS		(1 << 12)
#define TIM2_IPC		(1 << 13)
#define TIM2_IOE		(1 << 14)
#define TIM2_BCD		(1 << 26)

#define CNTL_LCDEN		(1 << 0)
#define CNTL_LCDBPP1		(0 << 1)
#define CNTL_LCDBPP2		(1 << 1)
#define CNTL_LCDBPP4		(2 << 1)
#define CNTL_LCDBPP8		(3 << 1)
#define CNTL_LCDBPP16		(4 << 1)
#define CNTL_LCDBPP16_565	(6 << 1)
#define CNTL_LCDBPP24		(5 << 1)
#define CNTL_LCDBW		(1 << 4)
#define CNTL_LCDTFT		(1 << 5)
#define CNTL_LCDMONO8		(1 << 6)
#define CNTL_LCDDUAL		(1 << 7)
#define CNTL_BGR		(1 << 8)
#define CNTL_BEBO		(1 << 9)
#define CNTL_BEPO		(1 << 10)
#define CNTL_LCDPWR		(1 << 11)
#define CNTL_LCDVCOMP(x)	((x) << 12)
#define CNTL_LDMAFIFOTIME	(1 << 15)
#define CNTL_WATERMARK		(1 << 16)

struct clcd_panel {
	struct fb_videomode	mode;
	signed short		width;	/* width in mm */
	signed short		height;	/* height in mm */
	u32			tim2;
	u32			tim3;
	u32			cntl;
	unsigned int		bpp:8,
				fixedtimings:1,
				grayscale:1;
	unsigned int		connector;
};

struct clcd_regs {
	u32			tim0;
	u32			tim1;
	u32			tim2;
	u32			tim3;
	u32			cntl;
	unsigned long		pixclock;
};

struct socle_clcd_fb;

struct platform_device;
struct clk;

/* this data structure describes each frame buffer device we find */
struct socle_clcd_fb {
	struct fb_info		fb;
	struct platform_device	*dev;
	struct clk		*clk;
	struct clcd_panel	*panel;
	void __iomem		*regs;
	u32			clcd_cntl;
	u32			cmap[16];
};


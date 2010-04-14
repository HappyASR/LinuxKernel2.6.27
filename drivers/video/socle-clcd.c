/*
 *  linux/drivers/video/socle-clcd.c
 *
 * Copyright (C) 2001 ARM Limited, by David A Rusling
 * Updated to 2.5, Deep Blue Solutions Ltd.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/list.h>

#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/sizes.h>

#include <mach/platform.h>
#include <mach/socle-clcd.h>


#ifdef  CONFIG_ANDROID_SYSTEM
/* Android will use double buffer in video if there is enough */
#define ANDROID_NUMBER_OF_BUFFERS 2
#else
#define ANDROID_NUMBER_OF_BUFFERS 1
#endif

#ifdef CONFIG_7INCH_PANEL
struct clcd_panel socle_clcd_info = {
		.mode	= {
			.name			= "Ampire",
			.refresh		= 30,
			.xres			= 800,
			.yres			= 480,
			.pixclock		= 39721,
			.left_margin		= 37,
			.right_margin		= 37,
			.upper_margin		= 17,
			.lower_margin		= 0,
			.hsync_len		= 20,
			.vsync_len		= 3,
			.sync			= 0,
			.vmode			= FB_VMODE_NONINTERLACED,
		},
		.width	= -1,
		.height	= -1,
		.tim2	= 0x071f0000,
		.tim3	= 0,
		.cntl	= 0x829 | CNTL_BGR |1<<16,
		.bpp	= 16,
};

#elif defined(CONFIG_10INCH_PANEL)
struct clcd_panel socle_clcd_info = {
		.mode	= {
			.name			= "Ampire",
			.refresh		= 30,
			.xres			= 1024,
			.yres			= 768,
			.pixclock		= 39721,
			.left_margin		= 37,
			.right_margin		= 37,
			.upper_margin		= 17,
			.lower_margin		= 0,
			.hsync_len		= 20,
			.vsync_len		= 3,
			.sync			= 0,
			.vmode			= FB_VMODE_NONINTERLACED,
		},
		.width	= -1,
		.height	= -1,
		.tim2	= 0x071f0000,
		.tim3	= 0,
		.cntl	= 0x829 | CNTL_BGR |1<<16,
		.bpp	= 16,
};

#else
struct clcd_panel socle_clcd_info = {
                .mode   = {
                        .name                   = "Ampire",
                        .refresh                = 60,
                        .xres                   = 320,
                        .yres                   = 240,
                        .pixclock               = 39721,
                        .left_margin            = 38,
                        .right_margin           = 20,
                        .upper_margin   	= 15,
                        .lower_margin           = 5,
                        .hsync_len              = 30,
                        .vsync_len              = 3,
                        .sync                   = 0,
                        .vmode                  = FB_VMODE_NONINTERLACED,
                },
                .width  = -1,
                .height = -1,
//              .tim2   = TIM2_IVS | TIM2_IHS,
#if defined(CONFIG_ARCH_CDK) || defined(CONFIG_ARCH_MSMV) || defined(CONFIG_ARCH_SCDK)
                	.tim2   = 0x053f2000, //6MHz
#else
			.tim2   = 0x013f2002, //27Mhz
#endif
   //           .tim2   = 0x53f2024,
                .tim3   = 0,
//              .cntl   = CNTL_LCDTFT,
#ifdef CONFIG_24BPP_FRAMEBUFFER
                .cntl   = 0x82b | CNTL_BGR |1<<16,
                .bpp    = 32,
#else
                .cntl   = 0x829 | CNTL_BGR |1<<16,
                .bpp    = 16,
#endif
};
#endif
/*
regs.tim0=0x25131d4c;
regs.tim1=0x0e0408ef;
regs.tim2=0x053f2000;
regs.tim3=0x0;
*/

#include <linux/dma-mapping.h>
static int socle_clcdfb_check(struct socle_clcd_fb *fb, struct fb_var_screeninfo *var)
{
	var->xres_virtual = var->xres = (var->xres + 15) & ~15;
	var->yres = (var->yres + 1) & ~1;	
	var->yres_virtual=var->yres*2;

//	var->yres_virtual = var->yres = (var->yres + 1) & ~1;

#define CHECK(e,l,h) (var->e < l || var->e > h)
	if (CHECK(right_margin, (5+1), 256) ||	/* back porch */
	    CHECK(left_margin, (5+1), 256) ||	/* front porch */
	    CHECK(hsync_len, (5+1), 256) ||
	    var->xres > 4096 ||
	    var->lower_margin > 255 ||		/* back porch */
	    var->upper_margin > 255 ||		/* front porch */
	    var->vsync_len > 32 ||
	    var->yres > 1024)
		return -EINVAL;
#undef CHECK

	/* single panel mode: PCD = max(PCD, 1) */
	/* dual panel mode: PCD = max(PCD, 5) */

	/*
	 * You can't change the grayscale setting, and
	 * we can only do non-interlaced video.
	 */
	if (var->grayscale != fb->fb.var.grayscale ||
	    (var->vmode & FB_VMODE_MASK) != FB_VMODE_NONINTERLACED)
		return -EINVAL;

#define CHECK(e) (var->e != fb->fb.var.e)
	if (fb->panel->fixedtimings &&
	    (CHECK(xres)		||
	     CHECK(yres)		||
	     CHECK(bits_per_pixel)	||
	     CHECK(pixclock)		||
	     CHECK(left_margin)		||
	     CHECK(right_margin)	||
	     CHECK(upper_margin)	||
	     CHECK(lower_margin)	||
	     CHECK(hsync_len)		||
	     CHECK(vsync_len)		||
	     CHECK(sync)))
		return -EINVAL;
#undef CHECK

	var->nonstd = 0;
	var->accel_flags = 0;

	return 0;
}

static int socle_clcd_mmap(struct socle_clcd_fb *fb, struct vm_area_struct *vma)
{
	return dma_mmap_writecombine(&fb->dev->dev, vma,
			fb->fb.screen_base,
			fb->fb.fix.smem_start,
			fb->fb.fix.smem_len);
}

#define to_clcd(info)	container_of(info, struct socle_clcd_fb, fb)

/* This is limited to 16 characters when displayed by X startup */
static const char *clcd_name = "CLCD FB";

/*
 * Unfortunately, the enable/disable functions may be called either from
 * process or IRQ context, and we _need_ to delay.  This is _not_ good.
 */
static inline void clcdfb_sleep(unsigned int ms)
{
	if (in_atomic()) {
		mdelay(ms);
	} else {
		msleep(ms);
	}
}

static inline void clcdfb_set_start(struct socle_clcd_fb *fb)
{
	unsigned long ustart = fb->fb.fix.smem_start;
	unsigned long lstart;

	ustart += fb->fb.var.yoffset * fb->fb.fix.line_length;
	lstart = ustart + fb->fb.var.yres * fb->fb.fix.line_length / 2;
	writel(ustart, fb->regs + CLCD_UBAS);
//	writel(lstart, fb->regs + CLCD_LBAS);
}

static void clcdfb_disable(struct socle_clcd_fb *fb)
{
	u32 val;

	val = readl(fb->regs + CLCD_CNTL);

	if (val & CNTL_LCDPWR) {
		val &= ~CNTL_LCDPWR;
		writel(val, fb->regs + CLCD_CNTL);

//		clcdfb_sleep(20);
	}
#ifdef CONFIG_7INCH_PANEL
#elif defined(CONFIG_10INCH_PANEL)
#else
	if (val & CNTL_LCDEN) {
		val &= ~CNTL_LCDEN;
		writel(val, fb->regs + CLCD_CNTL);
	}
#endif
}

static void clcdfb_enable(struct socle_clcd_fb *fb, u32 cntl)
{
	/*
	 * Bring up by first enabling..
	 */
#ifdef CONFIG_7INCH_PANEL
#elif defined(CONFIG_10INCH_PANEL)
#else
	cntl |= CNTL_LCDEN;
	writel(cntl, fb->regs + CLCD_CNTL);
#endif

//	clcdfb_sleep(20);

	/*
	 * and now apply power.
	 */
	cntl |= CNTL_LCDPWR;
	writel(cntl, fb->regs + CLCD_CNTL);

}

static int
clcdfb_set_bitfields(struct socle_clcd_fb *fb, struct fb_var_screeninfo *var)
{
	int ret = 0;

	memset(&var->transp, 0, sizeof(var->transp));

	var->red.msb_right = 0;
	var->green.msb_right = 0;
	var->blue.msb_right = 0;

	switch (var->bits_per_pixel) {
	case 1:
	case 2:
	case 4:
	case 8:
		var->red.length		= var->bits_per_pixel;
		var->red.offset		= 0;
		var->green.length	= var->bits_per_pixel;
		var->green.offset	= 0;
		var->blue.length	= var->bits_per_pixel;
		var->blue.offset	= 0;
		break;
	case 16:
		var->red.length = 5;
		var->blue.length = 5;
		/*
		 * Green length can be 5 or 6 depending whether
		 * we're operating in RGB555 or RGB565 mode.
		 */
		var->green.length = 5;
		break;
	case 24:
		if (fb->panel->cntl & CNTL_LCDTFT) {
			var->red.length		= 8;
			var->green.length	= 8;
			var->blue.length	= 8;
//Peter test
			var->transp.length=8;
			var->transp.offset=24;
			break;
		}
	case 32:
		if (fb->panel->cntl & CNTL_LCDTFT) {
			var->red.length		= 8;
			var->green.length	= 8;
			var->blue.length	= 8;
//Peter test
			var->transp.length=8;
			var->transp.offset=24;
			break;
		}
	default:
		ret = -EINVAL;
		break;
	}

	/*
	 * >= 16bpp displays have separate colour component bitfields
	 * encoded in the pixel data.  Calculate their position from
	 * the bitfield length defined above.
	 */
	if (ret == 0 && var->bits_per_pixel >= 16) {
		if (fb->panel->cntl & CNTL_BGR) {
			var->blue.offset = 0;
			var->green.offset = var->blue.offset + var->blue.length;
			var->red.offset = var->green.offset + var->green.length;
		} else {
			var->red.offset = 0;
			var->green.offset = var->red.offset + var->red.length;
			var->blue.offset = var->green.offset + var->green.length;
		}
	}

	return ret;
}
static int clcdfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct socle_clcd_fb *fb = to_clcd(info);
	int ret = -EINVAL;
	
		ret = socle_clcdfb_check(fb, var);

	if (ret == 0 &&
	    var->xres_virtual * var->bits_per_pixel / 8 *
	    var->yres_virtual > fb->fb.fix.smem_len)
		ret = -EINVAL;

	if (ret == 0)
		ret = clcdfb_set_bitfields(fb, var);

	return ret;
}

static void socle_clcdfb_decode(struct socle_clcd_fb *fb, struct clcd_regs *regs)
{
	u32 val, cpl;

	/*
	 * Program the CLCD controller registers and start the CLCD
	 */
	val = ((fb->fb.var.xres / 16) - 1) << 2;
	val |= (fb->fb.var.hsync_len - 1) << 8;
	val |= (fb->fb.var.right_margin - 1) << 16;
	val |= (fb->fb.var.left_margin - 1) << 24;
	regs->tim0 = val;

	val = fb->fb.var.yres;
	if (fb->panel->cntl & CNTL_LCDDUAL)
		val /= 2;
	val -= 1;
	val |= (fb->fb.var.vsync_len - 1) << 10;
	val |= fb->fb.var.lower_margin << 16;
	val |= fb->fb.var.upper_margin << 24;
	regs->tim1 = val;

	val = fb->panel->tim2;
	val |= fb->fb.var.sync & FB_SYNC_HOR_HIGH_ACT  ? 0 : TIM2_IHS;
	val |= fb->fb.var.sync & FB_SYNC_VERT_HIGH_ACT ? 0 : TIM2_IVS;

	cpl = fb->fb.var.xres_virtual;
	if (fb->panel->cntl & CNTL_LCDTFT)	  /* TFT */
		/* / 1 */;
	else if (!fb->fb.var.grayscale)		  /* STN color */
		cpl = cpl * 8 / 3;
	else if (fb->panel->cntl & CNTL_LCDMONO8) /* STN monochrome, 8bit */
		cpl /= 8;
	else					  /* STN monochrome, 4bit */
		cpl /= 4;

	regs->tim2 = val | ((cpl - 1) << 16);

	regs->tim3 = fb->panel->tim3;

	val = fb->panel->cntl;
	if (fb->fb.var.grayscale)
		val |= CNTL_LCDBW;

	switch (fb->fb.var.bits_per_pixel) {
	case 1:
		val |= CNTL_LCDBPP1;
		break;
	case 2:
		val |= CNTL_LCDBPP2;
		break;
	case 4:
		val |= CNTL_LCDBPP4;
		break;
	case 8:
		val |= CNTL_LCDBPP8;
		break;
	case 16:
		/*
		 * PL110 cannot choose between 5551 and 565 modes in
		 * its control register
		 */
		if ((fb->dev->id & 0x000fffff) == 0x00041110)
			val |= CNTL_LCDBPP16;
		else if (fb->fb.var.green.length == 5)
			val |= CNTL_LCDBPP16;
		else
			val |= CNTL_LCDBPP16_565;
		break;
	case 32:
		val |= CNTL_LCDBPP24;
		break;
	}

	regs->cntl = val;
	regs->pixclock = fb->fb.var.pixclock;
}

/* Pan the display if device supports it. */
static int socle_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct socle_clcd_fb *fbi = to_clcd(info);
#if 1 //try de-flicker
	static int quick_addr0;
	static int quick_addr1;
	if(!quick_addr0)
	{
		quick_addr0 = fbi->fb.fix.smem_start;
		quick_addr1 = fbi->fb.var.yres* fbi->fb.fix.line_length + fbi->fb.fix.smem_start;
	}
	if(var->yoffset)
	{
		writel(quick_addr1, fbi->regs + CLCD_UBAS);
		while(readl(fbi->regs + CLCD_LCUR) < quick_addr1);
	}
	else
	{
		writel(quick_addr0, fbi->regs + CLCD_UBAS);
		while(readl(fbi->regs + CLCD_LCUR) > quick_addr1);
	}

#else
	u32 addr = var->yoffset * fbi->fb.fix.line_length + fbi->fb.fix.smem_start;

	writel(addr, fbi->regs + CLCD_UBAS);
//try de-flicker
//	if(var->yoffset)
//		while(readl(fbi->regs + CLCD_LCUR) < addr);
//	else
//		while(readl(fbi->regs + CLCD_LCUR) > (fbi->fb.var.yres* fbi->fb.fix.line_length + fbi->fb.fix.smem_start));
#endif

	return 0;
}



static int clcdfb_set_par(struct fb_info *info)
{
	struct socle_clcd_fb *fb = to_clcd(info);
	struct clcd_regs regs;

	fb->fb.fix.line_length = fb->fb.var.xres_virtual *
				 fb->fb.var.bits_per_pixel / 8;

	if (fb->fb.var.bits_per_pixel <= 8)
		fb->fb.fix.visual = FB_VISUAL_PSEUDOCOLOR;
	else
		fb->fb.fix.visual = FB_VISUAL_TRUECOLOR;

	socle_clcdfb_decode(fb, &regs);
/*
printk("time %x,%x,%x,%x \n",regs.tim0,regs.tim1,regs.tim2,regs.tim3);

regs.tim0=0x25131d4c;
regs.tim1=0x0e0408ef;
regs.tim2=0x053f2000;
regs.tim3=0x0;
*/

	clcdfb_disable(fb);

	writel(regs.tim0, fb->regs + CLCD_TIM0);
	writel(regs.tim1, fb->regs + CLCD_TIM1);
	writel(regs.tim2, fb->regs + CLCD_TIM2);
	writel(regs.tim3, fb->regs + CLCD_TIM3);
//Peter test
	writel(0x1e, fb->regs + CLCD_STAT);

	clcdfb_set_start(fb);

	fb->clcd_cntl = regs.cntl;

	clcdfb_enable(fb, regs.cntl);

//#define DEBUG
#ifdef DEBUG
	printk(KERN_INFO "CLCD: Registers set to\n"
	       KERN_INFO "  %08x %08x %08x %08x\n"
	       KERN_INFO "  %08x %08x %08x %08x\n",
		readl(fb->regs + CLCD_TIM0), readl(fb->regs + CLCD_TIM1),
		readl(fb->regs + CLCD_TIM2), readl(fb->regs + CLCD_TIM3),
		readl(fb->regs + CLCD_UBAS), readl(fb->regs + CLCD_LBAS),
		readl(fb->regs + CLCD_IENB), readl(fb->regs + CLCD_CNTL));
#endif

	return 0;
}

static inline u32 convert_bitfield(int val, struct fb_bitfield *bf)
{
	unsigned int mask = (1 << bf->length) - 1;

	return (val >> (16 - bf->length) & mask) << bf->offset;
}

/*
 *  Set a single color register. The values supplied have a 16 bit
 *  magnitude.  Return != 0 for invalid regno.
 */
static int
clcdfb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
		 unsigned int blue, unsigned int transp, struct fb_info *info)
{

	struct socle_clcd_fb *fb = to_clcd(info);

	if (regno < 16)
		fb->cmap[regno] = convert_bitfield(transp, &fb->fb.var.transp) |
				  convert_bitfield(blue, &fb->fb.var.blue) |
				  convert_bitfield(green, &fb->fb.var.green) |
				  convert_bitfield(red, &fb->fb.var.red);

	if (fb->fb.fix.visual == FB_VISUAL_PSEUDOCOLOR && regno < 256) {
		int hw_reg = CLCD_PALETTE + ((regno * 2) & ~3);
		u32 val, mask, newval;

		newval  = (red >> 11)  & 0x001f;
		newval |= (green >> 6) & 0x03e0;
		newval |= (blue >> 1)  & 0x7c00;

		/*
		 * 3.2.11: if we're configured for big endian
		 * byte order, the palette entries are swapped.
		 */
		if (fb->clcd_cntl & CNTL_BEBO)
			regno ^= 1;

		if (regno & 1) {
			newval <<= 16;
			mask = 0x0000ffff;
		} else {
			mask = 0xffff0000;
		}

		val = readl(fb->regs + hw_reg) & mask;
		writel(val | newval, fb->regs + hw_reg);
	}

	return regno > 255;
}

/*
 *  Blank the screen if blank_mode != 0, else unblank. If blank == NULL
 *  then the caller blanks by setting the CLUT (Color Look Up Table) to all
 *  black. Return 0 if blanking succeeded, != 0 if un-/blanking failed due
 *  to e.g. a video mode which doesn't support it. Implements VESA suspend
 *  and powerdown modes on hardware that supports disabling hsync/vsync:
 *    blank_mode == 2: suspend vsync
 *    blank_mode == 3: suspend hsync
 *    blank_mode == 4: powerdown
 */
static int clcdfb_blank(int blank_mode, struct fb_info *info)
{

	struct socle_clcd_fb *fb = to_clcd(info);

	if (blank_mode != 0) {
		clcdfb_disable(fb);
	} else {
		clcdfb_enable(fb, fb->clcd_cntl);
	}
	return 0;
}

static int clcdfb_mmap(struct fb_info *info,
		       struct vm_area_struct *vma)
{
	struct socle_clcd_fb *fb = to_clcd(info);
	unsigned long len, off = vma->vm_pgoff << PAGE_SHIFT;
	int ret = -EINVAL;
	len = info->fix.smem_len;
#ifndef CONFIG_MMU
	vma->vm_start = info->fix.smem_start;
#endif
	if (off <= len && vma->vm_end - vma->vm_start <= len - off)
		ret = socle_clcd_mmap(fb, vma);

	return ret;
}

static struct fb_ops clcdfb_ops = {
	.owner		= THIS_MODULE,
	.fb_check_var	= clcdfb_check_var,
	.fb_set_par	= clcdfb_set_par,
	.fb_setcolreg	= clcdfb_setcolreg,
	.fb_blank		= clcdfb_blank,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
	.fb_mmap	= clcdfb_mmap,
	.fb_pan_display = socle_fb_pan_display,
};

static int socle_clcdfb_register(struct socle_clcd_fb *fb)
{
	int ret;
	fb->fb.fix.mmio_start	= SOCLE_AHB0_LCD;
	fb->fb.fix.mmio_len	= SZ_4K;

	fb->regs = (void *)IO_ADDRESS(SOCLE_AHB0_LCD);
	fb->fb.fbops		= &clcdfb_ops;
	fb->fb.flags		= FBINFO_FLAG_DEFAULT;
	fb->fb.pseudo_palette	= fb->cmap;

	strncpy(fb->fb.fix.id, clcd_name, sizeof(fb->fb.fix.id));
	fb->fb.fix.type		= FB_TYPE_PACKED_PIXELS;
	fb->fb.fix.type_aux	= 0;
	fb->fb.fix.xpanstep	= 0;
#ifndef CONFIG_ANDROID_SYSTEM
	fb->fb.fix.ypanstep	= 0;
#else
	fb->fb.fix.ypanstep	= 1;
#endif
	fb->fb.fix.ywrapstep	= 0;
	fb->fb.fix.accel	= FB_ACCEL_NONE;

	fb->fb.var.xres		= fb->panel->mode.xres;
	fb->fb.var.yres		= fb->panel->mode.yres;
	fb->fb.var.xres_virtual	= fb->panel->mode.xres;
#ifndef CONFIG_ANDROID_SYSTEM
	fb->fb.var.yres_virtual	= fb->panel->mode.yres;
#else
	fb->fb.var.yres_virtual	= fb->panel->mode.yres * ANDROID_NUMBER_OF_BUFFERS;
#endif
	fb->fb.var.bits_per_pixel = fb->panel->bpp;
	fb->fb.var.grayscale	= fb->panel->grayscale;
	fb->fb.var.pixclock	= fb->panel->mode.pixclock;
	fb->fb.var.left_margin	= fb->panel->mode.left_margin;
	fb->fb.var.right_margin	= fb->panel->mode.right_margin;
	fb->fb.var.upper_margin	= fb->panel->mode.upper_margin;
	fb->fb.var.lower_margin	= fb->panel->mode.lower_margin;
	fb->fb.var.hsync_len	= fb->panel->mode.hsync_len;
	fb->fb.var.vsync_len	= fb->panel->mode.vsync_len;
	fb->fb.var.sync		= fb->panel->mode.sync;
	fb->fb.var.vmode	= fb->panel->mode.vmode;
#if defined(CONFIG_ARCH_P7DK) || defined(CONFIG_ARCH_PDK_PC7210)
	fb->fb.var.activate	= FB_ACTIVATE_FORCE;
#else
	fb->fb.var.activate	= FB_ACTIVATE_NOW;
#endif
	fb->fb.var.nonstd	= 0;
	fb->fb.var.height	= fb->panel->height;
	fb->fb.var.width	= fb->panel->width;
	fb->fb.var.accel_flags	= 0;

	fb->fb.monspecs.hfmin	= 0;
	fb->fb.monspecs.hfmax   = 100000;
	fb->fb.monspecs.vfmin	= 0;
	fb->fb.monspecs.vfmax	= 400;
	fb->fb.monspecs.dclkmin = 1000000;
	fb->fb.monspecs.dclkmax	= 100000000;

	/*
	 * Make sure that the bitfields are set appropriately.
	 */
	clcdfb_set_bitfields(fb, &fb->fb.var);

	/*
	 * Allocate colourmap.
	 */
	fb_alloc_cmap(&fb->fb.cmap, 256, 0);

	/*
	 * Ensure interrupts are disabled.
	 */
	writel(0, fb->regs + CLCD_IENB);

//cyli fix
 	clcdfb_set_par(&fb->fb);
 	
	fb_set_var(&fb->fb, &fb->fb.var);

	ret = register_framebuffer(&fb->fb);
	if (ret != 0)
		printk(KERN_ERR "CLCD: cannot register framebuffer (%d)\n", ret);
	return ret;
		
	
}

static int socle_clcd_setup(struct socle_clcd_fb *fb)
{
	dma_addr_t dma;
	unsigned int framesize;

	fb->panel = &socle_clcd_info;

	framesize=fb->panel->mode.xres * fb->panel->mode.yres * fb->panel->bpp / 8 * ANDROID_NUMBER_OF_BUFFERS;
	fb->fb.screen_base = dma_alloc_writecombine(&fb->dev->dev, framesize,&dma, GFP_KERNEL);

	if (!fb->fb.screen_base) {
		printk(KERN_ERR "CLCD: unable to map framebuffer\n");
		return -ENOMEM;
	}

	fb->fb.fix.smem_start = dma;
	fb->fb.fix.smem_len	= framesize;
	return 0;
}

static void socle_clcd_remove(struct socle_clcd_fb *fb)
{
	dma_free_writecombine(&fb->dev->dev, fb->fb.fix.smem_len,
			fb->fb.screen_base, fb->fb.fix.smem_start);
}

static int socle_clcdfb_probe(struct platform_device *dev)
{
//	struct clcd_board *board = dev->dev.platform_data;
	struct socle_clcd_fb *fb;
	int ret;

	fb = (struct socle_clcd_fb *) kmalloc(sizeof(struct socle_clcd_fb), GFP_KERNEL);
	if (!fb) {
		printk(KERN_INFO "CLCD: could not allocate new socle_clcd_fb struct\n");
		ret = -ENOMEM;
		goto out;
	}
	memset(fb, 0, sizeof(struct socle_clcd_fb));

	fb->dev = dev;
//	fb->board = board;

	ret = socle_clcd_setup(fb);
	if (ret)
		goto free_fb;

	ret = socle_clcdfb_register(fb); 
	if (ret == 0) {
		platform_set_drvdata(dev, fb);
		goto out;
	}

	socle_clcd_remove(fb);
 free_fb:
	kfree(fb);
 out:
	return ret;
}

static int socle_clcdfb_remove(struct platform_device *dev)
{
	struct socle_clcd_fb *fb = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	clcdfb_disable(fb);
	unregister_framebuffer(&fb->fb);

	socle_clcd_remove(fb);

	kfree(fb);

	return 0;
}


#ifdef CONFIG_PM

#define SOCLE_CLCD_REG_NUM	13

u32 socle_clcd_save_addr[SOCLE_CLCD_REG_NUM]; 

static int
socle_clcdfb_suspend(struct platform_device *dev, pm_message_t msg)
{
	int tmp;	
	u32 *base;
	struct socle_clcd_fb *fb;
	
	pr_debug("sq_clcdfb_suspend\n");
	
	fb = platform_get_drvdata(dev);

	base = fb->regs;
			
	for(tmp=0;tmp<SOCLE_CLCD_REG_NUM;tmp++){
		iowrite32(ioread32(base+tmp), (socle_clcd_save_addr+tmp));
	}
	
        return 0;
}

static int 
socle_clcdfb_resume(struct platform_device *dev)
{	
	int tmp;	
	u32 *base;
	struct socle_clcd_fb *fb;
	
	pr_debug("sq_clcdfb_resume\n");
	
	fb = platform_get_drvdata(dev);

	base = fb->regs;
			
	for(tmp=0;tmp<SOCLE_CLCD_REG_NUM;tmp++){
		iowrite32(ioread32(socle_clcd_save_addr+tmp), (base+tmp));
	}
	
        return 0;
}
#else
#define socle_clcdfb_suspend NULL
#define socle_clcdfb_resume NULL
#endif

static struct platform_driver clcd_driver = {	
	.probe		= socle_clcdfb_probe,
	.remove		= socle_clcdfb_remove,
	.suspend = socle_clcdfb_suspend,
	.resume = socle_clcdfb_resume,
	.driver		= {
		.name	= "socle-clcd",
		.owner	= THIS_MODULE,
	},
	
};
static int __init platform_clcdfb_init(void)
{
	return platform_driver_register(&clcd_driver);
}

module_init(platform_clcdfb_init);

static void __exit platform_clcdfb_exit(void)
{
	platform_driver_unregister(&clcd_driver);
}

module_exit(platform_clcdfb_exit);

MODULE_DESCRIPTION("ARM PrimeCell PL110 CLCD core driver");
MODULE_LICENSE("GPL");

/*
 * linux/drivers/video/soclefb.c
 *	Copyright (c) Arnaud Patard, Ben Dooks
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 *
 *	    SQ LCD Controller Frame Buffer Driver
 *
 * ChangeLog
 * 2007-06-28: Ryan Chen Create it
 *
 */


#ifdef  CONFIG_ANDROID_SYSTEM
/* Android will use double buffer in video if there is enough */
#define ANDROID_NUMBER_OF_BUFFERS 2
#else
#define ANDROID_NUMBER_OF_BUFFERS 1
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/mach/map.h>
#include <mach/regs-lcd.h>
static test_int;


#ifdef  CONFIG_24BPP_FRAMEBUFFER
#define CTRL_24BPP	SOCLE_LCD_CTRL0_24BPP
#else
#define CTRL_24BPP	SOCLE_LCD_CTRL0_COLOUR_GREEN
#endif


#ifdef CONFIG_7INCH_PANEL
#define PANEL_W		800
#define PANEL_H		480
#define H_FRONT_PORCH	255	//128//230
#define H_BACK_PORCH	87	//0
#define H_RESET_SYNC	12	//(256/4)
#define H_ACTIVE	PANEL_W
#define V_FRONT_PORCH	8	//16
#define V_BACK_PORCH	18	//1
#define V_RESET_SYNC	1	//(44/4)
#define V_ACTIVE	PANEL_H
#define PCLK_DIV	2

#elif defined(CONFIG_10INCH_PANEL)
#define PANEL_W		1024
#define PANEL_H		768
#define H_FRONT_PORCH	230
#define H_BACK_PORCH	0
#define H_RESET_SYNC	(256/4)
#define H_ACTIVE	PANEL_W
#define V_FRONT_PORCH	16
#define V_BACK_PORCH	1
#define V_RESET_SYNC	(44/4)
#define V_ACTIVE	PANEL_H
#define PCLK_DIV	2

#else //3.5 Inch
#define PANEL_W		320
#define PANEL_H		240
#define H_FRONT_PORCH	20
#define H_BACK_PORCH	36
#define H_RESET_SYNC	1
#define H_ACTIVE	PANEL_W
#define V_FRONT_PORCH	4
#define V_BACK_PORCH	15
#define V_RESET_SYNC	2
#define V_ACTIVE	PANEL_H

#ifdef  CONFIG_24BPP_FRAMEBUFFER
#define PCLK_DIV	11
#else
#define PCLK_DIV	7
#endif


#endif



struct soclefb_lcd_panel {
	struct fb_videomode	mode;
	signed short		width;	/* width in mm */
	signed short		height;	/* height in mm */
 	u32			htiming;
	u32			vtiming;
	u32			ctrl0;
	unsigned int		bpp:8,
				fixedtimings:1,
				grayscale:1;
};

struct soclefb_lcd_panel socle_lcd_panel = {
		.mode	= {
			.name			= "Ampire",
			.refresh		= 60,
			.xres			= PANEL_W,
			.yres			= PANEL_H,
			.pixclock		= 39721,
			.left_margin		= 38,
			.right_margin		= 20,
			.upper_margin		= 15,
			.lower_margin		= 5,
			.hsync_len		= 30,
			.vsync_len		= 3,
			.sync			= 0,
			.vmode			= FB_VMODE_NONINTERLACED,
		},
		.width  = PANEL_W,
		.height	= PANEL_H,	
 		.htiming	= (H_FRONT_PORCH << 24) | (H_BACK_PORCH << 16)  | (H_RESET_SYNC << 10) | PANEL_W,	
 		.vtiming	= (V_FRONT_PORCH << 24) | (V_FRONT_PORCH << 16) | (V_RESET_SYNC << 10) | PANEL_H,		
/*		
#ifdef CONFIG_7INCH_PANEL
 		//.htiming	= 0x32320b20,
		//.vtiming	= 0x0a0a09e0,
 		.htiming	= (230 << 24) | (0 << 16) | (256/4 << 10) | PANEL_W,	
 		.vtiming	= (16 << 24)  | (1 << 16) | (44/4 << 10)  | PANEL_H,
 		//.htiming	= 0x10570f20,				
		//.vtiming	= 0x081201e0,	
#elif defined(CONFIG_10INCH_PANEL)
 		//.htiming	= 0x32320b20,
		//.vtiming	= 0x0a0a09e0,
 		.htiming	= (230 << 24) | (0 << 16) | (256/4 << 10) | PANEL_W,	
 		.vtiming	= (16 << 24)  | (1 << 16) | (44/4 << 10)  | PANEL_H,
 		//.htiming	= 0x10570f20,				
		//.vtiming	= 0x081201e0,	
#else		
 		.htiming	= (0x14 << 24) | (0x24 << 16) | (1 << 10) | PANEL_W,	
 		.vtiming	= (0x04 << 24) | (0x0f << 16) | (2 << 10) | PANEL_H,
 		//.htiming	= 0x14242140,
		//.vtiming	= 0x040f04f0,
#endif
*/




#if 1	//BPP 32
		.ctrl0 = SOCLE_LCD_CTRL0_PXCLK_POLAR | (SOCLE_LCD_CTRL0_PCLOCK * PCLK_DIV)| SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC | CTRL_24BPP,

/*
//		.ctrl0 = (SOCLE_LCD_CTRL0_PCLOCK * 4)| SOCLE_LCD_CTRL0_DTMG |SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_24BPP,
//		.ctrl0 = (SOCLE_LCD_CTRL0_PCLOCK * 0xb)| SOCLE_LCD_CTRL0_DTMG |SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_24BPP,

//------------------------------------------------
#if defined(CONFIG_ARCH_PDK_PC7210)
#ifdef CONFIG_7INCH_PANEL
		.ctrl0 = (SOCLE_LCD_CTRL0_PCLOCK * 0x3)| SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_24BPP,
#else
		.ctrl0 = (SOCLE_LCD_CTRL0_PCLOCK * 0x7)| SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_24BPP,		
#endif

//------------------------------------------------
#elif defined(CONFIG_ARCH_PDK_PC9220)
#ifdef CONFIG_7INCH_PANEL
#ifdef  CONFIG_24BPP_FRAMEBUFFER
		.ctrl0 = SOCLE_LCD_CTRL0_PXCLK_POLAR | (SOCLE_LCD_CTRL0_PCLOCK * 3)| SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_24BPP,
#else
		.ctrl0 = SOCLE_LCD_CTRL0_PXCLK_POLAR | (SOCLE_LCD_CTRL0_PCLOCK * 3)| SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_COLOUR_GREEN,
#endif

#elif defined(CONFIG_10INCH_PANEL)
#ifdef  CONFIG_24BPP_FRAMEBUFFER
		.ctrl0 = SOCLE_LCD_CTRL0_PXCLK_POLAR | (SOCLE_LCD_CTRL0_PCLOCK * 3)| SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_24BPP,
#else
		.ctrl0 = SOCLE_LCD_CTRL0_PXCLK_POLAR | (SOCLE_LCD_CTRL0_PCLOCK * 3)| SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_COLOUR_GREEN,
#endif

#else
#ifdef  CONFIG_24BPP_FRAMEBUFFER
		.ctrl0 = SOCLE_LCD_CTRL0_PXCLK_POLAR | (SOCLE_LCD_CTRL0_PCLOCK * 11)| SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_24BPP,
#else
		.ctrl0 = SOCLE_LCD_CTRL0_PXCLK_POLAR | (SOCLE_LCD_CTRL0_PCLOCK * 4)| SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_COLOUR_GREEN,
#endif
#endif	

//------------------------------------------------
#elif defined(CONFIG_ARCH_LDK3V21)
		.ctrl0 = (SOCLE_LCD_CTRL0_PCLOCK * 4)| SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC | SOCLE_LCD_CTRL0_24BPP,
#else
//		.ctrl0 = (SOCLE_LCD_CTRL0_PCLOCK * 4)| SOCLE_LCD_CTRL0_DTMG |SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_24BPP,
		.ctrl0 = (SOCLE_LCD_CTRL0_PCLOCK * 0xb)| SOCLE_LCD_CTRL0_DTMG |SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC |SOCLE_LCD_CTRL0_24BPP,
#endif
*/
//------------------------------------------------
#ifdef  CONFIG_24BPP_FRAMEBUFFER
		.bpp	= 32,		// 16 or 24bpp support for lcd
#else
		.bpp	= 16,		// 16 or 24bpp support for lcd
#endif

#else
#ifdef CONFIG_ARCH_PDK_PC7210
		.ctrl0 =  SOCLE_LCD_CTRL0_COLOUR_RED | (SOCLE_LCD_CTRL0_PCLOCK * 0xa)| SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC ,
#else
		.ctrl0 =  SOCLE_LCD_CTRL0_COLOUR_RED | (SOCLE_LCD_CTRL0_PCLOCK * 4)| SOCLE_LCD_CTRL0_DTMG |SOCLE_LCD_CTRL0_HSYNC | SOCLE_LCD_CTRL0_VSYNC ,
#endif		
		.bpp	= 16,		// 16 or 24bpp support for lcd
#endif

};

struct soclefb_info {
	struct device *dev;
	struct fb_info		*fb;	
	struct soclefb_lcd_panel *socle_panel;

	/* raw memory addresses */

	dma_addr_t		map_dma;	/* physical */
	u_char *		map_cpu;	/* virtual */
#ifdef CONFIG_DOUBLE_BUFFER		
	dma_addr_t		map_dma1;	/* physical */
	u_char *		map_cpu1;	/* virtual */
#endif
	u_int			map_size;

	/* addresses of pieces placed in raw buffer */
	dma_addr_t		screen_dma;	/* physical address of buffer */
	unsigned int		palette_ready;
	
	struct resource *fb_res;
	struct resource *fb_req;

	struct resource *reg_res;
	struct resource *reg_req;

	void __iomem *fb_virt_addr;
	unsigned long fb_phys_addr;

	void __iomem *reg_virt_addr;
	unsigned long reg_phys_addr;

	u32 pseudo_palette[16];

};

/* Debugging stuff */

#define DEBUG

#ifdef DEBUG
static int debug	   = 1;
#else
static int debug	   = 0;
#endif

#define dprintk(msg...)	if (debug) { printk(KERN_DEBUG "fb: " msg); }

/* useful functions */

/* Pan the display if device supports it. */
static int socle_fb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
#if (ANDROID_NUMBER_OF_BUFFERS > 1)
	struct soclefb_info *fbi = info->par;
	u32 addr = var->yoffset * fbi->fb->fix.line_length + fbi->screen_dma;

	writel(addr,SOCLE_LCD_PAGE0_ADDR);
#endif

	return 0;
}



/*
 *      soclefb_set_par - Optional function. Alters the hardware state.
 *      @info: frame buffer structure that represents a single frame buffer
 *
 */
static int soclefb_set_par(struct fb_info *info)
{
	struct soclefb_info *fbi = info->par;
	struct fb_var_screeninfo *var = &info->var;

//	printk("SQfb_set_par \n");

	if (var->bits_per_pixel <= 8)
		fbi->fb->fix.visual = FB_VISUAL_PSEUDOCOLOR;
	else
		fbi->fb->fix.visual = FB_VISUAL_TRUECOLOR;

	fbi->fb->fix.line_length     = (var->xres*var->bits_per_pixel)/8;

	/* activate this new configuration */

 
	return 0;
}

static inline u_int chan_to_field(u_int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int soclefb_setcolreg(unsigned regno,
			       unsigned red, unsigned green, unsigned blue,
			       unsigned transp, struct fb_info *info)
{


	struct soclefb_info *fbi = info->par;
	unsigned int val;

//	printk("SQfb_setcolreg \n");
//	printk("setcol: regno=%d, rgb=%d,%d,%d\n", regno, red, green, blue);

	switch (fbi->fb->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		/* true-colour, use pseuo-palette */
//		printk("FB_VISUAL_TRUECOLOR \n");
		if (regno < 16) {
			u32 *pal = fbi->fb->pseudo_palette;

			val  = chan_to_field(red,   &fbi->fb->var.red);
			val |= chan_to_field(green, &fbi->fb->var.green);
			val |= chan_to_field(blue,  &fbi->fb->var.blue);

			pal[regno] = val;
		}
		break;

	case FB_VISUAL_PSEUDOCOLOR:
//		printk("FB_VISUAL_PSEUDOCOLOR \n");		
		if (regno < 256) {
			/* currently assume RGB 5-6-5 mode */

			val  = ((red   >>  0) & 0xf800);
			val |= ((green >>  5) & 0x07e0);
			val |= ((blue  >> 11) & 0x001f);
		}

		break;

	default:
		return 1;   /* unknown type */
	}

	return 0;
}


/**
 *      soclefb_blank
 *	@blank_mode: the blank mode we want.
 *	@info: frame buffer structure that represents a single frame buffer
 *
  *	Returns negative errno on error, or zero on success.
 *
 */
static int soclefb_blank(int blank_mode, struct fb_info *info)
{
//	dprintk("blank(mode=%d, info=%p)\n", blank_mode, info);
	//RGB halt 
	if(blank_mode == 1)
		writel(readl(SOCLE_LCD_CTRL0) | SOCLE_LCD_CTRL0_RGBHALT , SOCLE_LCD_CTRL0);
	else
		writel(readl(SOCLE_LCD_CTRL0) & ~SOCLE_LCD_CTRL0_RGBHALT , SOCLE_LCD_CTRL0);
	return 0;
}

static int soclefb_check_var(struct fb_var_screeninfo *var,
			       struct fb_info *info)
{
	struct soclefb_info *fbi = info->par;
//	printk("SQfb_check_var \n");
	/* validate x/y resolution */

	var->yres = fbi->fb->var.yres;
	var->xres = fbi->fb->var.xres;

	/* validate bpp */
/*
	if (var->bits_per_pixel > fbi->fb->var.bits_per_pixel)
		var->bits_per_pixel = fbi->fb->var.bits_per_pixel;
	else if (var->bits_per_pixel < fbi->fb->var.bits_per_pixel)
		var->bits_per_pixel = fbi->fb->var.bits_per_pixel;
*/
	/* set r/g/b positions */

	if (var->bits_per_pixel == 16) {
//		printk("16 pixel \n");
		var->blue.length	= 5;
		var->blue.offset	= 0;	
		var->green.length	= 6;
		var->green.offset	= var->blue.offset + var->blue.length;	
		var->red.length = 5;
		var->red.offset = var->green.offset + var->green.length;
		var->transp.length = 0;
		var->transp.offset = 0;
		
	} else {
//		printk("else 32 pixel = %d \n",var->bits_per_pixel);	

		var->blue.length	= 8;
		var->blue.offset	= 0;	
		var->green.length	= 8;
		var->green.offset	= var->blue.offset + var->blue.length;	
		var->red.length = 8;
		var->red.offset = var->green.offset + var->green.length;

		var->transp.length=8;
		var->transp.offset=24;

		
	}

	return 0;
}

#ifdef SOCLEFB_YUV
static ssize_t soclefb_yuv_write(struct file *file, const char *buf,
			    size_t count, loff_t *ppos)
{
/*
	unsigned long dst, start, end, len;
	int ret, i;

	if (count) {
		char *base_addr;

		base_addr = info->screen_base;
		count -= copy_from_user(base_addr + p, buf, count);
		*ppos += count;
		err = -EFAULT;
	}
	if (count)
		return count;

	return err;
*/
} 
#endif

	
#define DOUBLE_BUFFER_SW 1
#define YUV_MODE 2

static int socle_ioctl(struct fb_info *info, unsigned int cmd,unsigned long arg)
{

	struct soclefb_info *fbi = info->par;
	switch (cmd) {
#ifdef CONFIG_DOUBLE_BUFFER	
	case DOUBLE_BUFFER_SW:
		if (arg == 1) {
			writel(fbi->map_dma1,SOCLE_LCD_PAGE0_ADDR);

			fbi->fb->screen_base = fbi->map_cpu1;
			fbi->screen_dma = fbi->map_dma1;
			fbi->fb->fix.smem_start  = fbi->screen_dma;
			
//			printk("double_buffer_sw 1\n");
//			printk("map_video_memory: dma=%08x cpu=%p size=%08x\n",
//				fbi->map_dma1, fbi->map_cpu1, fbi->fb->fix.smem_len);
			return 0;
		} else {
			writel(fbi->map_dma,SOCLE_LCD_PAGE0_ADDR);


			fbi->fb->screen_base = fbi->map_cpu;
			fbi->screen_dma = fbi->map_dma;
			fbi->fb->fix.smem_start  = fbi->screen_dma;

//			printk("double_buffer_sw 0\n");
//			printk("map_video_memory: dma=%08x cpu=%p size=%08x\n",
//				fbi->map_dma, fbi->map_cpu, fbi->fb->fix.smem_len);
			return 0;
		
		}
#endif

	case YUV_MODE:
		if(arg==0)
			writel(readl(SOCLE_LCD_YUV2RGB_CTRL) & ~SOCLE_LCD_YUV2RGB_EN , SOCLE_LCD_YUV2RGB_CTRL);
		else {
			u32 y_addr,u_addr,v_addr;
			writel(readl(SOCLE_LCD_YUV2RGB_CTRL) | SOCLE_LCD_YUV2RGB_EN , SOCLE_LCD_YUV2RGB_CTRL);
			writel(readl(SOCLE_LCD_YUV2RGB_CTRL) & ~SOCLE_LCD_YUV422, SOCLE_LCD_YUV2RGB_CTRL);
			y_addr=fbi->map_dma;
			u_addr=y_addr+ (fbi->socle_panel->mode.xres*fbi->socle_panel->mode.yres);
			v_addr=u_addr+(fbi->socle_panel->mode.xres*fbi->socle_panel->mode.yres/4);
			writel(y_addr,SOCLE_LCD_Y_PAGE0_ADDR);
			writel(u_addr,SOCLE_LCD_Cb_PAGE0_ADDR);
			writel(v_addr,SOCLE_LCD_Cr_PAGE0_ADDR);
		}
		return 0;	
	default:
		printk("SQfb: error command");
		return -EINVAL;
	}
		
}


static struct fb_ops soclefb_ops = {
	.owner		= THIS_MODULE,
	.fb_check_var	= soclefb_check_var,
	.fb_set_par	= soclefb_set_par,
	.fb_blank	= soclefb_blank,
	.fb_setcolreg	= soclefb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
#ifdef SOCLEFB_YUV
	.fb_write	= soclefb_yuv_write,	
#endif
	.fb_ioctl = socle_ioctl,
#if (ANDROID_NUMBER_OF_BUFFERS > 1)
	.fb_pan_display = socle_fb_pan_display,
#endif
};

/*
 * soclefb_map_video_memory():
 *	Allocates the DRAM memory for the frame buffer.  This buffer is
 *	remapped into a non-cached, non-buffered, memory region to
 *	allow palette and pixel writes to occur without flushing the
 *	cache.  Once this area is remapped, all virtual memory
 *	access to the video memory should occur at the new region.
 */
static int __init soclefb_map_video_memory(struct soclefb_info *fbi)
{
//	dprintk("map_video_memory(fbi=%p)\n", fbi);

	fbi->map_size = PAGE_ALIGN(fbi->fb->fix.smem_len + PAGE_SIZE);
#ifdef CONFIG_DOUBLE_BUFFER
	fbi->map_cpu  = dma_alloc_writecombine(fbi->dev, fbi->map_size * 2,
					       &fbi->map_dma, GFP_KERNEL);
#else
	fbi->map_cpu  = dma_alloc_writecombine(fbi->dev, fbi->map_size,
					       &fbi->map_dma, GFP_KERNEL);
#endif

	fbi->map_size = fbi->fb->fix.smem_len;

	
	if (fbi->map_cpu) {
		/* prevent initial garbage on screen */
		printk("map_video_memory: clear %p:%08x: %08x\n",
			fbi->map_cpu, fbi->map_size,fbi->map_dma);
#ifdef CONFIG_DOUBLE_BUFFER		
		memset(fbi->map_cpu, 0, fbi->map_size * 2);
		fbi->screen_dma		= fbi->map_dma;
		fbi->fb->screen_base	= fbi->map_cpu;
		fbi->fb->fix.smem_start  = fbi->screen_dma;


		fbi->map_dma1 = fbi->map_dma + fbi->map_size;
		fbi->map_cpu1 = fbi->map_dma + fbi->map_size;

#else
		memset(fbi->map_cpu, 0, fbi->map_size);

		fbi->screen_dma		= fbi->map_dma;
		fbi->fb->screen_base	= fbi->map_cpu;
		fbi->fb->fix.smem_start  = fbi->screen_dma;

#endif

		dprintk("map_video_memory: dma=%08x cpu=%p size=%08x\n",
			fbi->map_dma, fbi->map_cpu, fbi->fb->fix.smem_len);
	}

	return fbi->map_cpu ? 0 : -ENOMEM;
}

static inline void soclefb_unmap_video_memory(struct soclefb_info *fbi)
{
#ifdef CONFIG_DOUBLE_BUFFER		
	dma_free_writecombine(fbi->dev,fbi->map_size * 2,fbi->map_cpu, fbi->map_dma);
#else
	dma_free_writecombine(fbi->dev,fbi->map_size,fbi->map_cpu, fbi->map_dma);
#endif
}

/*
 * soclefb_init_registers - Initialise all LCD-related registers
 */

static int soclefb_init_registers(struct soclefb_info *fbi)
{
//	printk("SQfb_init_registers \n");
	/* Initialise LCD with values from haret */

	//Set Panel timing
	writel(fbi->socle_panel->htiming, SOCLE_LCD_H_TIMING);
	writel(fbi->socle_panel->vtiming, SOCLE_LCD_V_TIMING);
	writel(fbi->socle_panel->ctrl0,SOCLE_LCD_CTRL0);

//	printk("fb addr : %x \n",fbi->screen_dma);
	writel(fbi->screen_dma,SOCLE_LCD_PAGE0_ADDR);


	/* Enable LCD controller */
	
	fbi->socle_panel->ctrl0 |= SOCLE_LCD_CTRL0_ENABLE;
	writel(fbi->socle_panel->ctrl0 , SOCLE_LCD_CTRL0);

//	printk("ctrl0 %x, addr: %x \n",readl(SOCLE_LCD_CTRL0),readl(SOCLE_LCD_PAGE0_ADDR));
	

	return 0;
}

static void soclefb_write_palette(struct soclefb_info *fbi)
{
/*
	unsigned int i;
	unsigned long ent;

	fbi->palette_ready = 0;

	for (i = 0; i < 256; i++) {
		if ((ent = fbi->palette_buffer[i]) == PALETTE_BUFF_CLEAR)
			continue;

		writel(ent, S3C2410_TFTPAL(i));


		if (readw(S3C2410_TFTPAL(i)) == ent)
			fbi->palette_buffer[i] = PALETTE_BUFF_CLEAR;
		else
			fbi->palette_ready = 1;   
	}
*/
}

static char driver_name[]="fb";

static int __init soclefb_probe(struct platform_device *pdev)
{
	struct soclefb_info *info;
	struct fb_info	   *fbinfo;
	int ret;
//	printk("SQfb_probe \n");

/*
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "no irq for device\n");
		return -ENOENT;
	}
*/
	fbinfo = framebuffer_alloc(sizeof(struct soclefb_info), &pdev->dev);
	if (!fbinfo) {
		return -ENOMEM;
	}

	info = fbinfo->par;
	info->fb = fbinfo;
	
	platform_set_drvdata(pdev, fbinfo);

	strcpy(fbinfo->fix.id, driver_name);

	info->socle_panel		= &socle_lcd_panel;

	fbinfo->fix.mmio_start	= SOCLE_AHB0_LCD;
	fbinfo->fix.mmio_len	= SZ_4K;

	fbinfo->fix.type	    	= FB_TYPE_PACKED_PIXELS;
	fbinfo->fix.type_aux	    = 0;
#ifndef  CONFIG_ANDROID_SYSTEM
	fbinfo->fix.xpanstep	    = 0;
	fbinfo->fix.ypanstep	    = 0;
	fbinfo->fix.ywrapstep	    = 0;
#else
#if (ANDROID_NUMBER_OF_BUFFERS > 1)
	fbinfo->fix.ypanstep	    = 1;
	fbinfo->fix.xpanstep =	1,
	fbinfo->fix.ywrapstep =	1,
#else
	fbinfo->fix.xpanstep	    = 0;
	fbinfo->fix.ypanstep	    = 0;
	fbinfo->fix.ywrapstep	    = 0;
#endif
	fbinfo->fix.visual =	FB_VISUAL_TRUECOLOR,

#endif
	fbinfo->fix.accel	    	= FB_ACCEL_NONE;

	fbinfo->var.nonstd	    = 0;
	fbinfo->var.activate	    = FB_ACTIVATE_FORCE;
	fbinfo->var.height	    = info->socle_panel->height;
	fbinfo->var.width	    = info->socle_panel->width;
	fbinfo->var.accel_flags     = 0;
	fbinfo->var.vmode	    = FB_VMODE_NONINTERLACED;

	fbinfo->fbops		    = &soclefb_ops;
	fbinfo->flags		    = FBINFO_FLAG_DEFAULT;
	fbinfo->pseudo_palette      = info->pseudo_palette;	//?????

	fbinfo->var.xres	    = info->socle_panel->mode.xres;
	fbinfo->var.xres_virtual    = info->socle_panel->mode.xres;
	fbinfo->var.yres	    = info->socle_panel->mode.yres;
#ifndef  CONFIG_ANDROID_SYSTEM
	fbinfo->var.yres_virtual    = info->socle_panel->mode.yres;;
#else
	fbinfo->var.yres_virtual    = info->socle_panel->mode.yres * ANDROID_NUMBER_OF_BUFFERS;;
#endif
	fbinfo->var.bits_per_pixel  = info->socle_panel->bpp;


	fbinfo->var.grayscale	= info->socle_panel->grayscale;
	fbinfo->var.pixclock	= info->socle_panel->mode.pixclock;

	fbinfo->var.left_margin	= info->socle_panel->mode.left_margin;
	fbinfo->var.right_margin	= info->socle_panel->mode.right_margin;
	fbinfo->var.upper_margin	= info->socle_panel->mode.upper_margin;
	fbinfo->var.lower_margin	= info->socle_panel->mode.lower_margin;

	fbinfo->var.hsync_len	= info->socle_panel->mode.hsync_len;
	fbinfo->var.vsync_len	= info->socle_panel->mode.vsync_len;
	fbinfo->var.sync		= info->socle_panel->mode.sync;
	fbinfo->var.vmode		= info->socle_panel->mode.vmode;

#ifndef  CONFIG_ANDROID_SYSTEM
	fbinfo->fix.smem_len	= info->socle_panel->mode.xres * info->socle_panel->mode.yres * info->socle_panel->bpp / 8; //for allocate mem
#else
	fbinfo->fix.smem_len	= info->socle_panel->mode.xres * info->socle_panel->mode.yres * info->socle_panel->bpp / 8 * ANDROID_NUMBER_OF_BUFFERS; //for allocate mem
#endif
	
	fbinfo->monspecs.hfmin	= 0;
	fbinfo->monspecs.hfmax   = 100000;
	fbinfo->monspecs.vfmin	= 0;
	fbinfo->monspecs.vfmax	= 400;
	fbinfo->monspecs.dclkmin = 1000000;
	fbinfo->monspecs.dclkmax	= 100000000;

	/*
	 * Allocate colourmap.
	 */
	fb_alloc_cmap(&(fbinfo->cmap), 256, 0);

/*
	if (!request_mem_region((unsigned long)SOCLE_AHB0_LCD, SZ_16K, "fb-lcd")) {
		ret = -EBUSY;
		goto dealloc_fb;
	}


	ret = request_irq(IRQ_CLCD, soclefb_irq, IRQF_SHARED, "fb", info);
	if (ret) {
		printk("Can't request LCD irq");
		ret = -EBUSY;
		goto free_video_memory;
	}
*/

	/* Initialize video memory */
	ret = soclefb_map_video_memory(info);
	if (ret) {
		printk( KERN_ERR "Failed to allocate video RAM: %d\n", ret);
		goto free_video_memory;
	}

	ret = soclefb_init_registers(info);

	//20080702 JS add for mplayer play
	ret = soclefb_check_var(&fbinfo->var, fbinfo);
	
	ret = soclefb_set_par(fbinfo);
	
	ret = register_framebuffer(fbinfo);
	if (ret < 0) {
		printk(KERN_ERR "Failed to register framebuffer device: %d\n", ret);
		goto free_video_memory;
	}

	printk(KERN_INFO "fb%d: %s frame buffer device\n",
		fbinfo->node, fbinfo->fix.id);

	return 0;

free_video_memory:
	soclefb_unmap_video_memory(info);
//release_irq:
	//free_irq(IRQ_CLCD,info);
dealloc_fb:
	framebuffer_release(fbinfo);
	return ret;

}

/* soclefb_stop_lcd
 *
 * shutdown the lcd controller
*/

static void soclefb_stop_lcd(struct soclefb_info *fbi)
{
/*
	unsigned long flags;

	local_irq_save(flags);

	writel(readl(SOCLE_LCD_CTRL0) & (~SOCLE_LCD_CTRL0_ENABLE), SOCLE_LCD_CTRL0);

	local_irq_restore(flags);
*/
}

/*
 *  Cleanup
 */
static int soclefb_remove(struct platform_device *pdev)
{
	printk("SQfb_remove \n");
/*
	struct fb_info	   *fbinfo = platform_get_drvdata(pdev);
	struct soclefb_info *info = fbinfo->par;
	int irq;

	soclefb_stop_lcd(info);
	msleep(1);

	soclefb_unmap_video_memory(info);

 	if (info->clk) {
 		clk_disable(info->clk);
 		clk_put(info->clk);
 		info->clk = NULL;
	}

	irq = platform_get_irq(pdev, 0);
	free_irq(irq,info);
	release_mem_region((unsigned long)S3C24XX_VA_LCD, S3C24XX_SZ_LCD);
	unregister_framebuffer(fbinfo);
*/
	return 0;
}

#ifdef CONFIG_PM

/* suspend and resume support for the lcd controller */

static int soclefb_suspend(struct platform_device *dev, pm_message_t state)
{

	printk("SQfb_suspend \n");
/*
	struct fb_info	   *fbinfo = platform_get_drvdata(dev);
	struct soclefb_info *info = fbinfo->par;

	soclefb_stop_lcd(info);

	msleep(1);
	clk_disable(info->clk);
*/
	return 0;
}

static int soclefb_resume(struct platform_device *dev)
{
	printk("SQfb_resume \n");
/*
	struct fb_info	   *fbinfo = platform_get_drvdata(dev);
	struct soclefb_info *info = fbinfo->par;

	clk_enable(info->clk);
	msleep(1);

	soclefb_init_registers(info);
*/
	return 0;
}

#else
#define soclefb_suspend NULL
#define soclefb_resume  NULL
#endif

static struct platform_driver soclefb_driver = {
	.probe		= soclefb_probe,
	.remove		= soclefb_remove,
	.suspend		= soclefb_suspend,
	.resume		= soclefb_resume,
	.driver		= {
		.name	= "socle-lcd",
		.owner	= THIS_MODULE,
	},
};

int __devinit soclefb_init(void)
{
	return platform_driver_register(&soclefb_driver);
}

static void __exit soclefb_cleanup(void)
{
	platform_driver_unregister(&soclefb_driver);
}


module_init(soclefb_init);
module_exit(soclefb_cleanup);

MODULE_AUTHOR("Ryan Chen");
MODULE_DESCRIPTION("Framebuffer driver for the Socle LCD");
MODULE_LICENSE("GPL");

/********************************************************************************
* File Name     : drivers/char/socle-lcd-lighter.c
* Author        : cyli
* Description   : Socle LCD Lighter Driver
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

*   Version      : 2,0,0,1
*   History      :
*      1. 2007/03/22 cyli create this file
*
********************************************************************************/

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <asm/uaccess.h> //copy from/to user
#include <linux/ioctl.h>

#include <mach/regs-pwmt.h>
#include "socle-lcd-lighter.h"


//#define SOCLE_LCD_DEBUG
#ifdef SOCLE_LCD_DEBUG
#define LCD_DBG(fmt, args...) printk(KERN_DEBUG "SQ_LCD: " fmt, ## args)
#else
#define LCD_DBG(fmt, args...)
#endif


static int socle_lcd_major = SOCLE_LCD_MAJOR;
static int socle_lcd_minor = SOCLE_LCD_MINOR;
module_param(socle_lcd_major, int, 0644);
module_param(socle_lcd_minor, int, 0644);


static int cur_hrc;
static int cur_lrc;
static DEFINE_SPINLOCK(lcd_lock);


static struct socle_lcd_lighter {
	int busy;
	dev_t lcd_devt;
	struct class *class;
	struct class_device *class_dev;
	struct cdev *cdev;
	struct socle_pwmt *p_pwm;
} socle_lcd;

static char __initdata banner[] = "SQ LCD Lighter, (c) 2007 SQ Corp.\n";

static int     socle_lcd_ioctl   (struct inode *inodep, struct file *filep, unsigned int cmd, unsigned long arg);
static int     socle_lcd_open    (struct inode *inodep, struct file *filep);
static int     socle_lcd_release (struct inode *inodep, struct file *filep);


static struct file_operations socle_lcd_fops = {
   	  .owner = THIS_MODULE,
	  .ioctl = socle_lcd_ioctl,
	   .open = socle_lcd_open,
	.release = socle_lcd_release
};


static void socle_lcd_init_by_pwm(struct socle_lcd_lighter *p_lcd);


static int
socle_lcd_ioctl(struct inode *inodep, struct file *filep, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct socle_pwmt_driver *pwm_drv = socle_lcd.p_pwm->drv;

	switch (cmd) {
	case SOCLE_LCD_LT_RST:
		LCD_DBG("SQ_LCD_LT_RST\n");
		socle_lcd_init_by_pwm(&socle_lcd);
		break;

	case SOCLE_LCD_LT_DEC:
		LCD_DBG("SQ_LCD_LT_DEC\n");
		cur_hrc--;
		break;

	case SOCLE_LCD_LT_INC:
		LCD_DBG("SQ_LCD_LT_INC\n");
		cur_hrc++;
		break;

	default:
		return -ENOIOCTLCMD;
	}

	
	if (cur_hrc == 0) {
		cur_hrc++;
		printk("Can't decrease bright!\n");
	} else if (cur_hrc == cur_lrc) {
		cur_hrc--;
		printk("Can't increase bright!\n");
	} else {
		pwm_drv->claim_pwm_lock();

		pwm_drv->write_hrc(socle_lcd.p_pwm, cur_hrc);

		pwm_drv->release_pwm_lock();
	}
	
	LCD_DBG("Current HRC = 0x%08x, HLC = 0x%08x\n", cur_hrc, cur_lrc);
	
	return ret;
}

static int
socle_lcd_open(struct inode *inodep, struct file *filep)
{
	spin_lock(lcd_lock);

	if (socle_lcd.busy)
		return -EBUSY;

	LCD_DBG("sq_lcd_open\n");

	socle_lcd.busy = 1;

	spin_unlock(lcd_lock);

	return 0;
}

static int
socle_lcd_release(struct inode *inodep, struct file *filep)
{
	spin_lock(lcd_lock);

	LCD_DBG("sq_lcd_release\n");

	socle_lcd.busy = 0;

	spin_unlock(lcd_lock);

	return 0;
}



static void
socle_lcd_init_by_pwm(struct socle_lcd_lighter *p_lcd)
{
	struct socle_pwmt_driver *pwm_drv = p_lcd->p_pwm->drv;
	
	cur_hrc = DEFAULT_HRC;
	cur_lrc = DEFAULT_LRC;

	pwm_drv->claim_pwm_lock();

	pwm_drv->reset(p_lcd->p_pwm);
	pwm_drv->write_prescale_factor(p_lcd->p_pwm, DEFAULT_PRE_SCL);
	pwm_drv->write_hrc(p_lcd->p_pwm, cur_hrc);
	pwm_drv->write_lrc(p_lcd->p_pwm, cur_lrc);
	pwm_drv->output_enable(p_lcd->p_pwm, 1);
	pwm_drv->enable(p_lcd->p_pwm, 1);

	pwm_drv->release_pwm_lock();
}

static struct miscdevice socle_lcd_lighter_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "sq_lcd_lighter",
	.fops = &socle_lcd_fops
};

static int __init
socle_lcd_init(void)
{
	int ret = 0;
	
	printk(banner);
	
	socle_lcd.p_pwm = get_socle_pwmt_structure(USE_PWM_NUM);
	if (NULL == socle_lcd.p_pwm) {
		printk("SQ LCD lighter can't get PWMT structure!!\n");
		return -ENXIO;
	}

//	socle_lcd_setup_cdev(&socle_lcd);

	ret = misc_register(&socle_lcd_lighter_dev);
	
	socle_lcd_init_by_pwm(&socle_lcd);

	return ret;
}


static void __exit
socle_lcd_cleanup(void)
{
        misc_deregister( &socle_lcd_lighter_dev );
	release_socle_pwmt_structure(USE_PWM_NUM);
}

module_init(socle_lcd_init);
module_exit(socle_lcd_cleanup);


MODULE_DESCRIPTION("SQ LCD Lighter Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CY Li <cyli@socle-tech.com.tw>");

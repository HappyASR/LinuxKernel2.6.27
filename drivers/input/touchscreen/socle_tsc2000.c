/********************************************************************************
* File Name     : drivers/input/touchscreen/socle_tsc2000.c 
* Author         : ryan chen
* Description   : Socle  Touch Screen Driver
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
    
*   Version      : a.0
*   History      : 
*      1. 2007/05/26 ryan chen create this file 
*      2. 2007/11/27 cyli modify this file to use gpio share irq
*      3. 2008/01/24 cy tang modify this file for touch release issue
********************************************************************************/

//#define TS_DEBUG

#ifdef TS_DEBUG
#define DEBUG printk
#else
#define DEBUG 
#endif


#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/spi/spi.h>

#include <asm/io.h>
#include <asm/delay.h>

#include <mach/hardware.h>
#include <mach/platform.h>

#include <mach/gpio.h>

//#include <asm/semaphore.h>
#include <linux/semaphore.h>

#define MODNAME "sq_ts_input"

struct ts_tsc2000 {
	struct spi_device *spi;
	struct semaphore sem;
	struct input_dev *socle_ts_dev;	
	u32 size;
	u8	touch;	
	//20080104 add for support gpio irq and spi tx/rx len different
	int ts_irq;
	
};

struct ts_tsc2000 touchscreen;

#define SET_TX_RX_LEN(tx, rx)	(((tx) << 16) | (rx))

//TS2000 Tocuhscreen define


#ifndef CONFIG_ANDROID_SYSTEM
  #define BIT_RES 10
//ADC
//BIT15 BIT14 BIT13- 10   BIT 9-8  BIT 7-6  BIT 5-4   BIT 3-1    BIT 0
//PSM   STS   AD3  - AD0  RS1-RS0  AV1-AV0  CL1-CL0   PV2 -PV0   X
//1     0     0010        01       11       11        000        0       //0x89f0 8bit
//1     0     0010        10       11       11        000        0       //0x8af0 10bit 
//1     0     0010        11       11       11        000        0       //0x8bf0 12bit 

  #if (BIT_RES == 12)
	#define TS_ABS_X_MIN	0
	#define TS_ABS_X_MAX	4095
	#define TS_ABS_Y_MIN	0
	#define TS_ABS_Y_MAX	4095
	#define PRESS_MAX	4095
	#define TSC2000_BIT_MASK	0xfff
	#define PG1_SET_VALUE	0x8bf0
  #elif (BIT_RES == 10)
	#define TS_ABS_X_MIN	0
	#define TS_ABS_X_MAX	1023
	#define TS_ABS_Y_MIN	0
	#define TS_ABS_Y_MAX	1023
	#define PRESS_MAX	1023
	#define TSC2000_BIT_MASK	0x3ff
	#define PG1_SET_VALUE	0x8af0
  #elif (BIT_RES == 8)
	#define TS_ABS_X_MIN	0
	#define TS_ABS_X_MAX	255
	#define TS_ABS_Y_MIN	0
	#define TS_ABS_Y_MAX	255
	#define PRESS_MAX	255
	#define TSC2000_BIT_MASK	0xff
	#define PG1_SET_VALUE	0x89f0
  #endif
#else //manual calibration
//#define TS_ABS_X_MIN	0xf
//#define TS_ABS_X_MAX	0xe8
//#define TS_ABS_Y_MIN	7
//#define TS_ABS_Y_MAX	0xf4
#define TS_ABS_X_MIN	0
#define TS_ABS_X_MAX	320
#define TS_ABS_Y_MIN	0
#define TS_ABS_Y_MAX	240
#define TSC2000_BIT_MASK	0xff
#define PG1_SET_VALUE	0x89f0
#endif
#define TOUCH_MAX_DIFF	(TSC2000_BIT_MASK >> 4)	

#define IS_PEN_DOWN(x)  			( x & (1<<15) )
#define TS_PAGE1_PREFIX 			(0x0800)
#define TS_REG_OFFSET   				(5)
#define TS_ADC_DATA_READY       		(1<<14)
#define IS_TS_ADC_DATA_READY(x) 	(x & TS_ADC_DATA_READY)

//TSC2000 CMD WORD
#define READ_CMD 			0x8000
#define WRITE_CMD			0x7fff
#define ADDR_OFFSET		(5)


//TSC2000 PAGE ADDRESS
#define PG0 		0x0000
#define PG1		0x0800

static int old_absx, old_absy;
static int release_count;

static void do_touch (struct work_struct *work);

static DECLARE_DELAYED_WORK(dwork, do_touch);

static int sq_write_data(struct ts_tsc2000 *touchscreen, u16 page, u16 addr, u16 data)
{
/*  
Bit 7 start
Bit 6-4 A2 ~ A0
Bit 3-2 Mode1-0 
Bit 1-0 PD1-0
*/ 
  
	int err = 0;
	struct spi_message msg;
	struct spi_transfer xfer;
	u16 tx_buf[2] = {0};

	dev_dbg(&touchscreen->spi->dev, "sq_write_data()\n");

	tx_buf[0] = WRITE_CMD & ((addr<<ADDR_OFFSET) | page);
	tx_buf[1] = data;

	//spi_setup(touchscreen->spi);		//20080417 JS mask
	spi_message_init(&msg);
	memset(&xfer, 0, sizeof(struct spi_transfer));
//	xfer.speed_hz = 300;
	xfer.tx_buf = tx_buf;
//	xfer.rx_buf = &stat;
	xfer.len = SET_TX_RX_LEN(2, 0);

//	xfer.len = 2;

	spi_message_add_tail(&xfer, &msg);
	err = spi_sync(touchscreen->spi, &msg);
	
	return err;

}

static int tsc2000_write_data(struct ts_tsc2000 *touchscreen, u16 page, u16 addr, u16 data)
{
	int err = 0;
	struct spi_message msg;
	struct spi_transfer xfer;
	u16 tx_buf[2] = {0};

	dev_dbg(&touchscreen->spi->dev, "tsc2000_write_data()\n");

	tx_buf[0] = WRITE_CMD & ((addr<<ADDR_OFFSET) | page);
	tx_buf[1] = data;

	//spi_setup(touchscreen->spi);		//20080417 JS mask
	spi_message_init(&msg);
	memset(&xfer, 0, sizeof(struct spi_transfer));
//	xfer.speed_hz = 300;
	xfer.tx_buf = tx_buf;
//	xfer.rx_buf = &stat;
	xfer.len = SET_TX_RX_LEN(2, 0);

//	xfer.len = 2;

	spi_message_add_tail(&xfer, &msg);
	err = spi_sync(touchscreen->spi, &msg);
	
	return err;

}

static int tsc2000_read_data(struct ts_tsc2000 *touchscreen, u16 page, u16 addr, u16 *buf, unsigned xferlen)
{
	int err = 0;
	struct spi_message msg;
	struct spi_transfer xfer;
	u16 cmd=0;

	//dev_dbg(&touchscreen->spi->dev, "tsc2000_read_data()\n");	
	//spi_setup(touchscreen->spi);			//20080417 JS mask
	spi_message_init(&msg);
	memset(&xfer, 0, sizeof(struct spi_transfer));
	
	cmd =  READ_CMD | (addr<<ADDR_OFFSET) | page;
	
	xfer.tx_buf = &cmd;
	xfer.rx_buf = buf;
//	xfer.len = xferlen;
	xfer.len = SET_TX_RX_LEN(1, xferlen);
	spi_message_add_tail(&xfer, &msg);
	err = spi_sync(touchscreen->spi, &msg);
	
	return err;
	
}




static void do_touch (struct work_struct *work)
{
	static int pass_c = 0;
	static int absx = 0;
	static int absy = 0;
	static int absz1 = 0;
	static int absz2 = 0;
	static int absz = 0,diff_x,diff_y;			/* absz is a pressure factor */
	u16 buf[4] = {0};


	if (touchscreen.touch) {
	spi_setup(touchscreen.spi);			//20080417 JS Add

	tsc2000_read_data(&touchscreen, PG0, 0, buf, 1);	/* page 0, regaddr=0 */
	absy = buf[0] & TSC2000_BIT_MASK;		
	tsc2000_read_data(&touchscreen, PG0, 1, buf, 1);	/* page 0, regaddr=0 */
	absx = buf[0] & TSC2000_BIT_MASK;		


	tsc2000_read_data(&touchscreen, PG0, 2, buf, 1);	/* page 0, regaddr=0 */			
	absz1 = buf[0] & 0xfff;		/* z1 use 12 bit resolution */
	tsc2000_read_data(&touchscreen, PG0, 3, buf, 1);	/* page 0, regaddr=0 */			
	absz2 = buf[0] & 0xfff;		/* z2 use 12 bit resolution */
	//DEBUG("x: %x, y: %x, z: %x, z: %x \n",absx,absy,absz1,absz2);
	DEBUG(".");
		if (absz1) {
			absz = absx * ((absz2/absz1) -1 );
			//absz = (200/4069) * absx * ((absz2/absz1) -1 );
			//DEBUG("x: %x, y: %x, z: %x, z: %x Z:%x\n",absx,absy,absz1,absz2,absz);	
			pass_c++;
		}
		else {
#if defined CONFIG_ARCH_P7DK
				socle_enable_gpio_irq(touchscreen.ts_irq);
#else
				enable_irq (touchscreen.ts_irq);
#endif
			DEBUG("return\n");
			return;
		}
		if(pass_c == 1) {
			old_absx=absx;
			old_absy=absy;
		}

		if ((absx != old_absx)&(absy != old_absy)&(pass_c > 1)) { 			
			input_report_key (touchscreen.socle_ts_dev, BTN_TOUCH, 1);	
			
			diff_x = abs(absx-old_absx);
			diff_y = abs(absy-old_absy);
			printk("\nreport_abs x:%4d, y:%4d - ",absx,absy);
			printk("diff-x:%4d, y:%4d",diff_x,diff_y);
			
			if ((diff_x > TOUCH_MAX_DIFF) | (diff_y > TOUCH_MAX_DIFF)) {
				if(diff_x > TOUCH_MAX_DIFF) {
					if( absx > old_absx)
						absx =old_absx + TOUCH_MAX_DIFF;
					else
						absx = old_absx - TOUCH_MAX_DIFF;					
				}

				if(diff_y > TOUCH_MAX_DIFF) {
					if( absy > old_absy)
						absy = old_absy + TOUCH_MAX_DIFF;
					else
						absy = old_absy - TOUCH_MAX_DIFF;
				}
				printk("-modify x:%4d, y:%4d ",absx,absy);
			}



#ifndef CONFIG_ANDROID_SYSTEM

			input_report_abs (touchscreen.socle_ts_dev, ABS_X, absx);
			input_report_abs (touchscreen.socle_ts_dev, ABS_Y, abs (absy - TS_ABS_Y_MAX));
#else
			// calibration in driver
			input_report_abs (touchscreen.socle_ts_dev, ABS_X, abs(absy*320/(0xff)));
			input_report_abs (touchscreen.socle_ts_dev, ABS_Y, abs (240-absx*240/(0xff)));									
#endif

			input_report_abs (touchscreen.socle_ts_dev, ABS_PRESSURE, absz);	

			input_sync (touchscreen.socle_ts_dev);


			old_absx=absx;
			old_absy=absy;
		}
		touchscreen.touch =0;
#if defined CONFIG_ARCH_P7DK
		socle_enable_gpio_irq(touchscreen.ts_irq);
#else
		enable_irq (touchscreen.ts_irq);
#endif
		schedule_delayed_work (&dwork, HZ / HZ);
	}
	else
	{
		release_count++;	
		if(release_count > 5)
		{
			DEBUG("release\n");	
			input_report_key (touchscreen.socle_ts_dev, BTN_TOUCH, 0);			
			input_report_abs (touchscreen.socle_ts_dev, ABS_PRESSURE, 0);
			input_sync (touchscreen.socle_ts_dev);
			pass_c = 0;
			old_absx = 0;
			old_absy = 0;
			return;
		}
		schedule_delayed_work (&dwork, HZ / HZ);
	}
}

static irqreturn_t socle_ts_interrupt (int irq, void *dev, struct pt_regs *regs)
{
	disable_irq_nosync (irq);
	schedule_delayed_work (&dwork, HZ / HZ);
	touchscreen.touch =1;
	return IRQ_HANDLED;
}

static int __devinit ts_tsc2000_probe(struct spi_device *spi)
{
	//dev_dbg(&spi->dev, "ts_tsc2000_probe()\n");	
	memset(&touchscreen, 0, sizeof(struct ts_tsc2000));
	init_MUTEX(&touchscreen.sem);
	touchscreen.spi = spi;
	spi->bits_per_word = 16;
	
	touchscreen.socle_ts_dev = input_allocate_device ();

	if (!touchscreen.socle_ts_dev)
		return -ENOMEM;	
	touchscreen.socle_ts_dev->evbit[0] = BIT (EV_ABS) | BIT (EV_KEY);
	touchscreen.socle_ts_dev->absbit[0] = BIT (ABS_X) | BIT (ABS_Y) | BIT (ABS_PRESSURE);
	touchscreen.socle_ts_dev->keybit[BIT_WORD (BTN_TOUCH)] = BIT_MASK (BTN_TOUCH);

	touchscreen.socle_ts_dev->absmin[ABS_X] = TS_ABS_X_MIN;
	touchscreen.socle_ts_dev->absmin[ABS_Y] = TS_ABS_Y_MIN;
	touchscreen.socle_ts_dev->absmax[ABS_X] = TS_ABS_X_MAX;
	touchscreen.socle_ts_dev->absmax[ABS_Y] = TS_ABS_Y_MAX;
	input_set_abs_params(touchscreen.socle_ts_dev, ABS_PRESSURE, 0, PRESS_MAX, 0, 0); //PRESS_MAX = 255

	touchscreen.socle_ts_dev->name = "ts_tsc2000";
	touchscreen.socle_ts_dev->phys = "sq_ts/input0";
	touchscreen.touch =0;

	// 20080925 Ryan modify
	touchscreen.ts_irq = spi->irq;

	//20080417 JS Add
	spi_setup(touchscreen.spi);				
	
	/* Setup TSC2000 */
	tsc2000_write_data(&touchscreen, PG1, 0x4, 0xbb00);	//RESET
	tsc2000_write_data(&touchscreen, PG1, 0x5, 0x3f);	//CONFIG

//ADC
//BIT15 BIT14 BIT13- 10   BIT 9-8  BIT 7-6  BIT 5-4   BIT 3-1    BIT 0
//PSM   STS   AD3  - AD0  RS1-RS0  AV1-AV0  CL1-CL0   PV2 -PV0   X
//1     0     0010        01       11       11        000        0       //0x89f0 8bit
//1     0     0010        11       11       11        000        0       //0x8bf0 12bit 
	tsc2000_write_data(&touchscreen, PG1, 0x0, PG1_SET_VALUE);
	
	if(input_register_device (touchscreen.socle_ts_dev))
		printk("input_register_device() failed!!!\n");
#if defined (CONFIG_ARCH_P7DK) 
	//cyli 20071127 modify

	if (socle_request_gpio_irq (touchscreen.ts_irq, (irq_handler_t) socle_ts_interrupt, GPIO_INT_SENSE_LEVEL | GPIO_INT_SINGLE_EDGE | GPIO_INT_EVENT_LO, MODNAME, NULL)) {
		printk (KERN_ERR "sq_touchscreen.c: Can't allocate pin[%d]\n", touchscreen.ts_irq);
		input_unregister_device (touchscreen.socle_ts_dev);
	  	return -EBUSY;
	}
#elif defined(CONFIG_ARCH_PDK_PC7210)
	if (socle_request_gpio_irq (touchscreen.ts_irq, (irq_handler_t) socle_ts_interrupt, GPIO_INT_SENSE_LEVEL | GPIO_INT_SINGLE_EDGE | GPIO_INT_EVENT_LO, MODNAME, NULL)) {
		printk (KERN_ERR "sq_touchscreen.c: Can't allocate pin[%d]\n", touchscreen.ts_irq);
		input_unregister_device (touchscreen.socle_ts_dev);
	  	return -EBUSY;
	}
#else //INCLUDE , CDK , PC9002, pc9220
	if (request_irq (touchscreen.ts_irq, (irq_handler_t) socle_ts_interrupt, IRQF_DISABLED, MODNAME, NULL) < 0)
	{
		printk (KERN_ERR "sq_touchscreen.c: Can't allocate irq %d\n", touchscreen.ts_irq);
		input_unregister_device (touchscreen.socle_ts_dev);
	  	return -EBUSY;
	}
#endif
	dev_set_drvdata(&spi->dev, &touchscreen);
	return 0;
	
}

static int __devexit ts_tsc2000_remove(struct spi_device *spi)
{

	//dev_dbg(&spi->dev, "ts_tsc2000_remove()\n");
#if defined CONFIG_ARCH_P7DK
	socle_free_gpio_irq(touchscreen.ts_irq, NULL);
#else
	free_irq(touchscreen.ts_irq, NULL);
#endif
	cancel_delayed_work (&dwork);
	flush_scheduled_work ();
	input_unregister_device (touchscreen.socle_ts_dev);

	dev_set_drvdata(&spi->dev, NULL);
	return 0;
}

static int ts_tsc2000_suspend(struct spi_device *spi)
{
	return 0;
}
static int ts_tsc2000_resume(struct spi_device *spi)
{
	return 0;
}
static struct spi_driver ts_tsc2000_driver = {
	.driver = {
		.name = "ts_tsc2000",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
	},
	.probe = ts_tsc2000_probe,
	.remove = __devexit_p(ts_tsc2000_remove),
	.suspend	= ts_tsc2000_suspend,
	.resume		= ts_tsc2000_resume,
};

static int __init ts_tsc2000_init(void)
{
	return spi_register_driver(&ts_tsc2000_driver);
}
module_init(ts_tsc2000_init);

static void __exit ts_tsc2000_exit(void)
{
	spi_unregister_driver(&ts_tsc2000_driver);
}
module_exit(ts_tsc2000_exit);

MODULE_DESCRIPTION("SQ TSC2000");
MODULE_AUTHOR("Ryan Chen");
MODULE_LICENSE("GPL");


/********************************************************************************
* File Name     : drivers/input/touchscreen/socle_ar7646.c 
* Author         : ryan chen
* Description   : Socle  Touch Screen Driver
* 
* Copyright (C) Socle Tech. Corp.
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

#define TS_DEBUG

#ifdef TS_DEBUG
#define DEBUG printk
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

#define MODNAME "socle_ts_input"

struct ts_ar7646 {
	struct spi_device *spi;
	struct semaphore sem;
	struct input_dev *socle_ts_dev;	
	u32 size;
	u8	touch;	
	//20080104 add for support gpio irq and spi tx/rx len different
	int ts_irq;	
};

struct ser_req {
	u8			ref_on;
	u8			command;
	u8			ref_off;
	u16			scratch;
	__be16			sample;
	struct spi_message	msg;
	struct spi_transfer	xfer[6];
};


struct ts_ar7646 touchscreen;
static struct spi_device *spi_tmp;


#define SET_TX_RX_LEN(tx, rx)	(((tx) << 16) | (rx))

//TS2000 Tocuhscreen define

#ifndef CONFIG_ANDROID_SYSTEM
#define TS_ABS_X_MIN	0
#define TS_ABS_X_MAX	255
#define TS_ABS_Y_MIN	0
#define TS_ABS_Y_MAX	255
#else //manual calibration
#define TS_ABS_X_MIN	0
#define TS_ABS_X_MAX	320
#define TS_ABS_Y_MIN	0
#define TS_ABS_Y_MAX	240
#endif

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


/*--------------------------------------------------------------------------*/

/* The AR7646 has touchscreen and other sensors.
 * Earlier ar784x chips are somewhat compatible.
 */
#define	AR_START		(1 << 7)
#define	AR_A2A1A0_d_y		(1 << 4)	/* differential */
#define	AR_A2A1A0_d_z1		(3 << 4)	/* differential */
#define	AR_A2A1A0_d_z2		(4 << 4)	/* differential */
#define	AR_A2A1A0_d_x		(5 << 4)	/* differential */
#define	AR_A2A1A0_temp0		(0 << 4)	/* non-differential */
#define	AR_A2A1A0_vbatt		(2 << 4)	/* non-differential */
#define	AR_A2A1A0_vaux		(6 << 4)	/* non-differential */
#define	AR_A2A1A0_temp1		(7 << 4)	/* non-differential */
#define	AR_8_BIT		(1 << 3)
#define	AR_12_BIT		(0 << 3)
#define	AR_VREF			(1 << 2)	/* non-differential */
#define	AR_VDD			(0 << 2)	/* differential */
#define	AR_PD10_PDOWN		(0 << 0)	/* lowpower mode + penirq */
#define	AR_PD10_ADC_ON		(1 << 0)	/* ADC on */
#define	AR_PD10_REF_ON		(2 << 0)	/* vREF on + penirq */
#define	AR_PD10_ALL_ON		(3 << 0)	/* ADC + vREF on */

#define	MAX_12BIT		((1<<12)-1)

/* leave ADC powered up (disables penirq) between differential samples */
#define	READ_12BIT_VDD(x, adc, vref) (AR_START | AR_A2A1A0_d_ ## x \
	| AR_12_BIT | AR_VDD | \
	(adc ? AR_PD10_ADC_ON : 0) | (vref ? AR_PD10_REF_ON : 0))

#define	READ_Y(vref)	(READ_12BIT_VDD(y,  1, vref))
#define	READ_Z1(vref)	(READ_12BIT_VDD(z1, 1, vref))
#define	READ_Z2(vref)	(READ_12BIT_VDD(z2, 1, vref))

#define	READ_X(vref)	(READ_12BIT_VDD(x,  1, vref))
#define	PWRDOWN		(READ_12BIT_VDD(y,  0, 0))	/* LAST */

/* single-ended samples need to first power up reference voltage;
 * we leave both ADC and VREF powered
 */
#define	READ_12BIT_VREF(x) (AR_START | AR_A2A1A0_ ## x \
	| AR_12_BIT | AR_VREF)

#define	REF_ON	(READ_12BIT_VDD(x, 1, 1))
#define	REF_OFF	(READ_12BIT_VDD(y, 0, 0))

/*--------------------------------------------------------------------------*/


static int old_absx, old_absy;
static int release_count;
static struct spi_device *spi_tmp;
static void do_touch (struct work_struct *work);

static DECLARE_DELAYED_WORK(dwork, do_touch);

static int ar7646_write_data(struct ts_ar7646 *touchscreen, u16 page, u16 addr, u16 data)
{
	int err = 0;
	struct spi_message msg;
	struct spi_transfer xfer;
	u16 tx_buf[2] = {0};
	DEBUG("tsc2000_write_data \n");
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
	DEBUG("tsc2000_write_data err=%d\n",err);	
	return err;

}

static int ar7646_shift(u8 *rbuf)
{
   u16 x;
    
   DEBUG("h:%x, l: %x ",rbuf[0],rbuf[1]);
	  
   x = rbuf[1];  
   DEBUG("v=%x ",x);  
   
   x += (u16)(rbuf[0]&0x7f)<<8;   
   DEBUG("v=%x ",x); 
   
   x = x >> 3;
   DEBUG("v=%x \n",x);   
   return x;

}


static int ar7646_read_data(struct device *dev, u16 *x, u16 *y, u16 *z1, u16 *z2)
{
	struct spi_device	*spi = to_spi_device(dev);
	struct ar7646		*ts = dev_get_drvdata(dev);
	struct ser_req		*req = kzalloc(sizeof *req, GFP_KERNEL);
	int			status;
	int			use_internal;

	u8 tx_buf[6][2],c = 0;
	u16 rx_buf[6];
	if (!req)
		return -ENOMEM;

	spi_message_init(&req->msg);


	tx_buf[c][0] = 0x01;
	tx_buf[c][1] = (u8)((u16)0xd7 << 1);
	req->xfer[c].tx_buf = tx_buf[c];	
	req->xfer[c].len = SET_TX_RX_LEN(2, 2);		
	req->xfer[c].rx_buf = &rx_buf[c];//tt			
	spi_message_add_tail(&req->xfer[c], &req->msg);	
	c++;

	tx_buf[c][0] = 0x01;
	tx_buf[c][1] = ((READ_12BIT_VREF(d_x)| AR_PD10_ALL_ON) << 1);	
	req->xfer[c].tx_buf = tx_buf[c];	
	req->xfer[c].len = SET_TX_RX_LEN(2, 2);		
	req->xfer[c].rx_buf = &rx_buf[c];//tt			
	spi_message_add_tail(&req->xfer[c], &req->msg);	
	c++;

	tx_buf[c][0] = 0x01;
	tx_buf[c][1] = ((READ_12BIT_VREF(d_y)| AR_PD10_ALL_ON) << 1);	
	req->xfer[c].tx_buf = tx_buf[c];	
	req->xfer[c].len = SET_TX_RX_LEN(2, 2);		
	req->xfer[c].rx_buf = &rx_buf[c];//tt			
	spi_message_add_tail(&req->xfer[c], &req->msg);	
	c++;

	tx_buf[c][0] = 0x01;
	tx_buf[c][1] = ((READ_12BIT_VREF(d_z1)| AR_PD10_ALL_ON) << 1);	
	req->xfer[c].tx_buf = tx_buf[c];	
	req->xfer[c].len = SET_TX_RX_LEN(2, 2);		
	req->xfer[c].rx_buf = &rx_buf[c];//tt			
	spi_message_add_tail(&req->xfer[c], &req->msg);	
	c++;

	tx_buf[c][0] = 0x01;
	tx_buf[c][1] = ((READ_12BIT_VREF(d_z2)| AR_PD10_ALL_ON) << 1);	
	req->xfer[c].tx_buf = tx_buf[c];	
	req->xfer[c].len = SET_TX_RX_LEN(2, 2);		
	req->xfer[c].rx_buf = &rx_buf[c];//tt			
	spi_message_add_tail(&req->xfer[c], &req->msg);	
	c++;

	tx_buf[c][0] = 0x01;
	tx_buf[c][1] = 0xe0 << 1;
	req->xfer[c].tx_buf = tx_buf[c];	
	req->xfer[c].len = SET_TX_RX_LEN(2, 2);		
	req->xfer[c].rx_buf = &rx_buf[c];//tt			
	spi_message_add_tail(&req->xfer[c], &req->msg);		
	
	
//	ts->irq_disabled = 1;
	disable_irq(spi->irq);
	status = spi_sync(spi, &req->msg);
//	ts->irq_disabled = 0;
	enable_irq(spi->irq);

	/* on-wire is a must-ignore bit, a BE12 value, then padding */
	rx_buf[1] = be16_to_cpu(rx_buf[1]);	
	*x = rx_buf[1] >> 4;
	*x &= 0x0fff;

	rx_buf[2] = be16_to_cpu(rx_buf[2]);	
	*y = rx_buf[2] >> 4;
	*y &= 0x0fff;

	rx_buf[3] = be16_to_cpu(rx_buf[3]);	
	*z1 = rx_buf[3] >> 4;
	*z1 &= 0x0fff;

	rx_buf[4] = be16_to_cpu(rx_buf[4]);	
	*z2 = rx_buf[4] >> 4;
	*z2 &= 0x0fff;


//	DEBUG("--tx=0x%4x rx =0x%4x -> 0x%4x\n",command,req->sample,status);
	kfree(req);
	return status;
	
}


static int ar7646_read12_ser(struct device *dev, unsigned command)
{
	struct spi_device	*spi = to_spi_device(dev);
	struct ar7646		*ts = dev_get_drvdata(dev);
	struct ser_req		*req = kzalloc(sizeof *req, GFP_KERNEL);
	int			status;
	int			use_internal;
	u8 tx_buf[3][2],rx_buf[2];
	
	if (!req)
		return -ENOMEM;

	spi_message_init(&req->msg);


	//req->command <<= 1;
	tx_buf[0][0] = 0x01;
	tx_buf[0][1] = (u8)((u16)0xd7 << 1);
	//tx_buf[0][0] = 0xd7;	
	req->xfer[0].tx_buf = tx_buf[0];	
	req->xfer[0].len = SET_TX_RX_LEN(2, 2);		
	req->xfer[0].rx_buf = rx_buf;//tt			
	spi_message_add_tail(&req->xfer[0], &req->msg);	

	req->command = (u8) command;	
	tx_buf[1][0] = 0x01;
	tx_buf[1][1] = command << 1;	
	req->xfer[1].tx_buf = tx_buf[1];	
	//req->xfer[0].tx_buf = &req->command;
	req->xfer[1].len = SET_TX_RX_LEN(2, 2);		
	req->xfer[1].rx_buf = &req->sample;//tt			
	spi_message_add_tail(&req->xfer[1], &req->msg);	
	
	tx_buf[2][0] = 0x01;
	tx_buf[2][1] = 0xe0 << 1;
	//tx_buf[2][0] = 0xe0;	
	req->xfer[2].tx_buf = tx_buf[2];	
	//req->xfer[0].tx_buf = &req->command;
	req->xfer[2].len = SET_TX_RX_LEN(2, 2);		
	req->xfer[2].rx_buf = rx_buf;//tt			
	spi_message_add_tail(&req->xfer[2], &req->msg);		
	
	
//	ts->irq_disabled = 1;
	disable_irq(spi->irq);
	status = spi_sync(spi, &req->msg);
//	ts->irq_disabled = 0;
	enable_irq(spi->irq);

	/* on-wire is a must-ignore bit, a BE12 value, then padding */
	status = be16_to_cpu(req->sample);	
	status = status >> 4;
	status &= 0x0fff;
	
	DEBUG("--tx=0x%4x rx =0x%4x -> 0x%4x\n",command,req->sample,status);
	kfree(req);
	return status;
}


static void do_touch (struct work_struct *work)
{
	static u16 absx = 0;
	static u16 absy = 0;
	static u16 absz1 = 0;
	static u16 absz2 = 0;
	static u16 absz = 0;			/* absz is a pressure factor */
	int X,Y;
	u16 buf[4] = {0};

//	DEBUG("do_touch \n");
	if (touchscreen.touch) {

		spi_setup(touchscreen.spi);			//20080417 JS Add
		ar7646_read_data(&spi_tmp->dev,&absx,&absy,&absz1,&absz2);

		if (absz1) {
			//absz = absx * ((absz2/absz1) -1 );
			absz = absx * ((absz2/absz1) -1 );
			//absz = (200/4069) * absx * ((absz2/absz1) -1 );
		}
		else {
#if defined CONFIG_ARCH_P7DK
			socle_enable_gpio_irq(touchscreen.ts_irq);
#else
			enable_irq (touchscreen.ts_irq);
#endif
			return;
		}

		//DEBUG("z1:%x, z2:%x Z:%x",absz);
		if(absz < 0x1800) {
		if ((absx != old_absx)&(absy != old_absy)) { 
			input_report_key (touchscreen.socle_ts_dev, BTN_TOUCH, 1);			

#ifndef CONFIG_ANDROID_SYSTEM
			X = ((long)absx * (long)0xff) >> 12;
			Y = ((long)absy * (long)0xff) >> 12;

			input_report_abs (touchscreen.socle_ts_dev, ABS_X, Y);
			input_report_abs (touchscreen.socle_ts_dev, ABS_Y, abs( 0xff -X));

			//input_report_abs (touchscreen.socle_ts_dev, ABS_X, absx);
			//input_report_abs (touchscreen.socle_ts_dev, ABS_Y, abs (absy - TS_ABS_Y_MAX));
#else

			// calibration in driver
			input_report_abs (touchscreen.socle_ts_dev, ABS_X, abs(absy*320/(0xff)));
			input_report_abs (touchscreen.socle_ts_dev, ABS_Y, abs (240-absx*240/(0xff)));
#endif

			input_report_abs (touchscreen.socle_ts_dev, ABS_PRESSURE, absz);

			input_sync (touchscreen.socle_ts_dev);
			old_absx=absx;
			old_absy=absy;
			DEBUG("\nx:%4x y:%4x, z1:%4x z2:%4x Z:%4x -X:%4x Y:%4x",absx,absy,absz1,absz2,absz,X,Y);	
		}
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
		//DEBUG("put release \n");	
		if(release_count > 5)
		{
			input_report_key (touchscreen.socle_ts_dev, BTN_TOUCH, 0);			
			input_report_abs (touchscreen.socle_ts_dev, ABS_PRESSURE, 0);
			input_sync (touchscreen.socle_ts_dev);
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

static int __devinit ar7646_probe(struct spi_device *spi)
{
	u16 absx = 0;
	u16 absy = 0;
	u16 absz1 = 0;
	u16 absz2 = 0;
	u16 absz = 0;

	spi_tmp = spi;

	dev_dbg(&spi->dev, "ts_ar7646_probe()\n");
	DEBUG("ts_ar7646_probe \n");	
	memset(&touchscreen, 0, sizeof(struct ts_ar7646));
	init_MUTEX(&touchscreen.sem);
	touchscreen.spi = spi;
	//spi->bits_per_word = 16;
	spi->bits_per_word = 8;//8
	spi->mode = SPI_MODE_0;//add SPI_LSB_FIRST
	
	touchscreen.socle_ts_dev = input_allocate_device ();

	if (!touchscreen.socle_ts_dev)
		return -ENOMEM;
	DEBUG(">set touchscreen.socle_ts_dev \n");	
	touchscreen.socle_ts_dev->evbit[0] = BIT (EV_ABS) | BIT (EV_KEY);
	touchscreen.socle_ts_dev->absbit[0] = BIT (ABS_X) | BIT (ABS_Y) | BIT (ABS_PRESSURE);
	touchscreen.socle_ts_dev->keybit[BIT_WORD (BTN_TOUCH)] = BIT_MASK (BTN_TOUCH);

	touchscreen.socle_ts_dev->absmin[ABS_X] = TS_ABS_X_MIN;
	touchscreen.socle_ts_dev->absmin[ABS_Y] = TS_ABS_Y_MIN;
	touchscreen.socle_ts_dev->absmax[ABS_X] = TS_ABS_X_MAX;
	touchscreen.socle_ts_dev->absmax[ABS_Y] = TS_ABS_Y_MAX;
	DEBUG(">input_set_abs_params \n");
	input_set_abs_params(touchscreen.socle_ts_dev, ABS_PRESSURE, 0, 255, 0, 0);

	DEBUG(">set touchscreen.socle_ts_dev2 \n");
	touchscreen.socle_ts_dev->name = "ts_ar7646";
	touchscreen.socle_ts_dev->phys = "socle_ts/input0";
	touchscreen.touch =0;

	DEBUG(">spi->irq =%d\n",spi->irq);	
	// 20080925 Ryan modify
	touchscreen.ts_irq = spi->irq;

	//20080417 JS Add
	spi_setup(touchscreen.spi);				
	
	/* Setup TSC2000 */
	(void) ar7646_read12_ser(&spi_tmp->dev,READ_12BIT_VREF(vaux));
/*
while(1){
	//(void) ar7646_read12_ser(&spi_tmp->dev,READ_12BIT_VREF(d_x));	
	ar7646_read_data(&spi_tmp->dev,&absx,&absy,&absz1,&absz2);
	DEBUG("x: %x, y: %x, z: %x, z: %x \n",absx,absy,absz1,absz2);	
}
*/


	DEBUG(">input_register_device \n");
	if(input_register_device (touchscreen.socle_ts_dev))
		printk("input_register_device() failed!!!\n");
	DEBUG(">socle_request_gpio_irq \n");
#if defined (CONFIG_ARCH_P7DK) 
	//cyli 20071127 modify

	if (socle_request_gpio_irq (touchscreen.ts_irq, (irq_handler_t) socle_ts_interrupt, GPIO_INT_SENSE_LEVEL | GPIO_INT_SINGLE_EDGE | GPIO_INT_EVENT_LO, MODNAME, NULL)) {
		printk (KERN_ERR "socle_touchscreen.c: Can't allocate pin[%d]\n", touchscreen.ts_irq);
		input_unregister_device (touchscreen.socle_ts_dev);
	  	return -EBUSY;
	}
#elif defined(CONFIG_ARCH_PDK_PC7210)
	if (socle_request_gpio_irq (touchscreen.ts_irq, (irq_handler_t) socle_ts_interrupt, GPIO_INT_SENSE_LEVEL | GPIO_INT_SINGLE_EDGE | GPIO_INT_EVENT_LO, MODNAME, NULL)) {
		printk (KERN_ERR "socle_touchscreen.c: Can't allocate pin[%d]\n", touchscreen.ts_irq);
		input_unregister_device (touchscreen.socle_ts_dev);
	  	return -EBUSY;
	}
#else //INCLUDE , CDK , PC9002, pc9220
	if (request_irq (touchscreen.ts_irq, (irq_handler_t) socle_ts_interrupt, IRQF_DISABLED, MODNAME, NULL) < 0)
	{
		printk (KERN_ERR "socle_touchscreen.c: Can't allocate irq %d\n", touchscreen.ts_irq);
		input_unregister_device (touchscreen.socle_ts_dev);
	  	return -EBUSY;
	}
#endif





	DEBUG(">dev_set_drvdata end\n");
	dev_set_drvdata(&spi->dev, &touchscreen);
	DEBUG(">ts_ar7646_probe end\n");
	return 0;
	
}

static int __devexit ar7646_remove(struct spi_device *spi)
{

	dev_dbg(&spi->dev, "ts_ar7646_remove()\n");
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

static int ar7646_suspend(struct spi_device *spi)
{
	return 0;
}
static int ar7646_resume(struct spi_device *spi)
{
	return 0;
}
static struct spi_driver ts_ar7646_driver = {
	.driver = {
		.name = "ar7646",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
	},
	.probe = ar7646_probe,
	.remove = __devexit_p(ar7646_remove),
	.suspend	= ar7646_suspend,
	.resume		= ar7646_resume,
};

static int __init ar7646_init(void)
{
	DEBUG("ts_ar7646_init1 \n");
	return spi_register_driver(&ts_ar7646_driver);
}
module_init(ar7646_init);

static void __exit ar7646_exit(void)
{
	spi_unregister_driver(&ts_ar7646_driver);
}
module_exit(ar7646_exit);

MODULE_DESCRIPTION("SOCLE TSC2000");
MODULE_AUTHOR("Ryan Chen");
MODULE_LICENSE("GPL");


/********************************************************************************
* File Name     : drivers/char/msmv-keypad.c
* Author        : Ryan Chen
* Description   : Socle MP-GPIO KeyPad Driver
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
* 
*   Version      : 1,0,0,0
*   History      :
*      1. 2008/08/29 ryanchen create this file
*
********************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>

#include <linux/delay.h>

#include <asm/arch/regs-mp-gpio.h>
#include <asm/arch/mp-gpio.h>

#include <asm/io.h>


//#define CONFIG_MSMV_KPD_DEBUG

#ifdef CONFIG_MSMV_KPD_DEBUG
	#define KPD_DBG(fmt, args...) printk("MSMV_KPD: %s(): " fmt, __FUNCTION__, ## args)
#else
	#define KPD_DBG(fmt, args...)
#endif

#define KPD_NUM	8

static struct input_dev *msmv_kpd;
static unsigned char *msmvkpd_keycode;
static int kpd_irq;

static void msmv_keypad_input_report_key(void *param);
static DECLARE_WORK(work, msmv_keypad_input_report_key, 0);

static int row;
	
static char __initdata banner[] = "SQ MSMV KeyPad Driver, (c) 2007 SQ Corp.\n";

#define MP_PORT		MP_PA

static void
msmv_keypad_report_key(int row)
{
//	int i;

	if (row < 0) {
		KPD_DBG(KERN_ERR "Invalid argument: row = %d\n", row);
		return;
	}
	
	input_report_key(msmv_kpd, msmvkpd_keycode[row], 1);	//press

	input_report_key(msmv_kpd, msmvkpd_keycode[row], 0);	//release

	KPD_DBG("row = %d; msmvkpd_keycode = %d\n", row, msmvkpd_keycode[row]);

	input_sync(msmv_kpd);
}

static int
kpd_value_convert(int val)
{
	int ret;

	switch (val) {
		case (1<<0):
			ret = 0;
			break;
		case (1<<1):
			ret = 1;
			break;
		case (1<<2):
			ret = 2;
			break;
		case (1<<3):
			ret = 3;
			break;
		case (1<<4):
			ret = 4;
			break;
		case (1<<5):
			ret = 5;
			break;
		case (1<<6):
			ret = 6;
			break;
		case (1<<7):
			ret = 7;
			break;
	default:
		KPD_DBG("Warning! val = 0x%02x\n", val);
		ret = -1;
	}

	return ret;
}

static void
msmv_keypad_input_report_key(void *param)
{
	int value;

	value=socle_mp_gpio_get_port_value(MP_PORT);

	KPD_DBG("row = %d\n", row);	
	
	msmv_keypad_report_key(row);	

	// enable all interrupt (row - PB0, PB1, PB2, PB3)
	socle_mp_gpio_set_port_interrupt(MP_PORT,0xff);

}

static irqreturn_t
mp_gpio_kpd_isr(int irq, void *dev)
{
	
	int sts;
	sts = socle_mp_gpio_get_port_interrupt_status(MP_PORT);
	KPD_DBG("msmv_keypad_isr row = %x \n",sts);	
	
	if(sts !=0){
		row = kpd_value_convert(sts & 0xff);
		socle_mp_gpio_clear_port_interrupt(MP_PORT);
		socle_mp_gpio_set_port_interrupt(MP_PORT, 0);
	}
	else
		return IRQ_HANDLED;
		
	schedule_work(&work);

	return IRQ_HANDLED;
}

static void
gpio_kpd_init(void)
{

	int i;
	u8 irqflag;
	
	// normal mode
	socle_mp_gpio_test_mode_en(MP_PORT, 0);

	//socle_scu_iomode_gpio4_set();
	//socle_scu_iomode_gpio5_set();
	//socle_scu_iomode_gpio6_set();
	//socle_scu_iomode_gpio7_set();

	irqflag = (MP_GPIO_INT_SENSE_EDGE | MP_GPIO_INT_SINGLE_EDGE | MP_GPIO_INT_EVENT_LO);
	for(i=0;i<KPD_NUM;i++) {
		socle_mp_gpio_set_port_num_direction(MP_PORT,i,SOCLE_MP_GPIO_DIR_PIN);
		socle_mp_gpio_request_irq(MP_PORT, i, irqflag, mp_gpio_kpd_isr, NULL );
	}
}


extern irqreturn_t
mp_gpio_kpd_event(int key)
{
	
	row = key;
		
	schedule_work(&work);

	return IRQ_HANDLED;
}

EXPORT_SYMBOL(mp_gpio_kpd_event);


static int __devinit
msmv_keypad_probe(struct platform_device *pdev)
{
	int ret, i;

	msmvkpd_keycode = (unsigned char *)pdev->dev.platform_data;
	if (!msmvkpd_keycode) {
		printk(KERN_ERR "Unable to find keycode\n");
		return -ENOMEM;
	}

	msmv_kpd = input_allocate_device();
	if (!msmv_kpd) {
		printk(KERN_ERR "Unable to allocate input device\n");
		return -ENOMEM;
	}

	msmv_kpd->evbit[0] = BIT(EV_KEY);
	msmv_kpd->name = "MSMV KeyPad";
	msmv_kpd->phys = "msmvkpd/input0";
	msmv_kpd->id.bustype = BUS_HOST;
	msmv_kpd->id.vendor = 0x0001;
	msmv_kpd->id.product = 0x0001;
	msmv_kpd->id.version = 0x0001;

	for (i = 0; i < (KPD_NUM+2); i++) {
		set_bit(msmvkpd_keycode[i], msmv_kpd->keybit);
		KPD_DBG("msmvkpd_keycode[%d] = %d\n", i, msmvkpd_keycode[i]);
	}

	gpio_kpd_init();

	if (kpd_irq < 0) {
		printk(KERN_ERR "Unable to get IRQ(%d)\n", kpd_irq);
		ret = -ENOENT;
		goto err_no_irq;
	}

	//ret = request_irq(MP_GPIO_INT, mp_gpio_kpd_isr, IRQF_SHARED, msmv_kpd->name, msmv_kpd);
	//if (ret) {
	//	printk(KERN_ERR "Unable to claim IRQ(%d)\n", kpd_irq);
	//	goto err_no_irq;
	//}

	ret = input_register_device(msmv_kpd);
	if (ret) {
		printk(KERN_ERR "Unable to register %s input device\n", msmv_kpd->name);
		goto err_reg_dev;
	}

	return ret;

err_reg_dev:
	free_irq(kpd_irq, NULL);
err_no_irq:
	input_free_device(msmv_kpd);
	return ret;
}

static int __devexit
msmv_keypad_remove(struct platform_device *pdev)
{
	KPD_DBG("\n");

	free_irq(kpd_irq, msmv_kpd);
	flush_scheduled_work();
	input_unregister_device(msmv_kpd);

	return 0;
}

#ifdef CONFIG_PM

#define MSMV_KPD_REG_NUM	10
u32 msmv_keypad_save_addr[MSMV_KPD_REG_NUM]; 

static int
msmv_keypad_suspend(struct platform_device *pdev, pm_message_t msg)
{
	int tmp;	
	u32 *addr;
		
	pr_debug("msmv_keypad_suspend\n");

	addr = (u32 *)MP_GPIO_REG_BASE;
	
	for(tmp=0;tmp<MSMV_KPD_REG_NUM;tmp++){
		iowrite32(ioread32(addr+tmp), (msmv_keypad_save_addr+tmp));
	}

        return 0;
}

static int 
msmv_keypad_resume(struct platform_device *pdev)
{	
	int tmp;	
	u32 *addr;
		
	pr_debug("msmv_keypad_resume\n");
	
	addr = (u32 *)MP_GPIO_REG_BASE;
	
	for(tmp=0;tmp<MSMV_KPD_REG_NUM;tmp++){
		iowrite32(ioread32(msmv_keypad_save_addr+tmp), (addr+tmp));
	}

        return 0;
}
#else
#define msmv_keypad_suspend NULL
#define msmv_keypad_resume NULL
#endif

static struct platform_driver msmv_keypad_device_driver = {
	.probe		= msmv_keypad_probe,
	.remove		= __devexit_p(msmv_keypad_remove),
	.suspend = msmv_keypad_suspend,
	.resume = msmv_keypad_resume,
	.driver		= {
		.name	= "msmv-keypad",
	}
};

static int __init
msmv_keypad_init(void)
{
	printk(banner);
	return platform_driver_register(&msmv_keypad_device_driver);
}

static void __exit
msmv_keypad_exit(void)
{
	platform_driver_unregister(&msmv_keypad_device_driver);
}

module_init(msmv_keypad_init);
module_exit(msmv_keypad_exit);


MODULE_DESCRIPTION("SQ MSMV KeyPad Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CY Li <cyli@socle-tech.com.tw>");

/********************************************************************************
* File Name     : drivers/char/pdk-keypad.c
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


//#define CONFIG_PDK_KPD_DEBUG

#ifdef CONFIG_PDK_KPD_DEBUG
	#define KPD_DBG(fmt, args...) printk("PDK_KPD: %s(): " fmt, __FUNCTION__, ## args)
#else
	#define KPD_DBG(fmt, args...)
#endif

#define KPD_NUM	4

static struct input_dev *pdk_kpd;
static unsigned char *pdkkpd_keycode;
static int kpd_irq;

static void pdk_keypad_input_report_key(void *param);
static DECLARE_WORK(work, pdk_keypad_input_report_key, 0);

static int row;
	
static char __initdata banner[] = "SQ PDK KeyPad Driver, (c) 2007 SQ Corp.\n";

#ifdef CONFIG_ARCH_SCDK
#define MP_PORT		MP_PN
#else	//for SCDK
#define MP_PORT		MP_PB
#endif

static void
pdk_keypad_report_key(int row, int col)
{
//	int i;

	if ((row < 0) || (col < 0)) {
		KPD_DBG(KERN_ERR "Invalid argument: row = %d, col = %d\n", row, col);
		return;
	}
//20080201 mark for key report 
//	for (i = 0; i < 8; i++) {
//		input_report_key(pdk_kpd, pdkkpd_keycode[i], 1);
//	}

	input_report_key(pdk_kpd, pdkkpd_keycode[row * KPD_NUM + col + 8], 1);	//press

//	for (i = 0; i < 8; i++) {
//		input_report_key(pdk_kpd, pdkkpd_keycode[i], 0);
//	}

	input_report_key(pdk_kpd, pdkkpd_keycode[row * KPD_NUM + col + 8], 0);	//release

	KPD_DBG("row = %d, col = %d; pdkkpd_keycode = %d\n", row, col, pdkkpd_keycode[row * KPD_NUM + col + 8]);

	input_sync(pdk_kpd);
}

static int
kpd_value_convert(int val)
{
	int ret;

	switch (val) {
	case 1:
		ret = 0;
		break;
	case 2:
		ret = 1;
		break;
	case 4:
		ret = 2;
		break;
	case 8:
		ret = 3;
		break;
	default:
		KPD_DBG("Warning! val = 0x%02x\n", val);
		ret = -1;
	}

	return ret;
}

static void
pdk_keypad_input_report_key(void *param)
{
	int col,i,value;

	value=socle_mp_gpio_get_port_value(MP_PORT);

	col = kpd_value_convert((value & 0xf0)>>4);

	KPD_DBG("row = %d, col = %d\n", row, col);	
	

	if (col != -1)
		pdk_keypad_report_key(row, col);	

	for(i=4;i<8;i++) {
		socle_mp_gpio_set_port_num_value(MP_PORT,i,0);
	}

	// enable all interrupt (row - PB0, PB1, PB2, PB3)
	socle_mp_gpio_set_port_interrupt(MP_PORT,0x0f);

}

static irqreturn_t
mp_gpio_kpd_isr(int irq, void *dev)
{
	
	int i,sts;
	sts = socle_mp_gpio_get_port_interrupt_status(MP_PORT);
	KPD_DBG("pdk_keypad_isr row = %x \n",sts);	
	
	if(sts !=0)
	{
		row = kpd_value_convert(sts & 0xff);
		socle_mp_gpio_clear_port_interrupt(MP_PORT);
		socle_mp_gpio_set_port_interrupt(MP_PORT, 0);

		for(i=4;i<8;i++) {
			socle_mp_gpio_set_port_num_direction(MP_PORT,i,SOCLE_MP_GPIO_DIR_PIN);
		}
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

	irqflag = (MP_GPIO_INT_SENSE_EDGE | MP_GPIO_INT_SINGLE_EDGE | MP_GPIO_INT_EVENT_LO);
	for(i=0;i<4;i++) {
		socle_mp_gpio_set_port_num_direction(MP_PORT,i,SOCLE_MP_GPIO_DIR_PIN);
		socle_mp_gpio_request_irq(MP_PORT, i, irqflag, mp_gpio_kpd_isr, NULL );
	}

	for(i=4;i<8;i++) {
		socle_mp_gpio_set_port_num_value(MP_PORT,i,0);
	}

	//socle_mp_gpio_set_port_interrupt(MP_PORT,0x0f);

}

static int __devinit
pdk_keypad_probe(struct platform_device *pdev)
{
	int ret, i;

	pdkkpd_keycode = (unsigned char *)pdev->dev.platform_data;
	if (!pdkkpd_keycode) {
		printk(KERN_ERR "Unable to find keycode\n");
		return -ENOMEM;
	}

	pdk_kpd = input_allocate_device();
	if (!pdk_kpd) {
		printk(KERN_ERR "Unable to allocate input device\n");
		return -ENOMEM;
	}

	pdk_kpd->evbit[0] = BIT(EV_KEY);
	pdk_kpd->name = "PDK KeyPad";
	pdk_kpd->phys = "pdkkpd/input0";
	pdk_kpd->id.bustype = BUS_HOST;
	pdk_kpd->id.vendor = 0x0001;
	pdk_kpd->id.product = 0x0001;
	pdk_kpd->id.version = 0x0001;

	for (i = 0; i < (KPD_NUM * KPD_NUM + 8); i++) {
		set_bit(pdkkpd_keycode[i], pdk_kpd->keybit);
		KPD_DBG("pdkkpd_keycode[%d] = %d\n", i, pdkkpd_keycode[i]);
	}

	gpio_kpd_init();

	if (kpd_irq < 0) {
		printk(KERN_ERR "Unable to get IRQ(%d)\n", kpd_irq);
		ret = -ENOENT;
		goto err_no_irq;
	}

	//ret = request_irq(MP_GPIO_INT, mp_gpio_kpd_isr, IRQF_SHARED, pdk_kpd->name, pdk_kpd);
	//if (ret) {
	//	printk(KERN_ERR "Unable to claim IRQ(%d)\n", kpd_irq);
	//	goto err_no_irq;
	//}

	ret = input_register_device(pdk_kpd);
	if (ret) {
		printk(KERN_ERR "Unable to register %s input device\n", pdk_kpd->name);
		goto err_reg_dev;
	}

	return ret;

err_reg_dev:
	free_irq(kpd_irq, NULL);
err_no_irq:
	input_free_device(pdk_kpd);
	return ret;
}

static int __devexit
pdk_keypad_remove(struct platform_device *pdev)
{
	KPD_DBG("\n");

	free_irq(kpd_irq, pdk_kpd);
	flush_scheduled_work();
	input_unregister_device(pdk_kpd);

	return 0;
}

static struct platform_driver pdk_keypad_device_driver = {
	.probe		= pdk_keypad_probe,
	.remove		= __devexit_p(pdk_keypad_remove),
	.driver		= {
		.name	= "pdk-keypad",
	}
};

static int __init
pdk_keypad_init(void)
{
	printk(banner);
	return platform_driver_register(&pdk_keypad_device_driver);
}

static void __exit
pdk_keypad_exit(void)
{
	platform_driver_unregister(&pdk_keypad_device_driver);
}

module_init(pdk_keypad_init);
module_exit(pdk_keypad_exit);


MODULE_DESCRIPTION("SQ PDK KeyPad Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CY Li <cyli@socle-tech.com.tw>");

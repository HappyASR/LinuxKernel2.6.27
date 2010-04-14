/********************************************************************************
* File Name     : drivers/input/keyboard/inr-keypad.c
* Author        : cyli
* Description   : Socle PC9220 KeyPad Driver
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
*   Version      : 2,0,0,1
*   History      :
*      1. 2008/07/17 cyli create this file
*
********************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>

#include <linux/delay.h>

#include <mach/gpio.h>


//#define CONFIG_PDK_KPD_DEBUG
#ifdef CONFIG_PDK_KPD_DEBUG
	#define KPD_DBG(fmt, args...) printk("INR_KPD: %s(): " fmt, __FUNCTION__, ## args)
#else
	#define KPD_DBG(fmt, args...)
#endif

#define KPD_NUM	4

static struct input_dev *pdk_kpd;
static unsigned char *pdkkpd_keycode;
static int kpd_irq;

static void pdk_keypad_input_report_key(struct work_struct *work);
static DECLARE_WORK(work, pdk_keypad_input_report_key);

static char __initdata banner[] = "SQ PC9220 KeyPad Driver, (c) 2008 SQ Corp.\n";

static void
pdk_keypad_report_key(int row, int col)
{
	//int i;

	if ((row < 0) || (col < 0)) {
		KPD_DBG(KERN_ERR "Invalid argument: row = %d, col = %d\n", row, col);
		return;
	}

//	for (i = 0; i < 8; i++) {
//		input_report_key(pdk_kpd, pdkkpd_keycode[i], 1);
//	}

	input_report_key(pdk_kpd, pdkkpd_keycode[row * KPD_NUM + col + 8], 1);	//press

//	for (i = 0; i < 8; i++) {
//		input_report_key(pdk_kpd, pdkkpd_keycode[i], 0);
//	}

//	input_report_key(pdk_kpd, pdkkpd_keycode[row * KPD_NUM + col + 8], 0);	//release

	KPD_DBG("row = %d, col = %d; pdkkpd_keycode = %d\n", row, col, pdkkpd_keycode[row * KPD_NUM + col + 8]);

	input_sync(pdk_kpd);
}


static void
gpio_kpd_init(void)
{
	KPD_DBG("\n");

	socle_gpio_claim_lock();

	// normal mode
	socle_gpio_test_mode_en(PJ, 0);

	// single low edge trigger (row - PJ0, PJ1, PJ2, PJ3)
	socle_gpio_set_interrupt_sense_with_mask(PJ, 0x0, 0xf);
	socle_gpio_set_interrupt_both_edges_with_mask(PJ, 0x0, 0xf);
	socle_gpio_set_interrupt_event_with_mask(PJ, 0x0, 0xf);

	// set row as input (row - PJ0, PJ1, PJ2, PJ3)
	(void)socle_gpio_get_value_with_mask(PJ, 0xf);

	// write col pins as 0 (col - PJ4, PJ5, PJ6, PJ7)
	socle_gpio_set_value_with_mask(PJ, 0x0, 0xf0);

	// enable all interrupt (row - PJ0, PJ1, PJ2, PJ3)
	socle_gpio_set_interrupt_mask_with_mask(PJ, 0xf, 0xf);

	socle_gpio_release_lock();
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

static int tmp1, tmp2;
static void pdk_keypad_input_report_key(struct work_struct *work)
{
	int row, col, tmp;
	int old_row, old_col;
	int loop=0;

	KPD_DBG("\n");

	// 20080911 cyli fix to support repeat key
	while (1) {
		socle_gpio_claim_lock();

		// read col pins (col - PJ4, PJ5, PJ6, PJ7)
		tmp2 = socle_gpio_get_value_with_mask(PJ, 0xf0);

		socle_gpio_release_lock();

		KPD_DBG("tmp1 = 0x%x, tmp2 = 0x%x\n", tmp1, tmp2);

		row = kpd_value_convert(tmp1 & 0xf);
		col = kpd_value_convert((tmp2 & 0xf0) >> 4);

		KPD_DBG("row = %d, col = %d\n", row, col);

		if(!loop || (loop && (tmp == tmp2)))
			pdk_keypad_report_key(row, col);
		else break;

		old_row=row;
		old_col=col;

		mdelay(200);

		socle_gpio_claim_lock();

		// read col pins (col - PJ4, PJ5, PJ6, PJ7)
		tmp = socle_gpio_get_value_with_mask(PJ, 0xf0);

		socle_gpio_release_lock();
		if (!(tmp == tmp2)) {
			KPD_DBG("exit loop\n");
			break;
		}
		loop =1;
		KPD_DBG("repeat\n");
	}

	input_report_key(pdk_kpd, pdkkpd_keycode[row * KPD_NUM + col + 8], 0);	//release
	input_sync(pdk_kpd);


	socle_gpio_claim_lock();

	// write col pins as 0 (col - PJ4, PJ5, PJ6, PJ7)
	socle_gpio_set_value_with_mask(PJ, 0x0, 0xf0);

	// enable all interrupt (row - PJ0, PJ1, PJ2, PJ3)
	socle_gpio_set_interrupt_mask_with_mask(PJ, 0xf, 0xf);

	socle_gpio_release_lock();
}

static irqreturn_t
pdk_keypad_isr(int irq, void *dev)
{
	KPD_DBG("pdk_keypad_isr\n");

	// read row pins
	tmp1 = socle_gpio_get_interrupt_status_with_port(PJ);

	if (0xf & tmp1) {
		// disable all interrupt (row - PJ0, PJ1, PJ2, PJ3)
		socle_gpio_set_interrupt_mask_with_mask(PJ, 0x0, 0xf);
	} else {
		return IRQ_HANDLED;
	}

	schedule_work(&work);

	return IRQ_HANDLED;
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
//	pdk_kpd->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	pdk_kpd->name = "PC9220 KeyPad";
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

	kpd_irq = socle_gpio_get_irq(PJ);
	if (kpd_irq < 0) {
		printk(KERN_ERR "Unable to get IRQ(%d)\n", kpd_irq);
		ret = -ENOENT;
		goto err_no_irq;
	}

	ret = request_irq(kpd_irq, pdk_keypad_isr, IRQF_SHARED, pdk_kpd->name, pdk_kpd);
	if (ret) {
		printk(KERN_ERR "Unable to claim IRQ(%d)\n", kpd_irq);
		goto err_no_irq;
	}

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
		.name	= "pc9220-keypad",
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


MODULE_DESCRIPTION("SQ PC9220 KeyPad Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CY Li <cyli@socle-tech.com.tw>");

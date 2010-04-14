/********************************************************************************
* File Name     : drivers/char/p7dk-keypad.c
* Author        : cyli
* Description   : Socle P7DK KeyPad Driver
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
*      1. 2007/08/29 cyli create this file
*      2. 2007/11/27 cyli modify this file to use gpio share irq
*
********************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>

#include <linux/delay.h>

#include <asm/arch/gpio.h>


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

static void pdk_keypad_input_report_key(struct work_struct *work);
//static DECLARE_WORK(work, pdk_keypad_input_report_key, 0);
static DECLARE_WORK(work, pdk_keypad_input_report_key);

static char __initdata banner[] = "SQ PDK KeyPad Driver, (c) 2007 SQ Corp.\n";

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

	input_report_key(pdk_kpd, pdkkpd_keycode[row * KPD_NUM + col + 8], 0);	//release

	KPD_DBG("row = %d, col = %d; pdkkpd_keycode = %d\n", row, col, pdkkpd_keycode[row * KPD_NUM + col + 8]);

	input_sync(pdk_kpd);
}


static void
gpio_kpd_init(void)
{
	KPD_DBG("\n");

	socle_gpio_claim_lock();
#ifdef CONFIG_ARCH_PDK_PC7210
	// normal mode
	socle_gpio_test_mode_en(PA, 0);
	socle_gpio_test_mode_en(PF, 0);	

	// single low edge trigger (row - PA0, PA1, PA2, PA3)
	// PA0, PA1, PA2, PA3
	socle_gpio_set_interrupt_sense_with_mask(PA, 0x0, 0xf);
	socle_gpio_set_interrupt_both_edges_with_mask(PA, 0x0, 0xf);
	socle_gpio_set_interrupt_event_with_mask(PA, 0x0, 0xf);

	// set row as input
	// PA0, PA1, PA2, PA3
	socle_gpio_get_value_with_mask(PA, 0xf);

	// write col pins as 0
	// PA4, PA5
	socle_gpio_set_value_with_mask(PA, 0x0, 0x30);
	// PF4, PF5
	socle_gpio_set_value_with_mask(PF, 0x0, 0x30);

	// enable all interrupt (row - PA0, PA1, PA2, PA3)
	// PA0, PA1, PA2
	socle_gpio_set_interrupt_mask_with_mask(PA, 0xf, 0xf);

#else
	// normal mode
	socle_gpio_test_mode_en(PA, 0);	

	// single low edge trigger (row - PA0, PA1, PA2, PC0)
	// PA0, PA1, PA2
	socle_gpio_set_interrupt_sense_with_mask(PA, 0x0, 0x7);
	socle_gpio_set_interrupt_both_edges_with_mask(PA, 0x0, 0x7);
	socle_gpio_set_interrupt_event_with_mask(PA, 0x0, 0x7);
	// PC0
	socle_gpio_set_interrupt_sense_with_mask(PC, 0x0, 0x1);
	socle_gpio_set_interrupt_both_edges_with_mask(PC, 0x0, 0x1);
	socle_gpio_set_interrupt_event_with_mask(PC, 0x0, 0x1);

	// set row as input
	// PA0, PA1, PA2
	socle_gpio_get_value_with_mask(PA, 0x7);
	// PC0
	socle_gpio_get_value_with_mask(PC, 0x1);

	// write col pins as 0
	// PA3, PA4, PA5
	socle_gpio_set_value_with_mask(PA, 0x0, 0x18);
	// PC1
	socle_gpio_set_value_with_mask(PC, 0x0, 0x2);

	// enable all interrupt (row - PA0, PA1, PA2, PC0)
	// PA0, PA1, PA2
	socle_gpio_set_interrupt_mask_with_mask(PA, 0x7, 0x7);
	// PC0
	socle_gpio_set_interrupt_mask_with_mask(PC, 0x1, 0x1);
#endif
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
static void
//pdk_keypad_input_report_key(void *param)
pdk_keypad_input_report_key(struct work_struct *work)
{
	int tmp_1, tmp_2, row, col;

	KPD_DBG("\n");

	socle_gpio_claim_lock();
#ifdef CONFIG_ARCH_PDK_PC7210
	// read col pins
	// PA4, PA5
	tmp_1 = socle_gpio_get_value_with_mask(PA, 0x30);
	// PF4, PF5
	tmp_2 = socle_gpio_get_value_with_mask(PF, 0x30);

	socle_gpio_release_lock();

	KPD_DBG("tmp1 = 0x%x, tmp2 = 0x%x\n", tmp1, tmp2);
#if (KPD_NUM == 4)
	// 4 X 4
	col = kpd_value_convert(tmp1 & 0xf);
	row = kpd_value_convert(((tmp_1 & 0x30) >> 4) | (tmp_2 & 0x30) >> 2);
#endif
#if (KPD_NUM == 3)
	// 3 X 3
	col = kpd_value_convert(tmp1 & 0x7);
	row = kpd_value_convert((tmp_1 & 0x30) >> 4) | (tmp_2 & 0x10) >> 2);
#endif

	KPD_DBG("row = %d, col = %d\n", row, col);

	pdk_keypad_report_key(row, col);

//	mdelay(300);

	socle_gpio_claim_lock();

	// write col pins as 0
	// PA4, PA5
	socle_gpio_set_value_with_mask(PA, 0x0, 0x30);
	// PF4, PF5
	socle_gpio_set_value_with_mask(PF, 0x0, 0x30);

	// enable all interrupt (row - PA0, PA1, PA2, PA3)
	socle_gpio_set_interrupt_mask_with_mask(PA, 0xf, 0xf);
#else
	// read col pins
	// PA3, PA4, PA5
	// because PA5 is used as touch screen interrupt, it should not be reused (0x38=>0x18)
	tmp_1 = socle_gpio_get_value_with_mask(PA, 0x18);
	// PC1
	tmp_2 = socle_gpio_get_value_with_mask(PC, 0x2);

	socle_gpio_release_lock();

	KPD_DBG("tmp1 = 0x%x, tmp2 = 0x%x\n", tmp1, tmp2);
#if (KPD_NUM == 4)
	// 4 X 4
	col = kpd_value_convert((tmp1 & 0x7)  | ((tmp2 & 0x1) << 3));
	row = kpd_value_convert(((tmp_1 & 0x18) >> 3) | (tmp_2 & 0x2) << 2);
#endif
#if (KPD_NUM == 3)
	// 3 X 3
	col = kpd_value_convert(tmp1 & 0x7);
	row = kpd_value_convert((tmp_1 & 0x18) >> 3);
#endif

	KPD_DBG("row = %d, col = %d\n", row, col);

	pdk_keypad_report_key(row, col);

//	mdelay(300);

	socle_gpio_claim_lock();

	// write col pins as 0
	// PA3, PA4, PA5
	socle_gpio_set_value_with_mask(PA, 0x0, 0x18);
	// PC1
	socle_gpio_set_value_with_mask(PC, 0x0, 0x2);

	// enable all interrupt (row - PA0, PA1, PA2, PC0)
	socle_gpio_set_interrupt_mask_with_mask(PA, 0x7, 0x7);
	socle_gpio_set_interrupt_mask_with_mask(PC, 0x1, 0x1);
#endif
	socle_gpio_release_lock();
}

static irqreturn_t
pdk_keypad_isr(int irq, void *dev)
{
	KPD_DBG("pdk_keypad_isr\n");

#ifdef CONFIG_ARCH_PDK_PC7210
	
	// read row pins
	tmp1 = socle_gpio_get_interrupt_status_with_port(PA);

	if (0xf & tmp1) {
		// disable all interrupt (row - PA0, PA1, PA2, PA3)
		socle_gpio_set_interrupt_mask_with_mask(PA, 0x0, 0xf);
	} else {
		return IRQ_HANDLED;
	}
#else
	// read row pins
	tmp1 = socle_gpio_get_interrupt_status_with_port(PA);
	tmp2 = socle_gpio_get_interrupt_status_with_port(PC);

	if ((0x7 & tmp1) || (0x1 & tmp2)) {
		// disable all interrupt (row - PA0, PA1, PA2, PC0)
		socle_gpio_set_interrupt_mask_with_mask(PA, 0x0, 0x7);
		socle_gpio_set_interrupt_mask_with_mask(PC, 0x0, 0x1);
	} else {
		return IRQ_HANDLED;
	}
#endif
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

	kpd_irq = socle_gpio_get_irq(PA);
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

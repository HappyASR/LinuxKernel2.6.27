/*
 * Copyright (C) 2008 Google, Inc.
 * Author: Nick Pelly <npelly@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/* Control bluetooth power for socle platform */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/rfkill.h>
#include <linux/delay.h>


void rfkill_switch_all(enum rfkill_type type, enum rfkill_state state);

static struct rfkill *bt_rfk;
static const char bt_name[] = "sq fake rfkill switch";

static int bluetooth_set_power(void *data, enum rfkill_state state)
{
	switch (state) {
	case RFKILL_STATE_UNBLOCKED:
		break;
	case RFKILL_STATE_SOFT_BLOCKED:
		break;
	default:
		printk(KERN_ERR "bad bluetooth rfkill state %d\n", state);
	}
	return 0;
}

static int __init socle_rfkill_probe(struct platform_device *pdev)
{
	int rc = 0;

	/* default to bluetooth off */
	rfkill_switch_all(RFKILL_TYPE_BLUETOOTH, RFKILL_STATE_SOFT_BLOCKED);
	bluetooth_set_power(NULL, RFKILL_STATE_SOFT_BLOCKED);

	bt_rfk = rfkill_allocate(&pdev->dev, RFKILL_TYPE_BLUETOOTH);
	if (!bt_rfk)
		return -ENOMEM;

	bt_rfk->name = bt_name;
	bt_rfk->state = RFKILL_STATE_SOFT_BLOCKED;
	/* userspace cannot take exclusive control */
	bt_rfk->user_claim_unsupported = 1;
	bt_rfk->user_claim = 0;
	bt_rfk->data = NULL;  // user data
	bt_rfk->toggle_radio = bluetooth_set_power;

	rc = rfkill_register(bt_rfk);

	if (rc)
		rfkill_free(bt_rfk);
	return rc;
}

static struct platform_driver socle_rfkill_driver = {
	.probe = socle_rfkill_probe,
	.driver = {
		.name = "sq_rfkill",
		.owner = THIS_MODULE,
	},
};

static int __init socle_rfkill_init(void)
{
	return platform_driver_register(&socle_rfkill_driver);
}

module_init(socle_rfkill_init);
MODULE_DESCRIPTION("SQ rfkill");
MODULE_AUTHOR("Peter Chen <peterc@socle-tech.com.tw>");
MODULE_LICENSE("GPL");


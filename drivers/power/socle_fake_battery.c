#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

#include <asm/mach-types.h>

/* BATTERY */
#define SOCLE_FAKE_BAT_MAX_VOLTAGE		4000	/* 4.00v current voltage */
#define SOCLE_FAKE_BAT_MIN_VOLTAGE		3550	/* 3.55v critical voltage */
#define SOCLE_FAKE_BAT_MAX_CURRENT		0	/* unknokn */
#define SOCLE_FAKE_BAT_MIN_CURRENT		0	/* unknown */
#define SOCLE_FAKE_BAT_MAX_CHARGE		1	/* unknown */
#define SOCLE_FAKE_BAT_MIN_CHARGE		1	/* unknown */
#define SOCLE_FAKE_MAX_LIFE_MINS		360	/* on-life in minutes */

#define SOCLE_FAKE_BAT_MEASURE_DELAY	(HZ * 1)

static int socle_fake_bat_get_property(struct power_supply *bat_ps,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = POWER_SUPPLY_TECHNOLOGY_LIPO;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = SOCLE_FAKE_BAT_MAX_VOLTAGE;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = SOCLE_FAKE_BAT_MAX_VOLTAGE;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = SOCLE_FAKE_BAT_MIN_VOLTAGE;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = 25;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = 1;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = 80;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void socle_fake_bat_external_power_changed(struct power_supply *bat_ps)
{
}

static char *status_text[] = {
	[POWER_SUPPLY_STATUS_UNKNOWN] =		"Unknown",
	[POWER_SUPPLY_STATUS_CHARGING] =	"Charging",
	[POWER_SUPPLY_STATUS_DISCHARGING] =	"Discharging",
};

static enum power_supply_property socle_fake_bat_main_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_CAPACITY,
};

struct power_supply bat_ps = {
	.name			= "battery",
	.type			= POWER_SUPPLY_TYPE_BATTERY,
	.properties		= socle_fake_bat_main_props,
	.num_properties		= ARRAY_SIZE(socle_fake_bat_main_props),
	.get_property		= socle_fake_bat_get_property,
	.external_power_changed = socle_fake_bat_external_power_changed,
	.use_for_apm		= 1,
};


#ifdef CONFIG_PM
#if  0
static int socle_fake_bat_suspend(struct platform_device *dev, pm_message_t state)
{
	return 0;
}

static int socle_fake_bat_resume(struct platform_device *dev)
{
	return 0;
}
#endif
#define socle_fake_bat_suspend NULL
#define socle_fake_bat_resume NULL
#else
#define socle_fake_bat_suspend NULL
#define socle_fake_bat_resume NULL
#endif

static int __devinit socle_fake_bat_probe(struct platform_device *dev)
{
	int ret = 0;

	ret = power_supply_register(&dev->dev, &bat_ps);

	return ret;
}

static int __devexit socle_fake_bat_remove(struct platform_device *dev)
{
	power_supply_unregister(&bat_ps);
	return 0;
}

static struct platform_driver socle_fake_bat_driver = {
	.driver		= {
		.name	= "socle-fake-battery",
	},
	.probe		= socle_fake_bat_probe,
	.remove		= __devexit_p(socle_fake_bat_remove),
	.suspend	= socle_fake_bat_suspend,
	.resume		= socle_fake_bat_resume,
};

static int __init socle_fake_bat_init(void)
{
	return platform_driver_register(&socle_fake_bat_driver);
}

static void __exit socle_fake_bat_exit(void)
{
	platform_driver_unregister(&socle_fake_bat_driver);
}

module_init(socle_fake_bat_init);
module_exit(socle_fake_bat_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Peter <peterc@socle-tech.com.tw>");
MODULE_DESCRIPTION("Fake battery driver");

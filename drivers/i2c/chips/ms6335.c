#ifdef CONFIG_I2C_DEBUG_CHIP
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <mach/ms6335.h>
#include <linux/delay.h>

/* Addresses to scan */
static unsigned short normal_i2c[] = {
	MS6335_SLAVE_ADDR_DAC,
	I2C_CLIENT_END,
};

static struct i2c_client *ms6335_dac;

I2C_CLIENT_INSMOD_1(MS6335_DAC);

static int ms6335_command(struct i2c_client *client, unsigned int cmd, void *arg);
static int ms6335_attach_adapter(struct i2c_adapter *adap);
static int ms6335_detect(struct i2c_adapter *adap, int addr, int kind);
static int ms6335_detach_client(struct i2c_client *client);

/* This is the driver that will be inserted */
static struct i2c_driver ms6335_driver = {
	.driver = {
		.name = "MS6335 Audio CODEC"
	},
//	.id = I2C_DRIVERID_MS6335,
	.attach_adapter = ms6335_attach_adapter,
	.detach_client = ms6335_detach_client,
	.command = ms6335_command,
};

static int inline
ms6335_write(struct i2c_client *client, u8 val)
{
	int ret;
	u8 buf;

	buf = val;

	ret = i2c_master_send(client, &buf, 1);
	if (ret != 1)
	{
		printk("ms6335_write error ret = %x \n",ret);
		return -1;
	}
	else
		return 0;
}

//EXPORT_SYMBOL(ms6335_dac_set_system_frequency);

extern int
ms6335_dac_power_switch(int sw)
{
	int ret = 0;
	
	dev_dbg(&ms6335_dac->dev, "ms6335_dac_power_switch(%d)\n", sw);
// 20080926 fix by cyli	
	if (0 == sw) {
		//power off 
		ret |= ms6335_write(ms6335_dac, MS6335_PWR_MODE_PRE_PWR_OFF);
		mdelay(200);
		ret |= ms6335_write(ms6335_dac, MS6335_PWR_MODE_PWR_DOWN);
		if (ret) {
			dev_err(&ms6335_dac->dev, "ms6335_dac_power_switch(): pwr off fail \n");
			return -1;
		} else
			return 0;
	} else {
		// power on
		ret |= ms6335_write(ms6335_dac, MS6335_PWR_MODE_DAC_MUTE_OFF);
		if (ret) {
			dev_err(&ms6335_dac->dev, "ms6335_dac_power_switch(): pwr on fail \n");
			return -1;
		}
		else
			return 0;
	}
}

EXPORT_SYMBOL(ms6335_dac_power_switch);

extern int 
ms6335_dac_master_volume_l(int val)
{
	int ret;

	dev_dbg(&ms6335_dac->dev, "ms6335_dac_master_volume_l(%d)\n", val);

	if (val > 31)
		val = 31;
	if (val < 0)
		val = 0;

	ret = ms6335_write(ms6335_dac, val | MS6335_FUNC_VOL_L_CTRL);
	if (ret) {
		dev_err(&ms6335_dac->dev, "ms6335_dac_master_volume_l \n");
		return -1;
	}
	return 0;
}

EXPORT_SYMBOL(ms6335_dac_master_volume_l);

extern int 
ms6335_dac_master_volume_r(int val)
{
	int ret;

	dev_dbg(&ms6335_dac->dev, "ms6335_dac_master_volume_r(%d)\n", val);
	
	if (val > 31)
		val = 31;
	if (val < 0)
		val = 0;

	ret = ms6335_write(ms6335_dac, val | MS6335_FUNC_VOL_R_CTRL);
	if (ret) {
		dev_err(&ms6335_dac->dev, "ms6335_dac_master_volume_r \n");
		return -1;
	}
	return 0;
}

EXPORT_SYMBOL(ms6335_dac_master_volume_r);

struct ms6335_state {
	u8 muted;
};

static int 
ms6335_command(struct i2c_client *client, unsigned int cmd, void *arg)
{

	return 0;
}


static int
ms6335_attach_adapter(struct i2c_adapter *adap)
{
	int ret = 0;
// 20080926 fix by cyli	
	ret |= i2c_probe(adap, &addr_data, ms6335_detect);

	ret |= ms6335_write(ms6335_dac, MS6335_AUDIO_I2S_MODE);

	ret |= ms6335_write(ms6335_dac, MS6335_FUNC_PWR_MODE |
//					MS6335_PWR_DWN_CAPGD |
					MS6335_PWR_DWN_OPAPD |
					MS6335_PWR_DWN_DACPD |
					MS6335_PWR_DWN_HPPD |
					MS6335_PWR_DWN_DACM);
	mdelay(1000);
	ret |= ms6335_write(ms6335_dac, MS6335_FUNC_PWR_MODE | MS6335_PWR_DWN_DACM);
	ret |= ms6335_write(ms6335_dac, MS6335_FUNC_PWR_MODE);
	printk("MS6335: default volume 0x1f\n");
	ret |= ms6335_write(ms6335_dac, 0x1f);	// default volume 0x0f channing 
	
	return ret;
#if 0 
	ret = ms6335_write(ms6335_dac, 0x81);
	ret = ms6335_write(ms6335_dac, 0x6f);
	ret = ms6335_write(ms6335_dac, 0x62);
	mdelay(1000);
	ret = ms6335_write(ms6335_dac, 0x60);
	ret = ms6335_write(ms6335_dac, 0xf);
	//ret = ms6335_write(ms6335_dac, 0x7b);
	//ret = ms6335_write(ms6335_dac, 0x6b);
	//ret = ms6335_write(ms6335_dac, 0x61);
	//ret = ms6335_write(ms6335_dac, 0x60);
	//ret = ms6335_write(ms6335_dac, 0xf);//0x14

	return 0;
#endif
}

/* This function is called by i2c_probe */
static int
ms6335_detect(struct i2c_adapter *adap, int addr, int kind)
{
 	struct i2c_client *new_client;
	int err = 0;
	if (!i2c_check_functionality(adap, I2C_FUNC_I2C))
		goto exit;
	new_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
	if (NULL == new_client) {
		printk(KERN_ERR "UDA1342TS: cannot allocate memory to new client\n");
		err = -ENOMEM;
		goto exit;
	}
	new_client->addr = addr;
	new_client->driver = &ms6335_driver;
	new_client->adapter = adap;
	new_client->flags = 0;

	/* Fill in the remaining client fields */
	strlcpy(new_client->name, "MS6335 Audio CODEC", I2C_NAME_SIZE);
	ms6335_dac = new_client;

	/* Tell the I2C layer a new client has arrived */
	err = i2c_attach_client(new_client);
	if (err) {
		printk(KERN_ERR "MS6335: cannot attach the new client\n");
		goto exit_kfree;
	}

	return 0;
exit_kfree:
	kfree(new_client);
exit:
	return err;
}

static int
ms6335_detach_client(struct i2c_client *client)
{
	int err;

	err = i2c_detach_client(client);
	if (err)
		return err;
	kfree(client);
	return 0;
}

static int __init
ms6335_init(void)
{
	return i2c_add_driver(&ms6335_driver);
}

static void __exit
ms6335_exit(void)
{
	i2c_del_driver(&ms6335_driver);
}

module_init(ms6335_init);
module_exit(ms6335_exit);

MODULE_AUTHOR("Ryan Chen");
MODULE_DESCRIPTION("I2C MS6335 Audio CODEC");
MODULE_LICENSE("GPL");

#ifdef CONFIG_I2C_DEBUG_CHIP
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <mach/uda1342ts.h>

/* Addresses to scan */
static unsigned short normal_i2c[] = {
	UDA1342TS_SLAVE_ADDR_ADC,
	UDA1342TS_SLAVE_ADDR_DAC,
	I2C_CLIENT_END,
};

static struct i2c_client *uda1342ts_adc;
static struct i2c_client *uda1342ts_dac;

I2C_CLIENT_INSMOD_2(UDA1342TS_ADC, UDA1342TS_DAC);

static int uda1342ts_command(struct i2c_client *client, unsigned int cmd, void *arg);
static int uda1342ts_attach_adapter(struct i2c_adapter *adap);
static int uda1342ts_detect(struct i2c_adapter *adap, int addr, int kind);
static int uda1342ts_detach_client(struct i2c_client *client);

/* This is the driver that will be inserted */
static struct i2c_driver uda1342ts_driver = {
	.driver = {
		.name = "UDA1342TS Audio CODEC"
	},
//	.id = I2C_DRIVERID_UDA1342,
	.attach_adapter = uda1342ts_attach_adapter,
	.detach_client = uda1342ts_detach_client,
	.command = uda1342ts_command,
};

static int inline
uda1342ts_write(struct i2c_client *client, u8 reg, u16 val)
{
	int ret;
	u8 buf[3];

	buf[0] = reg;
	buf[1] = *((u8 *)&val + 1);
	buf[2] = *((u8 *)&val);
	ret = i2c_master_send(client, buf, 3);
	if (ret != 3)
		return -1;
	else
		return 0;
}

static int inline
uda1342ts_read(struct i2c_client *client, u8 reg)
{
	int ret;
	struct i2c_msg msg[2];
	u8 buf =reg;
	u8 ret_buf[2];
	int ret_val;

	memset((void *)msg, 0x00, 2*sizeof(struct i2c_msg));
	msg[0].addr = client->addr;
	msg[1].addr = client->addr;
	msg[0].buf = &buf;
	msg[0].len = 1;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = ret_buf;
	msg[1].len = 2;
	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret != 2)
		return -1;
	ret_val = (ret_buf[0] << 8) | ret_buf[1];
	return ret_val; 
}

static int uda1342ts_reset(struct i2c_client *client);
static int uda1342ts_set_system_frequency(struct i2c_client *client, u16 freq);
static int uda1342ts_adc_input_amplifier_gain(int val, u16 reg);

extern int
uda1342ts_dac_reset(void)
{
	return uda1342ts_reset(uda1342ts_dac);
}

EXPORT_SYMBOL(uda1342ts_dac_reset);

extern int
uda1342ts_adc_reset(void)
{
	return uda1342ts_reset(uda1342ts_adc);
}

EXPORT_SYMBOL(uda1342ts_adc_reset);

extern int
uda1342ts_dac_set_system_frequency(u16 freq)
{
	return uda1342ts_set_system_frequency(uda1342ts_dac, freq);
}

EXPORT_SYMBOL(uda1342ts_dac_set_system_frequency);

extern int
uda1342ts_adc_set_system_frequency(u16 freq)
{
	return uda1342ts_set_system_frequency(uda1342ts_adc, freq);
}

EXPORT_SYMBOL(uda1342ts_adc_set_system_frequency);

extern int
uda1342ts_dac_power_switch(int val)
{
	int ret;
	u16 tmp;

	dev_dbg(&uda1342ts_dac->dev, "uda1342ts_dac_power_switch(%d)\n", val);
	
	ret = uda1342ts_read(uda1342ts_dac, UDA1342TS_FUNC_SYSTEM);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "reading is fail, reg:0x%02x\n", UDA1342TS_FUNC_SYSTEM);
		goto out;
	}
	tmp = (u16)ret;
	if (0 == val)
		tmp &= ~UDA1342TS_FUNC_SYSTEM_DAC_POWER_ON;
	else
		tmp |= UDA1342TS_FUNC_SYSTEM_DAC_POWER_ON;
	ret = uda1342ts_write(uda1342ts_dac, UDA1342TS_FUNC_SYSTEM, tmp);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "writing is fail, reg:0x%02x\n", UDA1342TS_FUNC_SYSTEM);
		goto out;
	}
	
	return 0;
out:
	return ret;
}

EXPORT_SYMBOL(uda1342ts_dac_power_switch);

extern int 
uda1342ts_adc_set_input_channel(u32 ch)
{
	int ret;
	u16 val;
	
	val = uda1342ts_read(uda1342ts_adc, UDA1342TS_FUNC_SYSTEM);
	if (ch == 1)
		ch = UDA1342TS_FUNC_SYSTEM_INPUT_SEL_DOUBLE_DIFF_1;
	else
		ch = UDA1342TS_FUNC_SYSTEM_INPUT_SEL_DOUBLE_DIFF_2;
	
	val = (val & ~(0x7<<9)) | ch;
	
	ret = uda1342ts_write(uda1342ts_adc, UDA1342TS_FUNC_SYSTEM, val);
	if (-1 == ret) {
		dev_err(&uda1342ts_adc->dev, "reading is fail, reg:0x%02x\n", UDA1342TS_FUNC_SYSTEM);
		goto out;
	}
	ret =  uda1342ts_read(uda1342ts_adc, UDA1342TS_FUNC_SYSTEM);
	if (-1 == ret) {
		dev_err(&uda1342ts_adc->dev, "writing is fail, reg:0x%02x\n", UDA1342TS_FUNC_SYSTEM);
		goto out;
	}
	return 0;
out:
	return ret;

}

extern int
uda1342ts_adc_power_switch(int val)
{
	int ret;
	u16 tmp;

	dev_dbg(&uda1342ts_adc->dev, "uda1342ts_adc_power_switch(%d)\n", val);
	
	ret = uda1342ts_read(uda1342ts_adc, UDA1342TS_FUNC_SYSTEM);
	if (-1 == ret) {
		dev_err(&uda1342ts_adc->dev, "reading is fail, reg:0x%02x\n", UDA1342TS_FUNC_SYSTEM);
		goto out;
	}
	tmp = (u16)ret;
	tmp &= ~UDA1342TS_FUNC_SYSTEM_ADC_MODE_MASK;
	if (0 == val)
		tmp |= UDA1342TS_FUNC_SYSTEM_ADC_POWER_OFF;
	else
		tmp |= UDA1342TS_FUNC_SYSTEM_INPUT_SEL_DOUBLE_DIFF_1;
	ret = uda1342ts_write(uda1342ts_adc, UDA1342TS_FUNC_SYSTEM, tmp);
	if (-1 == ret) {
		dev_err(&uda1342ts_adc->dev, "writing is fail, reg:0x%02x\n", UDA1342TS_FUNC_SYSTEM);
		goto out;
	}
	return 0;
out:
	return ret;
}

EXPORT_SYMBOL(uda1342ts_adc_power_switch);

extern int 
uda1342ts_dac_master_volume_l(int val)
{
	int ret;
	u16 tmp;

	dev_dbg(&uda1342ts_dac->dev, "uda1342ts_dac_master_volume_l(%d)\n", val);
	
	ret = uda1342ts_read(uda1342ts_dac, UDA1342TS_FUNC_DAC_MASTER_VOLUME);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "reading is fail, reg:0x%02x\n", UDA1342TS_FUNC_DAC_MASTER_VOLUME);
		goto out;
	}
	tmp = (u16)ret;
	tmp &= ~UDA1342TS_FUNC_DAC_MASTER_VOLUME_L_MASK;
	tmp |= UDA1342TS_FUNC_DAC_MASTER_VOLUME_L(224- val);
	ret = uda1342ts_write(uda1342ts_dac, UDA1342TS_FUNC_DAC_MASTER_VOLUME, tmp);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "writing is fail, reg:0x%02x\n", UDA1342TS_FUNC_DAC_MASTER_VOLUME);
		goto out;
	}
	return 0;
out:
	return ret;
	
}

EXPORT_SYMBOL(uda1342ts_dac_master_volume_l);

extern int 
uda1342ts_dac_master_volume_r(int val)
{
	int ret;
	u16 tmp;

	dev_dbg(&uda1342ts_dac->dev, "uda1342ts_dac_master_volume_r(%d)\n", val);
	
	ret = uda1342ts_read(uda1342ts_dac, UDA1342TS_FUNC_DAC_MASTER_VOLUME);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "reading is fail, reg:0x%02x\n", UDA1342TS_FUNC_DAC_MASTER_VOLUME);
		goto out;
	}
	tmp = (u16)ret;
	tmp &= ~UDA1342TS_FUNC_DAC_MASTER_VOLUME_R_MASK;
	tmp |= UDA1342TS_FUNC_DAC_MASTER_VOLUME_R(224- val);
	ret = uda1342ts_write(uda1342ts_dac, UDA1342TS_FUNC_DAC_MASTER_VOLUME, tmp);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "writing is fail, reg:0x%02x\n", UDA1342TS_FUNC_DAC_MASTER_VOLUME);
		goto out;
	}
	return 0;
out:
	return ret;
}

EXPORT_SYMBOL(uda1342ts_dac_master_volume_r);

extern int 
uda1342ts_adc_input_amplifier_gain_channel_1(int val)
{
	dev_dbg(&uda1342ts_adc->dev, "uda1342ts_adc_input_amplifier_gain_channel_1(%d)\n", val);
	
	return uda1342ts_adc_input_amplifier_gain(val, UDA1342TS_FUNC_ADC_INPUT_MIXER_GAIN_CH1);
}

EXPORT_SYMBOL(uda1342ts_adc_input_amplifier_gain_channel_1);

extern int 
uda1342ts_adc_input_amplifier_gain_channel_2(int val)
{
	dev_dbg(&uda1342ts_adc->dev, "uda1342ts_adc_input_amplifier_gain_channel_2(%d)\n", val);
	
	return uda1342ts_adc_input_amplifier_gain(val, UDA1342TS_FUNC_ADC_INPUT_MIXER_GAIN_CH2);
}

EXPORT_SYMBOL(uda1342ts_adc_input_amplifier_gain_channel_2);

static int 
uda1342ts_adc_input_amplifier_gain(int val, u16 reg)
{
	int ret;
	u16 tmp;

	ret = uda1342ts_read(uda1342ts_adc, reg);
	if (-1 == ret) {
		dev_err(&uda1342ts_adc->dev, "reading is fail, reg:0x%02x\n", reg);
		goto out;
	}
	tmp = (u16)ret;
	tmp &= ~UDA1342TS_FUNC_ADC_INPUT_MIXER_GAIN_INPUT_MASK;
	tmp |= UDA1342TS_FUNC_ADC_INPUT_MIXER_GAIN_INPUT(val);
	ret = uda1342ts_write(uda1342ts_adc, reg, tmp);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "writing is fail, reg:0x%02x\n", reg);
		goto out;
	}
	return 0;
out:
	return ret;
}

extern int 
uda1342ts_dac_tone_switch(int val)
{
	int ret;
	u16 tmp;

	dev_dbg(&uda1342ts_dac->dev, "uda1342ts_dac_tone_switch(%d)\n", val);
	
	ret = uda1342ts_read(uda1342ts_dac, UDA1342TS_FUNC_DAC_FEATURES);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "reading is fail, reg:0x%02x\n", UDA1342TS_FUNC_DAC_FEATURES);
		goto out;
	}
	tmp = (u16)ret;
	tmp &= ~UDA1342TS_FUNC_DAC_FEATURES_MODE_MASK;
	if (0 == val)
		tmp |= UDA1342TS_FUNC_DAC_FEATURES_FLAT;
	else
		tmp |= UDA1342TS_FUNC_DAC_FEATURES_MAX;		;
	ret = uda1342ts_write(uda1342ts_dac, UDA1342TS_FUNC_DAC_FEATURES, tmp);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "writing is fail, reg:0x%02x\n", UDA1342TS_FUNC_DAC_FEATURES);
		goto out;
	}
	return 0;
out:
	return ret;
}

EXPORT_SYMBOL(uda1342ts_dac_tone_switch);

extern int 
uda1342ts_dac_tone_bass(int val)
{
	int ret;
	u16 tmp;

	dev_dbg(&uda1342ts_dac->dev, "uda1342ts_dac_tone_switch(%d)\n", val);
	
	ret = uda1342ts_read(uda1342ts_dac, UDA1342TS_FUNC_DAC_FEATURES);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "reading is fail, reg:0x%02x\n", UDA1342TS_FUNC_DAC_FEATURES);
		goto out;
	}
	tmp = (u16)ret;
	tmp &= ~UDA1342TS_FUNC_DAC_FEATURES_BASS_BOOST_MASK;
	tmp |= UDA1342TS_FUNC_DAC_FEATURES_BASS_BOOST(val);
	ret = uda1342ts_write(uda1342ts_dac, UDA1342TS_FUNC_DAC_FEATURES, tmp);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "writing is fail, reg:0x%02x\n", UDA1342TS_FUNC_DAC_FEATURES);
		goto out;
	}
	return 0;
out:
	return ret;
}

EXPORT_SYMBOL(uda1342ts_dac_tone_bass);

extern int 
uda1342ts_dac_tone_treble(int val)
{
	int ret;
	u16 tmp;

	dev_dbg(&uda1342ts_dac->dev, "uda1342ts_dac_tone_switch(%d)\n", val);
	
	ret = uda1342ts_read(uda1342ts_dac, UDA1342TS_FUNC_DAC_FEATURES);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "reading is fail, reg:0x%02x\n", UDA1342TS_FUNC_DAC_FEATURES);
		goto out;
	}
	tmp = (u16)ret;
	tmp &= ~UDA1342TS_FUNC_DAC_FEATURES_TREBLE_MASK;
	tmp |= UDA1342TS_FUNC_DAC_FEATURES_TREBLE(val);
	ret = uda1342ts_write(uda1342ts_dac, UDA1342TS_FUNC_DAC_FEATURES, tmp);
	if (-1 == ret) {
		dev_err(&uda1342ts_dac->dev, "writing is fail, reg:0x%02x\n", UDA1342TS_FUNC_DAC_FEATURES);
		goto out;
	}
	return 0;
out:
	return ret;
}

EXPORT_SYMBOL(uda1342ts_dac_tone_treble);

extern int 
uda1342ts_set_input_channel(u32 ch)
{
	int ret;
	u16 val;
	
	val = uda1342ts_read(uda1342ts_adc,UDA1342TS_FUNC_SYSTEM);
	if (ch == 1)
		ch = UDA1342TS_FUNC_SYSTEM_INPUT_SEL_DOUBLE_DIFF_1;
	else
		ch = UDA1342TS_FUNC_SYSTEM_INPUT_SEL_DOUBLE_DIFF_2;
	
	val = (val & ~(0x7<<9)) | ch;
	
	ret = uda1342ts_write(uda1342ts_adc,UDA1342TS_FUNC_SYSTEM, val);
	if (-1 == ret)
		printk("Writing UDA1342TS is fail, reg: 0x%02x\n", UDA1342TS_FUNC_SYSTEM);
	ret = uda1342ts_read(uda1342ts_adc,UDA1342TS_FUNC_SYSTEM);
	if (-1 == ret) {
		printk("Reading UDA1342TS is fail, reg: 0x%02x\n", UDA1342TS_FUNC_SYSTEM);
	}
//	printf("UDA1342TS_FUNC_SYSTEM: 0x%04x\n", ret);
	return ret;
}

EXPORT_SYMBOL(uda1342ts_set_input_channel);

static int
uda1342ts_reset(struct i2c_client *client)
{
	int ret;
	u16 val;

	dev_dbg(&client->dev, "UDA1342TS reset\n");

	ret = uda1342ts_read(client, UDA1342TS_FUNC_SYSTEM);
	if (-1 == ret) {
		dev_err(&client->dev, "reading is fail, reg:0x%02x\n", UDA1342TS_FUNC_SYSTEM);
		goto out;
	}
	val = (u16)ret;
	val |= UDA1342TS_FUNC_SYSTEM_RST;
	ret = uda1342ts_write(client, UDA1342TS_FUNC_SYSTEM, val);
	if (-1 == ret) {
		dev_err(&client->dev, "writing is fail, reg:0x%02x\n", UDA1342TS_FUNC_SYSTEM);
		goto out;
	}
	return 0;
out:
	return ret;
}

static int
uda1342ts_set_system_frequency(struct i2c_client *client, u16 freq)
{
	int ret;
	u16 val;

	dev_dbg(&client->dev, "UDA1342TS set system frequency\n");
	ret = uda1342ts_read(client, UDA1342TS_FUNC_SYSTEM);
	if (-1 == ret) {
		dev_err(&client->dev, "reading is fail, reg:0x%02x\n", UDA1342TS_FUNC_SYSTEM);
		goto out;
	}
	val = (u16)ret;
	val &= ~UDA1342TS_FUNC_SYSTEM_SYS_CLK_MASK;
	val |= freq;
	ret = uda1342ts_write(client, UDA1342TS_FUNC_SYSTEM, val);
	if (-1 == ret) {
		dev_err(&client->dev, "writing is fail, reg:0x%02x\n", UDA1342TS_FUNC_SYSTEM);
		goto out;
	}
	ret = uda1342ts_read(client, UDA1342TS_FUNC_SYSTEM);
	if (-1 == ret) {
		dev_err(&client->dev, "reading is fail, reg:0x%02x\n", UDA1342TS_FUNC_SYSTEM);
		goto out;
	}
	dev_dbg(&client->dev, "UDA1342TS_FUNC_SYSTEM:0x%04x\n", (u16)ret);
	return 0;
out:
	return ret;
}

struct uda1342ts_state {
	u8 muted;
};

static int 
uda1342ts_command(struct i2c_client *client, unsigned int cmd, void *arg)
{
	struct uda1342ts_state *state = i2c_get_clientdata(client);
//	struct v4l2_control *ctrl = arg;
	u32* freq = arg;
	printk("cmd = %x , arg = %x \n",cmd,arg);
#if 0
	switch (cmd) {
	case VIDIOC_INT_AUDIO_CLOCK_FREQ:
		switch (*freq) {
			case 32000: /* set sample rate to 32 kHz */
				uda1342ts_write(client, 8, 0x018);
				break;
			case 44100: /* set sample rate to 44.1 kHz */
				uda1342ts_write(client, 8, 0x022);
				break;
			case 48000: /* set sample rate to 48 kHz */
				uda1342ts_write(client, 8, 0x000);
				break;
			default:
				return -EINVAL;
		}
		break;

	case VIDIOC_G_CTRL:
		if (ctrl->id != V4L2_CID_AUDIO_MUTE)
			return -EINVAL;
		ctrl->value = state->muted;
		break;

	case VIDIOC_S_CTRL:
		if (ctrl->id != V4L2_CID_AUDIO_MUTE)
			return -EINVAL;
		state->muted = ctrl->value;
		tlv320aic23b_write(client, 0, 0x180); /* mute both channels */
		/* set gain on both channels to +3.0 dB */
		if (!state->muted)
			tlv320aic23b_write(client, 0, 0x119);
		break;

	case VIDIOC_LOG_STATUS:
		v4l_info(client, "Input: %s\n",
			    state->muted ? "muted" : "active");
		break;

	default:
		return -EINVAL;
	}
	return 0;
#endif
	return 0;
}


static int
uda1342ts_attach_adapter(struct i2c_adapter *adap)
{
	int ret;
	i2c_probe(adap, &addr_data, uda1342ts_detect);
#if 1
	//initial dac volume add by ryan 20080310
	ret = uda1342ts_write(uda1342ts_dac, 0x00,0x1a62);
	ret = uda1342ts_write(uda1342ts_dac, 0x10,0xc300);
	ret = uda1342ts_write(uda1342ts_dac, 0x20, 0x30);
#endif
	return 0;
}

/* This function is called by i2c_probe */
static int
uda1342ts_detect(struct i2c_adapter *adap, int addr, int kind)
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
	new_client->driver = &uda1342ts_driver;
	new_client->adapter = adap;
	new_client->flags = 0;
	if (0x1a == addr)
		kind = UDA1342TS_ADC;
	else if (0x01b == addr)
		kind = UDA1342TS_DAC;

	/* Fill in the remaining client fields */
	if (UDA1342TS_ADC == kind) {
		strlcpy(new_client->name, "UDA1342TS Audio CODEC ADC", I2C_NAME_SIZE);
		uda1342ts_adc = new_client;
	}
	else if (UDA1342TS_DAC == kind) {
		strlcpy(new_client->name, "UDA1342TS Audio CODEC DAC", I2C_NAME_SIZE);
		uda1342ts_dac = new_client;
	}

	/* Tell the I2C layer a new client has arrived */
	err = i2c_attach_client(new_client);
	if (err) {
		printk(KERN_ERR "UDA1342TS: cannot attach the new client\n");
		goto exit_kfree;
	}
	
	return 0;
exit_kfree:
	kfree(new_client);
exit:
	return err;
}

static int
uda1342ts_detach_client(struct i2c_client *client)
{
	int err;

	err = i2c_detach_client(client);
	if (err)
		return err;
	kfree(client);
	return 0;
}

static int __init
uda1342ts_init(void)
{
	return i2c_add_driver(&uda1342ts_driver);
}

static void __exit
uda1342ts_exit(void)
{
	i2c_del_driver(&uda1342ts_driver);
}

module_init(uda1342ts_init);
module_exit(uda1342ts_exit);

MODULE_AUTHOR("Obi Hsieh");
MODULE_DESCRIPTION("I2C UDA1342TS Audio CODEC");
MODULE_LICENSE("GPL");

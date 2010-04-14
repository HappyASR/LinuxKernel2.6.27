/*
 * File Name     : drivers/char/socle_adc_max1110.c 
 * Author         : Leonid Cheng
 * Description   : Socle ADC MAX1110 Controller Driver (ADC)
 *
 * Copyright (c) 2008 Marc Pignat <marc.pignat@hevs.ch>
 *
 * The socle_adc_max1110 communicates with a host processor via an SPI/Microwire Bus
 * interface. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/sysfs.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/mutex.h>
#include <linux/spi/spi.h>

#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <mach/regs-socle-adc-max1110.h>
#include <mach/regs-pwmt.h>


#define SOCLE_MAX1110_REFIN			2048    //2.048
#define SOCLE_MAX1110_PWM_INPUT		1563    //1.563         //JUMP 23
#define SOCLE_MAX1110_REF_INPUT		3248    //3.248         //JUMP 12

#define SOCLE_MAX1110_INPUT	SOCLE_MAX1110_PWM_INPUT

#define USE_PWM_NUM 1

#define SET_TX_RX_LEN(tx, rx)	(((tx) << 16) | (rx))

//#define SOCLE_ADC_MAX1110_DEBUG

#ifdef SOCLE_ADC_MAX1110_DEBUG
#define ADCDBUG(fmt, args...) printk("%s : " fmt, __FUNCTION__, ## args)
#else
#define ADCDBUG(fmt, args...) 
#endif


static int adc_max1110_read_data(u8 cmd, u16 *buf);
static int socle_adc_max1110_arg_to_control(int arg);
static void socle_pwmt_input_init(struct socle_pwmt *p_pwmt, int enable);
static int socle_spi_max1110_convert(u16 buf, int control);
static int socle_spi_max1110_unipolar_calculate(int reg_value);
static int socle_spi_max1110_bipolar_calculate(int reg_value);
static int socle_adc_max1110_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int socle_adc_max1110_read(struct file *filp, char __user *buf, size_t count, loff_t *pos);
static int socle_adc_max1110_open(struct inode *inode, struct file *file);
static int socle_adc_max1110_release(struct inode *inode, struct file *file);


static int adc_busy = 0;
struct spi_device *spi_max1110;

static int 
adc_max1110_read_data(u8 cmd, u16 *buf)
{
	int err = 0;
	struct spi_message msg;
	struct spi_transfer xfer;
	
	ADCDBUG("cmd= 0x%x\n", cmd);
	
	spi_message_init(&msg);
	memset(&xfer, 0, sizeof(struct spi_transfer));	
	
	xfer.tx_buf = &cmd;
	xfer.rx_buf = buf;
	xfer.len = SET_TX_RX_LEN(1, 2);
	spi_message_add_tail(&xfer, &msg);
	err = spi_sync(spi_max1110, &msg);

	ADCDBUG("xfer.rx_buf = 0x%x\n", *buf);
	
	return err;	
}


static void
socle_pwmt_input_init(struct socle_pwmt *p_pwmt, int enable)
{
	struct socle_pwmt_driver *pwmt_drv = p_pwmt->drv;

	pwmt_drv->claim_pwm_lock();

	pwmt_drv->reset(p_pwmt);

	pwmt_drv->write_hrc(p_pwmt, 0x100);
	pwmt_drv->write_lrc(p_pwmt, 0x100);
	pwmt_drv->capture_mode_enable(p_pwmt, 1);
	pwmt_drv->output_enable(p_pwmt, 1);
	pwmt_drv->enable(p_pwmt, 1);
    
	pwmt_drv->release_pwm_lock();
}


struct adcxx {
	struct device *hwmon_dev;
	struct mutex lock;
	u32 channels;
	u32 reference; /* in millivolts */
};

/* sysfs hook function */
static ssize_t adcxx_read(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct spi_device *spi = to_spi_device(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct adcxx *adc = dev_get_drvdata(&spi->dev);
	u8 tx_buf[1]={0} ;
	u8 rx_buf[2];
	int status;
	int value;

	if (mutex_lock_interruptible(&adc->lock))
		return -ERESTARTSYS;

	switch(attr->index){
		case 0:
			tx_buf[0]  = MAX1110_SINGLE_END_CH0;
			break;
		case 1:
			tx_buf[0]  = MAX1110_SINGLE_END_CH1;
			break;
		case 2:
			tx_buf[0]  = MAX1110_SINGLE_END_CH2;
			break;
		case 3:
			tx_buf[0]  = MAX1110_SINGLE_END_CH3;
			break;
		case 4:
			tx_buf[0]  = MAX1110_SINGLE_END_CH4;
			break;
		case 5:
			tx_buf[0]  = MAX1110_SINGLE_END_CH5;
			break;
		case 6:
			tx_buf[0]  = MAX1110_SINGLE_END_CH6;
			break;
		case 7:
			tx_buf[0]  = MAX1110_SINGLE_END_CH7;
			break;
	}

	tx_buf[0] |= MAX1110_START |MAX1110_UNI_BIP_UNIPOLAR | MAX1110_SGL_DIF_SGL | 
		MAX1110_PD1_FULL_OP | MAX1110_PD0_EXT_CLK;
	
	ADCDBUG("tx_buf[0] = 0x%x\n", tx_buf[0]);

	status = adc_max1110_read_data(tx_buf[0], (u16 *)rx_buf);
	if (status < 0) {
		dev_warn(dev, "spi_write_then_read failed with status %d\n",
				status);
		goto out;
	}
	ADCDBUG("rx_buf[0] = 0x%x\n", rx_buf[0]);
	ADCDBUG("rx_buf[1] = 0x%x\n", rx_buf[1]);
	value = ((rx_buf[0]&0x3f) << 2) + (rx_buf[1]>>6);
	ADCDBUG("raw value = 0x%x\n", value);
	
	value = socle_spi_max1110_unipolar_calculate(value);

	status = sprintf(buf, "%d\n", value);
out:
	mutex_unlock(&adc->lock);
	return status;
}

static ssize_t adcxx_show_min(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	/* The minimum reference is 0 for this chip family */
	return sprintf(buf, "0\n");
}

static ssize_t adcxx_show_max(struct device *dev,
		struct device_attribute *devattr, char *buf)
{
	struct spi_device *spi = to_spi_device(dev);
	struct adcxx *adc = dev_get_drvdata(&spi->dev);
	u32 reference;

	if (mutex_lock_interruptible(&adc->lock))
		return -ERESTARTSYS;

	reference = adc->reference;

	mutex_unlock(&adc->lock);

	return sprintf(buf, "%d\n", reference);
}

static ssize_t adcxx_set_max(struct device *dev,
	struct device_attribute *devattr, const char *buf, size_t count)
{
	struct spi_device *spi = to_spi_device(dev);
	struct adcxx *adc = dev_get_drvdata(&spi->dev);
	unsigned long value;

	if (strict_strtoul(buf, 10, &value))
		return -EINVAL;

	if (mutex_lock_interruptible(&adc->lock))
		return -ERESTARTSYS;

	adc->reference = value;

	mutex_unlock(&adc->lock);

	return count;
}

static ssize_t adcxx_show_name(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	//struct spi_device *spi = to_spi_device(dev);
	//struct adcxx *adc = dev_get_drvdata(&spi->dev);

	//return sprintf(buf, "adcxx%ds\n", adc->channels);
	return sprintf(buf, "hwmon_adc_max1110\n");
}

static struct sensor_device_attribute ad_input[] = {
	SENSOR_ATTR(name, S_IRUGO, adcxx_show_name, NULL, 0),
	SENSOR_ATTR(in_min, S_IRUGO, adcxx_show_min, NULL, 0),
	SENSOR_ATTR(in_max, S_IWUSR | S_IRUGO, adcxx_show_max,
					adcxx_set_max, 0),
	SENSOR_ATTR(in0_input, S_IRUGO, adcxx_read, NULL, 0),
	SENSOR_ATTR(in1_input, S_IRUGO, adcxx_read, NULL, 1),
	SENSOR_ATTR(in2_input, S_IRUGO, adcxx_read, NULL, 2),
	SENSOR_ATTR(in3_input, S_IRUGO, adcxx_read, NULL, 3),
	SENSOR_ATTR(in4_input, S_IRUGO, adcxx_read, NULL, 4),
	SENSOR_ATTR(in5_input, S_IRUGO, adcxx_read, NULL, 5),
	SENSOR_ATTR(in6_input, S_IRUGO, adcxx_read, NULL, 6),
	SENSOR_ATTR(in7_input, S_IRUGO, adcxx_read, NULL, 7),
};

static int 
socle_adc_max1110_arg_to_control(int arg)
{
	int control=0;
	
	control = MAX1110_START | MAX1110_PD1_FULL_OP | MAX1110_PD0_EXT_CLK;
	
	if((arg & ADC_MAX1110_AP_OUTPUT_TYPE) == ADC_MAX1110_AP_OUTPUT_TYPE_SGL){
		control |= MAX1110_SGL_DIF_SGL | MAX1110_UNI_BIP_UNIPOLAR;
		switch(arg & ADC_MAX1110_AP_CHx){
			case ADC_MAX1110_AP_SGL_CH0:
				control |= MAX1110_SINGLE_END_CH0;
				break;
			case ADC_MAX1110_AP_SGL_CH1:
				control |= MAX1110_SINGLE_END_CH1;
				break;
			case ADC_MAX1110_AP_SGL_CH2:
				control |= MAX1110_SINGLE_END_CH2;
				break;
			case ADC_MAX1110_AP_SGL_CH3:
				control |= MAX1110_SINGLE_END_CH3;
				break;
			case ADC_MAX1110_AP_SGL_CH4:
				control |= MAX1110_SINGLE_END_CH4;
				break;
			case ADC_MAX1110_AP_SGL_CH5:
				control |= MAX1110_SINGLE_END_CH5;
				break;
			case ADC_MAX1110_AP_SGL_CH6:
				control |= MAX1110_SINGLE_END_CH6;
				break;
			case ADC_MAX1110_AP_SGL_CH7:
				control |= MAX1110_SINGLE_END_CH7;
				break;
		}
	}else{
		control |= MAX1110_SGL_DIF_DIF | MAX1110_UNI_BIP_BIPOLAR;
		switch(arg & ADC_MAX1110_AP_CHx){
			case ADC_MAX1110_AP_DIFF_CH0_1:
				control |= MAX1110_DIFFERENTIAL_CH0_CH1;
				break;
			case ADC_MAX1110_AP_DIFF_CH1_0:
				control |= MAX1110_DIFFERENTIAL_CH1_CH0;
				break;
			case ADC_MAX1110_AP_DIFF_CH2_3:
				control |= MAX1110_DIFFERENTIAL_CH2_CH3;
				break;
			case ADC_MAX1110_AP_DIFF_CH3_2:
				control |= MAX1110_DIFFERENTIAL_CH3_CH2;
				break;
			case ADC_MAX1110_AP_DIFF_CH4_5:
				control |= MAX1110_DIFFERENTIAL_CH4_CH5;
				break;
			case ADC_MAX1110_AP_DIFF_CH5_4:
				control |= MAX1110_DIFFERENTIAL_CH5_CH4;
				break;
			case ADC_MAX1110_AP_DIFF_CH6_7:
				control |= MAX1110_DIFFERENTIAL_CH6_CH7;
				break;
			case ADC_MAX1110_AP_DIFF_CH7_6:
				control |= MAX1110_DIFFERENTIAL_CH7_CH6;
				break;
		}
	}
	
	return control;
}

static int
socle_spi_max1110_convert(u16 buf, int control)
{
	int reg_value, adc_value;
			
	reg_value = ((buf&0x3f)<<2) | ((buf&0xc000)>>14);

	ADCDBUG("reg value = %4d(0x%x)\n", reg_value, reg_value);
	ADCDBUG("control = 0x%x\n", control);


	if((control & MAX1110_UNI_BIP) == MAX1110_UNI_BIP_UNIPOLAR)
		adc_value = socle_spi_max1110_unipolar_calculate(reg_value);
	else
		adc_value = socle_spi_max1110_bipolar_calculate(reg_value);

	if(adc_value > 0)
		ADCDBUG("adc_value = %d.%03d\n", adc_value/1000, adc_value%1000);
	else
		ADCDBUG("adc_value = -%d.%03d\n", (-adc_value)/1000, (-adc_value)%1000);

	return adc_value;
}


static int
socle_spi_max1110_unipolar_calculate(int reg_value)
{
        int adc;

        ADCDBUG("sq_spi_max1110_unipolar_calculate\n");
        
        ADCDBUG("SQ_MAX1110_REFIN = %d\n", SOCLE_MAX1110_REFIN);
        ADCDBUG("reg_value = %d\n", reg_value);
        
        adc = SOCLE_MAX1110_REFIN * reg_value / 256;

        ADCDBUG("adc = %d\n", adc);
        
        return adc;

}

static int
socle_spi_max1110_bipolar_calculate(int reg_value)
{
        int adc;
        int n_flag=0;
        
        ADCDBUG("sq_spi_max1110_bipolar_calculate\n");

        if(reg_value>=128){           //- val
                n_flag=1;
                reg_value = 256 - reg_value;
        }
        
        ADCDBUG("SQ_MAX1110_REFIN = %d\n", SOCLE_MAX1110_REFIN);
        ADCDBUG("reg_value = %d\n", reg_value);

        adc = SOCLE_MAX1110_REFIN * reg_value / 256;

        ADCDBUG("adc = %d\n", adc);
        
        if(n_flag)
                adc = -adc;

        ADCDBUG("adc2 = %d\n", adc);
        
        return adc;

}


static int socle_adc_max1110_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int control;
	u16 buf;
	int adc_value;
		
	ADCDBUG("cmd=0x%x, arg=0x%lx\n", cmd, arg);

	switch (cmd){
		case ADC_MAX1110_IOC_START_ADC:
			ADCDBUG("Start ADC Convert\n");
			break;
		default:
			printk("unknown IOCTL command\n");
			return -EINVAL;
			break;
	}
	
	control = socle_adc_max1110_arg_to_control(arg);
	ADCDBUG("control = 0x%x\n", control);
	
	adc_max1110_read_data((u8)control, &buf);
	ADCDBUG("buf = 0x%x\n", buf);
	
	adc_value = socle_spi_max1110_convert(buf, control);
	ADCDBUG("adc_value = %d\n", adc_value);
	
	if(adc_value < 0)
		adc_value = -adc_value + 1000000;
	
	return adc_value;
}


static int socle_adc_max1110_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	ADCDBUG("\n");

	return 0;

}

static int socle_adc_max1110_open(struct inode *inode, struct file *file)
{
	if (adc_busy)
		return -EBUSY;

	ADCDBUG("\n");
	adc_busy = 1;

	return 0;	
}

static int socle_adc_max1110_release(struct inode *inode, struct file *file)
{
	ADCDBUG("\n");	
	adc_busy = 0;
	
	return 0;
}




static const struct file_operations adc_max1110_fops = {
        .owner =        THIS_MODULE,
        .ioctl =        socle_adc_max1110_ioctl,
        .open =         socle_adc_max1110_open,
        .read = 		socle_adc_max1110_read,
        .release =      socle_adc_max1110_release,
};


struct miscdevice misc_adc_max1110 = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "sq_adc_max1110",
	.fops = &adc_max1110_fops,
};


/*----------------------------------------------------------------------*/

static int __devinit adcxx_probe(struct spi_device *spi, int channels)
{
	struct adcxx *adc;
	int status;
	int i;
	int err;
	struct socle_pwmt *p_pwmt_cur;

	printk("adcxx_probe\n");

	adc = kzalloc(sizeof *adc, GFP_KERNEL);
	if (!adc)
		return -ENOMEM;

	/* set a default value for the reference */
	adc->reference = SOCLE_MAX1110_INPUT;	
	adc->channels = channels;
	mutex_init(&adc->lock);

	mutex_lock(&adc->lock);

	dev_set_drvdata(&spi->dev, adc);

	for (i = 0; i < 3 + adc->channels; i++) {
		status = device_create_file(&spi->dev, &ad_input[i].dev_attr);
		if (status) {
			dev_err(&spi->dev, "device_create_file failed.\n");
			goto out_err;
		}
	}

	adc->hwmon_dev = hwmon_device_register(&spi->dev);
	if (IS_ERR(adc->hwmon_dev)) {
		dev_err(&spi->dev, "hwmon_device_register failed.\n");
		status = PTR_ERR(adc->hwmon_dev);
		goto out_err;
	}
		
	p_pwmt_cur = get_socle_pwmt_structure(USE_PWM_NUM);
	
	if (NULL == p_pwmt_cur) {
		printk("Get PWMT structure error!!\n");
		return -1;
	}

	socle_pwmt_input_init(p_pwmt_cur, 1);
			
	spi_max1110 = spi;
	spi_max1110->bits_per_word = 8;
	spi_setup(spi_max1110);		

	
	err = misc_register(&misc_adc_max1110);
	if(err)
		goto out_err;

	mutex_unlock(&adc->lock);
	return 0;

out_err:
	for (i--; i >= 0; i--)
		device_remove_file(&spi->dev, &ad_input[i].dev_attr);

	dev_set_drvdata(&spi->dev, NULL);
	mutex_unlock(&adc->lock);
	kfree(adc);
	return status;
}

static int __devinit adcxx8s_probe(struct spi_device *spi)
{
	return adcxx_probe(spi, 8);
}

static int __devexit adcxx_remove(struct spi_device *spi)
{
	struct adcxx *adc = dev_get_drvdata(&spi->dev);
	int i;

	mutex_lock(&adc->lock);

	misc_deregister(&misc_adc_max1110);
	release_socle_pwmt_structure(USE_PWM_NUM);	
	
	hwmon_device_unregister(adc->hwmon_dev);
	for (i = 0; i < 3 + adc->channels; i++)
		device_remove_file(&spi->dev, &ad_input[i].dev_attr);

	dev_set_drvdata(&spi->dev, NULL);
	mutex_unlock(&adc->lock);
	kfree(adc);

	return 0;
}

static struct spi_driver hwmon_adc_max1110_driver = {
	.driver = {
		.name	= "hwmon_adc_max1110",
		.owner	= THIS_MODULE,
	},
	.probe	= adcxx8s_probe,
	.remove	= __devexit_p(adcxx_remove),
};

static int __init init_adc_max1110(void)
{
	int status;

	ADCDBUG("init_adc_max1110\n");

	status = spi_register_driver(&hwmon_adc_max1110_driver);
	
	return status;
}

static void __exit exit_adc_max1110(void)
{
	spi_unregister_driver(&hwmon_adc_max1110_driver);
}

module_init(init_adc_max1110);
module_exit(exit_adc_max1110);

MODULE_AUTHOR("Leonid Cheng");
MODULE_DESCRIPTION("SQ ADC MAX1110 Linux driver");
MODULE_LICENSE("GPL");

MODULE_ALIAS("adc max1110");

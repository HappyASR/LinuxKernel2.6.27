/********************************************************************************
* File Name     : drivers/char/socle-inr.c 
* Author        : CY Li
* Description   : SQ Internet Radio Driver
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
    
*   Version      : 2,0,0,1
*   History      : 
*      1. 2008/07/25 cyli create this file 
*    
********************************************************************************/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/input.h>

#include <asm/io.h>
#include <asm/memory.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <asm/uaccess.h>

#include "socle-inr.h"
#include "lcm-t6963c.h"

//#define CONFIG_SOCLE_INR_DEBUG
#ifdef CONFIG_SOCLE_INR_DEBUG
	#define INR_DBG(fmt, args...) printk("SQ_INR: %s(): " fmt, __FUNCTION__, ## args)
#else
	#define INR_DBG(fmt, args...)
#endif


static int t6963c_database;
static struct socle_pwmt *pwm_bklgt, *pwm_cntrt;

extern struct input_dev *pdk_kpd;
extern unsigned char *pdkkpd_keycode;

static void socle_inr_set_volume(void *data);
static DECLARE_WORK(work, socle_inr_set_volume, 0);

static void socle_inr_iPod_detection(unsigned long data);
static DEFINE_TIMER(inr_iPod, socle_inr_iPod_detection, 0, 0);

static void socle_inr_line_in_detection(unsigned long data);
static DEFINE_TIMER(inr_line, socle_inr_line_in_detection, 0, 0);

static char __initdata banner[] = "SQ Internet Radio, (c) 2010 SQ Corp.\n";

static inline u8
t6963c_read_data_port(void)
{
	return ioread16(t6963c_database);
}

static inline void
t6963c_write_data_port(u8 data)
{
	iowrite16(data, t6963c_database);
}

static inline u8
t6963c_read_cmd_port(void)
{
	return ioread16(t6963c_database + 2);
}

static inline void
t6963c_write_cmd_port(u8 data)
{
	iowrite16(data, t6963c_database + 2);
}

static inline int
t6963c_check_status(u8 flag)
{
#if 0
	int t = 0;

	while (!(t6963c_read_cmd_port() & flag)) {
		t++;
		if (t > 0x1000) {
			printk("t6963c_check_status timoe out!\n");
			return -1;
		}
	}
	udelay(10);
#else
	udelay(20);
#endif
	return 0;
}

static inline int
t6963c_write_command(u8 cmd)
{
	if (t6963c_check_status(STATUS_EXEC_CAP | STATUS_DATA_RW_CAP))
		return -1;
	t6963c_write_cmd_port(cmd);
	return 0;
}

static inline int
t6963c_write_data_8(u8 data)
{
	if (t6963c_check_status(STATUS_EXEC_CAP | STATUS_DATA_RW_CAP))
		return -1;
	t6963c_write_data_port(data);
	return 0;
}

static inline int
t6963c_write_data_16(u16 data)
{
	// low byte
	if (t6963c_check_status(STATUS_EXEC_CAP | STATUS_DATA_RW_CAP))
		return -1;
	t6963c_write_data_port(data & 0xff);

	// high byte
	if (t6963c_check_status(STATUS_EXEC_CAP | STATUS_DATA_RW_CAP))
		return -1;
	t6963c_write_data_port((data & 0xff00) >> 8);

	return 0;
}

static inline int
t6963c_auto_write_data(u8 data)
{
	if (t6963c_check_status(STATUS_AUTO_DATA_W_CAP))
		return -1;
	t6963c_write_data_port(data);
	return 0;
}

static inline int
t6963c_write_single_display_data(u16 addr, u8 data, int num)
{
	int i;

	// set address pointer
	if (t6963c_write_data_16(addr))
		return -1;
	if (t6963c_write_command(CMD_SET_ADR_PTR))
		return -1;

	if (t6963c_write_command(CMD_SET_DATA_AUTO_W))
		return -1;

	for (i = 0; i < num; i++)
		if (t6963c_auto_write_data(data))
			return -1;

	if (t6963c_write_command(CMD_SET_AUTO_RESET))
		return -1;

	return 0;
}

static inline int
t6963c_write_multi_display_data(u16 addr, u8 *data, int num)
{
	int i;

	//printk("addr=0x%04x, num=%d\n", addr, num);

	// set address pointer
	if (t6963c_write_data_16(addr))
		return -1;
	if (t6963c_write_command(CMD_SET_ADR_PTR))
		return -1;

	if (t6963c_write_command(CMD_SET_DATA_AUTO_W))
		return -1;

	for (i = 0; i < num; i++)
		if (t6963c_auto_write_data(data[i]))
			return -1;

	if (t6963c_write_command(CMD_SET_AUTO_RESET))
		return -1;

	return 0;
}

static inline int
t6963c_write_multi_display_char(u16 addr, u8 *data, int num)
{
	int i;

	//printk("addr=0x%04x, num=%d\n", addr, num);

	// set address pointer
	if (t6963c_write_data_16(addr))
		return -1;
	if (t6963c_write_command(CMD_SET_ADR_PTR))
		return -1;

	if (t6963c_write_command(CMD_SET_DATA_AUTO_W))
		return -1;

#ifdef T6963C_REVERSE
	for (i = 0; i < num; i++)
		if (t6963c_auto_write_data(data[num - i - 1] + 0x80 - CHAR_SPACE))
			return -1;
#else
	for (i = 0; i < num; i++)
		if (t6963c_auto_write_data(data[i] - CHAR_SPACE))
			return -1;
#endif

	if (t6963c_write_command(CMD_SET_AUTO_RESET))
		return -1;

	return 0;
}

static inline int
t6963c_write_ext_char_gen_data(u8 *data, int base, int num)
{
	// write external character genrator data
	if (t6963c_write_multi_display_data(CG_RAM_ADDR(base), data, num))
		return -1;
	return 0;
}

static inline int
t6963c_write_attribute_data(int line, int column, int num, int attr_data)
{
#ifdef T6963C_REVERSE
	if (t6963c_write_single_display_data(GRA_HOME_ADDR + (TXT_LINE - line - 1) * COLUMN + (COLUMN - column - 1) - num, attr_data, num))
		return -1;
#else
	if (t6963c_write_single_display_data(GRA_HOME_ADDR + line * COLUMN + column, attr_data, num))
		return -1;
#endif
	return 0;
}

static inline int
t6963c_write_string_data(int line, int column, u8 *str, int len)
{
#ifdef T6963C_REVERSE
	if (t6963c_write_multi_display_char(TXT_HOME_ADDR + (TXT_LINE - line - 1) * COLUMN + (COLUMN - column - 1) - len, str, len))
		return -1;
#else
	if (t6963c_write_multi_display_char(TXT_HOME_ADDR + line * COLUMN + column, str, len))
		return -1;
#endif
	return 0;
}

static int
socle_lcm_init(void)
{
	extern u32 apb_clock;
	u32 bklgt_lrc = apb_clock / 2 / 25000;	// 25K
	u32 cntrt_lrc = apb_clock / 2 / 1000;	// 1K

	// backlight
	pwm_bklgt = get_socle_pwmt_structure(T6963C_BKLGT_PWM_IDX);
	if (NULL == pwm_bklgt) {
		printk("Can't get PWMT structure (pwm_bklgt)!!\n");
		return -1;
	}
	pwm_bklgt->drv->reset(pwm_bklgt);
	pwm_bklgt->drv->write_hrc(pwm_bklgt, bklgt_lrc / 100 * (6 * 6));
	pwm_bklgt->drv->write_lrc(pwm_bklgt, bklgt_lrc);
	pwm_bklgt->drv->enable(pwm_bklgt, 1);
	pwm_bklgt->drv->output_enable(pwm_bklgt, 1);

	// contrast
	pwm_cntrt = get_socle_pwmt_structure(T6963C_CNTRT_PWM_IDX);
	if (NULL == pwm_cntrt) {
		printk("Can't get PWMT structure (pwm_cntrt)!!\n");
		return -1;
	}
	pwm_cntrt->drv->reset(pwm_cntrt);
	pwm_cntrt->drv->write_hrc(pwm_cntrt, cntrt_lrc / 100 * 100);
	pwm_cntrt->drv->write_lrc(pwm_cntrt, cntrt_lrc);
	pwm_cntrt->drv->enable(pwm_cntrt, 1);
	pwm_cntrt->drv->output_enable(pwm_cntrt, 1);

	// set text home address TXT_HOME_ADDR
	if (t6963c_write_data_16(TXT_HOME_ADDR))
		return -1;
	if (t6963c_write_command(CMD_SET_TXT_HOM_ADR))
		return -1;

	// set graphic home address GRA_HOME_ADDR
	if (t6963c_write_data_16(GRA_HOME_ADDR))
		return -1;
	if (t6963c_write_command(CMD_SET_GRA_HOM_ADR))
		return -1;

	// set text area COLUMN columns
	if (t6963c_write_data_16(COLUMN))
		return -1;
	if (t6963c_write_command(CMD_SET_TXT_ARE))
		return -1;

	// set graphic area COLUMN columns
	if (t6963c_write_data_16(COLUMN))
		return -1;
	if (t6963c_write_command(CMD_SET_GRA_ARE))
		return -1;

	// set "xor mode", "internal character generater mode"
	if (t6963c_write_command(CMD_SET_TXT_ATT_MODE))	//CMD_SET_XOR_MODE
		return -1;

	// set offset register
	if (t6963c_write_data_16(OFFSET_REG_DATA))
		return -1;
	if (t6963c_write_command(CMD_SET_OFF_REG))
		return -1;

	// set display mode
	if (t6963c_write_command(CMD_SET_TXT_ON_GRA_ON))
		return -1;

	// clear all code
	if (t6963c_write_single_display_data(TXT_HOME_ADDR, 0x00, COLUMN * TXT_LINE))
		return -1;
	if (t6963c_write_single_display_data(GRA_HOME_ADDR, 0x00, COLUMN * GRA_LINE))
		return -1;

	return 0;
}

static int
socle_lcm_t6963c_print_str_with_attr(int line, int column, u8 *str, int attr_data)
{
	int len;

	if ((line < 0) || (column < 0) || (line >= TXT_LINE) || (column >= COLUMN))
		return -1;

	len = strlen(str);
	if ((column + len) >= COLUMN)
		len = COLUMN - column - 1;

	if (t6963c_write_attribute_data(line, column, len, attr_data))
		return -1;

	if (t6963c_write_string_data(line, column, str, len))
		return -1;

	return 0;
}



#if 0

static int
socle_lcm_t6963c_print_bar(int line, int bar)
{
	if ((line < 0) || (line >= TXT_LINE))
		return -1;

	if (bar) {
		if (t6963c_write_attribute_data(line, 0, COLUMN - 1, REVERSE_DISP))
			return -1;
	} else {
		if (t6963c_write_attribute_data(line, 0, COLUMN - 1, NORMAL_DISP))
			return -1;
	}

	return 0;
}

#ifdef T6963C_REVERSE
static int
socle_lcm_t6963c_draw_line_with_attr(int line, int column, int vertical, int len, int attr_data)
{
	int i;
	char sq[2]= "";

	if ((line < 0) || (column < 0) || (line >= TXT_LINE) || (column >= COLUMN))
		return -1;

	if (vertical) {
		if ((len < 0) || (len > TXT_LINE))
			return -1;

		if ((line + len) > TXT_LINE)
			len = TXT_LINE - line;

		sq[0] = RECTANGLE_V;
		for (i = 0; i < len; i++) {
			if (socle_lcm_t6963c_print_str_with_attr(line + i, column, sq, attr_data))
				return -1;
		}
	} else {
		if ((len < 0) || (len >= COLUMN))
			return -1;

		if ((column + len) >= COLUMN)
			len = COLUMN - column - 1;

		sq[0] = RECTANGLE_H;
		for (i = 0; i < len; i++) {
			if (socle_lcm_t6963c_print_str_with_attr(line, column + i, sq, attr_data))
				return -1;
		}
	}

	return 0;
}

static int
socle_lcm_t6963c_draw_rectangle_with_attr(int line, int column, int w, int h, int attr_data)
{
	int i;
	char sq[2]= "";

	if ((line < 0) || (column < 0) || (line >= TXT_LINE) || (column >= COLUMN))
		return -1;
	if ((w < 2) || (h < 2) || (w >= COLUMN) || (h > TXT_LINE))
		return -1;

	if ((line + h) > TXT_LINE)
		h = TXT_LINE - line;
	if ((column + w) >= COLUMN)
		w = COLUMN - column - 1;

	sq[0] = RECTANGLE_U_L;
	if (socle_lcm_t6963c_print_str_with_attr(line, column, sq, attr_data))
		return -1;
	sq[0] = RECTANGLE_H;
	for (i = 0; i < (w - 2); i++) {
		if (socle_lcm_t6963c_print_str_with_attr(line, column + 1 + i, sq, attr_data))
			return -1;
	}
	sq[0] = RECTANGLE_U_R;
	if (socle_lcm_t6963c_print_str_with_attr(line, column + w - 1, sq, attr_data))
		return -1;

	for (i = 0; i < (h - 2); i++) {
		sq[0] = RECTANGLE_V;
		if (socle_lcm_t6963c_print_str_with_attr(line + 1 + i, column, sq, attr_data))
			return -1;
		sq[0] = RECTANGLE_V;
		if (socle_lcm_t6963c_print_str_with_attr(line + 1 + i, column + w - 1, sq, attr_data))
			return -1;
	}

	sq[0] = RECTANGLE_L_L;
	if (socle_lcm_t6963c_print_str_with_attr(line + h -1, column, sq, attr_data))
		return -1;
	sq[0] = RECTANGLE_H;
	for (i = 0; i < (w - 2); i++) {
		if (socle_lcm_t6963c_print_str_with_attr(line + h -1, column + 1 + i, sq, attr_data))
			return -1;
	}
	sq[0] = RECTANGLE_L_R;
	if (socle_lcm_t6963c_print_str_with_attr(line + h -1, column + w - 1, sq, attr_data))
		return -1;

	return 0;
}
#endif

#endif

////////

static void
socle_inr_set_volume(void *data)
{
	int clockwise = socle_gpio_get_value_with_mask(PF, 0x20);

	if (clockwise) {
		INR_DBG("clockwise\n");
		input_report_key(pdk_kpd, pdkkpd_keycode[1], 1);	// press
		input_report_key(pdk_kpd, pdkkpd_keycode[1], 0);	// release
	} else {
		INR_DBG("counterclockwise\n");
		input_report_key(pdk_kpd, pdkkpd_keycode[0], 1);	// press
		input_report_key(pdk_kpd, pdkkpd_keycode[0], 0);	// release
	}
	input_sync(pdk_kpd);
}

static irqreturn_t
volume_tuner_isr(int irq, void *pparam)
{
	schedule_work(&work);

	return IRQ_HANDLED;
}


static void
socle_inr_iPod_detection(unsigned long data)
{
	int plug = socle_gpio_get_value_with_mask(PA, 0x10);

	if (plug) {
		INR_DBG("iPod plug\n");
		input_report_key(pdk_kpd, pdkkpd_keycode[2], 1);	// press
		input_report_key(pdk_kpd, pdkkpd_keycode[2], 0);	// release
	} else {
		INR_DBG("iPod unplug\n");
		input_report_key(pdk_kpd, pdkkpd_keycode[3], 1);	// press
		input_report_key(pdk_kpd, pdkkpd_keycode[3], 0);	// release
	}
	input_sync(pdk_kpd);

	socle_enable_gpio_irq(IPOD_IRQ);
}

static irqreturn_t
iPod_detection_isr(int irq, void *pparam)
{
	del_timer(&inr_iPod);
	inr_iPod.expires = jiffies + 5 * HZ;
	add_timer(&inr_iPod);

	socle_disable_gpio_irq(IPOD_IRQ);

	return IRQ_HANDLED;
}


static void
socle_inr_line_in_detection(unsigned long data)
{
	int plug = socle_gpio_get_value_with_mask(PF, 0x80);

	if (plug) {
		INR_DBG("line-in plug\n");
		input_report_key(pdk_kpd, pdkkpd_keycode[4], 1);	// press
		input_report_key(pdk_kpd, pdkkpd_keycode[4], 0);	// release
	} else {
		INR_DBG("line-in unplug\n");
		input_report_key(pdk_kpd, pdkkpd_keycode[5], 1);	// press
		input_report_key(pdk_kpd, pdkkpd_keycode[5], 0);	// release
	}
	input_sync(pdk_kpd);

	socle_enable_gpio_irq(LINE_IN_IRQ);
}

static irqreturn_t
line_in_detection_isr(int irq, void *pparam)
{
	del_timer(&inr_line);
	inr_line.expires = jiffies + 5 * HZ;
	add_timer(&inr_line);

	socle_disable_gpio_irq(LINE_IN_IRQ);

	return IRQ_HANDLED;
}

static int socle_inr_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	INR_DBG("\n");

	switch (cmd) {
		case INR_GET_LCM_BASE:
			INR_DBG("INR_GET_LCM_BASE t6963c_database = %x\n", t6963c_database);
			*(u32 *)arg = t6963c_database;
			break;
		case INR_SEL_SRC_FM:
			INR_DBG("INR_SEL_SRC_FM\n");
			SET_AUDIO_SRC_TO_FM();
			break;
		case INR_SEL_SRC_IPOD:
			INR_DBG("INR_SEL_SRC_IPOD\n");
			SET_AUDIO_SRC_TO_IPOD();
			EN_AUDIO_SRC_TO_IPOD();
			break;
		case INR_SEL_SRC_LINEIN:
			INR_DBG("INR_SEL_SRC_LINEIN\n");
			SET_AUDIO_SRC_TO_LINEIN();
			EN_AUDIO_SRC_TO_LINEIN();
			break;
		case INR_SEL_SRC_I2S:
			INR_DBG("INR_SEL_SRC_I2S\n");
			SET_AUDIO_SRC_TO_I2S();
			break;
		case INR_SEL_LCM_MODE:
			INR_DBG("INR_SEL_LCM_MODE\n");
			SET_CS0_TO_LCM();
			break;
		case INR_SEL_NOR_MODE:
			INR_DBG("INR_SEL_NOR_MODE\n");
			SET_CS0_TO_NOR();
			break;
		case INR_SET_VOLUME:
			INR_DBG("INR_SET_VOLUME\n");
			if (ms6335_dac_master_volume_l(arg))
				printk("Can not set left volume to %02d\n", (int)arg);
			if (ms6335_dac_master_volume_r(arg))
				printk("Can not set right volume to %02d\n", (int)arg);
			break;
		case INR_SET_BACKLIGHT: {
			u32 bklgt_lrc = pwm_bklgt->drv->read_lrc(pwm_bklgt);

			INR_DBG("INR_SET_BACKLIGHT bklgt_lrc = %d\n", bklgt_lrc);
			pwm_bklgt->drv->enable(pwm_bklgt, 0);
			pwm_bklgt->drv->output_enable(pwm_bklgt, 0);

			if ((int)arg > MAX_BACKLIGHT)
				arg = MAX_BACKLIGHT;
			else if ((int)arg < 0)
				arg = 0;

			pwm_bklgt->drv->write_hrc(pwm_bklgt, bklgt_lrc / 100 * (arg * arg));

			pwm_bklgt->drv->enable(pwm_bklgt, 1);
			pwm_bklgt->drv->output_enable(pwm_bklgt, 1);
			}
			break;
		case INR_SET_CONTRAST: {
			u32 cntrt_lrc = pwm_cntrt->drv->read_lrc(pwm_cntrt);

			INR_DBG("INR_SET_CONTRAST cntrt_lrc = %d\n", cntrt_lrc);
			pwm_cntrt->drv->enable(pwm_cntrt, 0);
			pwm_cntrt->drv->output_enable(pwm_cntrt, 0);

			if ((int)arg > MAX_CONTRAST)
				arg = MAX_CONTRAST;
			else if ((int)arg < 0)
				arg = 0;

			pwm_cntrt->drv->write_hrc(pwm_cntrt, cntrt_lrc / 100 * (90 + arg));

			pwm_cntrt->drv->enable(pwm_cntrt, 1);
			pwm_cntrt->drv->output_enable(pwm_cntrt, 1);
			}
			break;
		case INR_EN_STANDBY_MODE:
			INR_DBG("INR_EN_STANDBY_MODE arg = %x\n", arg);
			if (arg) {
				// standby
				ms6335_dac_power_switch(0);

				DEVICES_POWER_OFF();
				STANDBY_LED_ON();
				//OSC_DISABLE();
				INR_DBG("standby\n");
			} else {
				// resume
				DEVICES_POWER_ON();
				STANDBY_LED_OFF();
				OSC_ENABLE();

				DEVICES_RESET_ON();
				mdelay(100);
				DEVICES_RESET_OFF();

				ms6335_dac_power_switch(1);
				INR_DBG("resume\n");
			}
			break;
		default:
			printk("SQ_inr_ioctl(): No such ioctl!\n");
			return -ENOTTY;
	}

	return 0;
}

static int socle_inr_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	INR_DBG("\n");

	return 0;
}

static int socle_inr_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos)
{
	static int i = 0;

	INR_DBG("%s\n", buf);

	if (!strncmp("nor", buf, 3)) {
		printk("SET_CS0_TO_NOR\n");
		SET_CS0_TO_NOR();
		return count;
	} else if (!strncmp("lcm", buf, 3)) {
		printk("SET_CS0_TO_LCM\n");
		SET_CS0_TO_LCM();
		return count;
	}

	if (i >= TXT_LINE)
		i = 0;

	socle_lcm_t6963c_print_str_with_attr(i, 0, (u8 *)buf, NORMAL_DISP);
	i++;

	return count;
}

static int socle_inr_open(struct inode *inode, struct file *file)
{
	INR_DBG("\n");
//	socle_enable_gpio_irq(VOLUME_IRQ);
	return 0;
}

static int socle_inr_release(struct inode *inode, struct file *file)
{
	INR_DBG("\n");
	return 0;	
}

static const struct file_operations inr_fops = {
        .owner =        THIS_MODULE,
        .llseek =       no_llseek,
        .ioctl =        socle_inr_ioctl,
        .open =         socle_inr_open,
        .read = 		socle_inr_read,
        .write =		socle_inr_write,
        .release =      socle_inr_release,
};

struct miscdevice misc_inr = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "inr",
	.fops = &inr_fops,
};

static int socle_inr_remove(struct platform_device *pdev)
{
	INR_DBG("\n");

	// free VOLUME_IRQ for volume_tuner_isr
	socle_free_gpio_irq(VOLUME_IRQ, NULL);

	// free IPOD_IRQ for iPod_detection_isr
	socle_free_gpio_irq(IPOD_IRQ, NULL);

	// free LINE_IN_IRQ for line_in_detection_isr
	socle_free_gpio_irq(LINE_IN_IRQ, NULL);

	release_socle_pwmt_structure(T6963C_BKLGT_PWM_IDX);
	release_socle_pwmt_structure(T6963C_CNTRT_PWM_IDX);

	misc_deregister(&misc_inr);
	flush_scheduled_work();

	return 0;	
}


static int socle_inr_probe(struct platform_device *pdev)
{	
	int err = 0;
	struct resource *res;

	INR_DBG("\n\n");
	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (NULL == res) {
		printk("SQ INR can't get IORESOURCE_MEM!!\n");
		return -ENXIO;
	}

	t6963c_database = IO_ADDRESS(res->start);

	SET_CS0_TO_LCM();

	if (socle_lcm_init())
		return -1;

#ifdef T6963C_REVERSE
	if (t6963c_write_ext_char_gen_data(ascii_table, 0x80, DATA_NUM(ascii_table)))
		return -1;
#endif

	if (socle_lcm_t6963c_print_str_with_attr(1, 2, "SQ Tech. Corp.", BLINK_NORMAL_DISP))
		return -1;
	if (socle_lcm_t6963c_print_str_with_attr(3, 2, " Internet Radio ", REVERSE_DISP))
		return -1;
	if (socle_lcm_t6963c_print_str_with_attr(5, 7, " 2008 ", NORMAL_DISP))
		return -1;
	if (socle_lcm_t6963c_print_str_with_attr(7, 4, "connecting...", NORMAL_DISP))
		return -1;

	SET_CS0_TO_NOR();


	SET_AUDIO_SRC_TO_I2S();

	// set VOLUME_IRQ as low level triggle interrupt for volume_tuner_isr
	if (socle_request_gpio_irq(VOLUME_IRQ, volume_tuner_isr, GPIO_INT_SENSE_EDGE | GPIO_INT_SINGLE_EDGE | GPIO_INT_EVENT_HI, "inr-volume_tuner", NULL)) {
		printk("SQ_inr_probe(): GPIO pin[%d] is busy!\n", VOLUME_IRQ);
		return -1;
	}
//	socle_disable_gpio_irq(VOLUME_IRQ);

	// set IPOD_IRQ as both edge triggle interrupt for iPod_detection_isr
	if (socle_request_gpio_irq(IPOD_IRQ, iPod_detection_isr, GPIO_INT_SENSE_EDGE | GPIO_INT_BOTH_EDGE, "inr-iPdod", NULL)) {
		printk("SQ_inr_probe(): GPIO pin[%d] is busy!\n", IPOD_IRQ);
		return -1;
	}

	// set LINE_IN_IRQ as both edge triggle interrupt for line_in_detection_isr
	if (socle_request_gpio_irq(LINE_IN_IRQ, line_in_detection_isr, GPIO_INT_SENSE_EDGE | GPIO_INT_BOTH_EDGE, "inr-iPdod", NULL)) {
		printk("SQ_inr_probe(): GPIO pin[%d] is busy!\n", LINE_IN_IRQ);
		return -1;
	}

	err = misc_register(&misc_inr);

	return err;	
}

static struct platform_driver socle_inr_drv = {
	.probe		= socle_inr_probe,
	.remove		= socle_inr_remove,
	.driver		= {
		.name	= "socle-inr",
		.owner	= THIS_MODULE,
	},
};


static int __init socle_inr_init(void)
{
	printk(banner);

	return platform_driver_register(&socle_inr_drv);
}

static void __exit socle_inr_exit(void)
{
	platform_driver_unregister(&socle_inr_drv);
}

module_init(socle_inr_init);
module_exit(socle_inr_exit);

MODULE_DESCRIPTION("SQ Internet Radio Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CY Li <cyli@socle-tech.com.tw>");

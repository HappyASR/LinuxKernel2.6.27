/********************************************************************************
* File Name     : socle-inr.h
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
*      1. 2008/09/01 cyli create this file 
*    
********************************************************************************/

#ifndef __SOCLE_INR_H
#define __SOCLE_INR_H

#include <asm/arch/regs-pwmt.h>
#include <asm/arch/gpio.h>

#include <asm/arch/ms6335.h>

#define	VOLUME_IRQ					SET_GPIO_PIN_NUM(PF, 4)
#define	IPOD_IRQ					SET_GPIO_PIN_NUM(PA, 4)
#define	LINE_IN_IRQ					SET_GPIO_PIN_NUM(PF, 7)

#define T6963C_BKLGT_PWM_IDX		0	// backlight
#define T6963C_CNTRT_PWM_IDX		1	// contrast


#define SET_CS0_TO_LCM()		socle_gpio_set_value_with_mask(PA, 0, SHIFT_MASK(5))						// PA5 = 0
#define SET_CS0_TO_NOR()		socle_gpio_set_value_with_mask(PA, SHIFT_MASK(5), SHIFT_MASK(5));			// PA5 = 1

#define SET_AUDIO_SRC_TO_FM()		socle_gpio_set_value_with_mask(PE, 0x18, 0x3f);				// PE[1:0] = [00]
#define SET_AUDIO_SRC_TO_IPOD()		socle_gpio_set_value_with_mask(PE, 0x19, 0x3f);				// PE[1:0] = [01]
#define EN_AUDIO_SRC_TO_IPOD()		socle_gpio_set_value_with_mask(PE, 0x39, 0x3f);				// PE[1:0] = [01], PE5 = 1
#define SET_AUDIO_SRC_TO_LINEIN()	socle_gpio_set_value_with_mask(PE, 0x1a, 0x3f);				// PE[1:0] = [10]
#define EN_AUDIO_SRC_TO_LINEIN()	socle_gpio_set_value_with_mask(PE, 0x1e, 0x3f);				// PE[1:0] = [10], PE2 = 1
#define SET_AUDIO_SRC_TO_I2S()		socle_gpio_set_value_with_mask(PE, 0x1b, 0x3f);				// PE[1:0] = [11]


#define DEVICES_POWER_ON()		socle_gpio_set_value_with_mask(PA, SHIFT_MASK(0), SHIFT_MASK(0))	// PA0 = 1
#define DEVICES_POWER_OFF()		socle_gpio_set_value_with_mask(PA, 0, SHIFT_MASK(0))				// PA0 = 0

#define STANDBY_LED_ON()		socle_gpio_set_value_with_mask(PA, SHIFT_MASK(3), SHIFT_MASK(3))	// PA3 = 1
#define STANDBY_LED_OFF()		socle_gpio_set_value_with_mask(PA, 0, SHIFT_MASK(3))				// PA3 = 0

#define OSC_ENABLE()			socle_gpio_set_value_with_mask(PA, SHIFT_MASK(2), SHIFT_MASK(2))	// PA2 = 1
#define OSC_DISABLE()			socle_gpio_set_value_with_mask(PA, 0, SHIFT_MASK(2))				// PA2 = 0

#define DEVICES_RESET_ON()		socle_gpio_set_value_with_mask(PA, 0, SHIFT_MASK(1))				// PA1 = 0
#define DEVICES_RESET_OFF()		socle_gpio_set_value_with_mask(PA, SHIFT_MASK(1), SHIFT_MASK(1))	// PA1 = 1

#define FM_VIO_POWER_ON()		socle_gpio_set_value_with_mask(PE, SHIFT_MASK(3), SHIFT_MASK(3))	// PE3 = 1
#define FM_VIO_POWER_OFF()		socle_gpio_set_value_with_mask(PE, 0, SHIFT_MASK(3))				// PE3 = 0


// ioctl
#define INR_GET_LCM_BASE		_IO('i', 0x00)

#define INR_SEL_SRC_FM			_IO('i', 0x01)
#define INR_SEL_SRC_IPOD		_IO('i', 0x02)
#define INR_SEL_SRC_LINEIN		_IO('i', 0x03)
#define INR_SEL_SRC_I2S			_IO('i', 0x04)

#define INR_SEL_LCM_MODE		_IO('i', 0x05)
#define INR_SEL_NOR_MODE		_IO('i', 0x06)

#define INR_SET_VOLUME			_IO('i', 0x07)
#define MAX_VOLUME				31

#define INR_SET_BACKLIGHT		_IO('i', 0x08)
#define MAX_BACKLIGHT			10

#define INR_SET_CONTRAST		_IO('i', 0x09)
#define MAX_CONTRAST			10

#define INR_EN_STANDBY_MODE		_IO('i', 0x0a)

#endif	//__SOCLE_INR_H


/********************************************************************************
* File Name     : include/asm-arm/arch-socle/gpio.h
* Author        : cyli
* Description   : Socle GPIO Driver Export Header
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
*      1. 2006/09/27 cyli create this file
*
********************************************************************************/

struct cdk_gpio {
	int busy;
	unsigned int base;
	int offset;
	int sup_pin_num;
	int irq;
	struct cdk_gpio_driver *drv;
};

struct cdk_gpio_driver {
	void (*claim_gpio_lock)(void);
	void (*release_gpio_lock)(void);
	void (*read_data)(struct cdk_gpio *, int *data, int from, int num);		// from: from pin num,	num: number of pin
	void (*write_data)(struct cdk_gpio *, int data, int from, int num);
	
	void (*pull_down_en)(struct cdk_gpio *, int en, int from, int num);		// en:	0: disable,	else: enable
	void (*int_sense_sel)(struct cdk_gpio *, int sel, int from, int num);
			// sel:	0: edge sensitive,	else: level sensitive
	void (*int_both_edge_en)(struct cdk_gpio *, int en, int from, int num);
			// en:	0: single edge,	else: both edges
	void (*int_event_sel)(struct cdk_gpio *, int sel, int from, int num);
			// sel:	0: falling edge or low level,	else: rising edge or high level
	void (*int_en)(struct cdk_gpio *, int en, int from, int num);
			// en:	0: disable,	else: enable
	void (*read_int_stat)(struct cdk_gpio *, int *status, int from, int num);
	void (*int_clear)(struct cdk_gpio *, int data, int from, int num);
			// data:	0: no effect,	1: clear interrupt
	void (*test_mode_ctrl)(struct cdk_gpio *, int ctrl);
			/* ctrl:
					0: Tmode0, loop back pb_out to pa_out
					1: Tmode1, loop back pa_out to pb_out
					2: Tmode2, loop back pd_out to pc_out
					3: Tmode3, loop back pc_out to pd_out
			*/
	void (*test_mode_en)(struct cdk_gpio *, int en);
			// en:	0: normal mode,	else: test mode
};


extern struct cdk_gpio * get_cdk_gpio_structure(int num);
extern int release_cdk_gpio_structure(int num);



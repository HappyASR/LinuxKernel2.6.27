/********************************************************************************
* File Name     : arch/arm/mach-socle/mp-gpio.c
* Author        : Ryan Chen
* Description   : Socle MP GPIO Service
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

*   Version      : 0,0,0,1
*   History      :
*      1. 2008/01/10 ryan chen create this file
*
********************************************************************************/

#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>

#include <mach/regs-mp-gpio.h>
#include <mach/mp-gpio.h>

//#define CONFIG_MP_GPIO_DEBUG

#ifdef CONFIG_MP_GPIO_DEBUG
	#define MP_GPIO_DBG(fmt, args...) printk("MP-GPIO: %s(): " fmt, __FUNCTION__, ## args)
#else
	#define MP_GPIO_DBG(fmt, args...)
#endif

#define GET_MP_GPIO_GROUP_OFFSET(port)		( port / MP_GPIO_PER_GROUP_PORT_NUM * SOCLE_MP_GPIO_GROUP )
#define GET_MP_GPIO_PORT_SHIFT_BIT(port)		( (port % MP_GPIO_PER_GROUP_PORT_NUM) * 8 )

static inline u32
socle_mp_gpio_read(u32 offset)
{
	MP_GPIO_DBG("read offset %x \n",offset);
	return (ioread32(MP_GPIO_REG_BASE + offset));
}

static inline void
socle_mp_gpio_write(u32 value, u32 offset)
{
	MP_GPIO_DBG("write %x,  offset %x \n",value,offset);
	iowrite32(value, MP_GPIO_REG_BASE + offset);
}

extern u8
socle_mp_gpio_get_port_value(u8 port)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_DR; 
	data = socle_mp_gpio_read(offset);
	data = (data >> GET_MP_GPIO_PORT_SHIFT_BIT(port)) & 0xff;

	return data;
}

EXPORT_SYMBOL(socle_mp_gpio_get_port_value);

extern void
socle_mp_gpio_set_port_value(u8 port, u8 value)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_DR;
	data = socle_mp_gpio_read(offset);
	data &= ~(0xff << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	data |= (value << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	socle_mp_gpio_write(data, offset);
}

extern void
socle_mp_gpio_set_port_num_value(u8 port, u8 num, u8 value)
{
	u16 offset;
	u32 data;
	
	socle_mp_gpio_set_port_num_direction(port, num, SOCLE_MP_GPIO_DIR_POUT);
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_DR;
	data = socle_mp_gpio_read(offset);
	num += GET_MP_GPIO_PORT_SHIFT_BIT(port); 
	if(value==0)
		data &= ~(0x1 << num);
	else
		data |= (0x1 << num);

	socle_mp_gpio_write(data, offset);
}

EXPORT_SYMBOL(socle_mp_gpio_set_port_num_value);

extern u8
socle_mp_gpio_get_port_direction(u8 port)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_DIR; 
	data = socle_mp_gpio_read(offset);
	data = (data >> GET_MP_GPIO_PORT_SHIFT_BIT(port)) & 0xff;

	return data;
}

extern void
socle_mp_gpio_set_port_direction(u8 port, u8 value)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_DIR;
	data = socle_mp_gpio_read(offset);
	data &= ~(0xff << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	data |= (value << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	socle_mp_gpio_write(data, offset);
}

extern void
socle_mp_gpio_set_port_num_direction(u8 port, u8 num, u8 value)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_DIR;
	data = socle_mp_gpio_read(offset);
	num += GET_MP_GPIO_PORT_SHIFT_BIT(port); 
	if(value==0)
		data &= ~(0x1 << num);
	else
		data |= (0x1 << num);

	socle_mp_gpio_write(data, offset);
}

EXPORT_SYMBOL(socle_mp_gpio_set_port_num_direction);

extern u8
socle_mp_gpio_get_port_interrupt(u8 port)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_INTE; 
	data = socle_mp_gpio_read(offset);
	data = (data >> GET_MP_GPIO_PORT_SHIFT_BIT(port)) & 0xff;

	return data;
}

extern void
socle_mp_gpio_set_port_interrupt(u8 port, u8 value)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_INTE;
	data = socle_mp_gpio_read(offset);
	data &= ~(0xff << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	data |= (value << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	socle_mp_gpio_write(data, offset);

}

EXPORT_SYMBOL(socle_mp_gpio_set_port_interrupt);

extern void
socle_mp_gpio_set_port_num_interrupt(u8 port, u8 num, u8 value)
{
	u16 offset;
	u32 data;

	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_INTE;
	data = socle_mp_gpio_read(offset);
	num += GET_MP_GPIO_PORT_SHIFT_BIT(port); 
	if(value==0) {
		data &= ~(0x1 << num);
		socle_mp_gpio_clear_port_num_interrupt(port, num); 
	}
	else
		data |= (0x1 << num);

	socle_mp_gpio_write(data, offset);
}

EXPORT_SYMBOL(socle_mp_gpio_set_port_num_interrupt);

extern u8
socle_mp_gpio_get_port_interrupt_sense(u8 port)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_IS; 
	data = socle_mp_gpio_read(offset);
	data = (data >> GET_MP_GPIO_PORT_SHIFT_BIT(port)) & 0xff;

	return data;
}

extern void
socle_mp_gpio_set_port_interrupt_sense(u8 port, u8 value)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_IS;
	data = socle_mp_gpio_read(offset);
	data &= ~(0xff << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	data |= (value << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	socle_mp_gpio_write(data, offset);
}

extern void
socle_mp_gpio_set_port_num_interrupt_sense(u8 port, u8 num, u8 value)
{
	u16 offset;
	u32 data;
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_IS;
	data = socle_mp_gpio_read(offset);
	num += GET_MP_GPIO_PORT_SHIFT_BIT(port); 
	if(value==0)
		data &= ~(0x1 << num);
	else
		data |= (0x1 << num);
		
	socle_mp_gpio_write(data, offset);
}

EXPORT_SYMBOL(socle_mp_gpio_set_port_num_interrupt_sense);
	
extern u8
socle_mp_gpio_get_port_interrupt_both_edges(u8 port)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_IBE; 
	data = socle_mp_gpio_read(offset);
	data = (data >> GET_MP_GPIO_PORT_SHIFT_BIT(port)) & 0xff;

	return data;
}

extern void
socle_mp_gpio_set_port_interrupt_both_edges(u8 port, u8 value)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_IBE;
	data = socle_mp_gpio_read(offset);
	data &= ~(0xff << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	data |= (value << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	socle_mp_gpio_write(data, offset);
}

extern void
socle_mp_gpio_set_port_num_interrupt_both_edges(u8 port, u8 num, u8 value)
{
	u16 offset;
	u32 data;
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_IBE;
	data = socle_mp_gpio_read(offset);
	num += GET_MP_GPIO_PORT_SHIFT_BIT(port); 
	if(value==0)
		data &= ~(0x1 << num);
	else
		data |= (0x1 << num);
		
	socle_mp_gpio_write(data, offset);
}

EXPORT_SYMBOL(socle_mp_gpio_set_port_num_interrupt_both_edges);
	
extern u8
socle_mp_gpio_get_port_interrupt_event(u8 port)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_IEV; 
	data = socle_mp_gpio_read(offset);
	data = (data >> GET_MP_GPIO_PORT_SHIFT_BIT(port)) & 0xff;

	return data;
}

extern void
socle_mp_gpio_set_port_interrupt_event(u8 port, u8 value)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_IEV;
	data = socle_mp_gpio_read(offset);
	data &= ~(0xff << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	data |= (value << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	socle_mp_gpio_write(data, offset);
}

EXPORT_SYMBOL(socle_mp_gpio_set_port_interrupt_event);

extern void
socle_mp_gpio_set_port_num_interrupt_event(u8 port, u8 num, u8 value)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_IEV;
	data = socle_mp_gpio_read(offset);
	num += GET_MP_GPIO_PORT_SHIFT_BIT(port); 
	if(value==0)
		data &= ~(0x1 << num);
	else
		data |= (0x1 << num);
		
	socle_mp_gpio_write(data, offset);
}

EXPORT_SYMBOL(socle_mp_gpio_set_port_num_interrupt_event);

extern void
socle_mp_gpio_clear_port_interrupt(u8 port)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_INTC;
	data = (0xff << GET_MP_GPIO_PORT_SHIFT_BIT(port));
	socle_mp_gpio_write(data, offset);
	
}

EXPORT_SYMBOL(socle_mp_gpio_clear_port_interrupt);

extern void
socle_mp_gpio_clear_port_num_interrupt(u8 port, u8 num)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_INTC;
	num += GET_MP_GPIO_PORT_SHIFT_BIT(port); 
	data = (0x1 << num);
	socle_mp_gpio_write(data, offset);
}

extern u8
socle_mp_gpio_get_port_interrupt_status(u8 port)
{
	u16 offset;
	u32 data;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_INTS;
	data = socle_mp_gpio_read(offset);
	data = (data >> GET_MP_GPIO_PORT_SHIFT_BIT(port)) & 0xff;

	return data;
}

EXPORT_SYMBOL(socle_mp_gpio_get_port_interrupt_status);

extern void
socle_mp_gpio_test_mode_en(u8 port, u8 en)
{
	u32 data;
	u16 offset;
	
	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_TMODE;
	data = socle_mp_gpio_read(offset);

	if (en==SOCLE_MP_GPIO_NORMAL_MODE)
		data &= ~SOCLE_MP_GPIO_TEST_MODE;
	else
		data |= SOCLE_MP_GPIO_TEST_MODE;

	socle_mp_gpio_write(data, offset);
}

EXPORT_SYMBOL(socle_mp_gpio_test_mode_en);
	
extern void
socle_mp_gpio_test_mode_ctrl(u8 port, u8 mode)
{
	u32 data;
	u16 offset;

	offset = GET_MP_GPIO_GROUP_OFFSET(port) + SOCLE_MP_GPIO_TMODE;
	data = socle_mp_gpio_read(offset) & ~0x6;
	data |= (mode << 1);
	socle_mp_gpio_write(data, offset);
}



struct mp_gpio_handler
{
	irq_handler_t sub_routine;
	void *pparam;							// interrupt handler parameter
};

// IRQ controller interrupt handle vector table
static struct mp_gpio_handler mp_gpio_irq_vector_table[MP_GPIO_PORT_NUM*MP_GPIO_PER_PORT_PIN_NUM];		

static inline void
socle_mp_gpio_set_irq_type (u8 port, u8 num, u8 irqflags)
{
	if (MP_GPIO_INT_SENSE_EDGE == (irqflags & MP_GPIO_INT_SENSE)) {
		socle_mp_gpio_set_port_num_interrupt_sense(port, num, 0x0);
		if (MP_GPIO_INT_SINGLE_EDGE == (irqflags & MP_GPIO_INT_BOTH_EDGE)) {
			socle_mp_gpio_set_port_num_interrupt_both_edges(port, num, 0x0);
			if (MP_GPIO_INT_EVENT_LO == (irqflags & MP_GPIO_INT_EVENT)) {
				MP_GPIO_DBG("Falling Edge\n");
				socle_mp_gpio_set_port_num_interrupt_event(port, num, 0x0);
			}
			else if (MP_GPIO_INT_EVENT_HI == (irqflags & MP_GPIO_INT_EVENT))  {
				MP_GPIO_DBG("Rising Edge\n");
				socle_mp_gpio_set_port_num_interrupt_event(port, num, 0x1);
			}

		}
		else if (MP_GPIO_INT_BOTH_EDGE == (irqflags & MP_GPIO_INT_BOTH_EDGE))  {
			MP_GPIO_DBG("Both Edge\n");
			socle_mp_gpio_set_port_num_interrupt_both_edges(port, num, 0x1);
		}
	}
	else if (MP_GPIO_INT_SENSE_LEVEL == (irqflags & MP_GPIO_INT_SENSE))  {
		socle_mp_gpio_set_port_num_interrupt_sense(port, num, 0x1);
		if (MP_GPIO_INT_EVENT_LO == (irqflags & MP_GPIO_INT_EVENT)) {
			MP_GPIO_DBG("Low Level\n");
			socle_mp_gpio_set_port_num_interrupt_event(port, num, 0x0);
		} 
		else if (MP_GPIO_INT_EVENT_HI == (irqflags & MP_GPIO_INT_EVENT))  {
			MP_GPIO_DBG("High Level\n");
			socle_mp_gpio_set_port_num_interrupt_event(port, num, 0x1);
		}
	}
}

extern void
socle_mp_gpio_request_irq (u8 port, u8 num, u8 irqflags, irq_handler_t handler, void* pparam)
{
	u8 pin = port*MP_GPIO_PER_PORT_PIN_NUM+num;

	struct mp_gpio_handler *irq = &mp_gpio_irq_vector_table[pin];		

	irq->sub_routine = handler;
	irq->pparam = pparam ;

	//set int type
	socle_mp_gpio_set_irq_type(port,num,irqflags);
	// enable interrupt
	socle_mp_gpio_set_port_num_interrupt(port, num, SOCLE_MP_GPIO_INTE_EN);
}

EXPORT_SYMBOL(socle_mp_gpio_request_irq);

extern void
socle_mp_gpio_free_irq (u8 port, u8 num)
{
	u8 pin = port*MP_GPIO_PER_PORT_PIN_NUM+num;

	struct mp_gpio_handler *irq = &mp_gpio_irq_vector_table[pin];		

	irq->sub_routine = NULL;
	irq->pparam = NULL;

	// gpio port # irq disable
	socle_mp_gpio_set_port_num_interrupt(port, num, SOCLE_MP_GPIO_INTE_DIS);

}

EXPORT_SYMBOL(socle_mp_gpio_free_irq);

static irqreturn_t
socle_mp_gpio_irq_dispatch (int irq, void *data)
{
	//read for which port & #
	struct mp_gpio_handler *mp_irq;	
	u8 port;
	u8 num;
	u8 status=0;
	u8 pin;

	//detect which port 
	for(port=MP_PA;port<=MP_PQ;port++) {
		status=socle_mp_gpio_get_port_interrupt_status(port);
		if(status)
			break;	
	}
	
	//detect which pin
	for(num=0;num<8;num++) {
		if(status & (0x1<<num))
			break;
	}
	
	pin=port*MP_GPIO_PER_PORT_PIN_NUM+num;
	mp_irq = &mp_gpio_irq_vector_table[pin];	
	
	
	if (NULL == mp_irq->sub_routine ) {
//		printk("Warning!! irq = %d but isr is NULL\n", pin);
    return IRQ_HANDLED;
	}
	// branch to the fn saved in irq_vector_table[v]
	mp_irq->sub_routine(pin, mp_irq->pparam);

	// clear gpio interrupt
	//socle_mp_gpio_set_port_num_interrupt(port, num, 1); //ryan for clear interrupt
  	socle_mp_gpio_clear_port_num_interrupt(port, num); 

	return IRQ_HANDLED;
}

static int __init socle_mp_gpio_init_irq(void)	
{
	int i;
	int ret=0;

	for(i=MP_PA;i<=MP_PQ;i++) {
	//	socle_mp_gpio_set_port_interrupt(i,0xff);
	//	socle_mp_gpio_clear_port_interrupt(i);
		socle_mp_gpio_set_port_interrupt(i,0);
	}

	socle_mp_gpio_write(0x4,SOCLE_MP_GPIO_PD); //ryan
	
	ret = request_irq(MP_GPIO_INT, socle_mp_gpio_irq_dispatch, IRQF_SHARED, "mp_gpio", mp_gpio_irq_vector_table);
	if (ret) {
		printk("Error! Fail to request irq MP_GPIO_INT\n");
	}
	return ret;

}

core_initcall(socle_mp_gpio_init_irq);
MODULE_LICENSE("GPL");


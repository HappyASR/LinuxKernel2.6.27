#ifndef __ASM_ARCH_SOCLE_MP_GPIO_H
#define __ASM_ARCH_SOCLE_MP_GPIO_H	1

#include <linux/interrupt.h>

extern u8 socle_mp_gpio_get_port_value(u8 port);
extern void socle_mp_gpio_set_port_value(u8 port, u8 value);
extern void socle_mp_gpio_set_port_num_value(u8 port, u8 num, u8 value);

extern u8 socle_mp_gpio_get_port_direction(u8 port);
extern void socle_mp_gpio_set_port_direction(u8 port, u8 dir);
extern void socle_mp_gpio_set_port_num_direction(u8 port, u8 num, u8 dir);

extern u8 socle_mp_gpio_get_port_interrupt(u8 port);
extern void socle_mp_gpio_set_port_interrupt(u8 port, u8 value);
extern void socle_mp_gpio_set_port_num_interrupt(u8 port, u8 num, u8 value);

extern u8 socle_mp_gpio_get_port_interrupt_sense(u8 port);
extern void socle_mp_gpio_set_port_interrupt_sense(u8 port, u8 value);
extern void socle_mp_gpio_set_port_num_interrupt_sense(u8 port, u8 num, u8 value);

extern u8 socle_mp_gpio_get_port_interrupt_both_edges(u8 port);
extern void socle_mp_gpio_set_port_interrupt_both_edges(u8 port, u8 value);
extern void socle_mp_gpio_set_port_num_interrupt_both_edges(u8 port, u8 num, u8 value);

extern u8 socle_mp_gpio_get_port_interrupt_event(u8 port);
extern void socle_mp_gpio_set_port_interrupt_event(u8 port, u8 value);
extern void socle_mp_gpio_set_port_num_interrupt_event(u8 port, u8 num, u8 value);

extern void socle_mp_gpio_clear_port_interrupt(u8 port);
extern void socle_mp_gpio_clear_port_num_interrupt(u8 port, u8 num);

extern u8 socle_mp_gpio_get_port_interrupt_status(u8 port);

extern void socle_mp_gpio_test_mode_en(u8 port, u8 en);

extern void socle_mp_gpio_test_mode_ctrl(u8 port, u8 mode);
	// mode: as follows
#define PB2PA	0x0
#define PA2PB	0x1
#define PD2PC	0x2
#define PC2PD	0x3


///////////////// * GPIO Interrupt Service * /////////////////

extern void socle_mp_gpio_free_irq (u8 port, u8 num);
extern void socle_mp_gpio_request_irq (u8 port, u8 num, u8 irqflags, irq_handler_t handler, void* pparam);
	// irqflags: as follows
#define MP_GPIO_INT_SENSE			(0x1 << 0)
#define MP_GPIO_INT_SENSE_EDGE	(0x0)
#define MP_GPIO_INT_SENSE_LEVEL	(0x1 << 0)

#define MP_GPIO_INT_SINGLE_EDGE	(0x0)
#define MP_GPIO_INT_BOTH_EDGE		(0x1 << 1)

#define MP_GPIO_INT_EVENT			(0x1 << 2)
#define MP_GPIO_INT_EVENT_LO		(0x0)
#define MP_GPIO_INT_EVENT_HI		(0x1 << 2)



#endif	//__MP_GPIO_H


#ifndef __ASM_ARCH_SOCLE_REGS_MP_GPIO_H
#define __ASM_ARCH_SOCLE_REGS_MP_GPIO_H	1

#include <mach/platform.h>
#include <mach/irqs.h>

#define MP_GPIO_PORT_NUM				17
#define MP_GPIO_PER_PORT_PIN_NUM		8
#define MP_GPIO_PER_GROUP_PORT_NUM 4

// port definition
#define MP_PA	0x00
#define MP_PB	0x01
#define MP_PC	0x02
#define MP_PD	0x03
#define MP_PE	0X04
#define MP_PF	0x05
#define MP_PG	0x06
#define MP_PH	0x07
#define MP_PI	0x08
#define MP_PJ	0x09
#define MP_PK	0x0A
#define MP_PL	0x0B
#define MP_PM	0x0C
#define MP_PN	0x0D
#define MP_PO	0x0E
#define MP_PP	0x0F
#define MP_PQ	0x10


#define MP_GPIO_REG_BASE			IO_ADDRESS(SOCLE_APB0_MP)
#define MP_GPIO_INT					IRQ_MPS2

#define SOCLE_MP_GPIO_DR					0X0000
#define SOCLE_MP_GPIO_DIR					0X0004
#define SOCLE_MP_GPIO_PD					0X0008
#define SOCLE_MP_GPIO_IS					0X000C
#define SOCLE_MP_GPIO_IBE					0X0010
#define SOCLE_MP_GPIO_IEV					0X0014
#define SOCLE_MP_GPIO_INTE					0X0018
#define SOCLE_MP_GPIO_INTS					0X001C
#define SOCLE_MP_GPIO_INTC					0X0020
#define SOCLE_MP_GPIO_TMODE				0X0024

//MP_GPIO_GROUP_SIZE
#define SOCLE_MP_GPIO_GROUP				0X40

//SOCLE_MP_GPIO_DIR 0x04
#define SOCLE_MP_GPIO_DIR_POUT			1
#define SOCLE_MP_GPIO_DIR_PIN				0

//SOCLE_MP_GPIO_PD 0x08
#define SOCLE_MP_GPIO_PD_EN				  1
#define SOCLE_MP_GPIO_PD_DIS				0

//SOCLE_MP_GPIO_IS 0x0C
#define SOCLE_MP_GPIO_IS_LEVEL			1
#define SOCLE_MP_GPIO_IS_EDGE				0

//SOCLE_MP_GPIO_IBE 0x10
#define SOCLE_MP_GPIO_IBE_BOTH			1
#define SOCLE_MP_GPIO_IBE_SINGLE		0

//SOCLE_MP_GPIO_IEV 0x14
#define SOCLE_MP_GPIO_IEV_RISING			1
#define SOCLE_MP_GPIO_IEV_FALLING			0

//SOCLE_MP_GPIO_INTE 0x18
#define SOCLE_MP_GPIO_INTE_EN				1
#define SOCLE_MP_GPIO_INTE_DIS			0

#define SOCLE_MP_GPIO_INTC_CLEAR		 1

#define SOCLE_MP_GPIO_TEST_MODE    1
#define SOCLE_MP_GPIO_NORMAL_MODE   0

// 20090209 cyli fix
#ifndef BIT_MASK
#define BIT_MASK(nbits)			((0x1 << (nbits)) - 1)
#endif

#endif	//__MP_GPIO_REG_H

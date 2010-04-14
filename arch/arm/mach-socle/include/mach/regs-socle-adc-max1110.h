/* drivers/char/regs-socle-adc-max1110.h
 *
 * Copyright (c) 2009 Socle-tech Corp
 *                    http://www.socle-tech.com.tw/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * SQ ADC MAX1110 configuration
*/

#ifndef __SOCLE_ADC_MAX1110_REG_H
#define __SOCLE_ADC_MAX1110_REG_H       1

/* MAX1110 register definition */
#define MAX1110_START                   (1 << 7)

#define MAX1110_CHx                     (7 << 4)
#define MAX1110_SINGLE_END_CH0  		(0 << 4)
#define MAX1110_SINGLE_END_CH1  		(4 << 4)
#define MAX1110_SINGLE_END_CH2  		(1 << 4)
#define MAX1110_SINGLE_END_CH3 			(5 << 4)
#define MAX1110_SINGLE_END_CH4 			(2 << 4)
#define MAX1110_SINGLE_END_CH5 			(6 << 4)
#define MAX1110_SINGLE_END_CH6  		(3 << 4)
#define MAX1110_SINGLE_END_CH7  		(7 << 4)
#define MAX1110_DIFFERENTIAL_CH0_CH1    (0 << 4)
#define MAX1110_DIFFERENTIAL_CH2_CH3    (1 << 4)
#define MAX1110_DIFFERENTIAL_CH4_CH5    (2 << 4)
#define MAX1110_DIFFERENTIAL_CH6_CH7    (3 << 4)
#define MAX1110_DIFFERENTIAL_CH1_CH0    (4 << 4)
#define MAX1110_DIFFERENTIAL_CH3_CH2    (5 << 4)
#define MAX1110_DIFFERENTIAL_CH5_CH4    (6 << 4)
#define MAX1110_DIFFERENTIAL_CH7_CH6    (7 << 4)

#define MAX1110_UNI_BIP                 (1 << 3)
#define MAX1110_UNI_BIP_UNIPOLAR        (1 << 3)
#define MAX1110_UNI_BIP_BIPOLAR 		(0 << 3)

#define MAX1110_SGL_DIF                 (1 << 2)
#define MAX1110_SGL_DIF_SGL             (1 << 2)
#define MAX1110_SGL_DIF_DIF             (0 << 2)

#define MAX1110_PD1                     (1 << 1)
#define MAX1110_PD1_FULL_OP             (1 << 1)
#define MAX1110_PD1_POWER_DOWN  		(0 << 1)

#define MAX1110_PD0                     (1 << 0)
#define MAX1110_PD0_EXT_CLK             (1 << 0)
#define MAX1110_PD0_INT_CLK             (0 << 0)


/*	ioctl commands	*/
#define ADC_MAX1110_IOC_START_ADC			_IO('s', 20)


/*	AP IOCTL parameter define for control commands	*/
#define ADC_MAX1110_AP_OUTPUT_TYPE			(1 << 0)
#define ADC_MAX1110_AP_OUTPUT_TYPE_SGL		(0 << 0)
#define ADC_MAX1110_AP_OUTPUT_TYPE_DIFF		(1 << 0)
#define ADC_MAX1110_AP_CH_SHIFT				1
#define ADC_MAX1110_AP_CHx					(7 << 1)
#define ADC_MAX1110_AP_SGL_CH0				(0 << 1)
#define ADC_MAX1110_AP_SGL_CH1				(1 << 1)
#define ADC_MAX1110_AP_SGL_CH2				(2 << 1)
#define ADC_MAX1110_AP_SGL_CH3				(3 << 1)
#define ADC_MAX1110_AP_SGL_CH4				(4 << 1)
#define ADC_MAX1110_AP_SGL_CH5				(5 << 1)
#define ADC_MAX1110_AP_SGL_CH6				(6 << 1)
#define ADC_MAX1110_AP_SGL_CH7				(7 << 1)
#define ADC_MAX1110_AP_DIFF_CH0_1			(0 << 1)
#define ADC_MAX1110_AP_DIFF_CH1_0			(1 << 1)
#define ADC_MAX1110_AP_DIFF_CH2_3			(2 << 1)
#define ADC_MAX1110_AP_DIFF_CH3_2			(3 << 1)
#define ADC_MAX1110_AP_DIFF_CH4_5			(4 << 1)
#define ADC_MAX1110_AP_DIFF_CH5_4			(5 << 1)
#define ADC_MAX1110_AP_DIFF_CH6_7			(6 << 1)
#define ADC_MAX1110_AP_DIFF_CH7_6			(7 << 1)

#endif

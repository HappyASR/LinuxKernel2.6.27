/********************************************************************************
* File Name     : drivers/char/socle-msmv.c 
* Author         : Leonid Cheng
* Description   : Socle MSMV Driver
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
    
*   Version      : a.0
*   History      : 
*      1. 2008/06/19 leonid cheng create this file 
*    
********************************************************************************/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>

#include <asm/io.h>
#include <linux/delay.h>
#include <asm/memory.h>
#include <linux/miscdevice.h>

#include <asm/arch/msmv-scu.h>

/*	for key event	*/
#include <linux/input.h>

#include <linux/i2c.h>

struct socle_i2c_host {
        wait_queue_head_t wq;
        struct resource *io_area;
        u32 va_base;
        int irq;
        u8 arbit_lose : 1;
        u8 ack_period : 1;
        u8 recv_ack : 1;
        struct device *dev;
        struct i2c_adapter adap;
};


#define CONFIG_SOCLE_MSMV_DEBUG
#ifdef CONFIG_SOCLE_MSMV_DEBUG
	#define MSMV_CHAR_DBG(fmt, args...) printk("%s(): " fmt, __FUNCTION__, ## args)
#else
	#define MSMV_CHAR_DBG(fmt, args...)
#endif


//Socle MSMV  ioctl
#define MSMV_FREQ		_IO('k', 0x05)
#define MSMV_PWR_MODE	_IO('k', 0x06)
#define MSMV_RETENT		_IO('k', 0x07)

#define MSMV_KEY_EVENT		_IO('k', 0x08)			/*	for key event	*/


#define	MSMV_CLK_CHECK	0
#define	MSMV_200_100	1
#define MSMV_266_133	2
#define MSMV_400_100	3

#define	MSMV_IDLE		0
#define MSMV_STOP		1
#define MSMV_SLEEP		2

#define	MSMV_RETENT_EN	0
#define	MSMV_RETENT_DIS	1
#define	MSMV_CORE_PWR_EN_SRAM	2
#define	MSMV_CORE_PWR_DIS_SRAM	3
#define	MSMV_CORE_PWR_EN_UHC	4
#define	MSMV_CORE_PWR_DIS_UHC	5
#define	MSMV_RESET_UHC_BLOCK		6

static int msmv_freq(unsigned long args);
static void msmv_freq_set(int *freq);
static void msmv_freq_get(int *freq);
static int msmv_power_mode(int mode);
static int msmv_retent(int pwr);
static void msmv_idle_mode_enter(void);
static void msmv_stop_mode_enter(void);
//extern void msmv_sleep_mode_enter(void);

//static int msmv_kpd_event(int key);

//static u32 *addr_virt;


static int
msmv_freq(unsigned long args)
{
	int freq[3];

	MSMV_CHAR_DBG("msmv_freq\n");
	
	if (copy_from_user(&freq,  (const void __user *)args, sizeof(freq)))
		return -EFAULT;
	
	MSMV_CHAR_DBG("freq[0] = %d, freq[1] = %d, freq[2] = %d\n", freq[0], freq[1], freq[2]);

	switch(freq[0]){
		case 0 :
			MSMV_CHAR_DBG("msmv_freq_set\n");
			msmv_freq_set(freq);
			break;
		case 1 :
			MSMV_CHAR_DBG("msmv_freq_get\n");
			msmv_freq_get(freq);
			if (copy_to_user((void __user *)args,  &freq, sizeof(freq)))
				return -EFAULT;			
			break;
		default :
			printk("unknown msmv freq args!!\n");
			break;
	}

	return 0;	
}

static void
msmv_freq_set(int *freq)
{	
	int clock, ratio;

	MSMV_CHAR_DBG("freq[1] = %d\n", freq[1]);
			
	switch(freq[1]){
		case 33 :			
			clock =SOCLE_SCU_CPU_CLOCK_33 ;
			break;
		case 66 :			
			clock = SOCLE_SCU_CPU_CLOCK_66;
			break;
		case 80 :			
			clock = SOCLE_SCU_CPU_CLOCK_80;
			break;
		case 100 :			
			clock = SOCLE_SCU_CPU_CLOCK_100;
			break;
		case 132 :			
			clock = SOCLE_SCU_CPU_CLOCK_132;
			break;
		case 133 :			
			clock = SOCLE_SCU_CPU_CLOCK_133;
			break;
		case 200 :			
			clock = SOCLE_SCU_CPU_CLOCK_200;
			break;
		case 264 :			
			clock = SOCLE_SCU_CPU_CLOCK_264;
			break;
		case 266 :			
			clock = SOCLE_SCU_CPU_CLOCK_266;
			break;
		case 280 :			
			clock = SOCLE_SCU_CPU_CLOCK_280;
			break;
		case 300 :			
			clock = SOCLE_SCU_CPU_CLOCK_300;
			break;
		case 320 :			
			clock = SOCLE_SCU_CPU_CLOCK_320;
			break;
		case 340 :			
			clock = SOCLE_SCU_CPU_CLOCK_340;
			break;
		case 350 :			
			clock = SOCLE_SCU_CPU_CLOCK_350;
			break;
		case 360 :			
			clock = SOCLE_SCU_CPU_CLOCK_360;
			break;
		case 400 :			
			clock = SOCLE_SCU_CPU_CLOCK_400;
			break;
		case 252 :			
			clock = SOCLE_SCU_CPU_CLOCK_252;
			break;
		case 240 :			
			clock = SOCLE_SCU_CPU_CLOCK_240;
			break;
		case 212 :			
			clock = SOCLE_SCU_CPU_CLOCK_212;
			break;
		case 292 :			
			clock = SOCLE_SCU_CPU_CLOCK_292;
			break;
		case 160 :			
			clock = SOCLE_SCU_CPU_CLOCK_160;
			break;
		default :		
			printk("unknow upll clock !!\n");
			return;
			break;
	}

		
	switch(freq[2]){
		case 2 :			
			ratio =SOCLE_SCU_CLOCK_RATIO_2_1 ;
			break;
		case 3 :			
			ratio = SOCLE_SCU_CLOCK_RATIO_4_1;
			break;
		case 4 :			
			ratio = SOCLE_SCU_CLOCK_RATIO_8_1;
			break;
		default :		
			printk("unknow ratio !!\n");
			return;
			break;
	}
#if 1
	if(freq[1] > 264){
		socle_scu_clock_ratio_set(ratio);
		socle_scu_pll_set(clock);
	}else{
		socle_scu_pll_set(clock);
		socle_scu_clock_ratio_set(ratio);
	}
#else
	socle_scu_clk_src_ext_input_set();
	socle_scu_pll_set(clock);
	socle_scu_clock_ratio_set(ratio);
	socle_scu_clk_src_mpll_output_set();
#endif

	return;
}


static void  
msmv_freq_get(int *freq)
{
	freq[1] = socle_get_cpu_clock() / 1000 / 1000;
	freq[2] = socle_scu_clock_ratio_get();

	printk("cpu clock : %d, ratio : %d\n", freq[1], freq[2]);

	return ;
}


static int
msmv_power_mode(int mode)
{
	MSMV_CHAR_DBG("msmv_power_mode\n");
	
	switch(mode){
		case MSMV_IDLE :
			MSMV_CHAR_DBG("msmv_idle_mode_enter\n");
			msmv_idle_mode_enter();
			break;
		case MSMV_STOP :
			MSMV_CHAR_DBG("msmv_stop_mode_enter\n");
			msmv_stop_mode_enter();		
			break;
		case MSMV_SLEEP :
			MSMV_CHAR_DBG("msmv_sleep_mode_enter\n");
			//msmv_sleep_mode_enter();			
			break;
		default :
			printk("unknown mode value\n");
			return -1;
			break;
	}
	return 0;
}


static int
msmv_retent(int pwr)
{
	MSMV_CHAR_DBG("msmv_retent\n");		

	switch(pwr){
		case MSMV_RETENT_EN :
			MSMV_CHAR_DBG("sq_scu_embedded_sram_retent_enable\n");
			socle_scu_embedded_sram_retent_enable();
			break;
		case MSMV_RETENT_DIS :
			MSMV_CHAR_DBG("sq_scu_embedded_sram_retent_disable\n");
			socle_scu_embedded_sram_retent_disable();
			break;
		case MSMV_CORE_PWR_EN_SRAM :
			MSMV_CHAR_DBG("sq_scu_embedded_sram_core_power_enable\n");
			socle_scu_embedded_sram_core_power_enable();
			break;
		case MSMV_CORE_PWR_DIS_SRAM :
			MSMV_CHAR_DBG("sq_scu_embedded_sram_core_power_disable\n");
			socle_scu_embedded_sram_core_power_disable();		
			break;
		case MSMV_CORE_PWR_EN_UHC :
			MSMV_CHAR_DBG("sq_scu_uhc_core_power_enable\n");
			printk("b socle_scu_uhc_core_power_enable\n");		
			socle_scu_uhc_core_power_enable();		
			printk("a socle_scu_uhc_core_power_enable\n");		
			break;
		case MSMV_CORE_PWR_DIS_UHC :
			MSMV_CHAR_DBG("sq_scu_uhc_core_power_disable\n");
			printk("b socle_scu_uhc_core_power_disable\n");		
			socle_scu_uhc_core_power_disable();
			printk("a socle_scu_uhc_core_power_disable\n");		
			break;
		case MSMV_RESET_UHC_BLOCK :
			MSMV_CHAR_DBG("sq_scu_uhc_block_reset\n");
			printk("b socle_scu_uhc_block_reset\n");		
			socle_scu_uhc_block_reset();		
			printk("a socle_scu_uhc_block_reset\n");		
			break;			
		default :
			printk("unknown retent value\n");
			return -1;
			break;
	}
	return 0;
}


static void
msmv_idle_mode_enter(void)
{	
	MSMV_CHAR_DBG("msmv_idle_mode_enter\n");	
	
	printk("enter to idle mode\n");
	socle_scu_power_mode_idle_set();
}


static void 
msmv_stop_mode_enter()
{
	int tmp;	
	
	MSMV_CHAR_DBG("msmv_stop_mode_enter\n");	

	printk("enter to stop mode\n");
	
	socle_scu_power_mode_stop_set();
	
	printk("wake up from stop mode\n");
	tmp = socle_scu_wkupstat_status();

	return;
}

#if 0
extern void msmv_cpu_irq_enable(void);
extern void msmv_cpu_irq_disable(u32 addr);

static void msmv_irq_backup(u32 addr);
static void msmv_irq_resume(u32 addr);

static void msmv_timer_backup(u32 addr);
static void msmv_timer_resume(u32 addr);
#if 0
static void msmv_serial_backup(u32 addr);
static void msmv_serial_resume(u32 addr);
	
static void msmv_lcd_backup(u32 addr);
static void msmv_lcd_resume(u32 addr);

static void msmv_pwm_backup(u32 addr);
static void msmv_pwm_resume(u32 addr);

static void msmv_gpio_backup(u32 addr);
static void msmv_gpio_resume(u32 addr);

#endif
//extern void loop_msmv(void);

#define MSMV_INTC_SAVE		0x100	
#define MSMV_TIMER_SAVE	0x230
	
#if 0
#define MSMV_SERIAL_SAVE	0xa0	
#define MSMV_LCD_SAVE		0x260
#define MSMV_GPIO_SAVE		0x2a0	
#define MSMV_PWM_SAVE		0x2d0		
#endif

//extern void msmv_system_backup(u32);
extern void socle_cpu_suspend(u32);

extern void socle_i2c_master_initialize(struct socle_i2c_host *host);

extern void 
msmv_sleep_mode_enter()
{
	u32 tmp;

#if 0
	struct socle_i2c_host host1, host2, host3;	
	host1.va_base = IO_ADDRESS(SOCLE_APB0_I2C);
	//printk("host1.va_base = %x\n", host1.va_base);	
	host2.va_base = IO_ADDRESS(SOCLE_APB0_I2C1);	
	//printk("host2.va_base = %x\n", host2.va_base);	
	host3.va_base = IO_ADDRESS(SOCLE_APB0_I2C2);	
	//printk("host3.va_base = %x\n", host3.va_base);	
#endif
	MSMV_CHAR_DBG("msmv_sleep_mode_enter\n");	
	
	MSMV_CHAR_DBG("addr_virt = 0x%08x\n", (u32)addr_virt);


	socle_scu_info2_set((u32)addr_virt);
	tmp = virt_to_phys((u32 *)addr_virt);
	socle_scu_info1_set(tmp);
	
	/*	irq backup	*/
	msmv_cpu_irq_disable((u32)(addr_virt+(0x3c>>2)));
	msmv_irq_backup((u32)(addr_virt+(MSMV_INTC_SAVE>>2)));		

	/*	un remap	*/
	iowrite32(0x0, 0xfc1a0024);
#if 0
	//save LCD register
	msmv_lcd_backup((u32)(addr_virt+(MSMV_LCD_SAVE>>2)));


	//save PWM register
	msmv_pwm_backup((u32)(addr_virt+(MSMV_PWM_SAVE>>2)));
	//save GPIO register
	msmv_gpio_backup((u32)(addr_virt+(MSMV_GPIO_SAVE>>2)));
#endif


	/*	timer backup	*/
	msmv_timer_backup((u32)(addr_virt+(MSMV_TIMER_SAVE>>2)));
	
	/*	UART backup	*/
	//msmv_serial_backup((u32)(addr_virt+(MSMV_SERIAL_SAVE>>2)));

	//mdelay(1000);
	
	//msmv_system_backup((u32)addr_virt);
	socle_cpu_suspend((u32)addr_virt);
		
	//continue when sleep wake up

	
	mdelay(1000);
		
		/*	UART resume	*/
		//msmv_serial_resume((u32)((addr_virt)+(MSMV_SERIAL_SAVE>>2)));
		printk("uart ok!!\n");

		
		//socle_i2c_master_initialize(&host1);	
		//socle_i2c_master_initialize(&host2);	
		//socle_i2c_master_initialize(&host3);	
#if 0
		//resume LCD register	
		msmv_lcd_resume((u32)(addr_virt+(MSMV_LCD_SAVE>>2)));

	
		//resume PWM register	
		msmv_pwm_resume((u32)(addr_virt+(MSMV_PWM_SAVE>>2)));
		//resume GPIO register	
		msmv_gpio_resume((u32)(addr_virt+(MSMV_GPIO_SAVE>>2)));
		
#endif
		msmv_timer_resume((u32)(addr_virt+(MSMV_TIMER_SAVE>>2)));
		printk("timer wake up from sleep mode\n");
		
		/*	remap	*/
		iowrite32(0xCAD20712, 0xfc1a0024);
		printk("remap after sleep mode\n");
		
		/*	irq resume	*/
		msmv_irq_resume((u32)(addr_virt+(MSMV_INTC_SAVE>>2)));
		printk("irq resume from sleep mode\n");
				
		msmv_cpu_irq_enable();	
		printk("msmv_cpu_irq_enable\n");
	
		

		printk("\nwake up from sleep mode\n");

		//loop_msmv();
	return;
}

EXPORT_SYMBOL(msmv_sleep_mode_enter);


static void
msmv_irq_backup(u32 addr)
{
	int tmp;
	u32 addr_int = 0xf7040000;
	
	iowrite32(ioread32(addr_int+0x10c), (addr+0x10c));
	iowrite32(0, addr_int+0x10c);

	for(tmp=0;tmp<0x10c;tmp+=4){
		iowrite32(ioread32(addr_int+tmp), (addr+tmp));
	}
	for(tmp=0x110;tmp<0x124;tmp+=4){
		iowrite32(ioread32(addr_int+tmp), (addr+tmp));
	}

	return ;
}

static void
msmv_irq_resume(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xf7040000;

	for(tmp=0;tmp<0x10c;tmp+=4){
		iowrite32(ioread32(addr+tmp), (addr_int+tmp));
	}
	for(tmp=0x110;tmp<0x124;tmp+=4){
		iowrite32(ioread32(addr+tmp), (addr_int+tmp));
	}
	iowrite32(ioread32(addr+0x10c), (addr_int+0x10c));
	
	return ;
}

static void 
msmv_timer_backup(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xfc100000;
	
	for(tmp=0;tmp<0x30;tmp+=0x10){
		iowrite32(ioread32(addr_int+tmp), (addr+tmp));
		iowrite32(ioread32(addr_int+tmp+0x8), (addr+tmp+0x8));
	}
	
	return ;
}

static void
msmv_timer_resume(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xfc100000;

	for(tmp=0;tmp<0x30;tmp+=0x10){
		iowrite32(ioread32(addr+tmp), (addr_int+tmp));
		iowrite32(ioread32(addr+tmp+0x8), (addr_int+tmp+0x8));
	}
	
	return ;
}
#if 0

static void 
msmv_serial_backup(u32 addr)
{
	u32 addr_int;

	addr_int = IO_ADDRESS(SOCLE_APB0_UART0);
	
	iowrite32(ioread32(addr_int+0xc), (addr));
	iowrite32(ioread32(addr_int+0x10), (addr+0x4));	

	iowrite32(0x80, (addr_int+0xc));

	iowrite32(ioread32(addr_int+0x0), (addr+0x8));
	iowrite32(ioread32(addr_int+0x4), (addr+0xc));	

	iowrite32(0, (addr_int+0xc));
	
	iowrite32(ioread32(addr_int+0x4), (addr+0x10));
	iowrite32(0x0, (addr_int+0x4));


	addr_int = IO_ADDRESS(SOCLE_APB0_UART1);
	
	iowrite32(ioread32(addr_int+0xc), (addr+0x20));
	iowrite32(ioread32(addr_int+0x10), (addr+0x24));	
	iowrite32(0x80, (addr_int+0xc));
	iowrite32(ioread32(addr_int+0x0), (addr+0x28));
	iowrite32(ioread32(addr_int+0x4), (addr+0x2c));	
	iowrite32(0, (addr_int+0xc));
	iowrite32(ioread32(addr_int+0x4), (addr+0x30));
	iowrite32(0x0, (addr_int+0x4));
		
	return;
}

static void 
msmv_serial_resume(u32 addr)
{
	u32 addr_int;

	addr_int = IO_ADDRESS(SOCLE_APB0_UART0);
	
	iowrite32(0x0, (addr_int+0x4));
	iowrite32(0x80, (addr_int+0xc));
	iowrite32(ioread32(addr+0x8), (addr_int+0x0));
	iowrite32(ioread32(addr+0xc), (addr_int+0x4));
	iowrite32(0x0, (addr_int+0xc));
	iowrite32(ioread32(addr+0x4), (addr_int+0x10));
	iowrite32(0x7, (addr_int+0x8));
	iowrite32(ioread32(addr+0x10), (addr_int+0x4));
	iowrite32(ioread32(addr+0x0), (addr_int+0xc));

	
	addr_int = IO_ADDRESS(SOCLE_APB0_UART1);
	
	iowrite32(0x0, (addr_int+0x4));
	iowrite32(0x80, (addr_int+0xc));
	iowrite32(ioread32(addr+0x28), (addr_int+0x0));
	iowrite32(ioread32(addr+0x2c), (addr_int+0x4));
	iowrite32(0x0, (addr_int+0xc));
	iowrite32(ioread32(addr+0x24), (addr_int+0x10));
	iowrite32(0x7, (addr_int+0x8));
	iowrite32(ioread32(addr+0x30), (addr_int+0x4));
	iowrite32(ioread32(addr+0x20), (addr_int+0xc));

	return;

}


static void 
msmv_lcd_backup(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xfbb00000;
	
	for(tmp=0;tmp<0x34;tmp+=0x4){
		iowrite32(ioread32(addr_int+tmp), (addr+tmp));
	}
	
	return ;
}

static void
msmv_lcd_resume(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xfbb00000;

	for(tmp=0;tmp<0x34;tmp+=0x4){
		iowrite32(ioread32(addr+tmp), (addr_int+tmp));
	}
	
	return ;
}

static void 
msmv_pwm_backup(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xfc120000;
	
	for(tmp=0;tmp<0x30;tmp+=0x4){
		iowrite32(ioread32(addr_int+tmp), (addr+tmp));
	}
	
	return ;
}

static void
msmv_pwm_resume(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xfc120000;

	for(tmp=0;tmp<0x30;tmp+=0x4){
		iowrite32(ioread32(addr+tmp), (addr_int+tmp));
	}
	
	return ;
}

static void 
msmv_gpio_backup(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xfc140000;
	
	for(tmp=0;tmp<=24;tmp+=0x4){
		iowrite32(ioread32(addr_int+tmp), (addr+tmp));
	}
	
	return ;
}

static void
msmv_gpio_resume(u32 addr)
{
	u32 tmp;
	u32 addr_int = 0xfc140000;

	for(tmp=0;tmp<=24;tmp+=0x4){
		iowrite32(ioread32(addr+tmp), (addr_int+tmp));
	}
	
	return ;
}

#endif

#endif

static int 
socle_msmv_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret, val;

	MSMV_CHAR_DBG("sq_msmv_ioctl\n");

	val = (int)arg;
	
	switch (cmd)	{
		case MSMV_FREQ :	
			MSMV_CHAR_DBG("MSMV_FREQ : %d\n", val);
			ret = msmv_freq(arg);
			break;
		case MSMV_PWR_MODE :	
			MSMV_CHAR_DBG("MSMV_PWR_MODE : %d\n", val);
			ret = msmv_power_mode(val);
			break;
		case MSMV_RETENT :	
			MSMV_CHAR_DBG("MSMV_RETENT : %d\n", val);
			ret = msmv_retent(val);
			break;
		/*	for key event	*/
	#if 0
		case MSMV_KEY_EVENT :	
			MSMV_CHAR_DBG("MSMV_KEY_EVENT : %d\n", val);
			ret = mp_gpio_kpd_event(val);
			break;			
	#endif
		default :			
			printk("SQ_msmv_ioctl command fail\n");
			ret = -ENOTTY;
			break;			
	}

	return ret;
}


static int socle_msmv_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	MSMV_CHAR_DBG("sq_msmv_read\n");

	return 0;
}

static int socle_msmv_write(struct file *flip, const char __user *buf, size_t count, loff_t *pos)
{
	MSMV_CHAR_DBG("sq_msmv_write\n");
		
	return 0;
}

static int socle_msmv_open(struct inode *inode, struct file *file)
{
	MSMV_CHAR_DBG("sq_msmv_open\n");
	printk("SQ_msmv_open\n");
	
	return 0;
}

static int socle_msmv_release(struct inode *inode, struct file *file)
{
	MSMV_CHAR_DBG("sq_msmv_release\n");
	
	return 0;	
}

static const struct file_operations msmv_fops = {
        .owner =        THIS_MODULE,
        .llseek =       no_llseek,
        .ioctl =        socle_msmv_ioctl,
        .open =         socle_msmv_open,
        .read = 	socle_msmv_read,
        .write =        socle_msmv_write,
        .release =      socle_msmv_release,
};

struct miscdevice misc_msmv = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "socle-msmv",
	.fops = &msmv_fops,
};

static int socle_msmv_remove(struct platform_device *pdev)
{
	MSMV_CHAR_DBG("sq_msmv_remove\n");
	
	//kfree(addr_virt);
			
	misc_deregister(&misc_msmv);

	return 0;	
}


static int socle_msmv_probe(struct platform_device *pdev)
{	
	int err = 0;

	MSMV_CHAR_DBG("sq_msmv_probe\n");	

	err = misc_register(&misc_msmv);
	
	//addr_virt = kmalloc(0x200, GFP_KERNEL);
	
	//MSMV_CHAR_DBG("addr_virt = 0x%08x\n", (u32)addr_virt);
	
	return err;	
}

#ifdef CONFIG_PM
static int
socle_msmv_suspend(struct platform_device *pdev, pm_message_t msg)
{
	pr_debug("sq_msmv_suspend\n");

        return 0;
}

static int 
socle_msmv_resume(struct platform_device *pdev)
{	
	pr_debug("sq_msmv_resume\n");
	
  	return 0;
}
#else
#define socle_msmv_suspend NULL
#define socle_msmv_resume NULL
#endif

static struct platform_driver socle_msmv_drv = {
	.probe		= socle_msmv_probe,
	.remove		= socle_msmv_remove,
	.suspend = socle_msmv_suspend,
	.resume = socle_msmv_resume,
	.driver		= {
		.name	= "socle-msmv",
		.owner	= THIS_MODULE,
	},
};

static char __initdata banner[] = "SQ MSMV, (c) 2008 SQ Corp. \n";

static int __init socle_msmv_init(void)
{
	printk(banner);

	return platform_driver_register(&socle_msmv_drv);
}

static void __exit socle_msmv_exit(void)
{
	platform_driver_unregister(&socle_msmv_drv);
}

module_init(socle_msmv_init);
module_exit(socle_msmv_exit);

MODULE_DESCRIPTION("SQ MSMV Driver");
MODULE_LICENSE("GPL");


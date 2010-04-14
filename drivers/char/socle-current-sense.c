/********************************************************************************
* File Name     : drivers/char/socle-current-sense.c 
* Author         : Leonid Cheng
* Description   : Socle  Current Sense LM3824 Driver
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
*      1. 2008/05/13 leonid cheng create this file 
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
#include <asm/arch/regs-pwmt.h>


//#define CONFIG_SOCLE_CUR_SEN_DEBUG
#ifdef CONFIG_SOCLE_CUR_SEN_DEBUG
	#define CUR_SEN_DBG(fmt, args...) printk("%s(): " fmt, __FUNCTION__, ## args)
#else
	#define CUR_SEN_DBG(fmt, args...)
#endif

//Socle Current Sense Ioctl
#define CURRENT_SENSE_IRQ		_IO('k', 0x03)
#define CURRENT_SENSE_PULL		_IO('k', 0x04)

#define USE_PWM_NUM 1
#define DUTY_TABLE_NUM 21

int duty_table[DUTY_TABLE_NUM]={5000, 5230, 5450, 5680, 5910, 6140, 6360, 6590, 6820, 7050, 
				7270, 7500, 7730, 7950, 8180, 8410, 8640, 8860, 9090, 9320, 9550};


static int socle_current_sense_enable_irq(int arg);
static int socle_current_sense_enable_pull(int arg);
static void socle_pwmt_capture_mode_init(struct socle_pwmt *p_pwmt, int enable);
static int current_sense_calc(int *hrc, int *lrc, int cnt);
static int current_duty_conver_table(int duty);
static int socle_current_sense_interrupt(int irq, void *data);

#define MAX_CNT 20

struct socle_pwmt *p_pwmt_cur;
int socle_pwmt_isr_flag;
int cur_cnt, sen_cnt;
u32 hrc_val[MAX_CNT];
u32 lrc_val[MAX_CNT];

static int socle_current_sense_enable_irq(int arg)
{
	int duty;
	u32 ma=0;	

	CUR_SEN_DBG("sq_current_sense_enable_irq\n");

	cur_cnt = 0;
	sen_cnt = 0;
	socle_pwmt_isr_flag=0;
			
	p_pwmt_cur->drv->set_counter (p_pwmt_cur, 0);
	p_pwmt_cur->drv->enable_interrupt (p_pwmt_cur, 1);	
	p_pwmt_cur->drv->enable(p_pwmt_cur, 1);

	while(1){	
		if(socle_pwmt_isr_flag){
			socle_pwmt_isr_flag = 0;
			CUR_SEN_DBG(" hrc_val[cur_cnt] = %x, lrc_val[cur_cnt] = %x, cur_cnt = %d\n",
					hrc_val[cur_cnt], lrc_val[cur_cnt], cur_cnt);
			duty = current_sense_calc(hrc_val, lrc_val, sen_cnt);
			if(duty == -1){
				cur_cnt = 0;			
				p_pwmt_cur->drv->set_counter (p_pwmt_cur, 0);
				p_pwmt_cur->drv->enable_interrupt (p_pwmt_cur, 1);
				printk("duty = -1\n");
				continue;
			}else{
				//printk("duty cycle = %d.%02d	", (duty/100), (duty%100));
				ma = current_duty_conver_table(duty);
				if(ma > 1000)
					printk("convert duty error\n");
				else
					CUR_SEN_DBG("current sense is %4d mA\n", ma);
				break;
			}					
		}else if(cur_cnt >= MAX_CNT){				
			p_pwmt_cur->drv->enable(p_pwmt_cur, 0);
			return -1;
		}
	}
		
	p_pwmt_cur->drv->enable(p_pwmt_cur, 0);
	
	return ma;	
}


static int socle_current_sense_enable_pull(int arg)
{
	int duty;
	u32 ma=0;	
	u32 hrc, lrc;
	
	CUR_SEN_DBG("sq_current_sense_enable_pull\n");	

	cur_cnt = 0;
	hrc_val[0]=0;
	lrc_val[0]=0;
		
	p_pwmt_cur->drv->set_counter (p_pwmt_cur, 0);
	p_pwmt_cur->drv->write_hrc (p_pwmt_cur, 0);
	p_pwmt_cur->drv->write_lrc (p_pwmt_cur, 0);
	p_pwmt_cur->drv->enable(p_pwmt_cur, 1);

	while(1){	
			if(cur_cnt > MAX_CNT-2)
				break;
			
			hrc =p_pwmt_cur->drv->read_hrc(p_pwmt_cur);
			lrc = p_pwmt_cur->drv->read_lrc(p_pwmt_cur);
			CUR_SEN_DBG("hrc = %x, lrc = %x\n", hrc, lrc);
			if(hrc != hrc_val[cur_cnt]){
				cur_cnt++;
				hrc_val[cur_cnt] = hrc;
				lrc_val[cur_cnt] = lrc;
				if((hrc_val[cur_cnt-1]==0) || (lrc_val[cur_cnt] != lrc_val[cur_cnt-1]))
					continue;
			} else if (lrc != lrc_val[cur_cnt]) {
				cur_cnt++;			
				hrc_val[cur_cnt] = hrc;
				lrc_val[cur_cnt] = lrc;
				if((lrc_val[cur_cnt-1]==0) || (hrc_val[cur_cnt] != hrc_val[cur_cnt-1]))
					continue;
			}else
				continue;
			
			CUR_SEN_DBG("hrc_val[%d]=%x, lrc_val[%d]=%x\n", 
						cur_cnt-1, hrc_val[cur_cnt-1], cur_cnt-1, lrc_val[cur_cnt-1]);
			CUR_SEN_DBG("hrc_val[%d]=%x, lrc_val[%d]=%x\n", 
						cur_cnt, hrc_val[cur_cnt], cur_cnt, lrc_val[cur_cnt]);
			duty = current_sense_calc(hrc_val, lrc_val, cur_cnt);
			if(duty == -1){
				cur_cnt = 0;			
				p_pwmt_cur->drv->set_counter (p_pwmt_cur, 0);
				printk("duty = -1\n");
				continue;
			}else{
				CUR_SEN_DBG("duty cycle = %d.%02d	", (duty/100), (duty%100));
				ma = current_duty_conver_table(duty);
				if(ma > 1000)
					printk("convert duty error\n");
				else
					CUR_SEN_DBG("current sense is %4d mA\n", ma);
				break;
			}	
	}
	p_pwmt_cur->drv->enable(p_pwmt_cur, 0);
	
	if(cur_cnt > 100)
		return -1;		
	
	return ma;	
}


static int
socle_current_sense_interrupt(int irq, void *data)
{
	CUR_SEN_DBG("sq_current_sense_interrupt\n");

	p_pwmt_cur->drv->clear_interrupt (p_pwmt_cur);
	
	hrc_val[cur_cnt] =p_pwmt_cur->drv->read_hrc(p_pwmt_cur);
	lrc_val[cur_cnt] = p_pwmt_cur->drv->read_lrc(p_pwmt_cur);
	CUR_SEN_DBG(" hrc_val[%d] = %x, lrc_val[%d] = %x\n", cur_cnt, hrc_val[cur_cnt], cur_cnt, lrc_val[cur_cnt]);
	
	if(cur_cnt>2){
		if((hrc_val[cur_cnt] == hrc_val[cur_cnt-1]) || (lrc_val[cur_cnt] == lrc_val[cur_cnt-1])){
			sen_cnt = cur_cnt;
			p_pwmt_cur->drv->enable_interrupt (p_pwmt_cur, 0);
			//printk(" cur_cnt = %d\n", cur_cnt);			
			cur_cnt++;
			socle_pwmt_isr_flag = 1;
			return IRQ_HANDLED;
		}
	}

	cur_cnt++;

	return IRQ_HANDLED;
}


static void
socle_pwmt_capture_mode_init(struct socle_pwmt *p_pwmt, int enable)
{
	struct socle_pwmt_driver *pwmt_drv = p_pwmt->drv;
	
	pwmt_drv->claim_pwm_lock();

	pwmt_drv->reset(p_pwmt);
	pwmt_drv->write_prescale_factor(p_pwmt, 4);
	
	if (enable) {
		pwmt_drv->write_hrc(p_pwmt, 0);
		pwmt_drv->write_lrc(p_pwmt, 0);
		pwmt_drv->capture_mode_enable(p_pwmt, 1);
		pwmt_drv->output_enable(p_pwmt, 0);
	}
	
	pwmt_drv->release_pwm_lock();
}




int
current_sense_calc(int *hrc_val, int *lrc_val, int cnt)
{
	u32 val1, val2, val3;
	int duty;

	if(hrc_val[cnt] == hrc_val[cnt-1]){
		val1 = lrc_val[cnt-1]; 
		val2 = hrc_val[cnt]; 
		val3 = lrc_val[cnt]; 
		duty = ((val3-val2)*10000) / (val3-val1) ;
	}else{
		val1 = hrc_val[cnt-1]; 
		val2 = lrc_val[cnt]; 
		val3 = hrc_val[cnt]; 
		duty = ((val2-val1)*10000) / (val3-val1) ;
	}
	
	CUR_SEN_DBG("hrc_val[cnt-1]=%x, lrc_val[cnt-1]=%x\n", hrc_val[cnt-1], lrc_val[cnt-1]);
	CUR_SEN_DBG("hrc_val[cnt]=%x, lrc_val[cnt]=%x\n", hrc_val[cnt], lrc_val[cnt]);
	CUR_SEN_DBG("val1=%x, val2=%x, val3=%x\n", val1, val2, val3);
	CUR_SEN_DBG("count = %d\n", cnt);

	if((duty<0) || (duty>10000))
		duty =-1;
	
	return duty; 
}

int
current_duty_conver_table(int duty)
{
	u32 val;
	int index;

	if (duty < 5000)
		duty = 10000 - duty;
	
	for(index=0; index < DUTY_TABLE_NUM; index++){
		if(duty <= duty_table[index]){
			val = 50 * (duty -duty_table[index-1]) / (duty_table[index]-duty_table[index-1]);
			val = val + 50*(index-1);
			
			return val;
		}
	}
	return 1001;		//it > max(1000)	
}


static int socle_current_sense_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;

	CUR_SEN_DBG("sq_current_sense_ioctl\n");
	
	switch (cmd)	{
		case CURRENT_SENSE_IRQ :	
			ret = socle_current_sense_enable_irq(arg);
			break;
		case CURRENT_SENSE_PULL :	
			ret = socle_current_sense_enable_pull(arg);
			break;
		default :			
			//printk("SQ_current_sense_ioctl command fail\n");
			ret = -ENOTTY;
			break;			
	}

	return ret;
}


static int socle_current_sense_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	int ma;

	CUR_SEN_DBG("sq_current_sense_read\n");

	//while(1){	
		ma = socle_current_sense_enable_pull(0);
		if(ma == -1)
			printk("not sample current sense\n");
		else
			printk("current sense : %3d ma\n", ma);
	//	mdelay(2000);
	//}
		
	//else		
		//printk("current sense done\n");
	return 0;
}

static int socle_current_sense_write(struct file *flip, const char __user *buf, size_t count, loff_t *pos)
{
	char cmd;
	int loop, ma;
	
	CUR_SEN_DBG("sq_current_sense_write\n");
	
	copy_from_user(&cmd, buf, 1);
	switch (cmd) {
		case 'i': 	
			ma = socle_current_sense_enable_irq(0);
			if(ma == -1){
				printk("not sample current sense\n");
				return 0;
			}else
				printk("current sense (IRQ): %3d ma\n", ma);
			break;
		case 'I': 
			loop = 30;
			while(loop){	
				ma = socle_current_sense_enable_irq(0);	
				if(ma == -1){
					printk("not sample current sense\n");
					return 0;
				}else
					printk("current sense (IRQ): %3d ma\n", ma);
				loop--;
				mdelay(2000);
			}
			break;
		case 'p': 
			ma = socle_current_sense_enable_pull(0);
			if(ma == -1){
				printk("not sample current sense\n");
				return 0;
			}else
				printk("current sense (Pulling): %3d ma\n", ma);
			break;
		case 'P': 
			loop = 30;
			while(loop){	
				ma = socle_current_sense_enable_pull(0);	
				if(ma == -1){
					printk("not sample current sense\n");
					return 0;
				}else
					printk("current sense (Pulling): %3d ma\n", ma);
				loop--;
				mdelay(2000);
			}
			break;
	}
	
	return 1;
}

static int socle_current_sense_open(struct inode *inode, struct file *file)
{
	CUR_SEN_DBG("sq_current_sense_open\n");
	
	return 0;
}

static int socle_current_sense_release(struct inode *inode, struct file *file)
{
	CUR_SEN_DBG("sq_current_sense_release\n");
	
	return 0;	
}

static const struct file_operations current_sense_fops = {
        .owner =        THIS_MODULE,
        .llseek =       no_llseek,
        .ioctl =        socle_current_sense_ioctl,
        .open =         socle_current_sense_open,
        .read = 		socle_current_sense_read,
        .write =        socle_current_sense_write,
        .release =      socle_current_sense_release,
};

struct miscdevice misc_current_sense = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "socle-current-sense",
	.fops = &current_sense_fops,
};

static int socle_current_sense_remove(struct platform_device *pdev)
{
	CUR_SEN_DBG("sq_current_sense_remove\n");
		
	misc_deregister(&misc_current_sense);
	free_irq(p_pwmt_cur->irq, p_pwmt_cur);
	release_socle_pwmt_structure(USE_PWM_NUM);

	return 0;	
}


static int socle_current_sense_probe(struct platform_device *pdev)
{	
	int err = 0, ret;

	CUR_SEN_DBG("sq_current_sense_probe\n");	

	p_pwmt_cur = get_socle_pwmt_structure(USE_PWM_NUM);

	if (NULL == p_pwmt_cur) {
		printk("Get PWMT structure error!!\n");
		return -1;
	}
	
	//First Set the triger level of interrupt controller  
	ret = request_irq(p_pwmt_cur->irq, (irq_handler_t)socle_current_sense_interrupt, 
							IRQF_DISABLED, "Socle Current Sense", p_pwmt_cur);	
	if (ret ){		
		printk(KERN_ERR "SQ Current Sense: failed to request interrupt\n");
		return ret;
	}
	
	socle_pwmt_capture_mode_init(p_pwmt_cur, 1);

	err = misc_register(&misc_current_sense);
	
	return err;	
}


#ifdef CONFIG_PM
static int
socle_current_sense_suspend(struct platform_device *pdev, pm_message_t msg)
{
	pr_debug("leonid : socle_current_sense_suspend\n");

        return 0;
}

static int 
socle_current_sense_resume(struct platform_device *pdev)
{	
	pr_debug("sq_current_sense_resume\n");
	
  	return 0;
}

static int 
socle_current_sense_resume_early(struct platform_device *pdev)
{	
	pr_debug("sq_current_sense_resume_early\n");
	
  	return 0;
}
#else
#define socle_current_sense_suspend NULL
#define socle_current_sense_resume NULL
#endif

static struct platform_driver socle_current_sense_drv = {
	.probe		= socle_current_sense_probe,
	.remove		= socle_current_sense_remove,
	.suspend = socle_current_sense_suspend,
	.resume = socle_current_sense_resume,
	.resume_early = socle_current_sense_resume_early,
	.driver		= {
		.name	= "socle-current-sense",
		.owner	= THIS_MODULE,
	},
};

static char __initdata banner[] = "SQ Current Sense, (c) 2010 SQ Corp. \n";

static int __init socle_current_sense_init(void)
{
	printk(banner);

	return platform_driver_register(&socle_current_sense_drv);
}

static void __exit socle_current_sense_exit(void)
{
	platform_driver_unregister(&socle_current_sense_drv);
}

module_init(socle_current_sense_init);
module_exit(socle_current_sense_exit);

MODULE_DESCRIPTION("SQ Current Sense Driver");
MODULE_LICENSE("GPL");


/********************************************************************************
* File Name     : drivers/char/socle-hdma-test.c 
* Author         : Leonid Cheng
* Description   : Socle  HDMA Debug Driver
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
*      1. 2007/12/21 leonid cheng create this file 
*    
********************************************************************************/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/types.h>

#include <asm/io.h>
#include <asm/memory.h>
#include <linux/miscdevice.h>
#include <asm/arch/dma.h>
#include <linux/dma-mapping.h>

#define SOCLE_HDMA_COHERENT
#define AUTO_COMPARE

//Socle HDMA PSEUDO Ioctl
#define HDMA_PSEUDO_ENABLE	_IO('k', 0x01)
#define HDMA_PSEUDO_COMPARE	_IO('k', 0x02)

#define	SOCLE_HDMA_PSEUDO_BUF_SIZE		0x20000
#define	LOOP_COUNT		0x10000

#if defined(CONFIG_ARCH_CDK) || defined(CONFIG_ARCH_PDK_PC9002) || defined(CONFIG_ARCH_SCDK)
#define	SOCLE_HDMA_PSEUDO_CH_NUM		5
#elif defined(CONFIG_ARCH_P7DK) || defined(CONFIG_ARCH_PDK_PC7210)
#define	SOCLE_HDMA_PSEUDO_CH_NUM		0
#else 
#define	SOCLE_HDMA_PSEUDO_CH_NUM		2	//for SEDK
#endif

static u32 *addr_virt_a;
static u32 *addr_virt_b;
static dma_addr_t addr_dma_a;
static dma_addr_t addr_dma_b;

static int	socle_hdma_pseudo_burst;
static int	cnt_flag;

static int socle_hdma_pseudo_enable(int arg);
static void socle_hdma_pseudo_compare(void);
static void socle_hdma_pseudo_dma(void);

static int socle_hdma_pseudo_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	//printk("SQ_hdma_pseudo_ioctl\n");
	
	switch (cmd)	{
		case HDMA_PSEUDO_ENABLE :	
			socle_hdma_pseudo_enable(arg);
			return 0;
		case HDMA_PSEUDO_COMPARE :	
			socle_hdma_pseudo_compare();
			return 0;
	}

	printk("SQ_hdma_pseudo_ioctl command fail\n");
	return 0;
}

static int socle_hdma_pseudo_enable(int arg)
{
	int cnt=SOCLE_HDMA_PSEUDO_BUF_SIZE/4;
	int pat=0;

	//printk ("sq_hdma_pseudo_enable\n");
	
	//if((cnt_flag & 0x3f) == 0)
        //      printk("#");

	switch(arg){
		case '0' :
			socle_hdma_pseudo_burst = SOCLE_DMA_BURST_SINGLE;
			break;
		case '1' :
			socle_hdma_pseudo_burst = SOCLE_DMA_BURST_INCR4;
			break;
		case '2' :
			socle_hdma_pseudo_burst = SOCLE_DMA_BURST_INCR8;
			break;
		case '3' :
			socle_hdma_pseudo_burst = SOCLE_DMA_BURST_INCR16;
			break;
		default : 
			printk("Invalid DMA Burst Type\n");
			return -1;
	}
			
	/*	make pattern	for 1st addr	*/
	while(cnt--){
		*(addr_virt_a + pat) = pat;
		*(addr_virt_b + pat) = 0xffffffff;
		pat++;
	}
#if 0
	pat=0x0;
	cnt=0x20;
	while(cnt--){
		printk("addr_virt_a : 0x%08x, pat : %d\n", *(addr_virt_a + pat), pat);
		pat++;
	}
	pat=0x0;
	cnt=0x20;
	while(cnt--){
		printk("addr_virt_b : 0x%08x, pat : %d\n", *(addr_virt_b + pat), pat);
		pat++;
	}
#endif
	//printk ("HDMA_PSEUDO_ENABLE pattern complete\n");
	cnt_flag = 0;
	socle_hdma_pseudo_dma();
	
	return 0;
}

static void socle_hdma_pseudo_compare(void)
{
	int cnt=SOCLE_HDMA_PSEUDO_BUF_SIZE/4;
	int pat=0, ret=0;
	
	while(pat < cnt){
		if(*(addr_virt_a + pat) != pat){
			printk("error1 -> addr : 0x%08x , val : 0x%08x, pat : 0x%08x, cnt : 0x%08x\n", 
				(u32)(addr_virt_a + pat), *(addr_virt_a + pat), pat, cnt);
			ret = -1;
			if(pat > 100)
				break;
		}
		pat++;		
	}
	printk("\ncompare a complete!!\n"); 
	pat=0;
	while(pat < cnt){
		if(*(addr_virt_b + pat) != pat){
			printk("error2 -> addr : 0x%08x , val : 0x%08x, pat : 0x%08x, cnt : 0x%08x\n", 
				(u32)(addr_virt_b + pat), *(addr_virt_b + pat), pat, cnt);
			ret = -1;
			if(pat > 100)
                                break;
		//}else{
			//if(ret == -1)
				//printk("addr : 0x%08x , val : 0x%08x\n", 	(u32)(addr_virt_b + pat), *(addr_virt_b + pat));
		}
		pat++;		
	}
	printk("compare b complete!!\n"); 
	if(ret == -1)
		printk ("HDMA_PSEUDO_COMPARE fail\n");	
	else
		printk ("HDMA_PSEUDO_COMPARE complete\n");
	
	return;
}

static void
socle_hdma_pseudo_dma(void)
{
	u32 flags;

	//printk ("sq_hdma_pseudo_dma\n");
	/*	Configure the sw dma settng of HDMA	*/
	flags = socle_claim_dma_lock();
	socle_disable_dma(SOCLE_HDMA_PSEUDO_CH_NUM);
	socle_set_dma_mode(SOCLE_HDMA_PSEUDO_CH_NUM, SOCLE_DMA_MODE_SW);
	socle_set_dma_burst_type(SOCLE_HDMA_PSEUDO_CH_NUM, socle_hdma_pseudo_burst);
	socle_set_dma_source_direction(SOCLE_HDMA_PSEUDO_CH_NUM, SOCLE_DMA_DIR_INCR);
	socle_set_dma_destination_direction(SOCLE_HDMA_PSEUDO_CH_NUM, SOCLE_DMA_DIR_INCR);
	socle_set_dma_data_size(SOCLE_HDMA_PSEUDO_CH_NUM, SOCLE_DMA_DATA_WORD);
	socle_set_dma_transfer_count(SOCLE_HDMA_PSEUDO_CH_NUM, SOCLE_HDMA_PSEUDO_BUF_SIZE);
	if((cnt_flag & 0x1) == 0){
		socle_set_dma_source_address(SOCLE_HDMA_PSEUDO_CH_NUM, addr_dma_a);
		socle_set_dma_destination_address(SOCLE_HDMA_PSEUDO_CH_NUM, addr_dma_b);
	}else{
		socle_set_dma_source_address(SOCLE_HDMA_PSEUDO_CH_NUM, addr_dma_b);
		socle_set_dma_destination_address(SOCLE_HDMA_PSEUDO_CH_NUM, addr_dma_a);
	}	
	/* Enable the dma to run*/
	socle_enable_dma(SOCLE_HDMA_PSEUDO_CH_NUM);
	//printk ("sq_enable_dma\n");
	socle_release_dma_lock(flags);
	
}


static void
socle_hdma_pseudo_interrupt(void *data)
{
	//printk("SQ_hdma_pseudo_interrupt\n");

	if((cnt_flag & 0x3f) == 0)
		printk("*");
	cnt_flag++;
	if(cnt_flag<LOOP_COUNT)
		socle_hdma_pseudo_dma();
	else{
		printk("\nsocle_hdma_pseudo_dma complete\n");
#ifdef AUTO_COMPARE
	socle_hdma_pseudo_compare();
#endif
	}

/*
	int i;

	for(i=0;i<0x40;){
		printk("reg 0x%08x, val 0x%08x\n", (SOCLE_AHB0_HDMA+i), ioread32(SOCLE_AHB0_HDMA+i));
		i+=4;
	}
*/
	return ;
}


static int socle_hdma_pseudo_read(struct file *filp, char __user *buf, size_t count, loff_t *pos)
{
	printk("SQ_hdma_pseudo_read\n");
	return 0;
}

static int socle_hdma_pseudo_open(struct inode *inode, struct file *file)
{
	printk("SQ_hdma_pseudo_open\n");	
	return 0;
}

static int socle_hdma_pseudo_release(struct inode *inode, struct file *file)
{
	printk("SQ_hdma_pseudo_release\n");
	return 0;	
}

static const struct file_operations hdma_pseudo_fops = {
        .owner =        THIS_MODULE,
        .llseek =       no_llseek,
        .ioctl =        socle_hdma_pseudo_ioctl,
        .open =         socle_hdma_pseudo_open,
        .read = 		socle_hdma_pseudo_read,
        .release =      socle_hdma_pseudo_release,
};

struct miscdevice misc_hdma_pseudo = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "socle-hdma-pseudo",
	.fops = &hdma_pseudo_fops,
};

static int socle_hdma_pseudo_remove(struct platform_device *pdev)
{
	printk("SQ_hdma_pseudo_remove\n");
	
	socle_free_dma(SOCLE_HDMA_PSEUDO_CH_NUM);
	misc_deregister(&misc_hdma_pseudo);

#ifdef 	SOCLE_HDMA_COHERENT	
	dma_free_coherent(NULL, SOCLE_HDMA_PSEUDO_BUF_SIZE, addr_virt_a, addr_dma_a);
	dma_free_coherent(NULL, SOCLE_HDMA_PSEUDO_BUF_SIZE, addr_virt_b, addr_dma_b);
#else
	kfree(addr_virt_a);
	kfree(addr_virt_b);
#endif

	return 0;	
}

static struct socle_dma_notifier socle_hdma_pseudo_notifier = {
	.complete = socle_hdma_pseudo_interrupt,
};

static int socle_hdma_pseudo_probe(struct platform_device *pdev)
{	
	int err = 0;

	printk("SQ_hdma_pseudo_probe\n");
	
	err = socle_request_dma(SOCLE_HDMA_PSEUDO_CH_NUM, pdev->name, &socle_hdma_pseudo_notifier);
	if (err) {
		dev_err(&pdev->dev, "SQ HDMA PSEUDO: cannot claim dma channel\n");
		return -1;
	}
	
	err = misc_register(&misc_hdma_pseudo);
	
#ifdef 	SOCLE_HDMA_COHERENT	
	addr_virt_a = dma_alloc_coherent(NULL, SOCLE_HDMA_PSEUDO_BUF_SIZE , &addr_dma_a, GFP_KERNEL);
	addr_virt_b = dma_alloc_coherent(NULL, SOCLE_HDMA_PSEUDO_BUF_SIZE , &addr_dma_b, GFP_KERNEL);
#else
	addr_virt_a = kmalloc(SOCLE_HDMA_PSEUDO_BUF_SIZE, GFP_KERNEL);
	addr_virt_b = kmalloc(SOCLE_HDMA_PSEUDO_BUF_SIZE, GFP_KERNEL);
	//addr_virt_a = kmalloc(SOCLE_HDMA_PSEUDO_BUF_SIZE, GFP_DMA);
	//addr_virt_b = kmalloc(SOCLE_HDMA_PSEUDO_BUF_SIZE, GFP_DMA);
	addr_dma_a = virt_to_phys(addr_virt_a);
	addr_dma_b = virt_to_phys(addr_virt_b);
#endif

	printk("addr_virt_a = 0x%08x, addr_phy_a = 0x%08x\n", (u32)addr_virt_a, addr_dma_a);
	printk("addr_virt_b = 0x%08x, addr_phy_b = 0x%08x\n", (u32)addr_virt_b, addr_dma_b);	
	
	return err;	
}

static struct platform_driver socle_hdma_pseudo_drv = {
	.probe		= socle_hdma_pseudo_probe,
	.remove		= socle_hdma_pseudo_remove,
	.driver		= {
		.name	= "socle-hdma-pseudo",
		.owner	= THIS_MODULE,
	},
};

static char __initdata banner[] = "SQ HDMA PSEUDO, (c) 2007 SQ Corp. \n";

static int __init socle_hdma_pseudo_init(void)
{
	printk(banner);

	return platform_driver_register(&socle_hdma_pseudo_drv);
}

static void __exit socle_hdma_pseudo_exit(void)
{
	platform_driver_unregister(&socle_hdma_pseudo_drv);
}

module_init(socle_hdma_pseudo_init);
module_exit(socle_hdma_pseudo_exit);

MODULE_DESCRIPTION("SQ HDMA PSEUDO Driver");
MODULE_LICENSE("GPL");


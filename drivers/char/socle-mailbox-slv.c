#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
//#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <mach/spi-regs.h>
#include <mach/platform.h>
#include <mach/spi-mailbox.h>


static u32 mail_size[MAXMAIL];
static u32 socle_spi_base;
static int spi_mailbox_slv_irq;

static struct spi_mailbox spi_mb = {
	.char_len = SOCLE_SPI_CHAR_LEN_8,
	.rx_tri_lv = SOCLE_SPI_RXFIFO_INT_TRIGGER_LEVEL_4,
	.tx_tri_lv = SOCLE_SPI_TXFIFO_INT_TRIGGER_LEVEL_4,
	.cpol = SOCLE_SPI_CPOL_0,
	.cpha = SOCLE_SPI_CPHA_0,
	.xsb = SOCLE_SPI_TX_MSB_FIRST,
	.rx_size = 0,
	.rx_cnt = 0,
	.tx_size = 0,
	.tx_cnt =0,
	.mail_cmd = RCMD,
	.mail_cnt = 0,
	.mail_rx_id= 0,
	.mail_tx_id= 0,
	.mail_state=0,
};

static inline void socle_spi_write(u32 val, u32 reg)
{
	iowrite32(val, socle_spi_base+reg);
}

static inline u32 socle_spi_read(u32 reg)
{
	return ioread32(socle_spi_base+reg);
}

static irqreturn_t spi_mailbox_slv_isr (int irq, void *pparm)
{
  u32 tmp = socle_spi_read(SOCLE_SPI_ISR);
	
	if (SOCLE_SPI_RX_REG0_INT == (tmp & SOCLE_SPI_RX_REG0_INT)) {
		socle_spi_write(SOCLE_SPI_RX_REG0_INT,SOCLE_SPI_ISR); //clear int
		if(spi_mb.mail_state!=0)
			return IRQ_HANDLED;
		
		spi_mb.mail_cmd=socle_spi_read(SOCLE_SPI_RX_REG0);    //read cmd from rx reg0
		//printk("mailbox slave: cmd=%d\n",spi_mb.mail_cmd);
		socle_spi_write(STS_BUSY, SOCLE_SPI_TX_REG0);         //set sts busy
		if(spi_mb.mail_cmd == RCMD) {                         //if read cmd
			if(spi_mb.mail_cnt==0) {                                   //if no mail return
				printk("mailbox slave: mail empty\n");
				socle_spi_write(STS_READY, SOCLE_SPI_TX_REG0);
				return IRQ_HANDLED;
			}				
			spi_mb.tx_size=mail_size[spi_mb.mail_tx_id];        	//get tx_size
			socle_spi_write((spi_mb.tx_size>>8)&0xff, SOCLE_SPI_TX_REG1); 	//write tx_size to tx reg1 for master read
			socle_spi_write(spi_mb.tx_size & 0xff, SOCLE_SPI_TX_REG2); 	//write tx_size to tx reg2 for master read
			
			socle_spi_write(socle_spi_read(SOCLE_SPI_IER) | SOCLE_SPI_IER_TXFIFO_INT_EN, SOCLE_SPI_IER); //enable tx int
			spi_mb.tx_cnt=0;
			spi_mb.mail_state=4;
			if((spi_mb.tx_size >> 16)& 0x1)
				socle_spi_write( STS_RDEN | LBIT, SOCLE_SPI_TX_REG0);       //write high bit and read enable status to tx reg0
			else
				socle_spi_write( STS_RDEN, SOCLE_SPI_TX_REG0);
		}
		else if(spi_mb.mail_cmd & WCMD) {                    //if write cmd
			if(spi_mb.mail_cnt==MAXMAIL) {                             //if mail full return
				printk("mailbox slave: mail full\n");
				socle_spi_write(STS_READY, SOCLE_SPI_TX_REG0);
				return IRQ_HANDLED;
			}	
			spi_mb.mail_state=1;                              //wait size state
		}
		else {
			printk("mailbox slave: unknow command\n");
			socle_spi_write(STS_READY, SOCLE_SPI_TX_REG0);
		}
		return IRQ_HANDLED;
	}	

	if (SOCLE_SPI_RX_REG1_INT == (tmp & SOCLE_SPI_RX_REG1_INT)) {
		socle_spi_write(SOCLE_SPI_RX_REG1_INT,SOCLE_SPI_ISR);      //clear int
		if(spi_mb.mail_state!=1)
			return IRQ_HANDLED;
		spi_mb.mail_state=2;
		return IRQ_HANDLED;
	}
	
	if (SOCLE_SPI_RX_REG2_INT == (tmp & SOCLE_SPI_RX_REG2_INT)) {
		
		socle_spi_write(SOCLE_SPI_RX_REG2_INT,SOCLE_SPI_ISR);      //clear int
		if(spi_mb.mail_state!=2)
			return IRQ_HANDLED;
		spi_mb.rx_size=socle_spi_read(SOCLE_SPI_RX_REG2) + (socle_spi_read(SOCLE_SPI_RX_REG1)<<8);          //get rx_size low
		if(spi_mb.mail_cmd & LBIT)
			spi_mb.rx_size |= (0x1 << 16);
		//printk("mailbox slave: size= %d\n",spi_mb.rx_size);	
		spi_mb.mail_storage[spi_mb.mail_rx_id]=(unsigned char *)kmalloc(spi_mb.rx_size,GFP_KERNEL);  //alloc mail storage size
		mail_size[spi_mb.mail_rx_id]=spi_mb.rx_size;               //record size
		socle_spi_write(socle_spi_read(SOCLE_SPI_IER) | SOCLE_SPI_IER_RXAVAIL_INT_EN, SOCLE_SPI_IER); //enable rx int
		spi_mb.rx_cnt=0;
		spi_mb.mail_state=3;
		
		return IRQ_HANDLED;
	}
		
	if (SOCLE_SPI_RX_DATA_AVAIL_INT == (tmp & SOCLE_SPI_RX_DATA_AVAIL_INT)) {			
		while (SOCLE_SPI_RXFIFO_DATA_AVAIL == (socle_spi_read(SOCLE_SPI_FCR) & SOCLE_SPI_RXFIFO_DATA_AVAIL)){
			spi_mb.mail_storage[spi_mb.mail_rx_id][spi_mb.rx_cnt] = socle_spi_read(SOCLE_SPI_RXR); //read from rx fifo
			spi_mb.rx_cnt++;
			//printk("%d\n",spi_mb.rx_cnt);
			if(spi_mb.rx_cnt == spi_mb.rx_size) {
				//printk("mailbox slave: recive finish!!\n");
				socle_spi_write(socle_spi_read(SOCLE_SPI_IER) & ~SOCLE_SPI_IER_RXAVAIL_INT_EN, SOCLE_SPI_IER); //disable rx int
				spi_mb.mail_rx_id = (spi_mb.mail_rx_id + 1)%MAXMAIL; //update tx id 
				spi_mb.mail_cnt++;                                   //update mail cnt
				spi_mb.mail_state=0;
				socle_spi_write(spi_mb.mail_cnt, SOCLE_SPI_TX_REG3); 
				socle_spi_write(STS_READY, SOCLE_SPI_TX_REG0);       //set sts ready
				break;
			}
		}
		return IRQ_HANDLED;
	}
	 	
	if (SOCLE_SPI_TXFIFO_INT == (tmp & SOCLE_SPI_TXFIFO_INT)) {
		do {   
			socle_spi_write(spi_mb.mail_storage[spi_mb.mail_tx_id][spi_mb.tx_cnt], SOCLE_SPI_TXR);  //write to tx fifo
			spi_mb.tx_cnt++;
			if(spi_mb.tx_cnt == spi_mb.tx_size) {
				socle_spi_write(socle_spi_read(SOCLE_SPI_IER) & ~SOCLE_SPI_IER_TXFIFO_INT_EN, SOCLE_SPI_IER);  //disable tx int
				kfree(spi_mb.mail_storage[spi_mb.mail_tx_id]);                       //free mail storage which have sent out
				mail_size[spi_mb.mail_tx_id]=0;                                     //clear size
				spi_mb.mail_tx_id = (spi_mb.mail_tx_id + 1)%MAXMAIL;                //update tx id        
				spi_mb.mail_cnt--;                                        //update mail cnt
				socle_spi_write(socle_spi_read(SOCLE_SPI_IER) | SOCLE_SPI_IER_TXFIFO_EMPTY_INT_EN, SOCLE_SPI_IER);
				break;
			} 
		} while (SOCLE_SPI_TXFIFO_FULL != (socle_spi_read(SOCLE_SPI_FCR) & SOCLE_SPI_TXFIFO_FULL));
		return IRQ_HANDLED;
	}
	
	if (SOCLE_SPI_TXFIFO_EMPTY_INT == (tmp & SOCLE_SPI_TXFIFO_EMPTY_INT)) {
			socle_spi_write(socle_spi_read(SOCLE_SPI_IER) & ~SOCLE_SPI_IER_TXFIFO_EMPTY_INT_EN, SOCLE_SPI_IER);
			spi_mb.mail_state=0;
    	socle_spi_write(spi_mb.mail_cnt, SOCLE_SPI_TX_REG3);
    	socle_spi_write(STS_READY, SOCLE_SPI_TX_REG0);                        //set sts ready
    	return IRQ_HANDLED; 
	}
	
	if(SOCLE_SPI_IER_CHAR_LEN_INT_EN == (tmp & SOCLE_SPI_CHAR_LEN_INT)) {
		socle_spi_write(SOCLE_SPI_CHAR_LEN_INT,SOCLE_SPI_ISR);
		printk("mailbox slave: Wrong bits\n");
	}
	return IRQ_HANDLED;
}

static void spi_mailbox_slv_setup(void)
{
	socle_spi_write(
		socle_spi_read(SOCLE_SPI_FWCR) & ~SOCLE_SPI_MODE_MASTER,
		SOCLE_SPI_FWCR);	

		/* Reset SPI controller */
  socle_spi_write( 
  	socle_spi_read(SOCLE_SPI_FWCR) | SOCLE_SPI_MASTER_SOFT_RST,
  	SOCLE_SPI_FWCR);
	
	socle_spi_write(spi_mb.char_len, SOCLE_SPI_SSCR);
	
	/* Configure FIFO and clear Tx & Rx FIFO */
	socle_spi_write(
			spi_mb.rx_tri_lv |
			spi_mb.tx_tri_lv |
			SOCLE_SPI_RXFIFO_CLR |
			SOCLE_SPI_TXFIFO_CLR,
			SOCLE_SPI_FCR);
     
	/* Enable SPI interrupt */
	socle_spi_write(
			SOCLE_SPI_IER_CHAR_LEN_INT_EN |
			//SOCLE_SPI_IER_RXFIFO_OVR_INT_EN |
			SOCLE_SPI_IER_RX_REG0_INT_EN | 
			SOCLE_SPI_IER_RX_REG1_INT_EN |
			SOCLE_SPI_IER_RX_REG2_INT_EN ,
			SOCLE_SPI_IER);
			
	socle_spi_write(
			SOCLE_SPI_MODE_SLAVE |
			SOCLE_SPI_MASTER_EN |
			spi_mb.cpol |
			spi_mb.cpha |
			spi_mb.xsb,
			SOCLE_SPI_FWCR);
}

static int
spi_mailbox_slv_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	if(cmd==1) {
		copy_to_user((char __user *)arg, &spi_mb.mail_cnt, 4);
	}
	return 0;
}

static ssize_t
spi_mailbox_slv_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	u32 tmp_size;
	//printk("mailbox: read\n");
	
	if(spi_mb.mail_cnt==0) {
		printk("mailbox slave: mail empty\n");
		return 0;
	}	
	
	if(spi_mb.mail_state!=0) {
		printk("mailbox slave: state busy\n");
		return 0;
	}
	
	socle_spi_write(0, SOCLE_SPI_TX_REG0); 
	
	spi_mb.mail_state=4;
	spi_mb.tx_cnt=0;
	spi_mb.tx_size=mail_size[spi_mb.mail_tx_id];
		
	if(count > spi_mb.tx_size)
		count =	spi_mb.tx_size;
	
	while(count) {
		if(count > PAGE_SIZE)
			tmp_size = PAGE_SIZE;
		else
			tmp_size = count;
		if(copy_to_user(buf, spi_mb.mail_storage[spi_mb.mail_tx_id]+ spi_mb.tx_cnt, tmp_size))
			return 0;
		
		buf+=	tmp_size;
		spi_mb.tx_cnt+=tmp_size;
		count-=tmp_size;
	}
	
	kfree(spi_mb.mail_storage[spi_mb.mail_tx_id]);
	mail_size[spi_mb.mail_tx_id]=0;                                     //clear size
	spi_mb.mail_tx_id = (spi_mb.mail_tx_id + 1)%MAXMAIL;                //update tx id        
	spi_mb.mail_cnt--;                                           //update mail cnt
	spi_mb.mail_state=0;
	socle_spi_write(STS_READY, SOCLE_SPI_TX_REG0);                        //set sts ready
  socle_spi_write(spi_mb.mail_cnt, SOCLE_SPI_TX_REG3); 
	return spi_mb.tx_cnt;
}

static ssize_t 
spi_mailbox_slv_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) 
{	
	int tmp_size;
	//printk("mailbox: write\n");
	
	if(spi_mb.mail_cnt==MAXMAIL) {
		printk("mailbox slave: mail full\n");
		return 0;
	}
	if(spi_mb.mail_state!=0) {
		printk("mailbox slave: state busy\n");
		return 0;
	}
	socle_spi_write(0, SOCLE_SPI_TX_REG0); 
	spi_mb.mail_state=3;
	spi_mb.rx_size=count;
	spi_mb.rx_cnt=0;
	spi_mb.mail_storage[spi_mb.mail_rx_id]=(unsigned char *)kmalloc(spi_mb.rx_size, GFP_KERNEL);  //alloc mail storage size
	while(count) {
		if(count > PAGE_SIZE)
			tmp_size = PAGE_SIZE;
		else
			tmp_size = count;
		if(copy_from_user(spi_mb.mail_storage[spi_mb.mail_rx_id] + spi_mb.rx_cnt, buf, tmp_size))
			return -1;
		
		buf+=	tmp_size;
		spi_mb.rx_cnt+=tmp_size;
		count-=tmp_size;
	}
	
	mail_size[spi_mb.mail_rx_id] = spi_mb.rx_cnt;
	spi_mb.mail_rx_id++;
	if(spi_mb.mail_rx_id==MAXMAIL)
		spi_mb.mail_rx_id=0;                //update rx id        
	spi_mb.mail_cnt++;
	spi_mb.mail_state=0;
	socle_spi_write(STS_READY, SOCLE_SPI_TX_REG0);                        //set sts ready
  socle_spi_write(spi_mb.mail_cnt, SOCLE_SPI_TX_REG3); 
	return spi_mb.rx_cnt;
}

static int
spi_mailbox_slv_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int
spi_mailbox_slv_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations spi_mailbox_slv_fops = {
        .owner =        THIS_MODULE,
        .ioctl =        spi_mailbox_slv_ioctl,
        .write =        spi_mailbox_slv_write,
        .read =         spi_mailbox_slv_read,
        .open =         spi_mailbox_slv_open,
        .release =      spi_mailbox_slv_release,
};

struct miscdevice misc_spi_mailbox_slv = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "spi_mailbox_slv",
	.fops = &spi_mailbox_slv_fops,
};

static int
spi_mailbox_slv_remove(struct platform_device *dev)
{
  free_irq(spi_mailbox_slv_irq, NULL);
	misc_deregister(&misc_spi_mailbox_slv);
	return 0;
}

static int
spi_mailbox_slv_probe(struct platform_device *pdev)
{
	int ret = 0;
	
	socle_spi_base = IO_ADDRESS(SOCLE_APB0_SPI0);
	spi_mailbox_slv_irq = IRQ_SPI0;
  
  spi_mailbox_slv_setup();
		/* Allocate the interrupt */
	ret |= request_irq(spi_mailbox_slv_irq, (irq_handler_t) spi_mailbox_slv_isr, IRQF_DISABLED, "spi_mailbox_slv", NULL);
	if (ret) {
		printk("mailbox slave: cannot claim IRQ\n");
		return ret;
	}
	
  ret = misc_register(&misc_spi_mailbox_slv);	
	if(ret!=0)
		printk("mailbox slave: probe fail\n");
  printk("mailbox slave: probe end\n");
  
  socle_spi_write(STS_READY, SOCLE_SPI_TX_REG0); 
	return ret;
}

static struct platform_driver spi_mailbox_slv_driver = {
	.probe		= spi_mailbox_slv_probe,
	.remove		= spi_mailbox_slv_remove,
	.driver		= {
		.name	= "spi_mailbox_slv",
		.owner	= THIS_MODULE,
	},
};

static int __init
spi_mailbox_slv_init(void)
{
	int err=0;
	err = platform_driver_register(&spi_mailbox_slv_driver);
	if(err)
		printk("mailbox slave: init register fail\n");
	return err;
}

static void __exit
spi_mailbox_slv_exit(void)
{
	platform_driver_unregister(&spi_mailbox_slv_driver);
}

module_init(spi_mailbox_slv_init);
module_exit(spi_mailbox_slv_exit);

MODULE_DESCRIPTION("SQ SPI Mailbox Slave");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("JS Ho <jsho@socle-tech.com.tw>");

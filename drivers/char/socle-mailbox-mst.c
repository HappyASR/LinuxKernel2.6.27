#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/cdev.h>
#include <asm/uaccess.h>	/* coyp_*_user */
#include <linux/miscdevice.h>
#include <linux/spi/spi.h>
#include <asm/arch/spi-regs.h>
#include <asm/arch/spi-mailbox.h>
#include <linux/delay.h>

#define SOCLE_HW_CMD_READ 0x01
#define SOCLE_HW_CMD_RDFS 0x02
#define SOCLE_HW_CMD_RDREG_0 0x03
#define SOCLE_HW_CMD_RDREG_1 0x04
#define SOCLE_HW_CMD_RDREG_2 0x05
#define SOCLE_HW_CMD_RDREG_3 0x06
#define SOCLE_HW_CMD_WRITE 0x08
#define SOCLE_HW_CMD_WRREG_0 0x09
#define SOCLE_HW_CMD_WRREG_1 0x0A
#define SOCLE_HW_CMD_WRREG_2 0x0B
#define SOCLE_HW_CMD_WRREG_3 0x0C

#define MAX_FIFO_DATA_CNT 8 
#define WAIT_CNT 100

#define SET_TX_RX_LEN(tx, rx)	(((tx) << 16) | (rx))

static struct spi_device *spi_mailbox_mst;
static u8 mail_cnt=0;

static int
socle_spi_slave_fifo_write(u8 *buf, size_t count)
{
	int err = 0;
	u8 tx_buf[MAX_FIFO_DATA_CNT+1] = {0};
	struct spi_message msg;
	struct spi_transfer xfer;
	
	tx_buf[0] = SOCLE_HW_CMD_WRITE;
	memcpy(&tx_buf[1], buf, count);
	spi_message_init(&msg);
	memset(&xfer, 0, sizeof(struct spi_transfer));
	xfer.tx_buf = tx_buf;
	xfer.len = SET_TX_RX_LEN(count+1, 0);
	spi_message_add_tail(&xfer, &msg);
	err = spi_sync(spi_mailbox_mst, &msg);
	return err;
	 
}

static int
socle_spi_slave_fifo_read(u8 *buf, size_t count)
{
	int err = 0;
	u8 tx_buf=SOCLE_HW_CMD_READ;
	struct spi_message msg;
	struct spi_transfer xfer;
	
	spi_message_init(&msg);
	memset(&xfer, 0, sizeof(struct spi_transfer));
	xfer.tx_buf = &tx_buf;
	xfer.rx_buf = buf;
	xfer.len = SET_TX_RX_LEN(1, count);
	spi_message_add_tail(&xfer, &msg);
	err = spi_sync(spi_mailbox_mst, &msg);
	return err;
}

static int
socle_spi_slave_reg_write(u8 reg, u8 val)
{
	int err = 0;
	u8 tx_buf[2] = {0};
	struct spi_message msg;
	struct spi_transfer xfer;
	
	tx_buf[0] = reg;
	tx_buf[1] = val;
	spi_message_init(&msg);
	memset(&xfer, 0, sizeof(struct spi_transfer));
	xfer.tx_buf = tx_buf;
	xfer.len = SET_TX_RX_LEN(2, 0);
	spi_message_add_tail(&xfer, &msg);
	err = spi_sync(spi_mailbox_mst, &msg);
	return err;
}

static int
socle_spi_slave_reg_read(u8 reg, u8 *buf)
{
	int err = 0;
	u8 tx_buf=0;
	struct spi_message msg;
	struct spi_transfer xfer;
	
	tx_buf = reg;
	spi_message_init(&msg);
	memset(&xfer, 0, sizeof(struct spi_transfer));
	xfer.tx_buf = &tx_buf;
	xfer.rx_buf = buf;
	xfer.len = SET_TX_RX_LEN(1, 1);
	spi_message_add_tail(&xfer, &msg);
	err = spi_sync(spi_mailbox_mst, &msg);
	return err;
}

static int
spi_mailbox_mst_get_fifo_sts(u8 *buf)
{
	int err = 0;
	err=socle_spi_slave_reg_read(SOCLE_HW_CMD_RDFS, buf);
	return err;
}

static int
spi_mailbox_mst_get_mail_cnt(u8 *buf)
{
	int err = 0;
	err=socle_spi_slave_reg_read(SOCLE_HW_CMD_RDREG_3, buf);
	return err;
}

static int
spi_mailbox_mst_get_mb_sts(u8 *buf)
{
	int err = 0;
	err=socle_spi_slave_reg_read(SOCLE_HW_CMD_RDREG_0, buf);
	return err;
}

static int
spi_mailbox_mst_wait_sts(u8 sts)
{
	int err = 0;
	u8 mb_sts=0;
	int rty_cnt=0;
	while(1) {
		if((err=spi_mailbox_mst_get_mb_sts(&mb_sts)))
			break;
		if(mb_sts & sts)
			break;
		if(rty_cnt>WAIT_CNT) {
			printk("mailbox master: Retry count (%d) exceed threshold\n", rty_cnt);
			err=-1;
			break;
		}
		rty_cnt++;
		msleep(20);
	}
	return err;
}

static int
spi_mailbox_mst_set_mail_size(size_t count)
{
	int err = 0;
	u8 size[2]= {0};
	u8 cmd=WCMD, mail_cnt=0;
	
	if((err=spi_mailbox_mst_get_mail_cnt(&mail_cnt)))
		return err;
	if(mail_cnt >= MAXMAIL) {
		err=-1;
		printk("mailbox master: mail full\n");
		return err;
	}	
	if((err=spi_mailbox_mst_wait_sts(STS_READY)))
		return err;
	
	if((count>>16)& 0x1)
		cmd |= LBIT;
	
	err |= socle_spi_slave_reg_write(SOCLE_HW_CMD_WRREG_0, cmd);
	
	size[0] = (count >> 8) & 0xff;
	size[1] = count & 0xff;
	//printk("size0=%d size1=%d\n",size[0],size[1]);
	err |= socle_spi_slave_reg_write(SOCLE_HW_CMD_WRREG_1, size[0]);
	err |= socle_spi_slave_reg_write(SOCLE_HW_CMD_WRREG_2, size[1]);
	
	return err;
}

static int
spi_mailbox_mst_get_mail_size(ssize_t *buf)
{
	int err = 0;
	u8 size[2]= {0};
	u8 mb_sts=0,mail_cnt=0;
	
	if((err=spi_mailbox_mst_get_mail_cnt(&mail_cnt)))
		return err;
	if(mail_cnt == 0) {
		err=-1;
		printk("mailbox master: mail empty\n");
		return err;
	}
	if((err=spi_mailbox_mst_wait_sts(STS_READY)))
		return err;
	
	if((err=socle_spi_slave_reg_write(SOCLE_HW_CMD_WRREG_0, RCMD)))
		return err;
	
	if((err=spi_mailbox_mst_wait_sts(STS_RDEN)))
		return err;
		
	err=spi_mailbox_mst_get_mb_sts(&mb_sts);
	err |= socle_spi_slave_reg_read(SOCLE_HW_CMD_RDREG_1, &size[0]);
	err |= socle_spi_slave_reg_read(SOCLE_HW_CMD_RDREG_2, &size[1]);
	buf[0] = (size[0]<<8)+ size[1];
	if(mb_sts&LBIT)
		buf[0]|=(0x1<<16);
	return err;
}

static int
spi_mailbox_mst_write_data(u8 *buf, size_t count)
{
	int err = 0;
	u8 ff_sts=0;
	u32 i=0;
	
	while(count) {
		if((err=spi_mailbox_mst_get_fifo_sts(&ff_sts)))
			goto WR_OUT;
		if(ff_sts & SOCLE_SPI_SLAVE_RxFF_EMPTY) {
			if(count>=8) {
				if((err=socle_spi_slave_fifo_write(buf+i, MAX_FIFO_DATA_CNT)))
					goto WR_OUT;
				count-=MAX_FIFO_DATA_CNT;
				i+=MAX_FIFO_DATA_CNT;
			}
			else {
				if((err=socle_spi_slave_fifo_write(buf+i, count)))
					goto WR_OUT;
				count=0;
			}
		}
	}
	
WR_OUT:
	return err;
}

static int
spi_mailbox_mst_read_data(u8 *buf, size_t count)
{
	int err = 0;
	u8 ff_sts=0;
	u32 i=0;
	
	while(count) {
		if((err=spi_mailbox_mst_get_fifo_sts(&ff_sts)))
			goto RD_OUT;
		if(ff_sts & SOCLE_SPI_SLAVE_TxFF_FULL) {
			if((err=socle_spi_slave_fifo_read(buf+i, MAX_FIFO_DATA_CNT)))
				goto RD_OUT;
			count-=MAX_FIFO_DATA_CNT;
			i+=MAX_FIFO_DATA_CNT;
		}
		else if( ff_sts & SOCLE_SPI_SLAVE_TxFF_HALFFULL ) {
			if((err=socle_spi_slave_fifo_read(buf+i, MAX_FIFO_DATA_CNT/2)))
				goto RD_OUT;
			count-=MAX_FIFO_DATA_CNT/2;
			i+=MAX_FIFO_DATA_CNT/2;
		}
		else if(ff_sts & SOCLE_SPI_SLAVE_TxFF_EMPTY)
			continue;
		else {
			if((err=socle_spi_slave_fifo_read(buf+i, 1)))
				goto RD_OUT;
			count--;
			i++;
		}
	} 
	 
RD_OUT:
	return err;
}

static ssize_t 
spi_mailbox_mst_write(struct file *flip, const char __user *buf, size_t count,
		     loff_t *f_pos) 
{
	
	u8 *data = NULL;
	ssize_t ret_cnt=count;
	
	data = kmalloc( ret_cnt, GFP_KERNEL);
	if (!data) {
		printk("mailbox master: allocate memory fail\n");
		return 0;
	}

	if(copy_from_user(data, buf, count)) {
		kfree(data);
		printk("mailbox master: copy from user fail\n");
		return 0;
	}

  if(spi_mailbox_mst_set_mail_size(count)) {
		kfree(data);
		printk("mailbox master: set size fail\n");
		return 0;
	}

	if(spi_mailbox_mst_write_data(data, count)) {
		kfree(data);
		printk("mailbox master: write data fail\n");
		return 0;
	}

	kfree(data);
	return ret_cnt;
}

static ssize_t
spi_mailbox_mst_read(struct file *flip, char __user *buf, size_t count,
		    loff_t *f_pos)
{
	u8 *data = NULL;
	ssize_t ret_cnt=0;
	
	if(spi_mailbox_mst_get_mail_size(&ret_cnt)) {
		printk("mailbox master: get size fail\n");
		return 0;
	}
  data = kmalloc(ret_cnt, GFP_KERNEL);
	if (!data) {
		printk("mailbox master: allocate memory fail\n");
		return 0;
	}
	
	if(spi_mailbox_mst_read_data(data, ret_cnt)) {
		kfree(data);
		printk("mailbox master: read data fail\n");
		return 0;
	}
	
	if(copy_to_user(buf, data, ret_cnt)) {
		kfree(data);
		printk("mailbox master: copy to user fail\n");
		return 0;
	}
	
	kfree(data);
	return ret_cnt;
}

static int
spi_mailbox_mst_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	if(cmd==1) {
		if(spi_mailbox_mst_get_mail_cnt(&mail_cnt)) {
			printk("mailbox master ioctl: get cnt error");
			return 0;
		}
		//printk("mailbox maste: mail_cnt= %d",mail_cnt);
		copy_to_user((char __user *)arg, &mail_cnt, 4);
	}
	return 0;
}

/*
 * Open and close
 * */
static int
spi_mailbox_mst_open(struct inode *inode, struct file *flip)
{
	return 0;
}

static int
spi_mailbox_mst_release(struct inode *inode, struct file *flip)
{
	return 0;
}


static struct file_operations spi_mailbox_mst_fops = {
	.owner = THIS_MODULE,
	.ioctl = spi_mailbox_mst_ioctl,
	.read = spi_mailbox_mst_read,
	.write = spi_mailbox_mst_write,
	.open = spi_mailbox_mst_open,
	.release = spi_mailbox_mst_release,
};

struct miscdevice misc_spi_mailbox_mst = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "spi_mailbox_mst",
	.fops = &spi_mailbox_mst_fops,
};

static int __devinit 
spi_mailbox_mst_probe(struct spi_device *spi)
{
	int err = 0;
		
	spi_mailbox_mst = spi;
	spi->bits_per_word = 8;
	err = misc_register(&misc_spi_mailbox_mst);
	if(err!=0)
		printk("mailbox master: probe fail\n");
	printk("mailbox master: probe end\n");
	return err;
}

static int __devexit
spi_mailbox_mst_remove(struct spi_device *spi)
{
	misc_deregister(&misc_spi_mailbox_mst);
	return 0;
}

static struct spi_driver spi_mailbox_mst_driver = {
	.driver = {
		.name = "spi_mailbox_mst",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
	},
	.probe = spi_mailbox_mst_probe,
	.remove = __devexit_p(spi_mailbox_mst_remove),
};

static int __init 
spi_mailbox_mst_init(void)
{
	int err=0;
	err=spi_register_driver(&spi_mailbox_mst_driver);
	if(err)
		printk("mailbox master: init register fail\n");
	return err;
}
module_init(spi_mailbox_mst_init);

static void __exit
spi_mailbox_mst_exit(void)
{
	spi_unregister_driver(&spi_mailbox_mst_driver);
}
module_exit(spi_mailbox_mst_exit);

MODULE_DESCRIPTION("SPI Mailbox Master Driver");
MODULE_AUTHOR("JS");
MODULE_LICENSE("GPL");

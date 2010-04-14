/********************************************************************************
* File Name     : drivers/mmc/card/socle_sdhc_slave.c
* Author        : CY Li
* Description   : Socle SDHC Slave Driver
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
*      1. 2009/04/17 CY Li create this file
*
********************************************************************************/

#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>

#include <mach/platform.h>
#include <mach/regs-sdhc_slave.h>
#include <mach/sdhc_slave.h>


//#define CONFIG_SDHC_SLAVE_DEBUG
#ifdef CONFIG_SDHC_SLAVE_DEBUG
	#define DBG(fmt, args...) printk("SDHC Slave-> %s(): " fmt, __FUNCTION__, ## args)
#else
	#define DBG(fmt, args...)
#endif

//#define USE_EM_SRAM

#define MAX_BLK_SIZE		512
#define BLK_NUM_OF_BUF		8
#define MAX_BUF_SIZE		(MAX_BLK_SIZE * BLK_NUM_OF_BUF)

static struct sdhc_slave_st {
	u32 base;
	u32 irq;

	spinlock_t lock;

	struct sdhc_slave_interrupt_flag {
		u8 wakeup : 1;
		u8 reset : 1;
		u8 dma_int : 1;
		u8 finish : 1;
		u8 terminate : 1;
	} flag;

	u8 buf_idx;
	void *buf_va[3], *buf_pa[3];		// 0 & 1: data buffer, 2: erase buffer

	enum sdhc_slave_state {
		SLV_STATE_IDLE = 0,
		SLV_STATE_RESET,
		SLV_STATE_READ,
		SLV_STATE_WRITE,
		SLV_STATE_ERASE,
	} cur_state, nxt_state;

	/* add for OS level usage -- begin */
	char *file;
	loff_t size;
	struct completion thread_notifier;
	struct task_struct *thread_task;
	struct file	*filp;
	struct resource *res;
	/* add for OS level usage -- end */
} sdhc_slave;


/* add for OS level usage -- begin */

static char __initdata banner[] = "SQ SDHC Slave Driver, (c) 2009 SQ Technology Corp.\n";

module_param_named(file, sdhc_slave.file, charp, S_IRUGO);
MODULE_PARM_DESC(file, "name of backing file or device");


static int
open_backing_file(struct sdhc_slave_st *slave, const char *filename)
{
	int ro = 0;
	struct file *filp = NULL;
	int rc = -EINVAL;
	struct inode *inode = NULL;
	loff_t size;
	loff_t num_sectors;

	filp = filp_open(filename, O_RDWR | O_LARGEFILE, 0);
	if (-EROFS == PTR_ERR(filp))
		ro = 1;

	if (IS_ERR(filp)) {
		printk("unable to open backing file: %s\n", filename);
		return PTR_ERR(filp);
	}

	if (!(filp->f_mode & FMODE_WRITE))
		ro = 1;

	if (filp->f_path.dentry)
		inode = filp->f_path.dentry->d_inode;
	if (inode && S_ISBLK(inode->i_mode)) {
		if (bdev_read_only(inode->i_bdev))
			ro = 1;
	} else if (!inode || !S_ISREG(inode->i_mode)) {
		printk("invalid file type: %s\n", filename);
		goto out;
	}

	if (!filp->f_op || !(filp->f_op->read || filp->f_op->aio_read)) {
		printk("file not readable: %s\n", filename);
		goto out;
	}
	if (!(filp->f_op->write || filp->f_op->aio_write))
		ro = 1;

	size = i_size_read(inode->i_mapping->host);
	if (size < 0) {
		printk("unable to find file size: %s\n", filename);
		rc = (int) size;
		goto out;
	}
	num_sectors = size >> 9;	// File size in 512-byte sectors
	if (num_sectors == 0) {
		printk("file too small: %s\n", filename);
		rc = -ETOOSMALL;
		goto out;
	}

	if (ro) {
		printk("Not support Read-only file: %s\n", filename);
		rc = -EROFS;
		goto out;
	}

	get_file(filp);
	slave->filp = filp;
	slave->size = size;
	DBG("open backing file: %s\n", filename);
	rc = 0;

out:
	filp_close(filp, current->files);
	return rc;
}

static inline void
close_backing_file(struct sdhc_slave_st *slave)
{
	if (slave->filp) {
		DBG("close backing file\n");
		fput(slave->filp);
		slave->filp = NULL;
	}
}

static inline int
fsync(struct file *filp)
{
	struct inode *inode;
	int rc, err;

	if (!filp)
		return 0;
	if (!filp->f_op->fsync)
		return -EINVAL;

	inode = filp->f_path.dentry->d_inode;
	mutex_lock(&inode->i_mutex);
	rc = filemap_fdatawrite(inode->i_mapping);
	err = filp->f_op->fsync(filp, filp->f_path.dentry, 1);
	if (!rc)
		rc = err;
	err = filemap_fdatawait(inode->i_mapping);
	if (!rc)
		rc = err;
	mutex_unlock(&inode->i_mutex);
	DBG("fdatasync -> %d\n", rc);
	return rc;
}

static inline void
wakeup_thread(struct sdhc_slave_st *slave)
{
	slave->flag.wakeup = 1;

	if (slave->thread_task)
		wake_up_process(slave->thread_task);
}

static inline int
sleep_thread(struct sdhc_slave_st *slave)
{
	int	rc = 0;

	while (1) {
		try_to_freeze();
		set_current_state(TASK_INTERRUPTIBLE);
		if (signal_pending(current)) {
			rc = -EINTR;
			break;
		}
		if (slave->flag.wakeup)
			break;
		schedule();
	}
	__set_current_state(TASK_RUNNING);

	spin_lock_irq(&slave->lock);
	slave->flag.wakeup = 0;
	spin_unlock_irq(&slave->lock);

	return rc;
}

/* add for OS level usage -- end */


static inline u32
socle_sdhc_slave_read(u32 reg, u32 base)
{
	u32 val = ioread32(base + reg);
//	DBG("reg = 0x%08x, val = 0x%08x, base = 0x%08x\n", reg, val, base);
	return val;
}

static inline void
socle_sdhc_slave_write(u32 reg, u32 val, u32 base)
{
//	DBG("reg = 0x%08x, val = 0x%08x, base = 0x%08x\n", reg, val, base);
	iowrite32(val, base + reg);
}

static irqreturn_t
sdhc_slave_isr(int irq, void *dev_id)
{
	struct sdhc_slave_st *slave = (struct sdhc_slave_st *) dev_id;
	u32 base = slave->base, status;

	spin_lock(&slave->lock);

	status = socle_sdhc_slave_read(SOCLE_SDHC_SLV_INT_STA, base);
	DBG("status = 0x%08x\n", status);
	//mdelay(1);

	if (status & INT_RES) {
		// clear interrupt
		socle_sdhc_slave_write(
			SOCLE_SDHC_SLV_INT_CLR,
			INT_RES,
			base);

		slave->flag.reset = 1;
		slave->nxt_state = SLV_STATE_RESET;
		wakeup_thread(slave);
		DBG("INT_RES: cur_state = %d, nxt_state = %d\n", slave->cur_state, slave->nxt_state);
		goto out;
	}

	if (status & INT_CSD_PRG) {
		// clear interrupt
		socle_sdhc_slave_write(
			SOCLE_SDHC_SLV_INT_CLR,
			INT_CSD_PRG,
			base);

		printk("\nNew CSD:\n"
			"CSD0: 0x%08x\n"
			"CSD1: 0x%08x\n"
			"CSD2: 0x%08x\n"
			"CSD3: 0x%08x\n",
			socle_sdhc_slave_read(SOCLE_SDHC_SLV_CSD0, base),
			socle_sdhc_slave_read(SOCLE_SDHC_SLV_CSD1, base),
			socle_sdhc_slave_read(SOCLE_SDHC_SLV_CSD2, base),
			socle_sdhc_slave_read(SOCLE_SDHC_SLV_CSD3, base));

		DBG("INT_CSD_PRG: cur_state = %d, nxt_state = %d\n", slave->cur_state, slave->nxt_state);
	}

	if (status & INT_ERA) {
		// clear interrupt
		socle_sdhc_slave_write(
			SOCLE_SDHC_SLV_INT_CLR,
			INT_ERA,
			base);

		slave->nxt_state = SLV_STATE_ERASE;
		wakeup_thread(slave);
		DBG("INT_ERA: cur_state = %d, nxt_state = %d\n", slave->cur_state, slave->nxt_state);
		goto out;
	}

	if (status & INT_MEM_RD_OV) {
		// clear interrupt
		socle_sdhc_slave_write(
			SOCLE_SDHC_SLV_INT_CLR,
			INT_MEM_RD_OV,
			base);

		slave->flag.finish = 1;
		slave->nxt_state = SLV_STATE_IDLE;
		wakeup_thread(slave);
		DBG("INT_MEM_RD_OV: cur_state = %d, nxt_state = %d, cnt = %d\n",
			slave->cur_state, slave->nxt_state, socle_sdhc_slave_read(SOCLE_SDHC_SLV_MEM_BLK_CNT_REG, base));
	}

	if (status & INT_MEM_WR_OV) {
		// clear interrupt
		socle_sdhc_slave_write(
			SOCLE_SDHC_SLV_INT_CLR,
			INT_MEM_WR_OV,
			base);

		slave->flag.finish = 1;
		slave->nxt_state = SLV_STATE_IDLE;
		wakeup_thread(slave);
		DBG("INT_MEM_WR_OV: cur_state = %d, nxt_state = %d, cnt = %d\n",
			slave->cur_state, slave->nxt_state, socle_sdhc_slave_read(SOCLE_SDHC_SLV_MEM_BLK_CNT_REG, base));
	}

	if (status & INT_DMA_INT) {
		// clear interrupt
		socle_sdhc_slave_write(
			SOCLE_SDHC_SLV_INT_CLR,
			INT_DMA_INT,
			base);

		slave->flag.dma_int = 1;
		slave->nxt_state = SLV_STATE_IDLE;
		wakeup_thread(slave);
		DBG("INT_DMA_INT: cur_state = %d, nxt_state = %d\n", slave->cur_state, slave->nxt_state);
	}

	if (status & INT_MEM_RD) {
		// clear interrupt
		socle_sdhc_slave_write(
			SOCLE_SDHC_SLV_INT_CLR,
			INT_MEM_RD,
			base);

		slave->nxt_state = SLV_STATE_READ;
		wakeup_thread(slave);
		DBG("INT_MEM_RD: cur_state = %d, nxt_state = %d\n", slave->cur_state, slave->nxt_state);
	}

	if (status & INT_MEM_WR) {
		// clear interrupt
		socle_sdhc_slave_write(
			SOCLE_SDHC_SLV_INT_CLR,
			INT_MEM_WR,
			base);

		slave->nxt_state = SLV_STATE_WRITE;
		wakeup_thread(slave);
		DBG("INT_MEM_WR: cur_state = %d, nxt_state = %d\n", slave->cur_state, slave->nxt_state);
	}

out:
	spin_unlock(&slave->lock);
	return IRQ_HANDLED;
}

static inline u32
socle_sdhc_slave_start_dma(struct sdhc_slave_st *slave)
{
	u32 ret = 0, base = slave->base;

	spin_lock_irq(&slave->lock);

	if (slave->flag.finish || slave->flag.reset)
		goto out;

	DBG("buf_addr = 0x%08x, buf_idx = %d\n", (u32) slave->buf_pa[slave->buf_idx], slave->buf_idx);
	// set DMA system address
	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_DMA_SYS_ADDR,
		(u32) slave->buf_pa[slave->buf_idx],
		base);
	slave->buf_idx = !slave->buf_idx;

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_DMA_BUF_SZ,
		DMA_BUF_SZ_4KB |
		DMA_BUF_SZ_SYS_ADDR_VLD,
		base);
	ret = 1;

out:
	spin_unlock_irq(&slave->lock);
	return ret;
}

static inline void
socle_sdhc_slave_reset_reg(u32 base)
{
	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_ESW_CCCR,
		ESW_CCCR_SD_REV(0x2) |
//		ESW_CCCR_SHS(0x1) |
		ESW_CCCR_MEM_PRE(0x1),
		base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_MEM_OCR,
		MEM_OCR_MEM_OCR(0xff8000) |
		MEM_OCR_CCS(0x1),
		base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_AHB_BST_SZ_REG,
		AHB_BST_SZ_REG_BST_SZ(AHB_BST_SZ_REG_BST_SZ_16),
		base);
}

static int
socle_sdhc_slave_main(void *data)
{
	struct sdhc_slave_st *slave = (struct sdhc_slave_st *) data;
	u32 arg, rw_num, total_num, base = slave->base;
	ssize_t rw_size;
	loff_t pos;
	struct file *filp = slave->filp;

//	filp->f_flags |= O_SYNC;

	while (1) {

		switch (slave->cur_state) {

		case SLV_STATE_IDLE:
			DBG("SLV_STATE_IDLE\n");

			sleep_thread(slave);

			break;

		case SLV_STATE_RESET:
reset:
			DBG("SLV_STATE_RESET\n");

			spin_lock_irq(&slave->lock);

			slave->flag.reset = 0;
			slave->flag.dma_int = 0;
			slave->flag.finish = 0;

			if (slave->cur_state == slave->nxt_state)
				slave->nxt_state = SLV_STATE_IDLE;

			socle_sdhc_slave_reset_reg(base);

			// set card ready
			socle_sdhc_slave_write(
				SOCLE_SDHC_SLV_ESW_CRD_RDY,
				ESW_CRD_RDY_CAD_RDY(0x1),
				base);

			spin_unlock_irq(&slave->lock);

			DBG("SLV_STATE_RESET: cur_state = %d, nxt_state = %d\n", slave->cur_state, slave->nxt_state);
			break;

		case SLV_STATE_READ:
			DBG("SLV_STATE_READ\n");
			rw_num = 0;

			// clear the block count register
			socle_sdhc_slave_write(
				SOCLE_SDHC_SLV_MEM_BLK_CNT_REG,
				0,
				base);

			arg = socle_sdhc_slave_read(SOCLE_SDHC_SLV_ARG, base);
			DBG("SLV_STATE_READ: arg = 0x%08x\n", arg);

			pos = ((loff_t) arg) << 9;
			rw_size = MAX_BUF_SIZE;

			if (rw_size) {
				rw_size -= vfs_read(filp,
								(char __user *) slave->buf_va[slave->buf_idx],
								rw_size, &pos);
				if (rw_size)
					printk("r 1 size = 0x%08x, pos = 0x%08llx\n", rw_size, pos);
			}

			rw_num += BLK_NUM_OF_BUF;

			socle_sdhc_slave_start_dma(slave);

			while (!slave->flag.finish) {
				sleep_thread(slave);

				if (slave->flag.dma_int) {
					slave->flag.dma_int = 0;

					pos = ((loff_t) (arg + rw_num)) << 9;
					rw_size = MAX_BUF_SIZE;

					if (rw_size) {
						rw_size -= vfs_read(filp,
										(char __user *) slave->buf_va[slave->buf_idx],
										rw_size, &pos);
						if (rw_size)
							printk("r 2 size = 0x%08x, pos = 0x%08llx\n", rw_size, pos);
					}

					rw_num += BLK_NUM_OF_BUF;

					socle_sdhc_slave_start_dma(slave);
				}

				if (slave->flag.reset)
					goto done;
			}
			slave->flag.dma_int = 0;
			slave->flag.finish = 0;

			DBG("SLV_STATE_READ: total_num = %d, prepare num = %d\n",
					socle_sdhc_slave_read(SOCLE_SDHC_SLV_MEM_BLK_CNT_REG, base), rw_num);
			break;

		case SLV_STATE_WRITE:
			DBG("SLV_STATE_WRITE\n");
			rw_num = 0;

			// clear the block count register
			socle_sdhc_slave_write(
				SOCLE_SDHC_SLV_MEM_BLK_CNT_REG,
				0,
				base);

			arg = socle_sdhc_slave_read(SOCLE_SDHC_SLV_ARG, base);
			DBG("SLV_STATE_WRITE: arg = 0x%08x\n", arg);

			socle_sdhc_slave_start_dma(slave);

			while (!slave->flag.finish) {
				sleep_thread(slave);

				if (slave->flag.dma_int) {
					slave->flag.dma_int = 0;

					if (!socle_sdhc_slave_start_dma(slave))
						continue;

					pos = ((loff_t) (arg + rw_num)) << 9;
					rw_size = MAX_BUF_SIZE;

					if (rw_size) {
						rw_size -= vfs_write(filp,
										(char __user *) slave->buf_va[slave->buf_idx],
										rw_size, &pos);
						if (rw_size)
							printk("w 1 size = 0x%08x, pos = 0x%08llx\n", rw_size, pos);
					}

					rw_num += BLK_NUM_OF_BUF;
				}

				if (slave->flag.reset)
					goto done;
			}
			slave->flag.dma_int = 0;
			slave->flag.finish = 0;

			pos = ((loff_t) (arg + rw_num)) << 9;
			total_num = socle_sdhc_slave_read(SOCLE_SDHC_SLV_MEM_BLK_CNT_REG, base);
			rw_size = (total_num - rw_num) << 9;

			if (rw_size) {
				rw_size -= vfs_write(filp,
								(char __user *) slave->buf_va[!slave->buf_idx],
								rw_size, &pos);
				if (rw_size)
					printk("w 2 size = 0x%08x, pos = 0x%08llx\n", rw_size, pos);
			}

			fsync(filp);

			// set number of written blocks
			socle_sdhc_slave_write(
				SOCLE_SDHC_SLV_NUM_OF_WR_BLKS,
				total_num,
				base);

			// set write program done
			socle_sdhc_slave_write(
				SOCLE_SDHC_SLV_SD_MEM_PRG,
				SD_MEM_PRG_MEM_PRG_DN,
				base);

			DBG("SLV_STATE_WRITE: total_num = %d\n", total_num);
			break;

		case SLV_STATE_ERASE: {
			u32 i, era_str_addr = socle_sdhc_slave_read(SOCLE_SDHC_SLV_ERA_STR_ADDR, base),
				era_end_addr = socle_sdhc_slave_read(SOCLE_SDHC_SLV_ERA_END_ADDR, base);
			DBG("SLV_STATE_ERASE\n");

			DBG("SLV_STATE_ERASE: era_str_addr(0x%08x), era_end_addr(0x%08x)\n", era_str_addr, era_end_addr);

			for (i = era_str_addr; i <= era_end_addr; i += BLK_NUM_OF_BUF) {
				if (slave->flag.reset)
					goto done;
				pos = ((loff_t) i) << 9;
				rw_size = min((u32) BLK_NUM_OF_BUF, (u32) era_end_addr - i + 1) << 9;
				if (rw_size) {
					rw_size -= vfs_write(filp,
									(char __user *) slave->buf_va[2],
									rw_size, &pos);
					if (rw_size)
						printk("erease size = 0x%08x, pos = 0x%016llx\n", rw_size, pos);
				}
			}

			fsync(filp);

			spin_lock_irq(&slave->lock);

			if (slave->cur_state == slave->nxt_state)
				slave->nxt_state = SLV_STATE_IDLE;

			// set write program done
			socle_sdhc_slave_write(
				SOCLE_SDHC_SLV_SD_MEM_PRG,
				SD_MEM_PRG_MEM_PRG_DN,
				base);

			spin_unlock_irq(&slave->lock);

			DBG("SLV_STATE_ERASE: cur_state = %d, nxt_state = %d\n", slave->cur_state, slave->nxt_state);
			break;
		}

		default:
			printk("SLV_STATE BUG: Error cur_state = %d!\n", slave->cur_state);
			BUG();
		}

done:
		DBG("done: cur_state = %d, nxt_state = %d\n", slave->cur_state, slave->nxt_state);
		slave->cur_state = slave->nxt_state;

		if (slave->flag.terminate)
			break;
		if (slave->flag.reset)
			goto reset;
	}

	DBG("kthread complete_and_exit\n");

	// set card not ready
	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_ESW_CRD_RDY,
		ESW_CRD_RDY_CAD_RDY(0x0),
		base);

	slave->thread_task = NULL;
	complete_and_exit(&slave->thread_notifier, 0);

	return 0;
}

static void
socle_sdhc_slave_info(u32 base)
{
	printk("\nOCR:\n"
		"OCR: 0x%08x\n",
		socle_sdhc_slave_read(SOCLE_SDHC_SLV_MEM_OCR, base));

	printk("\nCID:\n"
		"CID0: 0x%08x\n"
		"CID1: 0x%08x\n"
		"CID2: 0x%08x\n"
		"CID3: 0x%08x\n",
		socle_sdhc_slave_read(SOCLE_SDHC_SLV_CID0, base),
		socle_sdhc_slave_read(SOCLE_SDHC_SLV_CID1, base),
		socle_sdhc_slave_read(SOCLE_SDHC_SLV_CID2, base),
		socle_sdhc_slave_read(SOCLE_SDHC_SLV_CID3, base));

	printk("\nCSD:\n"
		"CSD0: 0x%08x\n"
		"CSD1: 0x%08x\n"
		"CSD2: 0x%08x\n"
		"CSD3: 0x%08x\n",
		socle_sdhc_slave_read(SOCLE_SDHC_SLV_CSD0, base),
		socle_sdhc_slave_read(SOCLE_SDHC_SLV_CSD1, base),
		socle_sdhc_slave_read(SOCLE_SDHC_SLV_CSD2, base),
		socle_sdhc_slave_read(SOCLE_SDHC_SLV_CSD3, base));

	printk("\nSCR:\n"
		"SCR0: 0x%08x\n"
		"SCR1: 0x%08x\n",
		socle_sdhc_slave_read(SOCLE_SDHC_SLV_SCR0, base),
		socle_sdhc_slave_read(SOCLE_SDHC_SLV_SCR1, base));

	printk("\n");
}

static void
socle_sdhc_slave_initial(struct sdhc_slave_st *slave)
{
	u32 base = slave->base;
	u8 oid[] = CID_OID, pnm[] = CID_PNM;

	DBG("\n");

	if (DATA_STAT_AFTER_ERASE)
		memset((char *) slave->buf_va[2], 0xff, MAX_BUF_SIZE);
	else
		memset((char *) slave->buf_va[2], 0x00, MAX_BUF_SIZE);

	socle_sdhc_slave_reset_reg(base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_CID0,
		CID0_AWS_1 |
		CID0_CRC(0x0) |
		CID0_MDT(CID_MDT_Y << 4 | CID_MDT_M) |
		CID0_PSN_L(CID_PSN & 0xff),
		base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_CID1,
		CID1_PSN_H(CID_PSN >> 8) |
		CID1_PRV(CID_PRV_N << 4 | CID_PRV_M),
		base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_CID2,
		CID2_PNM_L(pnm[1] << 24 | pnm[2] << 16 | pnm[3] << 8 | pnm[4]),
		base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_CID3,
		CID3_PNM_H(pnm[0]) |
		CID3_OID(oid[0] << 8 | oid[1]) |
		CID3_MID(CID_MID),
		base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_CSD0,
		CSD0_AWS_1 |
		CSD0_CRC(0x0) |
		CSD0_FILE_FORMAT(0x0) |
		CSD0_TMP_WRITE_PROTECT(0x0) |
		CSD0_PERM_WRITE_PROTECT(0x0) |
		CSD0_COPY(0x0) |
		CSD0_FILE_FORMAT_GRP(0x0) |
		CSD0_WRITE_BL_PARTIAL(0x0) |
		CSD0_WRITE_BL_LEN(0x9) |
		CSD0_R2W_FACTOR(0x2) |
		CSD0_WP_GRP_ENABLE(0x0),
		base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_CSD1,
		CSD1_WP_GRP_SIZE(0x00) |
		CSD1_SECTOR_SIZE(0x7f) |
		CSD1_ERASE_BLK_EN(0x1) |
		CSD1_C_SIZE_L((slave->size >> 19) - 2),	// Min = 512KB
		base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_CSD2,
		CSD2_C_SIZE_H(0x00) |		// should be 0, Max = 32GB
		CSD2_DSR_IMP(0x0) |
		CSD2_READ_BLK_MISALIGN(0x0) |
		CSD2_WRITE_BLK_MISALIGN(0x0) |
		CSD2_READ_BL_PARTIAL(0x0) |
		CSD2_READ_BL_LEN(0x9) |
		CSD2_CCC(0x1b5),			// support class 0, 2, 4, 5, 7, 8
		base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_CSD3,
		CSD3_TRAN_SPEED(0x32) |
		CSD3_NSAC(0x00) |
		CSD3_TAAC(0x0e) |
		CSD3_CSD_STRUCTURE(0x1),
		base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_SCR0,
		SCR0_RSV_4_MFT_USG(0x00000000),
		base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_SCR1,
		SCR1_SD_BUS_WIDTHS(0x5) |
		SCR1_SD_SECURITY(0x0) |
		SCR1_DATA_STAT_AFTER_ERASE(DATA_STAT_AFTER_ERASE) |
		SCR1_SD_SPEC(0x2) |
		SCR1_SCR_STRUCTURE(0x0),
		base);

	/* Setup Inetrrupt*/
#if 0
	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_INT_EN,
		INT_RES |
		INT_CSD_PRG |
		INT_ERA |
		INT_MEM_RD |
		INT_MEM_WR |
		INT_MEM_WR_OV |
		INT_MEM_RD_OV |
		INT_DMA_INT,
		base);
#else
	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_INT_EN,
//		0x00001fa0,
		0x1fffffff,
		base);
#endif

	socle_sdhc_slave_info(base);

	socle_sdhc_slave_write(
		SOCLE_SDHC_SLV_ESW_CRD_RDY,
		ESW_CRD_RDY_CAD_RDY(0x1),
		base);
}

static int __devinit
sdhc_slave_probe(struct platform_device *pdev)
{
	struct sdhc_slave_st *slave = &sdhc_slave;
	int err;
	struct resource *res;
	char *file = slave->file;

	if (!file)
		file = "/dev/ram0";
	memset((char *) slave, 0x00, sizeof(*slave));
	slave->file = file;

	if ((err = open_backing_file(slave, file)))
		return err;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (NULL == res) {
		printk("sdhc_slave_probe(): cannot get IORESOURCE_MEM\n");
		err = -ENXIO;
		goto err_no_io_res;
	}

	res = request_mem_region(res->start, (res->end - res->start) + 1, pdev->name);
	if (NULL == res) {
		printk("sdhc_slave_probe(): cannot reserve region\n");
		err = -ENXIO;
		goto err_no_io_res;
	}
	slave->base = IO_ADDRESS(res->start);

	slave->irq = platform_get_irq(pdev, 0);
	if (slave->irq < 0) {
		printk("sdhc_slave_probe(): no irq specified\n");
		err = -ENXIO;
		goto err_no_irq;
	}

	slave->buf_va[0] = dma_alloc_coherent(NULL, MAX_BUF_SIZE * 3,
									(dma_addr_t *) &slave->buf_pa[0], GFP_KERNEL);
	DBG("Allocated address: pa = 0x%08x, va = 0x%08x\n", (u32) slave->buf_pa[0], (u32) slave->buf_va[0]);

	if (!slave->buf_va[0]) {
		printk("sdhc_slave_probe(): cannot allocate buffer\n");
		goto err_no_buffer;
	}

	slave->buf_pa[1] = (void *) (slave->buf_pa[0] + MAX_BUF_SIZE);
	slave->buf_pa[2] = (void *) (slave->buf_pa[1] + MAX_BUF_SIZE);
	
	slave->buf_va[1] = (void *) (slave->buf_va[0] + MAX_BUF_SIZE);
	slave->buf_va[2] = (void *) (slave->buf_va[1] + MAX_BUF_SIZE);

	DBG("Buffer address:\n"
		"slave->buf_pa[0] = 0x%08x, for data buffer 0\n"
		"slave->buf_pa[1] = 0x%08x, for data buffer 1\n"
		"slave->buf_pa[2] = 0x%08x, for erase buffer\n",
		(u32) slave->buf_pa[0], (u32) slave->buf_pa[1], (u32) slave->buf_pa[2]);
	DBG("Buffer address:\n"
		"slave->buf_va[0] = 0x%08x, for data buffer 0\n"
		"slave->buf_va[1] = 0x%08x, for data buffer 1\n"
		"slave->buf_va[2] = 0x%08x, for erase buffer\n",
		(u32) slave->buf_va[0], (u32) slave->buf_va[1], (u32) slave->buf_va[2]);
	DBG("base = 0x%08x, irq = %d\n", slave->base, slave->irq);

	err = request_irq(slave->irq, sdhc_slave_isr, IRQF_DISABLED, pdev->name, slave);
	if (err) {
		printk("sdhc_slave_probe(): cannot claim IRQ\n");
		goto err_no_request_irq;
	}

	slave->thread_task = kthread_create(socle_sdhc_slave_main, (void *) slave, "sdhc-slave-kthread");
	if (IS_ERR(slave->thread_task)) {
		printk("sdhc_slave_probe(): cannot create kthread\n");
		err = PTR_ERR(slave->thread_task);
		goto err_no_kthread;
	}
	DBG("kthread pid: %d\n", slave->thread_task->pid);

	slave->res = res;
	platform_set_drvdata(pdev, slave);
	spin_lock_init(&slave->lock);
	init_completion(&slave->thread_notifier);

	wake_up_process(slave->thread_task);

	socle_sdhc_slave_initial(slave);

	return 0;

err_no_kthread:
	free_irq(slave->irq, slave);
err_no_request_irq:
	dma_free_coherent(NULL, MAX_BUF_SIZE * 3,
			slave->buf_va[0], (dma_addr_t) slave->buf_pa[0]);
err_no_irq:
err_no_buffer:
	release_resource(res);
err_no_io_res:
	return err;
}

static int __devexit
sdhc_slave_remove(struct platform_device *pdev)
{
	struct sdhc_slave_st *slave = platform_get_drvdata(pdev);

	DBG("\n");

	spin_lock_irq(&slave->lock);
	slave->flag.terminate = 1;
	wakeup_thread(slave);
	spin_unlock_irq(&slave->lock);

	/* Wait for the thread to finish up */
	wait_for_completion(&slave->thread_notifier);

	free_irq(slave->irq, slave);
	release_resource(slave->res);

	DBG("Free allocated address: pa = 0x%08x, va = 0x%08x\n", (u32) slave->buf_pa[0], (u32) slave->buf_va[0]);
	dma_free_coherent(NULL, MAX_BUF_SIZE * 3,
			slave->buf_va[0], (dma_addr_t) slave->buf_pa[0]);

	close_backing_file(slave);

	return 0;
}

static struct platform_driver sdhc_slave_device_driver = {
	.probe		= sdhc_slave_probe,
	.remove		= __devexit_p(sdhc_slave_remove),
	.driver		= {
		.name	= "sdhc-slave",
	},
};

static int __init
socle_sdhc_slave_init(void)
{
	printk(banner);
	return platform_driver_register(&sdhc_slave_device_driver);
}



static void __exit
socle_sdhc_slave_exit(void)
{
	platform_driver_unregister(&sdhc_slave_device_driver);
}

module_init(socle_sdhc_slave_init);
module_exit(socle_sdhc_slave_exit);

MODULE_DESCRIPTION("SQ SDHC Slave Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("CY Li <cyli@socle-tech.com.tw>");


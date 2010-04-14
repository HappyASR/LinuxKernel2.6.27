/********************************************************************************
* File Name     : linux/drivers/mmc/host/socle-sdhc.c 
* Author         : Ryan Chen
* Description   : Socle SDHC driver
* 
* Copyright (C) SQ Tech. Corp.
* This program is free software; you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by the Free Software Foundation; 
* either version 2 of the License, or (at your option) any later version. 
* This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY; 
*without even the implied warranty of MERCHANTABILITY or 
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. 
* You should have received a copy of the GNU General Public License 
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
    
*   Version      : x,y,a,b
*   History      : 
*      1. 2009/03/08 Ryan Chen create this file 
*    
********************************************************************************/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>

#include <linux/clk.h>

#include <linux/mmc/host.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/mmc.h>

#include <asm/io.h>
#include <asm/irq.h>
//#include <asm/gpio.h>

//#include <asm/mach/mmc.h>
//#include <mach/board.h>
//#include <mach/cpu.h>

#include <mach/socle-sdhc-regs.h>
#include <mach/cheetah-scu.h>

#define DRIVER_NAME "sq_sdhc"

//#define SDHC_DEBUG 

#ifdef SDHC_DEBUG
	#define SOCLE_SDHC_DBG(fmt, args...) printk("SDHC: " fmt, ## args)
#else
	#define SOCLE_SDHC_DBG(fmt, args...)
#endif

/*
 *  Low level type for this driver
 *  */
struct socle_sdhc_host {
	void __iomem *		ioaddr;		/* Mapped address */
	
	struct mmc_host		*mmc;		/* MMC structure */
	struct mmc_request	*mrq;		/* Current request */
	struct mmc_command	*cmd;		/* Current command */
	struct mmc_data		*data;		/* Current data request */

	spinlock_t		lock;		/* Mutex */	

	unsigned int		max_clk;	/* Max possible freq (MHz) */
	unsigned int		timeout_clk;	/* Timeout freq (KHz) */
	unsigned int		clock;		/* Current clock (MHz) */
	unsigned short		power;		/* Current voltage */

//	struct scatterlist	*cur_sg;	/* We're working on this */

	int			offset;		/* Offset into current sg */
//	int			remain;		/* Bytes left in current */

	char			slot_descr[20];	/* Name for reservations */

	int			irq;		/* Device IRQ */

	struct tasklet_struct	card_tasklet;	/* Tasklet structures */
	struct tasklet_struct	finish_tasklet;

	struct timer_list	timer;		/* Timer for timeouts */

	u8			*adma_desc;	/* ADMA descriptor table */
	u8			*align_buffer;	/* Bounce buffer */

	dma_addr_t		adma_addr;	/* Mapped ADMA descr. table */
	dma_addr_t		align_addr;	/* Mapped bounce buffer */

	int			sg_count;	/* Mapped sg entries */	
////////////////////////////////////////////////////////////


	u8 AutoCMD12Enable;
	u8	dma_mode;

#define SDMA_MODE       0x00
#define ADMA1_MODE      0x01
#define ADMA2_MODE      0x02
#define NONEDMA_MODE    0xFF
	
	u32 *buffer;
	u32 buf_idx;
	u32 cur_sg_idx;
	u32 blk_idx;
};


static inline void
socle_sdhc_write(struct socle_sdhc_host *host, u32 val, u32 reg)
{
	iowrite32(val, (u32) host->ioaddr + reg);
}

static inline u32
socle_sdhc_read(struct socle_sdhc_host *host, u32 reg)
{
	u32 val;
	val = ioread32((u32)host->ioaddr + reg);
	return val;
}

static u8 socle_sdhc_dma_prepare(struct socle_sdhc_host *host, struct mmc_data *data);
static void socle_sdhc_reset(struct socle_sdhc_host *host, u32 mask);
static void socle_sdhc_send_command(struct socle_sdhc_host *host, struct mmc_command *cmd);
static u8 socle_sdhc_host_initialize(struct socle_sdhc_host *host);


static char *socle_sdhc_kmap_atomic(struct scatterlist *sg, unsigned long *flags)
{
	local_irq_save(*flags);
	return kmap_atomic(sg_page(sg), KM_BIO_SRC_IRQ) + sg->offset;
}

static void socle_sdhc_kunmap_atomic(void *buffer, unsigned long *flags)
{
	kunmap_atomic(buffer, KM_BIO_SRC_IRQ);
	local_irq_restore(*flags);
}

static int socle_sdhc_adma_table_pre(struct socle_sdhc_host *host,
	struct mmc_data *data)
{
	int direction;

	u8 *desc;
	u8 *align;
	dma_addr_t addr;
	dma_addr_t align_addr;
	int len, offset;

	struct scatterlist *sg;
	int i;
	char *buffer;
	unsigned long flags;
//	printk("SQ_sdhc_adma_table_pre \n");

	/*
	 * The spec does not specify endianness of descriptor table.
	 * We currently guess that it is LE.
	 */

	if (data->flags & MMC_DATA_READ)
		direction = DMA_FROM_DEVICE;
	else
		direction = DMA_TO_DEVICE;
	/*
	 * The ADMA descriptor table is mapped further down as we
	 * need to fill it with data first.
	 */

	host->align_addr = dma_map_single(mmc_dev(host->mmc),
		host->align_buffer, 128 * 4, direction);
	
//	printk("host->align_addr : %x ,  host->align_buffer : %x \n", host->align_addr,host->align_buffer);
	
	if (dma_mapping_error(mmc_dev(host->mmc), host->align_addr))
		goto fail;
	BUG_ON(host->align_addr & 0x3);

	host->sg_count = dma_map_sg(mmc_dev(host->mmc),
		data->sg, data->sg_len, direction);

//	printk("host->align_addr : %x ,  host->align_buffer : %x , host->sg_count : %x , data->sg_len : %x \n", host->align_addr,host->align_buffer, host->sg_count, data->sg_len);
	
	if (host->sg_count == 0)
		goto unmap_align;

	desc = host->adma_desc;
	align = host->align_buffer;

	align_addr = host->align_addr;

	if(host->sg_count != 1)
		printk("ERROR host->sg_count = %d =====\n",host->sg_count );
	
	for_each_sg(data->sg, sg, host->sg_count, i) {
		addr = sg_dma_address(sg);
		len = sg_dma_len(sg);

		/*
		 * The SDHCI specification states that ADMA
		 * addresses must be 32-bit aligned. If they
		 * aren't, then we use a bounce buffer for
		 * the (up to three) bytes that screw up the
		 * alignment.
		 */
		offset = (4 - (addr & 0x3)) & 0x3;
		if(len > 65535)
			printk("len > 65535 ====\n");
//		printk("addr %x , len = %d , offset = %d ========\n",addr,len,offset);

		if (offset) {
			printk("offset ERROR !! \n");
			if (data->flags & MMC_DATA_WRITE) {
				buffer = socle_sdhc_kmap_atomic(sg, &flags);
				WARN_ON(((long)buffer & PAGE_MASK) > (PAGE_SIZE - 3));
				memcpy(align, buffer, offset);
				socle_sdhc_kunmap_atomic(buffer, &flags);
			}

			desc[7] = (align_addr >> 24) & 0xff;
			desc[6] = (align_addr >> 16) & 0xff;
			desc[5] = (align_addr >> 8) & 0xff;
			desc[4] = (align_addr >> 0) & 0xff;

			BUG_ON(offset > 65536);

			desc[3] = (offset >> 8) & 0xff;
			desc[2] = (offset >> 0) & 0xff;

			desc[1] = 0x00;
			desc[0] = 0x21; /* tran, valid */

			align += 4;
			align_addr += 4;

			desc += 8;

			addr += offset;
			len -= offset;
		}

		desc[7] = (addr >> 24) & 0xff;
		desc[6] = (addr >> 16) & 0xff;
		desc[5] = (addr >> 8) & 0xff;
		desc[4] = (addr >> 0) & 0xff;

		BUG_ON(len > 65536);

		desc[3] = (len >> 8) & 0xff;
		desc[2] = (len >> 0) & 0xff;

		desc[1] = 0x00;
		desc[0] = 0x21; /* tran, valid */

		desc += 8;

		/*
		 * If this triggers then we have a calculation bug
		 * somewhere. :/
		 */
		WARN_ON((desc - host->adma_desc) > (128 * 2 + 1) * 4);
	}

	/*
	 * Add a terminating entry.
	 */
	desc[7] = 0;
	desc[6] = 0;
	desc[5] = 0;
	desc[4] = 0;

	desc[3] = 0;
	desc[2] = 0;

	desc[1] = 0x00;
	desc[0] = 0x03; /* nop, end, valid */

//	for(j=0;j<8;j++)
//		printk("desc [%d] : %x \n", j, desc[j]);

	/*
	 * Resync align buffer as we might have changed it.
	 */
	if (data->flags & MMC_DATA_WRITE) {
		dma_sync_single_for_device(mmc_dev(host->mmc),
			host->align_addr, 128 * 4, direction);
	}

	host->adma_addr = dma_map_single(mmc_dev(host->mmc),
		host->adma_desc, (128 * 2 + 1) * 4, DMA_TO_DEVICE);
	if (dma_mapping_error(mmc_dev(host->mmc), host->adma_addr))
		goto unmap_entries;
	BUG_ON(host->adma_addr & 0x3);

//	printk("SQ_sdhc_adma_table_pre end \n");
	return 0;

unmap_entries:
	dma_unmap_sg(mmc_dev(host->mmc), data->sg,
		data->sg_len, direction);
unmap_align:
	dma_unmap_single(mmc_dev(host->mmc), host->align_addr,
		128 * 4, direction);
fail:
	return -EINVAL;

}

static void socle_sdhc_adma_table_post(struct socle_sdhc_host *host,
	struct mmc_data *data)
{
	int direction;

	struct scatterlist *sg;
	int i, size;
	u8 *align;
	char *buffer;
	unsigned long flags;

//	printk("SQ_sdhc_adma_table_post \n");
	
	if (data->flags & MMC_DATA_READ)
		direction = DMA_FROM_DEVICE;
	else
		direction = DMA_TO_DEVICE;

	dma_unmap_single(mmc_dev(host->mmc), host->adma_addr,
		(128 * 2 + 1) * 4, DMA_TO_DEVICE);

	dma_unmap_single(mmc_dev(host->mmc), host->align_addr,
		128 * 4, direction);

	if (data->flags & MMC_DATA_READ) {
		dma_sync_sg_for_cpu(mmc_dev(host->mmc), data->sg,
			data->sg_len, direction);

		align = host->align_buffer;

		for_each_sg(data->sg, sg, host->sg_count, i) {
			if (sg_dma_address(sg) & 0x3) {
				size = 4 - (sg_dma_address(sg) & 0x3);

				buffer = socle_sdhc_kmap_atomic(sg, &flags);
				WARN_ON(((long)buffer & PAGE_MASK) > (PAGE_SIZE - 3));
				memcpy(buffer, align, size);
				socle_sdhc_kunmap_atomic(buffer, &flags);

				align += 4;
			}
		}
	}

	dma_unmap_sg(mmc_dev(host->mmc), data->sg,
		data->sg_len, direction);

//	printk("SQ_sdhc_adma_table_post end \n");

}

static void socle_sdhc_finish_data(struct socle_sdhc_host *host)
{
	struct mmc_data *data;
//	printk("SQ_sdhc_finish_data \n");

	BUG_ON(!host->data);

	data = host->data;
	host->data = NULL;

	socle_sdhc_adma_table_post(host, data);	
	
	/*
	 * The specification states that the block count register must
	 * be updated, but it does not specify at what point in the
	 * data flow. That makes the register entirely useless to read
	 * back so we have to assume that nothing made it to the card
	 * in the event of an error.
	 */
	if (data->error)
		data->bytes_xfered = 0;
	else
		data->bytes_xfered = data->blksz * data->blocks;


	tasklet_schedule(&host->finish_tasklet);

}

/*
 * Send a command
 */
static void socle_sdhc_send_command(struct socle_sdhc_host *host, struct mmc_command *cmd)
{

	struct mmc_data *data;
	u32 	command_information;
	u8 	BusyCheck = 0;
	int no_resp = 0;
	u8 	Status = Status;
	u32 	timeout_cont = 0;

//	WARN_ON(host->cmd);

//	printk("SQ_sdhc_send_command \n");
	host->cmd = cmd;
	cmd->error = 0;


//	printk("cmd->op = %d, cmd->arg= %x \n",cmd->opcode, cmd->arg);
	
	// check response type
	if (!(cmd->flags & MMC_RSP_PRESENT)) {
		no_resp = 1;
		command_information = SFR3_NO_RESPONSE;
 	} else if (cmd->flags & MMC_RSP_136) { //R2
		command_information = SFR3_RESP_LENGTH_136 | SFR3_CRC_CHECK_EN;
	} else if (cmd->flags & MMC_RSP_BUSY) { //R1B , R5B
		command_information = SFR3_RESP_LENGTH_48B | SFR3_CRC_CHECK_EN| SFR3_INDEX_CHECK_EN;
		BusyCheck = 1;
	} else if (cmd->flags & (MMC_RSP_CRC | MMC_RSP_OPCODE)) { //R1,5,6,7
		command_information = SFR3_RESP_LENGTH_48 | SFR3_CRC_CHECK_EN | SFR3_INDEX_CHECK_EN;
	} else { //R3, R4
		command_information = SFR3_RESP_LENGTH_48;
	}
#if 0
	// check if command line is not busy
	timeout_cont = COMMANDS_TIMEOUT;

	while(socle_sdhc_read(host, SOCLE_SDHC_SRS9(0)) & SFR9_CMD_INHIBIT_CMD) {
		timeout_cont--;
		if(timeout_cont ==0) {
			printk("Command line is busy can't execute command !! \n ");
			return;
		}
			
	}

	// check if data line is not busy
	if ( ( cmd->data) || 
		( BusyCheck && ( cmd->opcode != MMC_STOP_TRANSMISSION ) && ( cmd->opcode != SD_IO_RW_DIRECT ) ) ){
		timeout_cont = COMMANDS_TIMEOUT;

		while(socle_sdhc_read(host , SOCLE_SDHC_SRS9(0)) & ( SFR9_CMD_INHIBIT_CMD | SFR9_CMD_INHIBIT_DAT )) {
			timeout_cont--;
			if(timeout_cont ==0) {
				printk("DAT line is busy can't execute command \n");
				return;
			}
		}
	}
#else
	// check if command/DAT line is not busy
	timeout_cont = COMMANDS_TIMEOUT;
        while((socle_sdhc_read(host, SOCLE_SDHC_SRS9(0)) & 0x307) && (!((socle_sdhc_read(host, SOCLE_SDHC_SRS9(0))>>20) & 0x1))) {
                timeout_cont--;
                if(timeout_cont == 0) {
//                        cmd->error = MMC_ERR_INVALID;
                        printk("busy can't execute command !! \n ");
                        return ;
                }

        }
	
#endif
	mod_timer(&host->timer, jiffies + 10 * HZ);
	
	data = cmd->data;

	if(data)
	{
		host->data = data;
//		printk("data->stop : %x \n", data->stop);
		//set time out 
//		printk("data->timeout_ns = %d \n",data->timeout_ns);
//		socle_sdhc_write(host, (socle_sdhc_read(host ,SOCLE_SDHC_SRS11(0)) & SFR11_TIMEOUT_MASK) | SFR11_TIMEOUT_TMCLK_X_2_POWER_27,
//						SOCLE_SDHC_SRS11(0));

		
		if(host->dma_mode!= NONEDMA_MODE) {
//			printk("DATA send with DMA \n");

			if(socle_sdhc_dma_prepare(host, cmd->data)) {
				printk("SQ_sdhc_dma_prepare 3 ERROR !!\n");
				return;
			}

			command_information |= SFR3_DMA_ENABLE;
		} else {
			host->buf_idx =0;
		}

		// set block size and block
//		printk("block count %d, blksz = %d \n", data->blocks, data->blksz);
//		printk("sg len %d, sg_dma_len  = %d , sg_dma_addr : %x, offset : %x , page : %x  \n", data->sg_len, sg_dma_len(data->sg), sg_dma_address(data->sg),data->sg->offset, data->sg->page_link);
		
//		printk("SRS 1 : %x \n ", (data->blocks <<16) |data->blksz | SFR1_DMA_BUFF_SIZE_512KB);
		socle_sdhc_write(host, (data->blocks <<16) |data->blksz | SFR1_DMA_BUFF_SIZE_512KB,
					SOCLE_SDHC_SRS1(0));

		// set data preset bit
		command_information |= SFR3_DATA_PRESENT;

		if ( data->blocks > 1 ) {
			command_information |= SFR3_MULTI_BLOCK_SEL | SFR3_BLOCK_COUNT_ENABLE;
				if ((host->AutoCMD12Enable) && (data->stop)) {
					command_information |= SFR3_AUTOCMD12_ENABLE;   
				}

		}

		if (data->flags & MMC_DATA_READ) {
			command_information |= SFR3_TRANS_DIRECT_READ;
//			printk("DATA R : %x \n", host->data);
		} else {
//			printk("DATA W : %x \n", host->data);
		}

		host->blk_idx=0;
		host->offset = 0;
		host->buf_idx = 0;
	}

	//write argument
//	printk("SRS2 : CMD arg = %x \n",cmd->arg);
	socle_sdhc_write(host, cmd->arg, SOCLE_SDHC_SRS2(0));

	if(cmd->opcode == MMC_STOP_TRANSMISSION)
		command_information |= SDIOHOST_CMD_TYPE_ABORT << 22 ;
	else
		command_information |= SDIOHOST_CMD_TYPE_OTHER << 22 ;

	command_information |= cmd->opcode <<24 ;
	
//	printk("SRS3 : %x \n",command_information);
	// execute command
	socle_sdhc_write(host, command_information, SOCLE_SDHC_SRS3(0));
}

/*
 * Handle a command that has been completed
 */
static void socle_sdhc_completed_command(struct socle_sdhc_host *host)
{
	u32 tmp0,tmp1,tmp2,tmp3;

	BUG_ON(host->cmd == NULL);

//	printk("SQ_sdhc_completed_command \n");			
			
	if (host->cmd->flags & MMC_RSP_PRESENT) {
		if (host->cmd->flags & MMC_RSP_136) {
			/* CRC is stripped so we need to do some shifting. */
			tmp3 = socle_sdhc_read(host, SOCLE_SDHC_SRS4(0));
			tmp2 = socle_sdhc_read(host, SOCLE_SDHC_SRS5(0));
			tmp1 = socle_sdhc_read(host, SOCLE_SDHC_SRS6(0));
			tmp0 = socle_sdhc_read(host, SOCLE_SDHC_SRS7(0));
			
			host->cmd->resp[3] = tmp3 <<8 | ((tmp0 & 0xff000000) >> 24);
			host->cmd->resp[2] = tmp2 <<8 | ((tmp3 & 0xff000000) >> 24);
			host->cmd->resp[1] = tmp1 <<8 | ((tmp2 & 0xff000000) >> 24);
			host->cmd->resp[0] = tmp0 <<8 | ((tmp1 & 0xff000000) >> 24);

//			printk("resp : %x, %x, %x, %x \n",host->cmd->resp[0],host->cmd->resp[1],host->cmd->resp[2],host->cmd->resp[3]);
		} else {
			host->cmd->resp[0] = socle_sdhc_read(host, SOCLE_SDHC_SRS4(0));	
		}
	}

	if (!host->cmd->data) {
		tasklet_schedule(&host->finish_tasklet);
	}
}

/*
 * Handle an MMC request
 */
static void socle_sdhc_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	unsigned long flags;
	struct socle_sdhc_host *host = mmc_priv(mmc);
	host->mrq = mrq;
	
	spin_lock_irqsave(&host->lock, flags);

	if(socle_sdhc_read(host, SOCLE_SDHC_SRS9(0)) & SFR9_CARD_INSERTED) {
		socle_sdhc_send_command(host, mrq->cmd);
	} else {
		host->mrq->cmd->error = -ENOMEDIUM;
		mmc_request_done(host->mmc, mrq);
	}

	spin_unlock_irqrestore(&host->lock, flags);
}

//------------------------------------------------------------------------------------------
static void socle_sdhc_set_power(struct socle_sdhc_host *host, unsigned short power)
{
	u32 pwr;
	

//	printk("SQ_sdhc_set_power \n");
	
	if (host->power == power)
		return;

	if (power == (unsigned short)-1) {
//		printk("set power = 0 \n");
		socle_sdhc_write(host, socle_sdhc_read(host, SOCLE_SDHC_SRS10(0)) & ~SFR10_SD_BUS_POWER, 	SOCLE_SDHC_SRS10(0));
		
		goto out;
	}

	pwr = socle_sdhc_read(host, SOCLE_SDHC_SRS10(0)) & ~SFR10_BUS_VOLTAGE_MASK;


	switch (1 << power) {
	case MMC_VDD_165_195:
		pwr |= SFR10_SET_1_8V_BUS_VOLTAGE | SFR10_SD_BUS_POWER;
		break;
	case MMC_VDD_29_30:
	case MMC_VDD_30_31:
		pwr |= SFR10_SET_3_0V_BUS_VOLTAGE | SFR10_SD_BUS_POWER;
		break;
	case MMC_VDD_32_33:
	case MMC_VDD_33_34:
		pwr |= SFR10_SET_3_3V_BUS_VOLTAGE | SFR10_SD_BUS_POWER;
		break;
	default:
		BUG();
	}

	socle_sdhc_write(host, pwr, SOCLE_SDHC_SRS10(0));
out:
	host->power = power;

}
//------------------------------------------------------------------------------------------
static u8 
socle_sdhc_set_clock(struct socle_sdhc_host *host,u32 clock )
{
	u32 i,sdclk;
	u32 Temp;
	

//	printk("set clock : %d \n", clock);

	if (clock == host->clock)
		return 0;

	// if requested frequency is more than 25MHz then return error
	if ( clock > 50000000 ) {
		printk("clock > 50000000 ERROR \n ");
		return -1;
	}

	// set SD clock off
	socle_sdhc_write(host, socle_sdhc_read(host, SOCLE_SDHC_SRS11(0)) & ~SFR11_SD_CLOCK_ENABLE, SOCLE_SDHC_SRS11(0));

	if (clock == 0)
		goto out;

    	//read base clock frequency for SD clock in kilo herz
	sdclk = socle_get_ahb_clock()/2; //FIXME
//	printk("sdclk = %d \n",sdclk);


	for ( i = 1; i < 512; i = 2 * i ){
//		printk("i = %d \n", i);
		if ( ( sdclk / i ) <= clock ){
			break; 
		}
	}

	if((i == 1) && (sdclk <= clock)){
//		printk("1111 \n");
		i = 0;
	} else {
//		printk("i = %x \n", i);
		i = i>>1;
	}


	// read current value of SFR11 register    
	Temp = socle_sdhc_read(host, SOCLE_SDHC_SRS11(0));

	// clear old frequency base settings 
	Temp &= ~SFR11_SEL_FREQ_BASE_MASK;

	// Set SDCLK Frequency Select and Internal Clock Enable 
//printk("CLK FIXME \n");
//	printk(" Set SDCLK i = %x \n",i);

	Temp |= ( i << 8 ) | SFR11_INT_CLOCK_ENABLE;

	socle_sdhc_write(host, Temp, SOCLE_SDHC_SRS11(0));

	// wait for clock stable is 1
	while(!(socle_sdhc_read(host, SOCLE_SDHC_SRS11(0)) &  SFR11_INT_CLOCK_STABLE));

	// set SD clock on
	socle_sdhc_write(host, socle_sdhc_read(host, SOCLE_SDHC_SRS11(0)) |SFR11_SD_CLOCK_ENABLE , SOCLE_SDHC_SRS11(0));

out:
	host->clock = clock;

	return 0;

}

//------------------------------------------------------------------------------------------

/*
 * Set the IOS
 */
static void socle_sdhc_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	unsigned long flags;	
	struct socle_sdhc_host *host = mmc_priv(mmc);

	SOCLE_SDHC_DBG("sq_sdmmc_set_ios\n");

	spin_lock_irqsave(&host->lock, flags);

	/*
	 * Reset the chip on each power off.
	 * Should clear out any weird states.
	 */
	if (ios->power_mode == MMC_POWER_OFF) {
//		printk("MMC_POWER_OFF \n");
		socle_sdhc_write(host, 0 ,SOCLE_SDHC_SRS14(0));
		socle_sdhc_host_initialize(host);
		host->AutoCMD12Enable = 1;
//		socle_sdhc_init(host);
	}

	socle_sdhc_set_clock(host, ios->clock);

	if (ios->power_mode == MMC_POWER_OFF)
		socle_sdhc_set_power(host, -1);
	else
		socle_sdhc_set_power(host, ios->vdd);

	if (ios->bus_width == MMC_BUS_WIDTH_4) {
		SOCLE_SDHC_DBG("MMC: Setting controller bus width to 4\n");
		socle_sdhc_write(host, socle_sdhc_read(host, SOCLE_SDHC_SRS10(0)) | SFR10_DATA_WIDTH_4BIT, SOCLE_SDHC_SRS10(0));
		// dissable mmc8 mode
	    	socle_sdhc_write(host, socle_sdhc_read(host, SOCLE_SDHC_HRS0) & ~HSFR0_MMC8_MASK , SOCLE_SDHC_HRS0);
	} else if (ios->bus_width == MMC_BUS_WIDTH_1){
		SOCLE_SDHC_DBG("MMC: Setting controller bus width to 1\n");
		socle_sdhc_write(host, socle_sdhc_read(host, SOCLE_SDHC_SRS10(0)) & ~SFR10_DATA_WIDTH_4BIT, SOCLE_SDHC_SRS10(0));
		// dissable mmc8 mode
	    	socle_sdhc_write(host, socle_sdhc_read(host, SOCLE_SDHC_HRS0) & ~HSFR0_MMC8_MASK , SOCLE_SDHC_HRS0);

	} else {
		printk("ERROR socle_sdhc_set_ios !! \n ");
	}

	spin_unlock_irqrestore(&host->lock, flags);


}

static u8 
socle_sdhc_dma_prepare(struct socle_sdhc_host *host, struct mmc_data *data)
{
	u32 ret;
	int sg_cnt;
	
	switch(host->dma_mode) {
		case SDMA_MODE:
			sg_cnt = dma_map_sg(mmc_dev(host->mmc),
					data->sg, data->sg_len,
					(data->flags & MMC_DATA_READ) ?
						DMA_FROM_DEVICE :
						DMA_TO_DEVICE);

			if (sg_cnt == 0) {
				printk("sg_cnt = 0 ---------------- \n");
				/*
				 * This only happens when someone fed
				 * us an invalid request.
				 */
				WARN_ON(1);
				host->dma_mode = NONEDMA_MODE;
			} else {
				WARN_ON(sg_cnt != 1);
//				printk("SDMA addr : %x , LEN : %d \n",sg_dma_address(data->sg), sg_dma_len(data->sg));
				socle_sdhc_write(host, sg_dma_address(data->sg), SOCLE_SDHC_SRS0(0));
				socle_sdhc_write(host, (socle_sdhc_read(host, SOCLE_SDHC_SRS10(0)) & ~SFR10_DMA_SELECT_MASK) 
					|SFR10_DMA_SELECT_SDMA, SOCLE_SDHC_SRS10(0));
			}
		break;

//		case ADMA1_MODE:
		case ADMA2_MODE:        
			// create descriptor table
			ret = socle_sdhc_adma_table_pre(host, data);
			if (ret) {
				/*
				 * This only happens when someone fed
				 * us an invalid request.
				 */
				WARN_ON(1);
//				host->flags &= ~SDHCI_REQ_USE_DMA;
				printk("WWWWWWW \n");
			} else {
//				printk("host->adma_addr = %x \n",host->adma_addr);
				socle_sdhc_write(host, host->adma_addr,SOCLE_SDHC_SRS22(0));
				socle_sdhc_write(host, (socle_sdhc_read(host, SOCLE_SDHC_SRS10(0)) & (~SFR10_DMA_SELECT_MASK))| SFR10_DMA_SELECT_ADMA2
					,SOCLE_SDHC_SRS10(0));
				
			}
			
		break;

		default:
			printk("ERROR !!!! \n");
		return -1;
	}

	return 0;
} 

static u8 socle_sdhc_check_err(struct socle_sdhc_host *host)
{
    u32 SRS12, SRS15;
    u8 Error =0;
    // number of interrupt to clear
    volatile u32 IntToClear = 0;

//	printk("SQ_sdhc_check_err \n");
	WARN_ON(!host->cmd);

	SRS12 = socle_sdhc_read(host, SOCLE_SDHC_SRS12(0));
	SRS15 = socle_sdhc_read(host, SOCLE_SDHC_SRS15(0));

	if(SRS12 & SFR12_AUTO_CMD12_ERROR) {
		printk("SFR12_AUTO_CMD12_ERROR \n");
		if ( SRS15 & SFR15_CMD_NOT_ISSUED_ERR )
			printk("SFR15_CMD_NOT_ISSUED_ERR \n");
		if ( SRS15 & SFR15_AUTO_CMD12_INDEX_ERR )
			printk("SFR15_AUTO_CMD12_INDEX_ERR \n");
		if ( SRS15 & SFR15_AUTO_CMD12_END_BIT_ERR )
			printk("SFR15_AUTO_CMD12_END_BIT_ERR \n");
		if ( SRS15 & SFR15_AUTO_CMD12_CRC_ERR ) {
			printk("SFR15_AUTO_CMD12_CRC_ERR \n");
//			if(SRS15 & SFR15_AUTO_CMD12_TIMEOUT_ERR)
//				printk("SFR15_AUTO_CMD12_TIMEOUT_ERR \n");
//			else
//				printk("!!! SFR15_AUTO_CMD12_TIMEOUT_ERR \n");
		}
		if ( SRS15 & SFR15_AUTO_CMD12_TIMEOUT_ERR ) 
			printk("SFR15_AUTO_CMD12_TIMEOUT_ERR \n");
		if ( SRS15 & SFR15_AUTO_CMD12_NOT_EXECUTED )
			printk("SFR15_AUTO_CMD12_NOT_EXECUTED \n");

		IntToClear = SFR12_AUTO_CMD12_ERROR | SFR12_ERROR_INTERRUPT;
	}

	if(SRS12 & SFR12_CURRENT_LIMIT_ERROR ) {
		printk("SFR12_CURRENT_LIMIT_ERROR \n");
		IntToClear = SFR12_ERROR_INTERRUPT | SFR12_CURRENT_LIMIT_ERROR;
	}
	if ( SRS12 & SFR12_DATA_END_BIT_ERROR ){
		printk("SFR12_DATA_END_BIT_ERROR \n");
		IntToClear = SFR12_ERROR_INTERRUPT | SFR12_DATA_END_BIT_ERROR;
		Error = 1;
		host->cmd->data->error = -EILSEQ;		
	}	
	if ( SRS12 & SFR12_DATA_CRC_ERROR ){
		printk("SFR12_DATA_CRC_ERROR \n");
		IntToClear = SFR12_ERROR_INTERRUPT | SFR12_DATA_CRC_ERROR;
		Error = 1;
		host->cmd->data->error = -EILSEQ;		
	}
	if ( SRS12 & SFR12_DATA_TIMEOUT_ERROR ){
		printk("SFR12_DATA_TIMEOUT_ERROR : CMD : %x \n", host->cmd->opcode);
		IntToClear = SFR12_ERROR_INTERRUPT | SFR12_DATA_TIMEOUT_ERROR;
		Error = 1;
		host->cmd->data->error = -ETIMEDOUT;
	}
	if ( SRS12 & SFR12_COMMAND_INDEX_ERROR ){
		printk("SFR12_COMMAND_INDEX_ERROR \n");
		host->cmd->error = -EILSEQ;
		Error = 1;
		IntToClear = SFR12_ERROR_INTERRUPT | SFR12_COMMAND_INDEX_ERROR;
	}
	if ( SRS12 & SFR12_COMMAND_END_BIT_ERROR ){
		printk("SFR12_COMMAND_END_BIT_ERROR \n");
		Error = 1;
		host->cmd->error = -EILSEQ;
		IntToClear = SFR12_ERROR_INTERRUPT | SFR12_COMMAND_END_BIT_ERROR;
	}
	if ( SRS12 & SFR12_COMMAND_CRC_ERROR ){
		printk("SFR12_COMMAND_CRC_ERROR \n");
		Error =1 ;
		host->cmd->error = -EILSEQ;
		if ( SRS12 & SFR12_COMMAND_TIMEOUT_ERROR ){
			IntToClear = SFR12_ERROR_INTERRUPT | SFR12_COMMAND_CRC_ERROR | SFR12_COMMAND_TIMEOUT_ERROR ;
		}
		else{
			IntToClear = SFR12_ERROR_INTERRUPT | SFR12_COMMAND_CRC_ERROR;
		}
	}
	if ( SRS12 & SFR12_COMMAND_TIMEOUT_ERROR ){
//		printk("SFR12_COMMAND_TIMEOUT_ERROR \n");
		Error = 1;
		IntToClear = SFR12_ERROR_INTERRUPT | SFR12_COMMAND_TIMEOUT_ERROR;
		host->cmd->error = -ETIMEDOUT;
	}

	if ( SRS12 & SFR12_ADMA_ERROR ){
		Error =1;
		printk("SFR12_ADMA_ERROR!! \n");
		IntToClear = SFR12_ERROR_INTERRUPT | SFR12_ADMA_ERROR;
	}

	// clear interrupt
	socle_sdhc_write(host, IntToClear, SOCLE_SDHC_SRS12(0));

//	printk("SQ_sdhc_check_err end \n");
	
	return Error;
}

/*
 * Handle an interrupt
 */
static irqreturn_t socle_sdhc_irq(int irq, void *devid)
{
	struct socle_sdhc_host *host = devid;
	u32	Status;
//	printk("SQ_sdhc_irq \n");
	spin_lock(&host->lock);

	Status = socle_sdhc_read(host, SOCLE_SDHC_SRS12(0));

	// check the source of the interrupt
	if ( Status & SFR12_ERROR_INTERRUPT ){
//		printk("SFR12_ERROR_INTERRUPT \n");
		if(socle_sdhc_check_err(host)) {
			tasklet_schedule(&host->finish_tasklet);
		} else {
			printk("EEEEEERRRRRRR \n");
			if(!host->cmd) {
				goto out;
			} 
		}
	}

	if ( Status & SFR12_COMMAND_COMPLETE ){      
//		printk("SFR12_COMMAND_COMPLETE \n");
		// clear command complete status
		socle_sdhc_write(host, SFR12_COMMAND_COMPLETE, SOCLE_SDHC_SRS12(0));
		socle_sdhc_completed_command(host);
	}

	if ( Status & SFR12_BUFFER_READ_READY ){
		printk("ISR : Buffer read ready interrupt =====\n");
	}

	if ( Status & SFR12_BUFFER_WRITE_READY ) {
		printk("ISR : Buffer write ready interrupt =====\n");
		
	}

	if (Status & SFR12_TRANSFER_COMPLETE) {
//		printk("ISR : Transfer complete interrupt\n");
		// clear transfer complete status
		socle_sdhc_write(host, SFR12_TRANSFER_COMPLETE, SOCLE_SDHC_SRS12(0));

		if(!host->cmd) {
//			printk("No cmd transfer ==============\n");
			goto out;
		}

		if(!host->cmd->data) {
			printk("No data transfer ==============\n");
			goto out;
		}
		
		if(host->cmd->data->error) {
			printk("EEEEEERRRRRROOOOORRR ===== \n");
		}

		socle_sdhc_finish_data(host);

		if ( Status & SFR12_BLOCK_GAP_EVENT ) {
			printk("SFR12_BLOCK_GAP_EVENT ???? \n");
			// continue request
			socle_sdhc_write(host, socle_sdhc_read(host, SOCLE_SDHC_SRS10(0)) |SFR10_CONTINUE_REQUEST, SOCLE_SDHC_SRS10(0));
		}
		
	}

	if ( Status & SFR12_DMA_INTERRUPT ){         
		printk("SFR12_DMA_INTERRUPT \n");
		socle_sdhc_write(host, SFR12_DMA_INTERRUPT, SOCLE_SDHC_SRS12(0));
		printk("SET SDMA again ============\n");
		// set system address register
		socle_sdhc_write(host, sg_dma_address(host->data->sg), SOCLE_SDHC_SRS0(0));		
	}

	if ( Status & SFR12_CARD_INTERRUPT ){      
//		printk("SFR12_CARD_INTERRUPT \n");
		// clear card interrupt interrupt
		socle_sdhc_write(host, SFR12_CARD_INTERRUPT, SOCLE_SDHC_SRS12(0));
		mmc_signal_sdio_irq(host->mmc);
		spin_unlock(&host->lock);
		return IRQ_HANDLED;	
		
	}

	if (Status & SFR12_CARD_REMOVAL) {
		socle_sdhc_write(host, SFR12_CARD_REMOVAL, SOCLE_SDHC_SRS12(0));
		while(socle_sdhc_read(host, SOCLE_SDHC_SRS12(0)) & SFR12_CARD_REMOVAL); 
		tasklet_schedule(&host->card_tasklet);
	}
	
	if(Status & SFR12_CARD_INSERTION) {
		socle_sdhc_write(host, SFR12_CARD_INSERTION,SOCLE_SDHC_SRS12(0));
		while(socle_sdhc_read(host, SOCLE_SDHC_SRS12(0)) & SFR12_CARD_INSERTION);
		tasklet_schedule(&host->card_tasklet);
	}

out:	
	spin_unlock(&host->lock);
	return IRQ_HANDLED;	
}

static int socle_sdhc_get_ro(struct mmc_host *mmc)
{
	struct socle_sdhc_host *host;
	unsigned long flags;
	u32 present;

	host = mmc_priv(mmc);

	spin_lock_irqsave(&host->lock, flags);

	present = socle_sdhc_read(host, SOCLE_SDHC_SRS9(0)) & SFR9_WP_SWITCH_LEVEL;

	spin_unlock_irqrestore(&host->lock, flags);

	return present;


}

//------------------------------------------------------------------------------------------
static u8 SDIOHost_SetTimeout (struct socle_sdhc_host *host, u32 TimeoutVal )
{
	if( !(TimeoutVal & SFR11_TIMEOUT_MASK) )
		return -1;

	socle_sdhc_write(host, (socle_sdhc_read(host, SOCLE_SDHC_SRS11(0)) & SFR11_TIMEOUT_MASK) | TimeoutVal, SOCLE_SDHC_SRS11(0));

	return 0;
}
//------------------------------------------------------------------------------------------


static u8 
socle_sdhc_host_initialize(struct socle_sdhc_host *host)
{
	u8 SlotsAvailability;
	u8 NumberOfSlots;
	u32 i;
	u32 DP;

//   	printk("SDIOHost_HostInitialize : Start host initializing... \n" );


//	socle_sdhc_reset(host, SFR11_SOFT_RESET_ALL)
	// reset controller for sure
	socle_sdhc_write(host, socle_sdhc_read(host, SOCLE_SDHC_HRS0) | HSFR0_SOFTWARE_RESET, SOCLE_SDHC_HRS0);
	// wait for clear 
	while(socle_sdhc_read(host, SOCLE_SDHC_HRS0) & HSFR0_SOFTWARE_RESET);

	//DP * Tclk = 20ms
	DP = socle_get_apb_clock() * 20 /1000; 
//	printk("DP = %x \n", DP);
	socle_sdhc_write(host, DEBOUNCING_TIME, SOCLE_SDHC_HRS1);

	SlotsAvailability = (socle_sdhc_read(host, SOCLE_SDHC_HRS0) & HSFR0_AVAILABLE_SLOT) >> 16;
	NumberOfSlots = 0;
	for (i = 0; i < 4; i++){
		if ((SlotsAvailability >> i) & 1){
			NumberOfSlots++;
		}
	}
//	printk("SDHC : Found = %d slots \n",NumberOfSlots);

        // NumberOfSlots can't be 0
        if ( NumberOfSlots == 0 )
            return -1;

// IRQ ENABLE
	socle_sdhc_write(host, SFR13_AUTO_CMD12_ERR_STAT_EN
            | SFR13_CURRENT_LIMIT_ERR_STAT_EN   
            | SFR13_DATA_END_BIT_ERR_STAT_EN      
            | SFR13_DATA_CRC_ERR_STAT_EN          
            | SFR13_DATA_TIMEOUT_ERR_STAT_EN      
            | SFR13_COMMAND_INDEX_ERR_STAT_EN     
            | SFR13_COMMAND_END_BIT_ERR_STAT_EN   
            | SFR13_COMMAND_CRC_ERR_STAT_EN       
            | SFR13_COMMAND_TIMEOUT_ERR_STAT_EN   
            | SFR13_CARD_REMOVAL_STAT_EN          
            | SFR13_CARD_INERTION_STAT_EN         
            | SFR13_BUFFER_READ_READY_STAT_EN     
            | SFR13_BUFFER_WRITE_READY_STAT_EN    
            | SFR13_DMA_INTERRUPT_STAT_EN         
            | SFR13_BLOCK_GAP_EVENT_STAT_EN       
            | SFR13_TRANSFER_COMPLETE_STAT_EN     
            | SFR13_COMMAND_COMPLETE_STAT_EN
		,SOCLE_SDHC_SRS13(0));

	socle_sdhc_write(host , SFR14_AUTO_CMD12_ERR_SIG_EN
            | SFR14_CURRENT_LIMIT_ERR_SIG_EN
            | SFR14_DATA_END_BIT_ERR_SIG_EN   
            | SFR14_DATA_CRC_ERR_SIG_EN       
            | SFR14_DATA_TIMEOUT_ERR_SIG_EN   
            | SFR14_COMMAND_INDEX_ERR_SIG_EN  
            | SFR14_COMMAND_END_BIT_ERR_SIG_EN
            | SFR14_COMMAND_CRC_ERR_SIG_EN    
            | SFR14_COMMAND_TIMEOUT_ERR_SIG_EN
            | SFR14_CARD_REMOVAL_SIG_EN       
            | SFR14_CARD_INERTION_SIG_EN      
            | SFR14_BUFFER_READ_READY_SIG_EN  
            | SFR14_BUFFER_WRITE_READY_SIG_EN 
            | SFR14_DMA_INTERRUPT_SIG_EN      
            | SFR14_BLOCK_GAP_EVENT_SIG_EN    
            | SFR14_TRANSFER_COMPLETE_SIG_EN  
            | SFR14_COMMAND_COMPLETE_SIG_EN
		,SOCLE_SDHC_SRS14(0));


//FIXME
//	SDIOHost_SetTimeout( SFR11_TIMEOUT_TMCLK_X_2_POWER_21);
	SDIOHost_SetTimeout(host,  SFR11_TIMEOUT_TMCLK_X_2_POWER_27);
	
	return 0;

}

static void socle_sdhc_enable_sdio_irq(struct mmc_host *mmc, int enable)
{
	struct socle_sdhc_host *host;
	unsigned long flags;
	u32 ier,iser;

	host = mmc_priv(mmc);
//	printk("SQ_sdhc_enable_sdio_irq \n");

	spin_lock_irqsave(&host->lock, flags);

	iser = socle_sdhc_read(host, SOCLE_SDHC_SRS13(0)); 
	ier = socle_sdhc_read(host, SOCLE_SDHC_SRS14(0));
	if(ier != iser)
		printk("IER ERROR !!\n");

	ier &= ~SFR13_CARD_INTERRUPT_STAT_EN;
	if (enable) {
//		printk("enable SFR13_CARD_INTERRUPT_STAT_EN \n");
		ier |= SFR13_CARD_INTERRUPT_STAT_EN;
		host->AutoCMD12Enable = 0;
	}

//	mdelay(100); // if panic
	socle_sdhc_write(host, ier, SOCLE_SDHC_SRS13(0));
	socle_sdhc_write(host, ier, SOCLE_SDHC_SRS14(0));
	
	spin_unlock_irqrestore(&host->lock, flags);
	
}

static const struct mmc_host_ops socle_sdhc_ops = {
	.request	= socle_sdhc_request,
	.set_ios	= socle_sdhc_set_ios,
	.get_ro		= socle_sdhc_get_ro,
	.enable_sdio_irq = socle_sdhc_enable_sdio_irq,	
};

/*****************************************************************************\
 *                                                                           *
 * Low level functions                                                       *
 *                                                                           *
\*****************************************************************************/

static void socle_sdhc_reset(struct socle_sdhc_host *host, u32 mask)
{
	socle_sdhc_write(host, mask | (socle_sdhc_read(host, SOCLE_SDHC_SRS11(0)) & ~(SFR11_SOFT_RESET_CMD_LINE | SFR11_SOFT_RESET_DAT_LINE)), SOCLE_SDHC_SRS11(0));
	
	if (mask & SFR11_SOFT_RESET_ALL)
		host->clock = 0;

	/* hw clears the bit when it's done */
	while(socle_sdhc_read(host, SOCLE_SDHC_SRS11(0)) & (SFR11_SOFT_RESET_DAT_LINE | SFR11_SOFT_RESET_CMD_LINE) );

}

/*****************************************************************************\
 *                                                                           *
 * Tasklets                                                                  *
 *                                                                           *
\*****************************************************************************/

static void socle_sdhc_tasklet_card(unsigned long param)
{
	struct socle_sdhc_host *host;
	unsigned long flags;

	host = (struct socle_sdhc_host*)param;

	spin_lock_irqsave(&host->lock, flags);

//	printk("SQ_sdhc_tasklet_card \n");
	
	if(!(socle_sdhc_read(host, SOCLE_SDHC_SRS9(0)) & SFR9_CARD_INSERTED)) {
		if (host->mrq) {
			printk(KERN_ERR "%s: Card removed during transfer!\n",
				mmc_hostname(host->mmc));
			printk(KERN_ERR "%s: Resetting controller.\n",
				mmc_hostname(host->mmc));

			socle_sdhc_reset(host, SFR11_SOFT_RESET_CMD_LINE);
			socle_sdhc_reset(host, SFR11_SOFT_RESET_DAT_LINE);

			host->mrq->cmd->error = -ENOMEDIUM;
			tasklet_schedule(&host->finish_tasklet);
		}
	}

//	printk("SQ_sdhc_tasklet_card end \n");
	
	spin_unlock_irqrestore(&host->lock, flags);

	mmc_detect_change(host->mmc, msecs_to_jiffies(200));

}

static void socle_sdhc_tasklet_finish(unsigned long param)
{
	struct socle_sdhc_host *host;
	unsigned long flags;
	struct mmc_request *mrq;

	host = (struct socle_sdhc_host*)param;


	spin_lock_irqsave(&host->lock, flags);

//	printk("SQ_sdhc_tasklet_finish \n");
	del_timer(&host->timer);

	mrq = host->mrq;

	/*
	 * The controller needs a reset of internal state machines
	 * upon error conditions.
	 */
	if (mrq->cmd->error ||
		(mrq->data && (mrq->data->error ||
		(mrq->data->stop && mrq->data->stop->error)))) {

		/* Some controllers need this kick or reset won't work here */

		/* Spec says we should do both at the same time, but Ricoh
		   controllers do not like that. */
		socle_sdhc_reset(host, SFR11_SOFT_RESET_CMD_LINE);
		socle_sdhc_reset(host, SFR11_SOFT_RESET_DAT_LINE);
	}

//	printk("TF : host->cmd %x , host->data : %x \n",host->cmd,host->data),
	host->mrq = NULL;
	host->cmd = NULL;
	host->data = NULL;
//	printk("TF : host->mrq = NULL, host->cmd = NULL,  host->data = NULL \n");

//	printk("SQ_sdhc_tasklet_finish end \n");
	spin_unlock_irqrestore(&host->lock, flags);

	mmc_request_done(host->mmc, mrq);

}

static void socle_sdhc_timeout_timer(unsigned long data)
{
	struct socle_sdhc_host *host;
	unsigned long flags;

	host = (struct socle_sdhc_host*)data;

	spin_lock_irqsave(&host->lock, flags);
	printk("SQ_sdhc_timeout_timer \n");
	if (host->mrq) {
		printk(KERN_ERR "%s: Timeout waiting for hardware "
			"interrupt.\n", mmc_hostname(host->mmc));

		if (host->data) {
			host->data->error = -ETIMEDOUT;
			printk("FIX ME \n");
			socle_sdhc_finish_data(host);
		} else {
			if (host->cmd)
				host->cmd->error = -ETIMEDOUT;
			else
				host->mrq->cmd->error = -ETIMEDOUT;

			tasklet_schedule(&host->finish_tasklet);
		}
	}
	printk("SQ_sdhc_timeout_timer end \n");
	spin_unlock_irqrestore(&host->lock, flags);

}

/*****************************************************************************\
 *                                                                           *
 * Device allocation/registration                                            *
 *                                                                           *
\*****************************************************************************/

struct socle_sdhc_host *socle_sdhc_alloc_host(struct device *dev)
{
	struct mmc_host *mmc;
	struct socle_sdhc_host *host;

	WARN_ON(dev == NULL);

	mmc = mmc_alloc_host(sizeof(struct socle_sdhc_host), dev);
	if (!mmc)
		return ERR_PTR(-ENOMEM);

	host = mmc_priv(mmc);
	host->mmc = mmc;

	return host;
}

void socle_sdhc_free_host(struct socle_sdhc_host *host)
{
	mmc_free_host(host->mmc);
}

/*
 * Probe for the device
 */
static int __init socle_sdhc_probe(struct platform_device *pdev)
{

	struct socle_sdhc_host *host;
	struct resource *res;
	u32 caps;
	int ret =0;
	int i;

//	printk("SQ_sdhc_probe \n");
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENXIO;

	if (!request_mem_region(res->start, res->end - res->start + 1, DRIVER_NAME))
		return -EBUSY;


	
	
	host = socle_sdhc_alloc_host(&pdev->dev);
	if (IS_ERR(host)) {
		ret = PTR_ERR(host);
		goto fail6;
	}

	host->ioaddr = (void __iomem *) IO_ADDRESS(res->start);
	
	caps = socle_sdhc_read(host, SOCLE_SDHC_SRS16(0));
//	printk("ahb = %ld , apb = %ld,apb_clock = %d \n", socle_get_ahb_clock(),socle_get_apb_clock(),apb_clock);

	host->max_clk = socle_get_ahb_clock(); //FIXME
	
	if (host->max_clk == 0) {
		printk(KERN_ERR "%s: Hardware doesn't specify base clock "
			"frequency.\n", mmc_hostname(host->mmc));
		return -ENODEV;
	}
	
	host->AutoCMD12Enable = 1;

	host->timeout_clk = SFR16_GET_TIMEOUT_CLK_FREQ(caps);

	if (host->timeout_clk == 0) {
		printk(KERN_ERR "%s: Hardware doesn't specify timeout clock "
			"frequency.\n", mmc_hostname(host->mmc));
		return -ENODEV;
	}

	if (caps & SFR16_TIMEOUT_CLOCK_UNIT_MHZ)
		host->timeout_clk *= 1000000;
	else
		host->timeout_clk *= 1000;


//	printk("host->timeout_clk: %d , host->max_clk = %d \n",host->timeout_clk , host->max_clk);

	host->mmc->parent = &pdev->dev;
	host->mmc->ops = &socle_sdhc_ops;

	for(i=7;i>0;i--) {
//		printk("min = %d \n",host->max_clk / (1<<i));
		if((host->max_clk / (1<<i)) > 300000)
			break;
	}

//	printk("divider = %x \n",1<<i);
	host->mmc->f_min = host->max_clk / (1<<i);
	host->mmc->f_max = host->max_clk;

//	printk("host->mmc->f_max = %d ",host->mmc->f_max);

	host->mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_29_30|MMC_VDD_30_31 | MMC_VDD_165_195;

	if (host->mmc->ocr_avail == 0) {
		printk(KERN_ERR "%s: Hardware doesn't report any "
			"support voltages.\n", host->slot_descr);
		ret = -ENODEV;
		goto fail5;
	}

	spin_lock_init(&host->lock);


	host->dma_mode = ADMA2_MODE; // NONEDMA_MODE;SDMA_MODE,ADMA2_MODE
//	printk("host->dma_mode = %x \n ",host->dma_mode);

	/*
	 * Maximum number of sectors in one transfer. Limited by DMA boundary
	 * size (512KiB).
	 */
	if(host->dma_mode == ADMA2_MODE)
		host->mmc->max_req_size = 65535;
	else
		host->mmc->max_req_size = 524288;

	/*
	 * Maximum segment size. Could be one segment with the maximum number
	 * of bytes.
	 */
	host->mmc->max_seg_size = host->mmc->max_req_size;


	host->mmc->max_blk_size = 512;	//FIXME
	host->mmc->max_blk_count = 65536;

//	host = mmc_priv(mmc);
//	host->mmc = mmc;
	host->buffer = NULL;


	
//	host->mmc->caps = MMC_CAP_4_BIT_DATA |MMC_CAP_SD_HIGHSPEED | MMC_CAP_MULTIWRITE | MMC_CAP_SDIO_IRQ;
	host->mmc->caps = MMC_CAP_4_BIT_DATA |MMC_CAP_SD_HIGHSPEED | MMC_CAP_SDIO_IRQ;


	if (host->dma_mode == ADMA2_MODE) {
		/*
		 * We need to allocate descriptors for all sg entries
		 * (128) and potentially one alignment transfer for
		 * each of those entries.
		 */
		host->adma_desc = kmalloc(128 * 4, GFP_KERNEL);
		host->align_buffer = kmalloc(128 * 4, GFP_KERNEL);
		if (!host->adma_desc || !host->align_buffer) {
			kfree(host->adma_desc);
			kfree(host->align_buffer);
			printk(KERN_WARNING "%s: Unable to allocate ADMA "
				"buffers. Falling back to standard DMA.\n",
				mmc_hostname(host->mmc));
			host->dma_mode = SDMA_MODE;
			printk("FORCE SDMA_MODE ********************\n");
		}
	}

	/*
	 * Reset hardware
	 */
	 socle_sdhc_host_initialize(host);

	/*
	 * Init tasklets.
	 */
	tasklet_init(&host->card_tasklet,
		socle_sdhc_tasklet_card, (unsigned long)host);
	tasklet_init(&host->finish_tasklet,
		socle_sdhc_tasklet_finish, (unsigned long)host);

	setup_timer(&host->timer, socle_sdhc_timeout_timer, (unsigned long)host);


	
	/*
	 * Allocate the MCI interrupt
	 */
	host->irq = platform_get_irq(pdev, 0);

	if (host->irq < 0) {
		dev_err(&pdev->dev, "\nSocle SD/MMC host: no irq specified\n");
//		err = -ENOENT;
		goto fail5;
	}

	/* Allocate the interrupt */
	ret = request_irq(host->irq, socle_sdhc_irq, IRQF_DISABLED, pdev->name, host);
	if (ret) {
		dev_dbg(&pdev->dev, "request MCI interrupt failed\n");
//		goto fail0;
	}

	platform_set_drvdata(pdev, host->mmc);

	mmc_add_host(host->mmc);


//	printk("Added MCI driver \n");

	return 0;

fail5:
	mmc_free_host(host->mmc);
fail6:
	release_mem_region(res->start, res->end - res->start + 1);
	dev_err(&pdev->dev, "probe failed, err %d\n", ret);

	return ret;

}

/*
 * Remove a device
 */
static int __exit socle_sdhc_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct socle_sdhc_host *host;
	unsigned long flags;

	if (!mmc) {
		printk("SQ_sdhc_remove ERROR !! \n");
		return -1;
	}

	host = mmc_priv(mmc);

	spin_lock_irqsave(&host->lock, flags);

	if (host->mrq) {
		printk(KERN_ERR "%s: Controller removed during "
			" transfer!\n", mmc_hostname(host->mmc));

		host->mrq->cmd->error = -ENOMEDIUM;
		tasklet_schedule(&host->finish_tasklet);
	}

	spin_unlock_irqrestore(&host->lock, flags);


	mmc_remove_host(host->mmc);

	free_irq(host->irq, host);

	del_timer_sync(&host->timer);

	tasklet_kill(&host->card_tasklet);
	tasklet_kill(&host->finish_tasklet);

	kfree(host->adma_desc);
	kfree(host->align_buffer);

	host->adma_desc = NULL;
	host->align_buffer = NULL;

	socle_sdhc_free_host(host);
	
	return 0;
}

//#ifdef CONFIG_PM
#if 0
static int socle_sdhc_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct socle_sdhc_host *host = mmc_priv(mmc);
	int ret = 0;

	if (device_may_wakeup(&pdev->dev))
		enable_irq_wake(host->board->det_pin);

	if (mmc)
		ret = mmc_suspend_host(mmc, state);

	return ret;
}

static int socle_sdhc_resume(struct platform_device *pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct socle_sdhc_host *host = mmc_priv(mmc);
	int ret = 0;

	if (device_may_wakeup(&pdev->dev))
		disable_irq_wake(host->board->det_pin);

	if (mmc)
		ret = mmc_resume_host(mmc);

	return ret;
}
#else
#define socle_sdhc_suspend	NULL
#define socle_sdhc_resume		NULL
#endif

static struct platform_driver socle_sdhc_driver = {
	.remove		= __exit_p(socle_sdhc_remove),
	.suspend	= socle_sdhc_suspend,
	.resume		= socle_sdhc_resume,
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init socle_sdhc_drv_init(void)
{
	return platform_driver_probe(&socle_sdhc_driver, socle_sdhc_probe);
}

static void __exit socle_sdhc_drv_exit(void)
{
	platform_driver_unregister(&socle_sdhc_driver);
}

module_init(socle_sdhc_drv_init);
module_exit(socle_sdhc_drv_exit);

MODULE_DESCRIPTION("SQ SD Host driver");
MODULE_AUTHOR("Ryan Chen");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:socle_sdhc");

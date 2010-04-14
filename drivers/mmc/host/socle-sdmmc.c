//#define CONFIG_MMC_DEBUG
//#define CONFIG_DEBUG_SPINLOCK_SLEEP
#define USE_ATOMIC_VERSION_KMAP

#ifdef CONFIG_MMC_DEBUG
#define DEBUG
#else
#undef DEBUG
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/blkdev.h>
#include <linux/err.h>
#include <linux/mmc/host.h>
//#include <linux/mmc/protocol.h>
#include <linux/mmc/core.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/scatterlist.h>


#include <asm/io.h>
#include <asm/mach/mmc.h>
#include <linux/byteorder/swab.h>
#include <mach/hardware.h>
#include <mach/regs-sdmmc.h>

#ifndef CONFIG_MMC_SOCLE_SDMMC_AUTO_SWAP
#undef swab32
#define swab32(x) x
#endif

#define SDMMC_SENT_CMD 1
#define SDMMC_SENT_STP (1 << 1)

/* Error indicator */
#define SDMMC_CMD_RESP_ERR 1
#define SDMMC_DATA_TX_ERR (1 << 1)
#define dprintk(args...)

/*
 *  Low level type for this driver
 *  */
struct socle_sdmmc_host {
	struct mmc_host *mmc;
	struct mmc_command *cmd;
	struct mmc_request *request;
	struct resource *io_area;
	int irq;

	/* Flag indicating when the command has been sent.
	 * This is used to work out whether or not to send the stop*/
	unsigned int flags;

	u8 err_flag;

	/* Flag for current bus setting */
	u32 bus_mode;

	u32 *buf;
	u32 buf_idx;
	u32 cur_sg_idx;
	u32 blk_idx;
};

static void socle_sdmmc_send_command(struct socle_sdmmc_host *host, struct mmc_command *cmd);
static void socle_sdmmc_process_next(struct socle_sdmmc_host *host);
static void socle_sdmmc_completed_command(struct socle_sdmmc_host *host);
static void socle_sdmmc_pre_read_data(struct socle_sdmmc_host *host);
static void socle_sdmmc_post_read_data(struct socle_sdmmc_host *host);
static void socle_sdmmc_pre_write_data(struct socle_sdmmc_host *host);
static void socle_sdmmc_post_write_data(struct socle_sdmmc_host *host);
static irqreturn_t socle_sdmmc_isr(int irq, void *_host);

extern u32 apb_clock;
static u32 socle_sdmmc_base;

/*
 *  Read register
 *  */
static inline u32
socle_sdmmc_read(u32 reg)
{
	return ioread32(socle_sdmmc_base+reg);
}

/*
 *  Write to register
 *  */
static inline void
socle_sdmmc_write(u32 reg, u32 value)
{
	iowrite32(value, socle_sdmmc_base+reg);
}

#define SOCLE_SDMMC_DATABUF_1_TX_WIDTH_MASK 0x3
#define SOCLE_SDMMC_DATABUF_2_TX_WIDTH_MASK 0x30
#define SOCLE_SDMMC_DATABUF_SWAP_MASK 0x300
#define SOCLE_SDMMC_MMU_SWAP_MASK 0x3FF

static inline void
socle_sdmmc_mmu_swap(void)
{
	u32 val;
	u32 databuf_swap_val;
	u32 buf_2_tx_width;
	u32 buf_1_tx_width;

	val = socle_sdmmc_read(SOCLE_SDMMC_MMU_CTRL);
	databuf_swap_val = (val & SOCLE_SDMMC_DATABUF_SWAP_MASK) ^ SOCLE_SDMMC_DATABUF_SWAP_MASK;
	buf_1_tx_width = (val & SOCLE_SDMMC_DATABUF_2_TX_WIDTH_MASK) >> 4;
	buf_2_tx_width = (val & SOCLE_SDMMC_DATABUF_1_TX_WIDTH_MASK) << 4;
	val &= ~SOCLE_SDMMC_MMU_SWAP_MASK;
	socle_sdmmc_write(SOCLE_SDMMC_MMU_CTRL,
			  val |
			  databuf_swap_val |
			  SOCLE_SDMMC_DATABUF_2_POINTER_RST |
			  SOCLE_SDMMC_DATABUF_2_POINTER_END_SIGNAL_LOW |
			  buf_2_tx_width |
			  SOCLE_SDMMC_DATABUF_1_POINTER_RST |
			  SOCLE_SDMMC_DATABUF_1_POINTER_END_SIGNAL_LOW |
			  buf_1_tx_width);
}
static inline void
socle_sdmmc_wait_for_card_busy_end(void)
{
	u32 tmp;
  
	do {
		tmp = socle_sdmmc_read(SOCLE_SDMMC_SD_CARDA);
	} while (SOCLE_SDMMC_CARD_BUSY_SIGNAL_HIGH != (tmp & SOCLE_SDMMC_CARD_BUSY_SIGNAL_HIGH));
}

static void
socle_sdmmc_send_command(struct socle_sdmmc_host *host, struct mmc_command *cmd)
{
	struct mmc_data *data;
	u32 resp_type = 0;
	int no_resp = 0;
	u32 blocks = 0, blksz = 0;

	dev_dbg(mmc_dev(host->mmc), "sq_sdmmc_send_command\n");
	host->cmd = cmd;
	switch(mmc_resp_type(cmd)) {
	case MMC_RSP_R6:
		resp_type = SOCLE_SDMMC_RESP_TX_TYPE_R6;
		break;
	}

	// check response type
	if (!(cmd->flags & MMC_RSP_PRESENT)) {
		dprintk("MMC_RSP_NONE \n");
		no_resp = 1;
 	} else if (cmd->flags & MMC_RSP_136) {
 		//R2
 		dprintk("R2  \n");
		resp_type = SOCLE_SDMMC_RESP_TX_TYPE_R2;
	} else if (cmd->flags & MMC_RSP_BUSY) { 
		dprintk("R1B, R5B  \n");
		//R1B , R5B
		resp_type = SOCLE_SDMMC_RESP_TX_TYPE_R1B;
	} else if (cmd->flags & (MMC_RSP_CRC | MMC_RSP_OPCODE)) {
		dprintk("R1, R5,6,7  \n");
		if(cmd->opcode == SD_SEND_RELATIVE_ADDR)
			resp_type = SOCLE_SDMMC_RESP_TX_TYPE_R6;
		else
			resp_type = SOCLE_SDMMC_RESP_TX_TYPE_R1;

	} else {
		dprintk("R3,4  \n");	
		//R3, R4
		resp_type = SOCLE_SDMMC_RESP_TX_TYPE_R3;
	}
	
	data = cmd->data;
	if (data) {
		struct scatterlist *sg;
		unsigned long flags;

		/* Initialize the MMU buffer pointer */
		socle_sdmmc_write(SOCLE_SDMMC_MMU_PNRI, data->blksz-1);
		socle_sdmmc_write(SOCLE_SDMMC_MMU_PNRII, data->blksz-1);

		socle_sdmmc_write(SOCLE_SDMMC_MMU_CTRL, SOCLE_SDMMC_MMU_DATA_WIDTH_WORD);

		host->cur_sg_idx = 0;
		sg = &data->sg[host->cur_sg_idx];
		host->blk_idx = 0;
		host->buf_idx = 0;
//		host->buf = kmap_atomic(sg_page(sg), KM_BIO_SCR_IRQ) + sg->offset;
#ifdef USE_ATOMIC_VERSION_KMAP
		host->buf = kmap_atomic(sg_page(sg), KM_BIO_SRC_IRQ) + sg->offset;
		kunmap_atomic(host->buf, KM_BIO_SRC_IRQ);
#else
		local_irq_save(flags);
		host->buf = kmap(sg_page(sg)) + sg->offset;
		local_irq_restore(flags);
#endif
		dev_dbg(mmc_dev(host->mmc), "first pre read, buffer = %p, offset = %d, length = %d\n", host->buf, sg->offset, sg->length);
		blocks = data->blocks;
		blksz = data->blksz;
		if (data->flags & MMC_DATA_READ)
			socle_sdmmc_pre_read_data(host);
	}

	/* Set the arguments and send the command */
	dev_dbg(mmc_dev(host->mmc), "sending command %d, arg = 0x%08x, blocks = %d, blksz = %d\n", cmd->opcode, cmd->arg, blocks,
		 blksz);

	socle_sdmmc_write(SOCLE_SDMMC_SD_CMD, cmd->arg);
	if (no_resp)
		socle_sdmmc_write(SOCLE_SDMMC_SD_CMDREST,
				  SOCLE_SDMMC_CMD_TX_SIGNAL_BEGIN |
				  SOCLE_SDMMC_RESP_TX_SIGNAL_END |
				  SOCLE_SDMMC_RESP_TX_TYPE_R1 |
				  SOCLE_SDMMC_CMD_RESP_TX_STAT_N_ERR |
				  SOCLE_SDMMC_CMD_INDEX(cmd->opcode));
	else
		socle_sdmmc_write(SOCLE_SDMMC_SD_CMDREST,
				  SOCLE_SDMMC_CMD_TX_SIGNAL_BEGIN |
				  SOCLE_SDMMC_RESP_TX_SIGNAL_BEGIN |
				  resp_type |
				  SOCLE_SDMMC_CMD_RESP_TX_STAT_N_ERR |
				  SOCLE_SDMMC_CMD_INDEX(cmd->opcode));
}

static void
socle_sdmmc_process_next(struct socle_sdmmc_host *host)
{
	dev_dbg(mmc_dev(host->mmc), "sq_sdmmc_process_next()\n");
	if (!(host->flags & SDMMC_SENT_CMD)) {
		host->flags |= SDMMC_SENT_CMD;
		socle_sdmmc_send_command(host, host->request->cmd);
	} else if ((!(host->flags & SDMMC_SENT_STP)) && host->request->stop) {
		host->flags |= SDMMC_SENT_STP;
		socle_sdmmc_send_command(host, host->request->stop);
	} else
		mmc_request_done(host->mmc, host->request);
}

static void
socle_sdmmc_completed_command(struct socle_sdmmc_host *host)
{
	struct mmc_command *cmd;
unsigned long flags;

	dev_dbg(mmc_dev(host->mmc), "sq_sdmmc_completed_command()\n");
	cmd = host->cmd;
	cmd->error = 0;
	cmd->resp[0] = socle_sdmmc_read(SOCLE_SDMMC_SD_RESA3);
	cmd->resp[1] = socle_sdmmc_read(SOCLE_SDMMC_SD_RESA2);
	cmd->resp[2] = socle_sdmmc_read(SOCLE_SDMMC_SD_RESA1);
	cmd->resp[3] = socle_sdmmc_read(SOCLE_SDMMC_SD_RESA0);
	if (host->buf) {
#ifdef USE_ATOMIC_VERSION_KMAP
//		kunmap_atomic(host->buf, KM_BIO_SRC_IRQ);
#else
		local_irq_save(flags);
		kunmap(host->buf);
		local_irq_restore(flags);
#endif
		host->buf = NULL;
	}
	dev_dbg(mmc_dev(host->mmc), "resp = [0x%08x 0x%08x 0x%08x 0x%08x]\n", cmd->resp[0], cmd->resp[1], cmd->resp[2], 
		 cmd->resp[3]);
	if (host->err_flag) {
		/* Check if transfer for command and response cause an error */
		if (host->err_flag & SDMMC_CMD_RESP_ERR) {
			u32 tmp;

			tmp = socle_sdmmc_read(SOCLE_SDMMC_SD_CMDRESA);
			if (SOCLE_SDMMC_CARD_RESP_TO_ERR == (tmp & SOCLE_SDMMC_CARD_RESP_TO_ERR)) {
				cmd->error = -ETIMEDOUT;
				dev_dbg(mmc_dev(host->mmc), "command and response cause a timeout error\n");
			} else if (SOCLE_SDMMC_CARD_RESP_CRC_ERR == (tmp & SOCLE_SDMMC_CARD_RESP_CRC_ERR)) {
				cmd->error = -EILSEQ;
				dev_dbg(mmc_dev(host->mmc), "command and response cause a crc error\n");
			} else {
				cmd->error = -EINVAL;
				if (SOCLE_SDMMC_CARD_CMD_RESP_BUS_CONFLICT_ERR == (tmp & SOCLE_SDMMC_CARD_CMD_RESP_BUS_CONFLICT_ERR)) 
					dev_dbg(mmc_dev(host->mmc), "command and response cause a bus conflict error\n");
				if (SOCLE_SDMMC_CARD_RESP_TX_BIT_ERR == (tmp & SOCLE_SDMMC_CARD_RESP_TX_BIT_ERR))
					dev_dbg(mmc_dev(host->mmc), "command and response cause a transmission bit error\n");
				if (SOCLE_SDMMC_CARD_RESP_IDX_ERR == (tmp & SOCLE_SDMMC_CARD_RESP_IDX_ERR))
					dev_dbg(mmc_dev(host->mmc), "command and response cause a index error\n");
				if (SOCLE_SDMMC_CARD_RESP_END_BIT_ERR == (tmp & SOCLE_SDMMC_CARD_RESP_END_BIT_ERR))
					dev_dbg(mmc_dev(host->mmc), "command and response cause a end bit error\n");
			}
		}

		/* Check if data transfer cause an error */
		if (host->err_flag & SDMMC_DATA_TX_ERR) {
			u32 tmp;

			tmp = socle_sdmmc_read(SOCLE_SDMMC_SD_DATAT);
			if (SOCLE_SDMMC_DATA_TX_TO_ERR == (tmp & SOCLE_SDMMC_DATA_TX_TO_ERR)) {
				cmd->error = -ETIMEDOUT;
				dev_dbg(mmc_dev(host->mmc), "data transfer cause a timeout error\n");
			} else if (SOCLE_SDMMC_DATA_TX_CRC_ERR == (tmp & SOCLE_SDMMC_DATA_TX_CRC_ERR)) {
				cmd->error = -EILSEQ;
				dev_dbg(mmc_dev(host->mmc), "data transfer cause a crc error\n");
				if (SOCLE_SDMMC_WRITE_DATA_TX_CRC_STAT_CRC_ERR == (tmp & SOCLE_SDMMC_WRITE_DATA_TX_CRC_STAT_CRC_ERR))
					dev_dbg(mmc_dev(host->mmc), "writing data transfer cause a crc error\n");
				if (SOCLE_SDMMC_WRITE_DATA_TX_CRC_STAT_NO_RESP == (tmp & SOCLE_SDMMC_WRITE_DATA_TX_CRC_STAT_NO_RESP))
					dev_dbg(mmc_dev(host->mmc), "writing data transfer cause a crc error and no response error\n");	      
			} else {
				cmd->error = -EINVAL;
				if (SOCLE_SDMMC_DATA_TX_BUS_CONFLICT_ERR == (tmp & SOCLE_SDMMC_DATA_TX_BUS_CONFLICT_ERR))
					dev_dbg(mmc_dev(host->mmc), "data transfer cause a bus conflict error\n");
				if (SOCLE_SDMMC_READ_DATA_TX_STR_BIT_ERR == (tmp & SOCLE_SDMMC_READ_DATA_TX_STR_BIT_ERR))
					dev_dbg(mmc_dev(host->mmc), "reading data transfer cause a start bit error\n");
				if (SOCLE_SDMMC_READ_DATA_TX_END_BIT_ERR == (tmp & SOCLE_SDMMC_READ_DATA_TX_END_BIT_ERR))
					dev_dbg(mmc_dev(host->mmc), "reading data transfer cause a end bit error\n");
			}
			dev_dbg(mmc_dev(host->mmc), "error detected and set %d (cmd = %d, retries = %d)\n", cmd->error, cmd->opcode, cmd->retries);
		}
	}
	socle_sdmmc_process_next(host);
}

static void
socle_sdmmc_pre_read_data(struct socle_sdmmc_host *host)
{
	struct mmc_data *data;

	dev_dbg(mmc_dev(host->mmc), "sq_sdmmc_pre_read_data()\n");
	data = host->cmd->data;
	socle_sdmmc_mmu_swap();
	dev_dbg(mmc_dev(host->mmc), "host->blk_idx: %d\n", host->blk_idx);
	if (1 == (data->blocks - host->blk_idx)) {
		dev_dbg(mmc_dev(host->mmc), "single read, bus_mode: 0x%08x\n", host->bus_mode);
		socle_sdmmc_write(SOCLE_SDMMC_SD_DATAT,
				  SOCLE_SDMMC_DATA_TX_SIGNAL_BEGIN |
				  SOCLE_SDMMC_DATA_TX_DIR_READ |
				  host->bus_mode |
				  SOCLE_SDMMC_DATA_TX_DMA_DIS |
				  SOCLE_SDMMC_DATA_TX_CYC_SINGLE);
	} else {
		dev_dbg(mmc_dev(host->mmc), "multiple read, bus_mode: 0x%08x\n", host->bus_mode);
		socle_sdmmc_write(SOCLE_SDMMC_SD_DATAT,
				  SOCLE_SDMMC_DATA_TX_SIGNAL_BEGIN |
				  SOCLE_SDMMC_DATA_TX_DIR_READ |
				  host->bus_mode |
				  SOCLE_SDMMC_DATA_TX_DMA_DIS |
				  SOCLE_SDMMC_DATA_TX_CYC_MULTIPLE);
	}
}

static void
socle_sdmmc_post_read_data(struct socle_sdmmc_host *host)
{
	struct mmc_data *data;
	struct scatterlist *sg;
	int i, test;

	dev_dbg(mmc_dev(host->mmc), "sq_sdmmc_post_read_data()\n");
	data = host->cmd->data;
	sg = &data->sg[host->cur_sg_idx];
	if ((host->buf_idx << 2) == sg->length) {
		host->cur_sg_idx++;
		sg = &data->sg[host->cur_sg_idx];
		kunmap_atomic(host->buf, KM_BIO_SRC_IRQ);
		host->buf = kmap_atomic(sg_page(sg), KM_BIO_SRC_IRQ) + sg->offset;
		host->buf_idx = 0;
		dev_dbg(mmc_dev(host->mmc), "sg[%d]: buffer = %p, offset = %d, length = %d\n", host->cur_sg_idx, host->buf, sg->offset, 
			 sg->length);
	}
	socle_sdmmc_mmu_swap();
//cyli fix for performance
	test = host->buf_idx;
	for (i = 0; i < (data->blksz >> 2); i++) {
		u32 tmp;

		tmp = socle_sdmmc_read(SOCLE_SDMMC_MMU_DATA);
		host->buf[test++] = swab32(tmp);
	}
	host->buf_idx = test;
	data->bytes_xfered += data->blksz;
	host->blk_idx++;
	/* Is there another transfer to trigger? */
	if (host->blk_idx  < data->blocks)
		socle_sdmmc_pre_read_data(host);
}

static void
socle_sdmmc_pre_write_data(struct socle_sdmmc_host *host)
{
	struct mmc_data *data;
	struct scatterlist *sg;
	int i, test;

	dev_dbg(mmc_dev(host->mmc), "sq_sdmmc_pre_write_data()\n");
	data = host->cmd->data;
	socle_sdmmc_mmu_swap();
	sg = &data->sg[host->cur_sg_idx];
	if ((host->buf_idx << 2) == sg->length) {
		host->cur_sg_idx++;
		sg = &data->sg[host->cur_sg_idx];
		kunmap_atomic(host->buf, KM_BIO_SRC_IRQ);
		host->buf = kmap_atomic(sg_page(sg), KM_BIO_SRC_IRQ) + sg->offset;
		host->buf_idx = 0;
		dev_dbg(mmc_dev(host->mmc), "sg[%d]: buffer = %p, offset = %d, length = %d\n", host->cur_sg_idx, host->buf, sg->offset, 
			 sg->length);
	}
//cyli fix for performance
	test = host->buf_idx;
	for (i = 0; i < (data->blksz >> 2); i++) {
		u32 tmp;

		tmp = swab32(host->buf[test++]);
		socle_sdmmc_write(SOCLE_SDMMC_MMU_DATA, tmp);
	}
	host->buf_idx = test;
	socle_sdmmc_mmu_swap();
	dev_dbg(mmc_dev(host->mmc), "host->blk_idx: %d\n", host->blk_idx);
	if (1 == (data->blocks - host->blk_idx)) {
		dev_dbg(mmc_dev(host->mmc), "single write, bus_mode: 0x%08x\n", host->bus_mode);
		socle_sdmmc_write(SOCLE_SDMMC_SD_DATAT,
				  SOCLE_SDMMC_DATA_TX_SIGNAL_BEGIN |
				  SOCLE_SDMMC_DATA_TX_DIR_WRITE |
				  host->bus_mode |
				  SOCLE_SDMMC_DATA_TX_DMA_DIS |
				  SOCLE_SDMMC_DATA_TX_CYC_SINGLE);
	} else {
		dev_dbg(mmc_dev(host->mmc), "multiple write, bus_mode: 0x%08x\n", host->bus_mode);
		socle_sdmmc_write(SOCLE_SDMMC_SD_DATAT,
				  SOCLE_SDMMC_DATA_TX_SIGNAL_BEGIN |
				  SOCLE_SDMMC_DATA_TX_DIR_WRITE |
				  host->bus_mode |
				  SOCLE_SDMMC_DATA_TX_DMA_DIS |
				  SOCLE_SDMMC_DATA_TX_CYC_MULTIPLE);
	}
}

static void
socle_sdmmc_post_write_data(struct socle_sdmmc_host *host)
{
	struct mmc_data *data;

	dev_dbg(mmc_dev(host->mmc), "sq_sdmmc_post_write_data\n");
	host->blk_idx++;
	data = host->cmd->data;
	data->bytes_xfered += data->blksz;
	if (host->blk_idx < data->blocks)
		socle_sdmmc_pre_write_data(host);
	else
		socle_sdmmc_wait_for_card_busy_end();    
}

static irqreturn_t
socle_sdmmc_isr(int irq, void *_host)
{
	struct socle_sdmmc_host *host = _host;
	int completed = 0;
	u32 int_stat;
	int card_detect_signal = 0;
	struct mmc_data *data = host->cmd->data;
	struct mmc_command *cmd;
	int tmp;

	dev_dbg(mmc_dev(host->mmc), "sq_sdmmc_isr()\n");
	cmd = host->cmd;
	data = cmd->data;
	host->err_flag = 0;
	int_stat = socle_sdmmc_read(SOCLE_SDMMC_SD_INT);
	if (SOCLE_SDMMC_HOST_CARD_DETECT_INT_STAT_YES == (int_stat & SOCLE_SDMMC_HOST_CARD_DETECT_INT_STAT_YES)) {
		/* Clear corresponding interrupt flag */
		socle_sdmmc_write(SOCLE_SDMMC_SD_INT,
				  socle_sdmmc_read(SOCLE_SDMMC_SD_INT) & (~SOCLE_SDMMC_HOST_CARD_DETECT_INT_STAT_YES));
		
		card_detect_signal = socle_sdmmc_read(SOCLE_SDMMC_SD_CARDA);
		//card dectect fix by JS 20080318 
		if ((card_detect_signal & 0x1)== SOCLE_SDMMC_CARD_DETECT_SIGNAL_HIGH)
		{
			dev_dbg(mmc_dev(host->mmc), "card has been removed\n");
			//disable card power ryan add 20071019
#ifdef CONFIG_ARCH_SCDK
			socle_sdmmc_write(SOCLE_SDMMC_SD_CARDA,socle_sdmmc_read(SOCLE_SDMMC_SD_CARDA) & (~(0x1 << 5)));
#else
			socle_sdmmc_write(SOCLE_SDMMC_SD_CARDA,socle_sdmmc_read(SOCLE_SDMMC_SD_CARDA)|(0x1 << 5));
#endif
		}
		else
		{
			//Enable card power ryan add 20071019
#ifdef CONFIG_ARCH_SCDK
			socle_sdmmc_write(SOCLE_SDMMC_SD_CARDA,socle_sdmmc_read(SOCLE_SDMMC_SD_CARDA)|(0x1 << 5));
#else
			socle_sdmmc_write(SOCLE_SDMMC_SD_CARDA,socle_sdmmc_read(SOCLE_SDMMC_SD_CARDA) & (~(0x1 << 5)));
#endif
			dev_dbg(mmc_dev(host->mmc), "card has been inserted\n");
		}
		
		mmc_detect_change(host->mmc, msecs_to_jiffies(100));
	}
	if (SOCLE_SDMMC_CMD_RESP_TX_INT_STAT_YES == (int_stat & SOCLE_SDMMC_CMD_RESP_TX_INT_STAT_YES)) {
		/* Clear corresponding interrupt flag */
		socle_sdmmc_write(SOCLE_SDMMC_SD_INT,
				  socle_sdmmc_read(SOCLE_SDMMC_SD_INT) & (~SOCLE_SDMMC_CMD_RESP_TX_INT_STAT_YES));

		/* Check wether command transfer cause any error */
		tmp = socle_sdmmc_read(SOCLE_SDMMC_SD_CMDREST);
		if (SOCLE_SDMMC_CMD_RESP_TX_STAT_ERR == (tmp & SOCLE_SDMMC_CMD_RESP_TX_STAT_ERR)) {
			dev_dbg(mmc_dev(host->mmc), "command and response error\n");
			host->err_flag |= SDMMC_CMD_RESP_ERR;
			completed = 1;
		} else {
			if (!data)
				completed = 1;
			else {
				if (data->flags & MMC_DATA_WRITE)		
					socle_sdmmc_pre_write_data(host);
			}
		}
	}
	if (SOCLE_SDMMC_DATA_TX_INT_STAT_YES == (int_stat & SOCLE_SDMMC_DATA_TX_INT_STAT_YES)) {
		/* Clear corresponding interrupt flag */
		socle_sdmmc_write(SOCLE_SDMMC_SD_INT,
				  socle_sdmmc_read(SOCLE_SDMMC_SD_INT) & (~SOCLE_SDMMC_DATA_TX_INT_STAT_YES));

		/* Check wether data transfer cause any error */
		tmp = socle_sdmmc_read(SOCLE_SDMMC_SD_DATAT);
		if (SOCLE_SDMMC_DATA_TX_STAT_ERR == (tmp & SOCLE_SDMMC_DATA_TX_STAT_ERR)) {
			dev_dbg(mmc_dev(host->mmc), "data transfer error\n");
			host->err_flag |= SDMMC_DATA_TX_ERR;
			completed = 1;
		} else {
			if (data->flags & MMC_DATA_READ)
				socle_sdmmc_post_read_data(host);
			else
				socle_sdmmc_post_write_data(host);
			if (host->blk_idx == data->blocks)
				completed = 1;
		}
	}
	if (completed)
		socle_sdmmc_completed_command(host);
	return IRQ_HANDLED;
}

static void 
socle_sdmmc_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
	struct socle_sdmmc_host *host = mmc_priv(mmc);

	host->request = mrq;
	host->flags = 0;
	socle_sdmmc_process_next(host);
}

static u32
socle_sdmmc_power(u32 base, u32 power)
{
	int i = 0;
	u32 val = 1;

	if (0 == power)
		return 1;
	else {
		for (i = 0; i < power; i++)
			val *= base;
		return val;
	}
}

static void
socle_sdmmc_set_ios(struct mmc_host *mmc, struct mmc_ios *ios)
{
	int clk_div = 0, power;
	u32 sdmmc_clk;
	struct socle_sdmmc_host *host = mmc_priv(mmc);

	dev_dbg(mmc_dev(host->mmc), "sq_sdmmc_set_ios()\n");
	if (0 == ios->clock) {
		dev_dbg(mmc_dev(host->mmc), "disable the SD/MMC host controller\n");
		socle_sdmmc_write(SOCLE_SDMMC_SD_CTRL,
				  socle_sdmmc_read(SOCLE_SDMMC_SD_CTRL) | SOCLE_SDMMC_CARD_CLK_STP);
	} else {
		/* SD/MMC clock formula is apb_clk * 1/2power(divider) + 1 */
		while (1) {
			power = socle_sdmmc_power(2, clk_div);
			sdmmc_clk = apb_clock / (power + 1);
			if (sdmmc_clk < ios->clock)
				break;
			clk_div++;
		}
		dev_dbg(mmc_dev(host->mmc), "divider is %d\n", clk_div);
		dev_dbg(mmc_dev(host->mmc), "clock set to %d now\n", sdmmc_clk);

		/* Set the divider */
		socle_sdmmc_write(SOCLE_SDMMC_SD_CTRL,
				  SOCLE_SDMMC_CARD_POWER_CTRL_CPU |
				  SOCLE_SDMMC_CARD_DETECT_FUNC_MECH |
				  SOCLE_SDMMC_CARD_CLK_RUN |
				  SOCLE_SDMMC_CARD_CLK_DIVIDER(clk_div));
	}
	if ((MMC_BUS_WIDTH_4 == ios->bus_width)) {
		dev_dbg(mmc_dev(host->mmc), "bus width is 4 bits\n");
		host->bus_mode = SOCLE_SDMMC_DATA_TX_BUS_WIDTH_LINE_4;
	} else {
		dev_dbg(mmc_dev(host->mmc), "bus width is 1 bit\n");
		host->bus_mode = SOCLE_SDMMC_DATA_TX_BUS_WIDTH_LINE_1;
	}
	switch (ios->power_mode) {
	case MMC_POWER_OFF:
		dev_dbg(mmc_dev(host->mmc), "power off\n");
		break;
	case MMC_POWER_UP:
		dev_dbg(mmc_dev(host->mmc), "power up\n");
		break;
	case MMC_POWER_ON:
		dev_dbg(mmc_dev(host->mmc), "power on\n");
		break;
	}
}

static int
socle_sdmmc_get_ro(struct mmc_host *mmc)	
{
	int read_only = 0;

	dev_dbg(mmc_dev(mmc), "does not support reading read-only switch. "
		 "Assuimg write-enable\n");
	return read_only;
}

static struct mmc_host_ops socle_sdmmc_ops = {
	.request = socle_sdmmc_request,
	.set_ios = socle_sdmmc_set_ios,
	.get_ro = socle_sdmmc_get_ro,
};

/*
 *  Probe fo the device
 *  */
static int 
socle_sdmmc_probe(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct socle_sdmmc_host *host;
	struct resource *res;
	int err;

	dev_dbg(&pdev->dev, "sq_sdmmc_probe()\n");
	mmc = mmc_alloc_host(sizeof(struct socle_sdmmc_host), &pdev->dev);
	if (!mmc) {
		dev_err(&pdev->dev, "failed to allocate mmc host\n");
		return -ENOMEM;
	}
	mmc->ops = &socle_sdmmc_ops;
	mmc->f_min = 375000;
	mmc->f_max = 25000000;
	mmc->ocr_avail = MMC_VDD_32_33 | MMC_VDD_33_34;
//	mmc->caps = MMC_CAP_BYTEBLOCK;
	host = mmc_priv(mmc);
	host->mmc = mmc;
	host->buf = NULL;
	host->bus_mode = 0;
	mmc_dev(host->mmc) = &pdev->dev;

	/* Host suppot 4 bits bus transfer */
	mmc->caps |= MMC_CAP_4_BIT_DATA;

	/* Find and claim our resources */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (NULL == res) {
		dev_err(&pdev->dev, "cannot get IORESOURCE_MEM\n");
		err = -ENOENT;
		goto err_no_io_res;
	}
	host->io_area = request_mem_region(res->start, (res->end - res->start)+1, pdev->name);
	if (NULL == host->io_area) {
		dev_err(&pdev->dev, "cannot reserve region\n");
		err = -ENXIO;
		goto err_no_io_res;
	}
	socle_sdmmc_base = IO_ADDRESS(host->io_area->start);
	host->irq = platform_get_irq(pdev, 0);
	if (host->irq < 0) {
		dev_err(&pdev->dev, "no irq specified\n");
		err = -ENOENT;
		goto err_no_irq;
	}

	/* Allocate the interrupt */
	err = request_irq(host->irq, socle_sdmmc_isr, IRQF_DISABLED, pdev->name, host);
	if (err) {
		dev_err(&pdev->dev, "cannot claim IRQ\n");
		mmc_free_host(mmc);
		goto err_no_irq;
	}

	platform_set_drvdata(pdev, mmc);

	/* Initialize the SD/MMC host controller */
	socle_sdmmc_write(SOCLE_SDMMC_SD_CARDA,
			  SOCLE_SDMMC_CARD_SEL_EN | 
			  SOCLE_SDMMC_CARD_POWER_CTRL_SIGNAL_EN |
			  SOCLE_SDMMC_CARD_DETECT_INT_EN);
	socle_sdmmc_write(SOCLE_SDMMC_SD_INT,
			  SOCLE_SDMMC_CMD_RESP_TX_INT_STAT_NO |
			  SOCLE_SDMMC_DATA_TX_INT_STAT_NO |
			  SOCLE_SDMMC_HOST_CARD_DETECT_INT_STAT_NO |
			  SOCLE_SDMMC_CMD_RESP_TX_INT_EN |
			  SOCLE_SDMMC_DATA_TX_INT_EN |
			  SOCLE_SDMMC_HOST_CARD_DETECT_INT_EN);

	/* Add host to MMC layer */
	mmc_add_host(mmc);

	dev_dbg(mmc_dev(host->mmc), "added socle sd/mmc host driver\n");
	return 0;
err_no_irq:
	release_resource(host->io_area);
err_no_io_res:
	return err;
}

/*
  Remove a device
*/
static int
socle_sdmmc_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	struct socle_sdmmc_host *host = mmc_priv(mmc);;

	dev_dbg(mmc_dev(host->mmc), "SD/MMC host removed\n");
	if (!mmc)
		return -1;
	platform_set_drvdata(pdev, NULL);
	mmc_remove_host(mmc);
	free_irq(host->irq, host);
	release_resource(host->io_area);
	return 0;
}

#ifdef CONFIG_PM
static int
socle_sdmmc_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	int ret = 0;

	if (mmc)
		ret = mmc_suspend_host(mmc, state);
	return ret;
}

static int 
socle_sdmmc_resume(struct platform_device *pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	int ret = 0;

	if (mmc)
		ret = mmc_resume_host(mmc);
	return ret;
}
#else
#define socle_sdmmc_suspend NULL
#define socle_sdmmc_resume NULL
#endif

static struct platform_driver socle_sdmmc_driver = {
	.probe = socle_sdmmc_probe,
	.remove = socle_sdmmc_remove,
	.suspend = socle_sdmmc_suspend,
	.resume = socle_sdmmc_resume,
	.driver = {
		.name = "sq_sdmmc",
		.owner = THIS_MODULE,
	},
};

static int __init
socle_sdmmc_init(void)
{
	return platform_driver_register(&socle_sdmmc_driver);
}

static void __exit
socle_sdmmc_exit(void)
{
	platform_driver_unregister(&socle_sdmmc_driver);
}

module_init(socle_sdmmc_init);
module_exit(socle_sdmmc_exit);

MODULE_DESCRIPTION("SQ SD/MMC Interface Driver");
MODULE_AUTHOR("Obi Hsieh");
MODULE_LICENSE("GPL");

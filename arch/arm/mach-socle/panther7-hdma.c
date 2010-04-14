#include <asm/io.h>
#include <mach/dma.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <mach/platform.h>
#include <mach/panther7-hdma-regs.h>

/*
 *  Read register
 *  */
static inline u32
socle_reg_read(u32 reg, u32 base)
{
	return ioread32(base+reg);
}

/*
 *  Write register
 *  */
static inline void
socle_reg_write(u32 reg, u32 value, u32 base)
{
	iowrite32(value, base+reg);
}

static int panther7_hdma_request(u32 ch, struct socle_dma *dma);
static void panther7_hdma_free(u32 ch, struct socle_dma *dma);
static void panther7_hdma_enable(u32 ch, struct socle_dma *dma);
static void panther7_hdma_disable(u32 ch, struct socle_dma *dma);
static void panther7_hdma_set_page_number(u32 ch, struct socle_dma *dma, u32 num);
static irqreturn_t panther7_hdma_isr_0(int irq, void *dma);
static irqreturn_t panther7_hdma_isr_1(int irq, void *dma);
static void panther7_hdma_set_count(struct socle_dma *dma);

struct panther7_hdma_host {
	u8 ch;
	u32 va_base;
};

static struct panther7_hdma_host host[] = {
	{
		.ch = 0,
	}, {
		.ch = 1,
	},
};

static struct resource panther7_hdma_resource[] = {
	[0] = {
		.start = PANTHER7_AHB0_HDMA,
		.end = PANTHER7_AHB0_HDMA + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},
};

struct socle_dma_ops panther7_hdma_ops = {
	.request = panther7_hdma_request,
	.free = panther7_hdma_free,
	.enable = panther7_hdma_enable,
	.disable = panther7_hdma_disable,
	.set_page_number = panther7_hdma_set_page_number,
};

struct socle_dma panther7_hdma_channel_0 = {
	.dma_name = "PANTHER7 HDMA Channel 0",
	.res = panther7_hdma_resource,
	.irq = IRQ_PANTHER7_HDMA,
	.private_data = &host[0],
	.ops = &panther7_hdma_ops,
};

struct socle_dma panther7_hdma_channel_1 = {
	.dma_name = "PANTHER7 HDMA Channel 1",
	.res = panther7_hdma_resource,
	.irq = IRQ_PANTHER7_HDMA,
	.private_data = &host[1],
	.ops = &panther7_hdma_ops,
};

static int 
panther7_hdma_request(u32 ch, struct socle_dma *dma)
{
	int ret = 0;
	struct panther7_hdma_host *host = dma->private_data;

	// 20090209 cyli fix from SA_SHIRQ to IRQF_SHARED
	if (0 == host->ch) {
		host->va_base = IO_ADDRESS(dma->res->start);
		ret = request_irq(dma->irq, panther7_hdma_isr_0, IRQF_SHARED, "PANTHER7 HDMA Channel 0", dma);
	}
	else if (1 == host->ch) {
		host->va_base = IO_ADDRESS(dma->res->start);
		ret = request_irq(dma->irq, panther7_hdma_isr_1, IRQF_SHARED, "PANTHER7 HDMA Channel 1", dma);
	}
	if (ret)
		printk(KERN_ERR "%s: failed to request interrupt\n", dma->dma_name);
	return ret;
}

static void 
panther7_hdma_free(u32 ch, struct socle_dma *dma)
{
	free_irq(dma->irq, dma);
}

static void 
panther7_hdma_enable(u32 ch, struct socle_dma *dma)
{
	struct panther7_hdma_host *host = dma->private_data;
	u32 tmp, data_size = 1;
	u32 conf = PANTHER7_HDMA_SLICE_MODE_DIS |
		PANTHER7_HDMA_CH_EN |
		PANTHER7_HDMA_INT_MODE_INTERRUPT |
		PANTHER7_HDMA_FLY_DIS |
		PANTHER7_HDMA_BURST_SINGLE |
		PANTHER7_HDMA_EXT_HDREQ_SEL(0) |
		PANTHER7_HDMA_DIR_SRC_INC |
		PANTHER7_HDMA_DIR_DST_INC |
		PANTHER7_HDMA_DATA_BYTE |
		PANTHER7_HDMA_SWDMA_OP_NO |
		PANTHER7_HDMA_HWDMA_TRIGGER_DIS;

	if (SOCLE_DMA_MODE_SLICE == dma->mode)		
		conf |= PANTHER7_HDMA_SLICE_MODE_EN;
	switch (dma->burst_type) {
	case SOCLE_DMA_BURST_SINGLE:
		conf |= PANTHER7_HDMA_BURST_SINGLE;
		break;
	case SOCLE_DMA_BURST_INCR4:
		conf |= PANTHER7_HDMA_BURST_INCR4;
		break;
	case SOCLE_DMA_BURST_INCR8:
		conf |= PANTHER7_HDMA_BURST_INCR8;
		break;
	case SOCLE_DMA_BURST_INCR16:
		conf |= PANTHER7_HDMA_BURST_INCR16;
		break;
	}
	conf |= PANTHER7_HDMA_EXT_HDREQ_SEL(dma->ext_hdreq);
	if (SOCLE_DMA_DIR_FIXED == dma->src_dir)
		conf |= PANTHER7_HDMA_DIR_SRC_FIXED;
	if (SOCLE_DMA_DIR_FIXED == dma->dst_dir)
		conf |= PANTHER7_HDMA_DIR_DST_FIXED;
	switch (dma->data_size) {
	case SOCLE_DMA_DATA_BYTE:
		data_size = 1;
		conf |= PANTHER7_HDMA_DATA_BYTE;
		break;
	case SOCLE_DMA_DATA_HALFWORD:
		data_size = 2;
		conf |= PANTHER7_HDMA_DATA_HALFWORD;
		break;
	case SOCLE_DMA_DATA_WORD:
		data_size = 4;
		conf |= PANTHER7_HDMA_DATA_WORD;
		break;
	}
	if ((SOCLE_DMA_MODE_SLICE == dma->mode) ||
	    (SOCLE_DMA_MODE_HW == dma->mode))
		conf |= PANTHER7_HDMA_HWDMA_TRIGGER_EN;
	else
		conf |= PANTHER7_HDMA_SWDMA_OP_START;
	switch (host->ch) {
	case 0:
		/* Unmask channel 0 all interrupt */
		tmp = PANTHER7_HDMA_CH0_INT_MASK | PANTHER7_HDMA_CH0_PAGE_INT_MASK | PANTHER7_HDMA_CH0_PAGE_ACCUM_OVF_INT_MASK;
		socle_reg_write(PANTHER7_HDMA_ISR,
				    socle_reg_read(PANTHER7_HDMA_ISR, host->va_base)&(~tmp),
				    host->va_base);
    
		socle_reg_write(PANTHER7_HDMA_ISRC0, dma->src_addr , host->va_base);
		socle_reg_write(PANTHER7_HDMA_IDST0, dma->dst_addr , host->va_base);
		panther7_hdma_set_count(dma);
		if (SOCLE_DMA_MODE_SLICE == dma->mode) {
			socle_reg_write(PANTHER7_HDMA_ISCNT0, dma->slice_cnt-1, host->va_base);
			socle_reg_write(PANTHER7_HDMA_IADDR_BS0, dma->buf_size-data_size, host->va_base);
		}
		socle_reg_write(PANTHER7_HDMA_CON0, conf, host->va_base);
		break;
	case 1:
		/* Unmask channel  all interrupt */
		tmp = PANTHER7_HDMA_CH1_INT_MASK | PANTHER7_HDMA_CH1_PAGE_INT_MASK | PANTHER7_HDMA_CH1_PAGE_ACCUM_OVF_INT_MASK;
		socle_reg_write(PANTHER7_HDMA_ISR,
				    socle_reg_read(PANTHER7_HDMA_ISR, host->va_base)&(~tmp),
				    host->va_base);

		socle_reg_write(PANTHER7_HDMA_ISRC1, dma->src_addr , host->va_base);
		socle_reg_write(PANTHER7_HDMA_IDST1, dma->dst_addr , host->va_base);
		panther7_hdma_set_count(dma);
		if (SOCLE_DMA_MODE_SLICE == dma->mode) {
			socle_reg_write(PANTHER7_HDMA_ISCNT1, dma->slice_cnt-1, host->va_base);
			socle_reg_write(PANTHER7_HDMA_IADDR_BS1, dma->buf_size-data_size, host->va_base);
		}
		socle_reg_write(PANTHER7_HDMA_CON1, conf, host->va_base);
		break;
	default:
		printk(KERN_ERR "%s: unknown channel number %d\n", dma->dma_name, ch);
	}
}

static void 
panther7_hdma_disable(u32 ch, struct socle_dma *dma)
{
	struct panther7_hdma_host *host = dma->private_data;
	u32 tmp;

	switch (host->ch) {
	case 0:
		/* Clear channel 0 configuration register */
		socle_reg_write(PANTHER7_HDMA_CON0, PANTHER7_HDMA_CPNCNTD_CLR|0x00000000, host->va_base);

		/* Clear channel 0 interrupt flag */
		tmp = PANTHER7_HDMA_CH0_INT_ACT | PANTHER7_HDMA_CH0_PAGE_INT_ACT | PANTHER7_HDMA_CH0_PAGE_ACCUM_OVF_INT_ACT;
		socle_reg_write(PANTHER7_HDMA_ISR,
				    socle_reg_read(PANTHER7_HDMA_ISR, host->va_base)&(~tmp),
				    host->va_base);

		/* Mask channel 0 all interrupt */
		socle_reg_write(PANTHER7_HDMA_ISR,
				    socle_reg_read(PANTHER7_HDMA_ISR, host->va_base) | 
				    PANTHER7_HDMA_CH0_INT_MASK |
				    PANTHER7_HDMA_CH0_PAGE_INT_MASK |
				    PANTHER7_HDMA_CH0_PAGE_ACCUM_OVF_INT_MASK,
				    host->va_base);

		socle_reg_write(PANTHER7_HDMA_ISRC0, 0x00000000 , host->va_base);
		socle_reg_write(PANTHER7_HDMA_IDST0, 0x00000000 , host->va_base);
		socle_reg_write(PANTHER7_HDMA_ICNT0, 0x0000, host->va_base);
		if (SOCLE_DMA_MODE_SLICE == dma->mode) {
			socle_reg_write(PANTHER7_HDMA_ISCNT0, 0x00, host->va_base);
			socle_reg_write(PANTHER7_HDMA_IADDR_BS0, 0x00000000, host->va_base);
		}
		break;
	case 1:
		/* Clear channel 1configuration register */
		socle_reg_write(PANTHER7_HDMA_CON1, PANTHER7_HDMA_CPNCNTD_CLR|0x00000000, host->va_base);

		/* Clear channel 1 interrupt flag */
		tmp = PANTHER7_HDMA_CH1_INT_ACT | PANTHER7_HDMA_CH1_PAGE_INT_ACT | PANTHER7_HDMA_CH1_PAGE_ACCUM_OVF_INT_ACT;
		socle_reg_write(PANTHER7_HDMA_ISR,
				    socle_reg_read(PANTHER7_HDMA_ISR, host->va_base)&(~tmp),
				    host->va_base);

		/* Mask channel 1 all interrupt */
		socle_reg_write(PANTHER7_HDMA_ISR,
				    socle_reg_read(PANTHER7_HDMA_ISR, host->va_base) | 
				    PANTHER7_HDMA_CH1_INT_MASK |
				    PANTHER7_HDMA_CH1_PAGE_INT_MASK |
				    PANTHER7_HDMA_CH1_PAGE_ACCUM_OVF_INT_MASK,
				    host->va_base);

		socle_reg_write(PANTHER7_HDMA_ISRC1, 0x00000000, host->va_base);
		socle_reg_write(PANTHER7_HDMA_IDST1, 0x00000000, host->va_base);
		socle_reg_write(PANTHER7_HDMA_ICNT1, 0x0000, host->va_base);
		if (SOCLE_DMA_MODE_SLICE == dma->mode) {
			socle_reg_write(PANTHER7_HDMA_ISCNT1, 0x00, host->va_base);
			socle_reg_write(PANTHER7_HDMA_IADDR_BS1, 0x00000000, host->va_base);
		}
		break;
	default:
		printk(KERN_ERR "%s: unknown channel number %d\n", dma->dma_name, ch);
		return;
	}
}

static void 
panther7_hdma_set_page_number(u32 ch, struct socle_dma *dma, u32 num)
{
	struct panther7_hdma_host *host = dma->private_data;
	switch (host->ch) {
	case 0:
		socle_reg_write(PANTHER7_HDMA_IPNCNT0 ,num, host->va_base);
		break;
	case 1:
		socle_reg_write(PANTHER7_HDMA_IPNCNT1 ,num, host->va_base);
		break;
	default:
		printk(KERN_ERR "%s: unknown channel number %d\n", dma->dma_name, ch);
	}
}

static irqreturn_t 
panther7_hdma_isr_0(int irq, void *_dma)
{
	u32 int_stat;
	struct socle_dma *dma = _dma;
	struct panther7_hdma_host *host = dma->private_data;		//20080111 leonid+ for host struct 

	int_stat = socle_reg_read(PANTHER7_HDMA_ISR, host->va_base);
	int_stat &= (PANTHER7_HDMA_CH0_INT_ACT | 
		     PANTHER7_HDMA_CH0_PAGE_INT_ACT |
		     PANTHER7_HDMA_CH0_PAGE_ACCUM_OVF_INT_ACT);

	/* Clear the interrupt flag */
	socle_reg_write(PANTHER7_HDMA_ISR,
			    socle_reg_read(PANTHER7_HDMA_ISR, host->va_base) & (~int_stat),
			    host->va_base);
	if (int_stat & PANTHER7_HDMA_CH0_INT_ACT) {
		if (dma->notifier->complete)
			dma->notifier->complete(dma->notifier->data);
	}
	if (int_stat & PANTHER7_HDMA_CH0_PAGE_INT_ACT) {
		if (dma->notifier->page_interrupt)
			dma->notifier->page_interrupt(dma->notifier->data);
	}
	if (int_stat & PANTHER7_HDMA_CH0_PAGE_ACCUM_OVF_INT_ACT) {
		if (dma->notifier->page_accumulation_overflow)
			dma->notifier->page_accumulation_overflow(dma->notifier->data);
	}
	return IRQ_HANDLED;
}

static irqreturn_t 
panther7_hdma_isr_1(int irq, void *_dma)
{
	u32 int_stat;
	struct socle_dma *dma = _dma;
	struct panther7_hdma_host *host = dma->private_data;		//20080111 leonid+ for host struct 

	int_stat = socle_reg_read(PANTHER7_HDMA_ISR, host->va_base);
	int_stat &= (PANTHER7_HDMA_CH1_INT_ACT | 
		     PANTHER7_HDMA_CH1_PAGE_INT_ACT |
		     PANTHER7_HDMA_CH1_PAGE_ACCUM_OVF_INT_ACT);

	/* Clear the interrupt flag */
	socle_reg_write(PANTHER7_HDMA_ISR,
			    socle_reg_read(PANTHER7_HDMA_ISR, host->va_base) & (~int_stat),
			    host->va_base);
	if (int_stat & PANTHER7_HDMA_CH1_INT_ACT) {
		if (dma->notifier->complete)
			dma->notifier->complete(dma->notifier->data);
	}
	if (int_stat & PANTHER7_HDMA_CH1_PAGE_INT_ACT) {
		if (dma->notifier->page_interrupt)
			dma->notifier->page_interrupt(dma->notifier->data);
	}
	if (int_stat & PANTHER7_HDMA_CH1_PAGE_ACCUM_OVF_INT_ACT) {
		if (dma->notifier->page_accumulation_overflow)
			dma->notifier->page_accumulation_overflow(dma->notifier->data);
	}
	return IRQ_HANDLED;
}

static void 
panther7_hdma_set_count(struct socle_dma *dma)
{
	int burst_val = 0;
	int data_size_val = 0;
	struct panther7_hdma_host *host = dma->private_data;

	switch(dma->burst_type) {
	case SOCLE_DMA_BURST_SINGLE:
		burst_val = 1;
		break;
	case SOCLE_DMA_BURST_INCR4:
		burst_val = 4;
		break;
	case SOCLE_DMA_BURST_INCR8:
		burst_val = 8;
		break;
	case SOCLE_DMA_BURST_INCR16:
		burst_val = 16;
		break;
	default:
		printk(KERN_ERR "%s: burst type (%d) is unknown\n", dma->dma_name,  dma->burst_type);
		return;
	}
	switch (dma->data_size) {
	case SOCLE_DMA_DATA_BYTE:
		data_size_val = 1;
		break;
	case SOCLE_DMA_DATA_HALFWORD:
		data_size_val = 2;
		break;
	case SOCLE_DMA_DATA_WORD:
		data_size_val = 4;
		break;
	}
	if (0 == (dma->tx_cnt % (burst_val * data_size_val))) {
		if (0 == host->ch) 
			socle_reg_write(PANTHER7_HDMA_ICNT0, (dma->tx_cnt/data_size_val)-1, host->va_base);
		else
			socle_reg_write(PANTHER7_HDMA_ICNT1, (dma->tx_cnt/data_size_val)-1, host->va_base);
	} else
		printk(KERN_ERR "%s: %d is not a multiple of %d (%d * %d)\n", dma->dma_name, dma->tx_cnt, (burst_val*data_size_val), burst_val,
		       data_size_val);
}


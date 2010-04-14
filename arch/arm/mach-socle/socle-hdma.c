#include <asm/io.h>
#include <mach/dma.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <mach/platform.h>
#include <mach/hdma-regs.h>

static inline void
socle_hdma_write(u32 reg, u32 value, u32 base) 
{
	iowrite32(value, base+reg);
}

static inline u32
socle_hdma_read(u32 reg, u32 base)
{
	return ioread32(base+reg);
}

static int socle_hdma_request(u32 ch, struct socle_dma *dma);
static void socle_hdma_free(u32 ch, struct socle_dma *dma);
static void socle_hdma_enable(u32 ch, struct socle_dma *dma);
static void socle_hdma_disable(u32 ch, struct socle_dma *dma);
static irqreturn_t socle_hdma_isr_0(int irq, void *dma);
static irqreturn_t socle_hdma_isr_1(int irq, void *dma);
static irqreturn_t socle_hdma_isr_2(int irq, void *dma);
static irqreturn_t socle_hdma_isr_3(int irq, void *dma);	
static void socle_hdma_set_count(struct socle_dma *dma);

static u32 socle_hdma_channel[] = {0, 1, 2, 3};

static struct resource socle_hdma_resource[] = {
	[0] = {
		.start = SOCLE_AHB0_HDMA,
		.end = SOCLE_AHB0_HDMA + SZ_128K - 1,
		.flags = IORESOURCE_MEM,
	},
};

struct socle_dma_ops socle_hdma_ops = {
	.request = socle_hdma_request,
	.free = socle_hdma_free,
	.enable = socle_hdma_enable,
	.disable = socle_hdma_disable,
};


struct socle_dma socle_hdma_channel_0 = {
	.dma_name = "SQ HDMA Channel 0",
	.res = socle_hdma_resource,
	.irq = IRQ_HDMA0,
	.private_data = &socle_hdma_channel[0],
	.ops = &socle_hdma_ops,
};

struct socle_dma socle_hdma_channel_1 = {
	.dma_name = "SQ HDMA Channel 1",
	.res = socle_hdma_resource,
	.irq = IRQ_HDMA0,
	.private_data = &socle_hdma_channel[1],
	.ops = &socle_hdma_ops,
};

struct socle_dma socle_hdma_channel_2 = {
	.dma_name = "SQ HDMA Channel 2",
	.res = socle_hdma_resource,
	.irq = IRQ_HDMA1,
	.private_data = &socle_hdma_channel[2],
	.ops = &socle_hdma_ops,
};

struct socle_dma socle_hdma_channel_3 = {
	.dma_name = "SQ HDMA Channel 3",
	.res = socle_hdma_resource,
	.irq = IRQ_HDMA1,
	.private_data = &socle_hdma_channel[3],
	.ops = &socle_hdma_ops,
};

static int
socle_hdma_request(u32 ch, struct socle_dma *dma)
{
	int ret = 0;
	u32 inter_ch = *((u32 *)dma->private_data);

	// 20090209 cyli fix from SA_SHIRQ to IRQF_SHARED
	// and add return value
	if (0 == inter_ch) 
		ret = request_irq(dma->irq, socle_hdma_isr_0, IRQF_SHARED, "SQ HDMA channel 0", dma);
	else if (1 == inter_ch)
		ret = request_irq(dma->irq, socle_hdma_isr_1, IRQF_SHARED, "SQ HDMA channel 1", dma);
	else if (2 == inter_ch)
		ret = request_irq(dma->irq, socle_hdma_isr_2, IRQF_SHARED, "SQ HDMA channel 2", dma);
	else if (3 == inter_ch)
		ret = request_irq(dma->irq, socle_hdma_isr_3, IRQF_SHARED, "SQ HDMA channel 3", dma);
	if (ret)
		printk(KERN_ERR "HDMA: failed to request interrupt\n");
	return ret;
}

static void 
socle_hdma_free(u32 ch, struct socle_dma *dma)
{
	free_irq(dma->irq, dma);
}

static void 
socle_hdma_enable(u32 ch, struct socle_dma *dma)
{
	u32 inter_ch = *((u32 *)dma->private_data);
	u32 conf = SOCLE_HDMA_AUTORELOAD_DIS |
		SOCLE_HDMA_CH_EN |
		SOCLE_HDMA_INT_MODE_INT |
		SOCLE_HDMA_FLY_DIS |
		SOCLE_HDMA_TX_MODE_SINGLE |
		SOCLE_HDMA_HDREQ0(0) |
		SOCLE_HDMA_DIR_SRC_INC |
		SOCLE_HDMA_DIR_DST_INC |
		SOCLE_HDMA_DATA_SIZE_BYTE |
		SOCLE_HDMA_SWDMA_OP_NO |
		SOCLE_HDMA_HWDMA_TRIGGER_DIS;

	switch (dma->burst_type) {
	case SOCLE_DMA_BURST_SINGLE:
		conf |= SOCLE_HDMA_TX_MODE_SINGLE;
		break;
	case SOCLE_DMA_BURST_INCR4:
		conf |= SOCLE_HDMA_TX_MODE_INCR4;
		break;
	case SOCLE_DMA_BURST_INCR8:
		conf |= SOCLE_HDMA_TX_MODE_INCR8;
		break;
	case SOCLE_DMA_BURST_INCR16:
		conf |= SOCLE_HDMA_TX_MODE_INCR16;
		break;
	}
	conf |= SOCLE_HDMA_HDREQ0(dma->ext_hdreq);
	if (SOCLE_DMA_DIR_FIXED == dma->src_dir)
		conf |= SOCLE_HDMA_DIR_SRC_FIXED;
	if (SOCLE_DMA_DIR_FIXED == dma->dst_dir)
		conf |= SOCLE_HDMA_DIR_DST_FIXED;		
	if (SOCLE_DMA_FLY_READ == dma->fly_op)
		conf |= SOCLE_HDMA_FLY_READ;		
	if (SOCLE_DMA_FLY_WRITE == dma->fly_op)
		conf |= SOCLE_HDMA_FLY_WRITE;
	switch (dma->data_size) {
	case SOCLE_DMA_DATA_BYTE:
		conf |= SOCLE_HDMA_DATA_SIZE_BYTE;
		break;
	case SOCLE_DMA_DATA_HALFWORD:
		conf |= SOCLE_HDMA_DATA_SIZE_HALFWORD;
		break;
	case SOCLE_DMA_DATA_WORD:
		conf |= SOCLE_HDMA_DATA_SIZE_WORD;
		break;
	}	
	if (SOCLE_DMA_MODE_HW == dma->mode)
		conf |= SOCLE_HDMA_HWDMA_TRIGGER_EN;
	else
		conf |= SOCLE_HDMA_SWDMA_OP_START;
	switch (inter_ch) {
	case 0:    
		socle_hdma_write(SOCLE_HDMA_ISRC0, dma->src_addr , IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_IDST0, dma->dst_addr , IO_ADDRESS(dma->res->start));
		socle_hdma_set_count(dma);
		socle_hdma_write(SOCLE_HDMA_CON0, conf, IO_ADDRESS(dma->res->start));
		break;
	case 1:
		socle_hdma_write(SOCLE_HDMA_ISRC1, dma->src_addr , IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_IDST1, dma->dst_addr , IO_ADDRESS(dma->res->start));		
		socle_hdma_set_count(dma);
		socle_hdma_write(SOCLE_HDMA_CON1, conf, IO_ADDRESS(dma->res->start));
		break;
	case 2:    
		socle_hdma_write(SOCLE_HDMA_ISRC2, dma->src_addr , IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_IDST2, dma->dst_addr , IO_ADDRESS(dma->res->start));	
		socle_hdma_set_count(dma);
		socle_hdma_write(SOCLE_HDMA_CON2, conf, IO_ADDRESS(dma->res->start));
		break;
	case 3:
		socle_hdma_write(SOCLE_HDMA_ISRC3, dma->src_addr , IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_IDST3, dma->dst_addr , IO_ADDRESS(dma->res->start));	
		socle_hdma_set_count(dma);
		socle_hdma_write(SOCLE_HDMA_CON3, conf, IO_ADDRESS(dma->res->start));
		break;
	default:
		printk(KERN_ERR "HDMA: unknown channel number %d\n", ch);
	}
}

static void 
socle_hdma_disable(u32 ch, struct socle_dma *dma)
{
	u32 inter_ch = *((u32 *)dma->private_data);
	u32 tmp;

		
	
	switch (inter_ch) {
	case 0:
			/* Clear channel 0 interrupt flag */
		tmp = SOCLE_HDMA_CH0_INT_ACT;
		socle_hdma_write(SOCLE_HDMA_ISR01,
				    socle_hdma_read(SOCLE_HDMA_ISR01, IO_ADDRESS(dma->res->start))&(~tmp),
				    IO_ADDRESS(dma->res->start));
			/* Clear channel 0 configuration register */
		socle_hdma_write(SOCLE_HDMA_CON0, 0, IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_ISRC0, 0 , IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_IDST0, 0 , IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_ICNT0, 0, IO_ADDRESS(dma->res->start));		
		break;
	case 1:
			/* Clear channel 0 interrupt flag */
		tmp = SOCLE_HDMA_CH1_INT_ACT;
		socle_hdma_write(SOCLE_HDMA_ISR01,
				    socle_hdma_read(SOCLE_HDMA_ISR01, IO_ADDRESS(dma->res->start))&(~tmp),
				    IO_ADDRESS(dma->res->start));
			/* Clear channel 1 configuration register */
		socle_hdma_write(SOCLE_HDMA_CON1, 0, IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_ISRC1, 0, IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_IDST1, 0, IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_ICNT1, 0, IO_ADDRESS(dma->res->start));		
		break;
	case 2:
			/* Clear channel 0 interrupt flag */
		tmp = SOCLE_HDMA_CH2_INT_ACT;
		socle_hdma_write(SOCLE_HDMA_ISR23,
				    socle_hdma_read(SOCLE_HDMA_ISR23, IO_ADDRESS(dma->res->start))&(~tmp),
				    IO_ADDRESS(dma->res->start));
		/* Clear channel 2 configuration register */
		socle_hdma_write(SOCLE_HDMA_CON2, 0, IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_ISRC2, 0 , IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_IDST2, 0 , IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_ICNT2, 0, IO_ADDRESS(dma->res->start));		
		break;
	case 3:
			/* Clear channel 0 interrupt flag */
		tmp = SOCLE_HDMA_CH3_INT_ACT;
		socle_hdma_write(SOCLE_HDMA_ISR23,
				    socle_hdma_read(SOCLE_HDMA_ISR23, IO_ADDRESS(dma->res->start))&(~tmp),
				    IO_ADDRESS(dma->res->start));
		/* Clear channel 3 configuration register */
		socle_hdma_write(SOCLE_HDMA_CON3, 0, IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_ISRC3, 0, IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_IDST3, 0, IO_ADDRESS(dma->res->start));
		socle_hdma_write(SOCLE_HDMA_ICNT3, 0, IO_ADDRESS(dma->res->start));		
		break;
	default:
		printk(KERN_ERR "HDMA: unknown channel number %d\n", ch);
		return;
	}
}

static irqreturn_t
socle_hdma_isr_0(int irq, void *_dma)
{
	u32 int_stat;
	struct socle_dma *dma = _dma;

	int_stat = socle_hdma_read(SOCLE_HDMA_ISR01, IO_ADDRESS(dma->res->start));
	int_stat &= SOCLE_HDMA_CH0_INT_ACT;
	if (int_stat & SOCLE_HDMA_CH0_INT_ACT){
		if (dma->notifier->complete) 
			dma->notifier->complete(dma->notifier->data);
	}
	return IRQ_HANDLED;
}

static irqreturn_t
socle_hdma_isr_1(int irq, void *_dma)
{
	u32 int_stat;
	struct socle_dma *dma = _dma;

	int_stat = socle_hdma_read(SOCLE_HDMA_ISR01, IO_ADDRESS(dma->res->start));
	int_stat &= SOCLE_HDMA_CH1_INT_ACT;

	if (int_stat & SOCLE_HDMA_CH1_INT_ACT){
		if (dma->notifier->complete)
			dma->notifier->complete(dma->notifier->data);
	}
	return IRQ_HANDLED;
}

static irqreturn_t
socle_hdma_isr_2(int irq, void *_dma)
{
	u32 int_stat;
	struct socle_dma *dma = _dma;

	int_stat = socle_hdma_read(SOCLE_HDMA_ISR23, IO_ADDRESS(dma->res->start));
	int_stat &= SOCLE_HDMA_CH2_INT_ACT;

	if (int_stat & SOCLE_HDMA_CH2_INT_ACT){
		if (dma->notifier->complete)
			dma->notifier->complete(dma->notifier->data);
	}
	return IRQ_HANDLED;
}

static irqreturn_t
socle_hdma_isr_3(int irq, void *_dma)
{
	u32 int_stat;
	struct socle_dma *dma = _dma;

	int_stat = socle_hdma_read(SOCLE_HDMA_ISR23, IO_ADDRESS(dma->res->start));
	int_stat &= SOCLE_HDMA_CH3_INT_ACT;

	if (int_stat & SOCLE_HDMA_CH3_INT_ACT){
		if (dma->notifier->complete)
			dma->notifier->complete(dma->notifier->data);
	}
	return IRQ_HANDLED;
}

static void 
socle_hdma_set_count(struct socle_dma *dma)
{
	int burst_val;
	int data_size_val=0;
	u32 inter_ch = *((u32 *)dma->private_data);

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
		printk(KERN_ERR "SQ HDMA: burst type (%d) is unknown\n", dma->burst_type);
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
		if (0 == inter_ch) 
			socle_hdma_write(SOCLE_HDMA_ICNT0, (dma->tx_cnt/data_size_val)-1, IO_ADDRESS(dma->res->start));
		else if(1 == inter_ch)
			socle_hdma_write(SOCLE_HDMA_ICNT1, (dma->tx_cnt/data_size_val)-1, IO_ADDRESS(dma->res->start));
		else if(2 == inter_ch)
			socle_hdma_write(SOCLE_HDMA_ICNT2, (dma->tx_cnt/data_size_val)-1, IO_ADDRESS(dma->res->start));
		else 
			socle_hdma_write(SOCLE_HDMA_ICNT3, (dma->tx_cnt/data_size_val)-1, IO_ADDRESS(dma->res->start));
	} else
		printk(KERN_ERR "SQ HDMA: %d is not a multiple of %d (%d * %d)\n", dma->tx_cnt, (burst_val*data_size_val), burst_val,
		       data_size_val);
}

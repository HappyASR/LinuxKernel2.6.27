#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/ide.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <asm/io.h>


#include <mach/platform.h>
#include <mach/irqs.h>
#include <mach/regs-ide.h>
#ifdef CONFIG_ARCH_PDK_PC9002
#include <mach/regs-mp-gpio.h>
#include <mach/mp-gpio.h>
#endif

#define ACSIDE_FPGA_IDE 1
#define ACSIDE_FPGA_IDE_BEST_MODE XFER_UDMA_0

static void acside_tune_drive(ide_drive_t *drive, byte pio);
static byte acside_dma_2_pio(byte xfer_rate);
static int acside_tune_chipset(ide_drive_t *drive, byte speed);
static int acside_config_drive_for_dma(ide_drive_t *drive);
static void acside_ide_setup(ide_hwif_t *hwif, unsigned long data_port, unsigned long ctrl_port, int irq);
static void acside_ide_dma_setup(ide_hwif_t *hwif, unsigned long dma_base);
static int acside_ide_dma_off_quietly(ide_drive_t *drive);
static int acside_ide_dma_on(ide_drive_t *drive);
static int acside_ide_dma_check(ide_drive_t *drive);
static void acside_ide_dma_start(ide_drive_t *drive);
static int acside_ide_dma_timeout(ide_drive_t *drive);

#ifndef ACSIDE_FPGA_IDE
static unsigned char pio_timing[5] = {0xff, 0xff, 0xff, 0x59, 0x59};
static unsigned char timing_mdma[3] = {0x88, 0x32, 0x31};
static unsigned char timing_udma[7] = {0x03, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00};
#else
//static unsigned char pio_timing[5] = {0xff, 0xff, 0xff, 0x59, 0x59};
//static unsigned char timing_mdma[3] = {0x33, 0x11, 0x11};
//static unsigned char timing_udma[7] = {0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00};
static unsigned char pio_timing[5] = {0xff, 0xff, 0xff, 0xff, 0xff};
static unsigned char timing_mdma[3] = {0xff, 0xff, 0xff};
static unsigned char timing_udma[7] = {0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf};

#endif

static void
acside_tune_drive(ide_drive_t *drive, byte pio)
{
     pio = ide_get_best_pio_mode(drive, pio, 5, NULL);
     iowrite8(pio_timing[pio], PIO_TIMING_REG);
}

static byte
acside_dma_2_pio(byte xfer_rate)
{
     switch (xfer_rate) {
     case XFER_UDMA_5:
     case XFER_UDMA_4:
     case XFER_UDMA_3:
     case XFER_UDMA_2:
     case XFER_UDMA_1:
     case XFER_UDMA_0:
     case XFER_MW_DMA_2:
     case XFER_PIO_4:
	  return 4;
     case XFER_MW_DMA_1:
     case XFER_PIO_3:
	  return 3;
     case XFER_SW_DMA_2:
     case XFER_PIO_2:
	  return 2;
     case XFER_MW_DMA_0:
     case XFER_SW_DMA_1:
     case XFER_SW_DMA_0:
     case XFER_PIO_1:
     case XFER_PIO_0:
     case XFER_PIO_SLOW:
     default:
	  return 0;
     }
}

static int
acside_tune_chipset(ide_drive_t *drive, byte speed)
{
     int err = 0;

     if (XFER_UDMA_6 == speed)
	  iowrite8(timing_udma[6], ACSIDE_IDE0_BASE+IDE_TIME_CTRL);
     else if (XFER_UDMA_5 == speed)
	  iowrite8(timing_udma[5], ACSIDE_IDE0_BASE+IDE_TIME_CTRL);
     else if (XFER_UDMA_4 == speed)
	  iowrite8(timing_udma[4], ACSIDE_IDE0_BASE+IDE_TIME_CTRL);
     else if (XFER_UDMA_3 == speed)
	  iowrite8(timing_udma[3], ACSIDE_IDE0_BASE+IDE_TIME_CTRL);
     else if (XFER_UDMA_2 == speed)
	  iowrite8(timing_udma[2], ACSIDE_IDE0_BASE+IDE_TIME_CTRL);
     else if (XFER_UDMA_1 == speed)
	  iowrite8(timing_udma[1], ACSIDE_IDE0_BASE+IDE_TIME_CTRL);
     else if (XFER_UDMA_0 == speed)
	  iowrite8(timing_udma[0], ACSIDE_IDE0_BASE+IDE_TIME_CTRL);
     else if (XFER_MW_DMA_2 == speed)
	  iowrite8(timing_mdma[2], ACSIDE_IDE0_BASE+IDE_TIME_CTRL+1);
     else if (XFER_MW_DMA_1 == speed)
	  iowrite8(timing_mdma[1], ACSIDE_IDE0_BASE+IDE_TIME_CTRL+1);
     else if (XFER_SW_DMA_2 == speed)
	  iowrite8(timing_mdma[0], ACSIDE_IDE0_BASE+IDE_TIME_CTRL+1);
     printk("speed = %x, speed & XFER_UDMA_2 = %x, TIMING_UDMA[2] = %x\n\r",
	    speed, (speed&XFER_UDMA_2), timing_udma[2]);
     acside_tune_drive(drive, acside_dma_2_pio(speed));
     if (!drive->init_speed)
	  drive->init_speed = speed;
     printk("ide_config_speed(drive, speed)\n\r");
     err = ide_config_drive_speed(drive, speed);
     printk("ide_cofnig_speed_err=%d\n\r", err);
     drive->current_speed = speed;
     return err;
}

static int 
acside_config_drive_for_dma(ide_drive_t *drive)
{
     struct hd_driveid *id = drive->id;
     byte speed;
     ide_hwif_t *hwif = HWIF(drive);

     if (id->dma_ultra & 0x0040)
	  speed = XFER_UDMA_6;
     else if (id->dma_ultra & 0x0020)
	  speed = XFER_UDMA_5;
     else if (id->dma_ultra & 0x0010)
	  speed = XFER_UDMA_4;
     else if (id->dma_ultra & 0x0008)
	  speed = XFER_UDMA_3;
     else if (id->dma_ultra & 0x0004)
	  speed = XFER_UDMA_2;
     else if (id->dma_ultra & 0x0002)
	  speed = XFER_UDMA_1;
     else if (id->dma_ultra & 0x0001)
	  speed = XFER_UDMA_0;
     else if (id->dma_mword & 0x0004)
	  speed = XFER_MW_DMA_2;
     else if (id->dma_mword & 0x0002)
	  speed = XFER_MW_DMA_1;
     else if (id->dma_1word & 0x0004)
	  speed = XFER_SW_DMA_2;
     else
	  speed = XFER_PIO_0 + ide_get_best_pio_mode(drive, 255, 5, NULL);
#ifdef ACSIDE_FPGA_IDE
     if (speed > ACSIDE_FPGA_IDE_BEST_MODE)
	  speed = ACSIDE_FPGA_IDE_BEST_MODE;
#endif
     (void)acside_tune_chipset(drive, speed);
     if (id->config & 0x8000)
	  return hwif->ide_dma_off_quietly(drive); /* CDROM dma disable */
     else if ((id->capability & 1) && hwif->autodma) {
	  /*
	   *  Enable DMA on any drive that has
	   *  UltraDMA (mode 0/1/2/3/4/5/6) enabled
	   *  */
	  if ((id->field_valid & 4) && ((id->dma_ultra >> 8) & 0x7f))
	       return hwif->ide_dma_on(drive);

	  /*
	   *  Enable DMA on any drive that has mode2 DMA
	   *  (multi or single) enabled
	   *  */
	  if (id->field_valid & 2) /* regular DMA */
	       if ((0x404 == (id->dma_mword & 0x404)) ||
		   (0x404 == (id->dma_1word & 0x404)))
		    return hwif->ide_dma_on(drive);

	  /* Consult the list of known "good" drives */
	  if (__ide_dma_good_drive(drive))
	       return hwif->ide_dma_on(drive);
     }
     return hwif->ide_dma_off_quietly(drive);
}

static int 
acside_ide_dma_off_quietly(ide_drive_t *drive)
{
     drive->using_dma = 0;

     if (HWIF(drive)->ide_dma_host_off(drive))
	  return 1;
     return 0;
}

static int
acside_ide_dma_on(ide_drive_t *drive)
{
     /* Consult the list of known "bad" drives */
     if (__ide_dma_bad_drive(drive))
	  return 1;

     drive->using_dma = 1;
     if (HWIF(drive)->ide_dma_host_on(drive))
	  return 1;
     return 0;
}

static int
acside_ide_dma_check(ide_drive_t *drive)
{
     return acside_config_drive_for_dma(drive);
}

static void
acside_ide_dma_start(ide_drive_t *drive)
{
     struct hd_driveid *id = drive->id;

     if (id->dma_ultra & 0x0040)
	  iowrite8((ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)&0xfd)|0x44, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);
     else if (id->dma_ultra & 0x0020)
	  iowrite8((ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)&0xfd)|0x44, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);
     else if (id->dma_ultra & 0x0010)
	  iowrite8((ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)&0xfd)|0x44, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);
     else if (id->dma_ultra & 0x0008)
	  iowrite8((ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)&0xfd)|0x44, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);
     else if (id->dma_ultra & 0x0004)
	  iowrite8((ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)&0xfd)|0x44, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);
     else if (id->dma_ultra & 0x0002)
	  iowrite8((ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)&0xfd)|0x44, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);
     else if (id->dma_ultra & 0x0001)
	  iowrite8((ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)&0xfd)|0x44, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);
     else if (id->dma_mword & 0x0004)
	  iowrite8((ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)&0xfb)|0x42, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);
     else if (id->dma_mword & 0x0002)
	  iowrite8((ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)&0xfb)|0x42, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);
     else if (id->dma_1word & 0x0004)
	  iowrite8((ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)&0xfb)|0x42, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);
     else
	  iowrite8((ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)&0xf9)|0x40, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);
     ide_dma_start(drive);
}

static int
acside_ide_dma_timeout(ide_drive_t *drive)
{
     unsigned long timeout;
     ide_hwif_t *hwif = HWIF(drive);

     iowrite8(ioread8(ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3)|0x40, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);

     /* Software reset */
     iowrite8(ioread8(ACSIDE_IDE0_BASE+IO_DEVCTL)|0x4, ACSIDE_IDE0_BASE+IO_DEVCTL);
     mdelay(50);
     iowrite8(ioread8(ACSIDE_IDE0_BASE+IO_DEVCTL)&0xfb, ACSIDE_IDE0_BASE+IO_DEVCTL);
     mdelay(50);

     timeout = jiffies;
     while ((!(0x50 == hwif->INB(IDE_STATUS_REG))) && time_before(jiffies, timeout+WAIT_WORSTCASE))
	  mdelay(50);
     if (!time_before(jiffies, timeout+WAIT_WORSTCASE) && (hwif->INB(IDE_STATUS_REG) & BUSY_STAT))
	  printk("Device Software Reset Time is too long! Device Status = %x.\r\n", hwif->INB(IDE_STATUS_REG));
     if (!(0x50 == hwif->INB(IDE_STATUS_REG))) {
	  printk("Software Reset Retry Again!\n");

	  /* Software reset */
	  iowrite8(ioread8(ACSIDE_IDE0_BASE+IO_DEVCTL)|0x4, ACSIDE_IDE0_BASE+IO_DEVCTL);
	  mdelay(50);
	  iowrite8(ioread8(ACSIDE_IDE0_BASE+IO_DEVCTL)&0xfb, ACSIDE_IDE0_BASE+IO_DEVCTL);
	  mdelay(50);

	  timeout = jiffies;
	  while ((!(0x50 ==hwif->INB(IDE_STATUS_REG))) && time_before(jiffies, timeout+WAIT_WORSTCASE))
	       mdelay(50);
	  if (!time_before(jiffies, timeout+WAIT_WORSTCASE) && (hwif->INB(IDE_STATUS_REG) & BUSY_STAT))
	       printk("Device Software Reset Time is too long! Device Status = %x.\r\n", hwif->INB(IDE_STATUS_REG));

     }
     return __ide_dma_timeout(drive);
}

static void
acside_ide_setup(ide_hwif_t *hwif, unsigned long data_port, unsigned long ctrl_port, int irq)
{
     unsigned long port = data_port;
     int i;

     for (i = IDE_DATA_OFFSET; i <= IDE_STATUS_OFFSET; i++) {
	  hwif->hw.io_ports[i] = port;
	  hwif->io_ports[i] = port;
	  port++;
     }
     hwif->hw.io_ports[IDE_CONTROL_OFFSET] = ctrl_port;
     hwif->io_ports[IDE_CONTROL_OFFSET] = ctrl_port;
     hwif->hw.io_ports[IDE_IRQ_OFFSET] = 0;
     hwif->hw.irq = irq;
     hwif->irq = irq;
     hwif->chipset = ide_socle;		//Add define in ide.h
     hwif->drives[0].autotune = 1;
     hwif->drives[1].autotune = 1;
     hwif->noprobe = 0;
}

static void
acside_ide_dma_setup(ide_hwif_t *hwif, unsigned long dma_base)
{
     hwif->autodma = 1;
     hwif->ide_dma_off_quietly = acside_ide_dma_off_quietly;
     hwif->ide_dma_on = acside_ide_dma_on;
     hwif->ide_dma_check = acside_ide_dma_check;
     hwif->dma_start = acside_ide_dma_start;
     hwif->ide_dma_timeout = acside_ide_dma_timeout;
     ide_setup_dma(hwif, dma_base, 8);
}

static int 
socle_ide_init(void)
{
	ide_hwif_t *hwif;
	unsigned long cmd_base, ctrl_base;

#ifdef CONFIG_ARCH_PDK_PC9002
	socle_mp_gpio_set_port_num_value(MP_PA,3,0);
	socle_mp_gpio_set_port_num_value(MP_PA,4,0);
#endif
	 

//     printk("__init ide_init_socle()\n");

	hwif = &ide_hwifs[0];
	hwif->irq = IRQ_IDE;
	cmd_base = ACSIDE_IDE0_BASE;
	ctrl_base = ACSIDE_IDE0_BASE + IO_ALTSTATUS;

	/* Transfer reset, enable channel,hardware reset */
	iowrite8(0x7e, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);

	/* Transfer reset, enable channel */
	iowrite8(0x1e, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+3);

	/* PIO tranfer I/O control */
	//iowrite8(0x59, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+1);
	iowrite8(0xff, ACSIDE_IDE0_BASE+IDE_TIME_CTRL+1);

	iowrite8(0x06, ACSIDE_IDE0_BASE+ACSIDE_BUS_MA_STATUS);

	default_hwif_mmiops(hwif);
	hwif->speedproc = acside_tune_chipset;
	acside_ide_setup(hwif, cmd_base, ctrl_base, IRQ_IDE);
#if (defined(CONFIG_ARCH_CDK) || defined(CONFIG_ARCH_SCDK) || defined(CONFIG_ARCH_PDK_PC9002))
#else
	acside_ide_dma_setup(hwif, ACSIDE_IDE0_DMA);
#endif
	probe_hwif_init(hwif);
	create_proc_ide_interfaces();
	return 0;
}

module_init(socle_ide_init);

MODULE_DESCRIPTION("SQ IDE Driver");
MODULE_LICENSE("GPL");

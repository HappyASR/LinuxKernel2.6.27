/*
 * File Name     : drivers/usb/host/ehci-socle.c 
 * Author         : ryan chen
 * Description   : EHCI HCD (Host Controller Driver) for USB
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
    
 *   Version      : 2,0,0,1
 *   History      : 
 *      1. 2006/09/11 ryan chen create this file 
*/


#include <linux/platform_device.h>

#include <mach/hardware.h>

#include <mach/irqs.h>
#include <mach/platform.h>

#ifndef CONFIG_ARCH_SOCLE
#error "EHCI CONFIG_ARCH_SOCLE must be defined."
#endif				


#ifdef CONFIG_SOCLE_EHCI_PERFORMANCE_MEASURE		//leonid+ host performance
#include <linux/jiffies.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#endif

extern int usb_disabled(void);
/* SQ EHCI USB Host Controller */

/*-------------------------------------------------------------------------*/

static void socle_start_ehc(struct platform_device *pdev)
{
	pr_debug(__FILE__ ": starting SQ EHCI USB Controller\n");


	/*
	 * Start the USB clocks.
	 */
	//clk_enable(iclk);
	//clk_enable(fclk);

	/*
	 * The USB host controller must remain in reset.
	 */
}

static void socle_stop_ehc(struct platform_device *pdev)
{
	pr_debug(__FILE__ ": stopping socle EHCI USB Controller\n");

	/*
	 * Put the USB host controller into reset.
	 */

	/*
	 * Stop the USB clocks.
	 */
}

#ifdef CONFIG_SOCLE_EHCI_PERFORMANCE_MEASURE		//leonid+ host performance

static u32 read_rate;

static void*
socle_otg_seq_start(struct seq_file *s, loff_t *pos)
{
     if (*pos > 0)
	  return NULL;
     else
	  return &read_rate;
}

static void*
socle_otg_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
     (*pos)++;
     return NULL;
}

static void 
socle_otg_seq_stop(struct seq_file *s, void *v)
{
     /* Actually, there's nothing to do here */
}

unsigned long tran_timers=0;
unsigned long tran_jiffies=0;
unsigned long tran_bytes=0;
unsigned long otg_timer_load=0;

static int socle_otg_seq_show(struct seq_file *s, void *v)
{	
	struct timespec ts;
	u32 m_sec;
	  u32 tran_speed_rate;

	jiffies_to_timespec(1, &ts);
	  m_sec = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;


	printk("tran_jiffies = %ld\n", tran_jiffies);
	printk("tran_timers = %ld\n", tran_timers);

	m_sec = (m_sec * tran_jiffies) + (m_sec * tran_timers / otg_timer_load);

	  
	  seq_printf(s, "Total time for tran: %10d ms\n", m_sec);
	  seq_printf(s, "Total bytes for tran: %10ld Bytes\n", tran_bytes);
	  if (0 == m_sec)
	       tran_speed_rate = 0;
	  else
	       tran_speed_rate = tran_bytes / m_sec;
	  seq_printf(s, "Speed rate of tran: %10d B/ms\n", tran_speed_rate);

	tran_timers=0;
	tran_jiffies=0;
	tran_bytes=0;	  
	
	return 0;  
}	

/*
 *  Tie the sequence operators up.
 *  */
static struct seq_operations socle_otg_seq_ops = {
     .start = socle_otg_seq_start,
     .next = socle_otg_seq_next,
     .stop = socle_otg_seq_stop,
     .show = socle_otg_seq_show
};

/*
 *  Now to implement the /proc file we need only make an open
 *  method which sets up the sequence operators.
 *  */
static int socle_otg_proc_open(struct inode *inode, struct file *file)
{
     return seq_open(file, &socle_otg_seq_ops);
}

/*
 *  Create a set of file operations for our proc file.
 *  */
static struct file_operations socle_otg_proc_ops = {
     .owner = THIS_MODULE,
     .open = socle_otg_proc_open,
     .read = seq_read,
     //.write = socle_otg_write,
     .llseek = seq_lseek,
     .release = seq_release
};

static struct proc_dir_entry *socle_otg_proc_entry;
#endif

/*-------------------------------------------------------------------------*/

/* configure so an HC device and id are always provided */
/* always called with process context; sleeping is OK */

/**
 * usb_ehci_socle_probe - initialize socle-based HCDs
 * Context: 
 *
 * Allocates basic resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 *
 */
int usb_ehci_socle_probe(const struct hc_driver *driver, struct platform_device *pdev)
{
	int retval;
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;
	struct resource *io_area;
	
	int			irq;

#ifdef CONFIG_SOCLE_EHCI_PERFORMANCE_MEASURE		//leonid+ host performance
	otg_timer_load = ioread32(IO_ADDRESS(SOCLE_APB0_TIMER) + 0x10);
		printk("platform device NULL \n");

     /* Install the proc_fs entry */
     socle_otg_proc_entry = create_proc_entry("sq_otg", 
					       S_IRUGO | S_IFREG,
					       &proc_root);
     if (socle_otg_proc_entry) {
	  socle_otg_proc_entry->proc_fops = &socle_otg_proc_ops;
	  socle_otg_proc_entry->data = NULL;
     } else
	  return -ENOMEM;
#endif

	if(pdev == NULL)
		printk("platform device NULL \n");
	

	if (pdev->resource[1].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		retval = -ENOMEM;
	}

	hcd = usb_create_hcd(driver, &pdev->dev, "SOCLE");
	if (!hcd)
		return -ENOMEM;
	hcd->rsrc_start = pdev->resource[0].start;
	hcd->rsrc_len = pdev->resource[0].end - pdev->resource[0].start + 1;

	/* Find and claim our resources */
	io_area = request_mem_region(hcd->rsrc_start, hcd->rsrc_len, pdev->name);
	if (NULL == io_area) {
		dev_err(&pdev->dev, "cannot reserved EHCI region\n");
		retval = -ENXIO;
		goto err2;
	}
	
	hcd->regs = (void *) (IO_ADDRESS(io_area->start));

	if (!hcd->regs) {
		pr_debug("ioremap failed");
		retval = -ENOMEM;
		goto error;
	}

	socle_start_ehc(pdev);

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs + HC_LENGTH(readl(&ehci->caps->hc_capbase));

	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = readl(&ehci->caps->hcs_params);

#ifdef CONFIG_USB_EHCI_ROOT_HUB_TT
	ehci->is_tdi_rh_tt=1;	//leonid + 
#endif

	irq = platform_get_irq(pdev, 0);
//leonid fix from SA_SHIRQ to IRQF_SHARED
	retval = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (retval == 0)
		return retval;

	/* Error handling */
//	socle_stop_hc(pdev);

//	clk_put(fclk);
//	clk_put(iclk);

//	iounmap(hcd->regs);
err2:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	
error:	
	return retval;
}

/* may be called without controller electrically present */
/* may be called with controller, bus, and devices active */

/**
 * usb_ehci_hcd_socle_remove - shutdown processing SQ HCDs
 * @dev: USB Host Controller being removed
 * Context: !in_interrupt()
 *
 * Reverses the effect of usb_ehci_hcd_socle_probe(), first invoking
 * the HCD's stop() method.  It is always called from a thread
 * context, normally "rmmod", "apmd", or something similar.
 *
 */
static int usb_ehci_socle_remove(struct usb_hcd *hcd, struct platform_device *dev)
{
	usb_remove_hcd(hcd);
	socle_stop_ehc(dev);
#ifndef CONFIG_SOCLE_USB_OTG
	iounmap(hcd->regs);
#endif
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
	return 0;
}


/*-------------------------------------------------------------------------*/

static const struct hc_driver ehci_socle_hc_driver = {
	.description = hcd_name,
	.product_desc = "EHCI",
	.hcd_priv_size = sizeof(struct ehci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq = ehci_irq,
	.flags = HCD_MEMORY | HCD_USB2,

	/*
	 * basic lifecycle operations
	 */
	.reset = ehci_init,
	.start = ehci_run,
	.stop = ehci_stop,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue = ehci_urb_enqueue,
	.urb_dequeue = ehci_urb_dequeue,
	.endpoint_disable = ehci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number = ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data = ehci_hub_status_data,
	.hub_control = ehci_hub_control,
#ifdef	CONFIG_PM
	.bus_suspend = ehci_bus_suspend,
	.bus_resume = ehci_bus_resume,
#endif
};


/*-------------------------------------------------------------------------*/

static int ehci_hcd_socle_drv_probe(struct platform_device *dev)
{

	return usb_ehci_socle_probe(&ehci_socle_hc_driver, dev);
}

static int ehci_hcd_socle_drv_remove(struct platform_device *dev)
{
	return usb_ehci_socle_remove(platform_get_drvdata(dev), dev);

}

 /*TBD*/
#ifdef CONFIG_PM
static int ehci_hcd_socle_drv_suspend(struct platform_device *dev)
{

	printk("ehci_hcd_socle_drv_suspend \n");
	return 0;
}
static int ehci_hcd_socle_drv_resume(struct platform_device *dev)
{
	//struct platform_device *pdev = to_platform_device(dev);
	//struct usb_hcd *hcd = dev_get_drvdata(dev);
	printk("ehci_hcd_socle_drv_resume \n");
	return 0;
}
#endif

static struct platform_driver ehci_hcd_socle_driver = {
	.probe 		= ehci_hcd_socle_drv_probe,
	.remove 		= ehci_hcd_socle_drv_remove,
#ifdef CONFIG_PM	
	.suspend      	= ehci_hcd_socle_drv_suspend,
	.resume       	= ehci_hcd_socle_drv_resume,
#endif	
	.driver		= {
		.name	= "SQ EHCI",
		.owner	= THIS_MODULE,
	},
};


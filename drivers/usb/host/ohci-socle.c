/*
 * File Name     : drivers/usb/host/ohci-socle.c 
 * Author         : ryan chen
 * Description   : OHCI HCD (Host Controller Driver) for USB.
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

//#include <linux/clk.h>
#include <linux/platform_device.h>

#include <mach/hardware.h>

#include <mach/irqs.h>
#include <mach/platform.h>

#ifndef CONFIG_ARCH_SOCLE
#error "OHCI CONFIG_ARCH_SOCLE must be defined."
#endif

/* interface and function clocks */
//static struct clk *iclk, *fclk;

extern int usb_disabled(void);

/*-------------------------------------------------------------------------*/

static void socle_start_hc(struct platform_device *pdev)
{
//	struct usb_hcd *hcd = platform_get_drvdata(pdev);
//	struct ohci_regs __iomem *regs = hcd->regs;

	dev_dbg(&pdev->dev, "starting SQ OHCI USB Controller\n");

	/*
	 * Start the USB clocks.
	 */
	//clk_enable(iclk);
	//clk_enable(fclk);

	/*
	 * The USB host controller must remain in reset.
	 */
}

static void socle_stop_hc(struct platform_device *pdev)
{
//	struct usb_hcd *hcd = platform_get_drvdata(pdev);
//	struct ohci_regs __iomem *regs = hcd->regs;

	dev_dbg(&pdev->dev, "stopping SQ OHCI USB Controller\n");

	/*
	 * Put the USB host controller into reset.
	 */

	/*
	 * Stop the USB clocks.
	 */
}


/*-------------------------------------------------------------------------*/


/* configure so an HC device and id are always provided */
/* always called with process context; sleeping is OK */


/**
 * usb_hcd_socle_probe - initialize SQ HCDs
 * Context: 
 *
 * Allocates basic resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 */
int usb_hcd_socle_probe (const struct hc_driver *driver, struct platform_device *pdev)
{
	int retval;
	int			irq;
	struct resource *io_area;
	struct usb_hcd *hcd = NULL;
	

	if (pdev->num_resources != 2) {
		pr_debug("hcd probe: invalid num_resources");
		return -ENODEV;
	}

	if ((pdev->resource[0].flags != IORESOURCE_MEM) || (pdev->resource[1].flags != IORESOURCE_IRQ)) {
		pr_debug("hcd probe: invalid resource type\n");
		return -ENODEV;
	}

	hcd = usb_create_hcd(driver, &pdev->dev, "SOCLE");

	if (!hcd)
		return -ENOMEM;

	hcd->rsrc_start = pdev->resource[0].start;
	hcd->rsrc_len = pdev->resource[0].end - pdev->resource[0].start + 1;

	io_area = request_mem_region(hcd->rsrc_start, hcd->rsrc_len, pdev->name);
	if (NULL == io_area) {
		dev_err(&pdev->dev, "cannot reserved EHCI region\n");
		retval = -ENXIO;
		goto err2;
	}
	
	hcd->regs = (void *) (IO_ADDRESS(io_area->start));

	if (!hcd->regs) {
		pr_debug("ioremap failed\n");
		retval = -EIO;
		goto err2;
	}

	socle_start_hc(pdev);
	ohci_hcd_init(hcd_to_ohci(hcd));

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

// err1:
//	usb_put_hcd(hcd);
	return retval;
}


/* may be called with controller, bus, and devices active */

/**
 * usb_hcd_socle_remove - shutdown processing for SOCLE-HCDs
 * @dev: USB Host Controller being removed
 * Context: !in_interrupt()
 *
 * Reverses the effect of usb_hcd_socle_probe(), first invoking
 * the HCD's stop() method.  It is always called from a thread
 * context, normally "rmmod", "apmd", or something similar.
 *
 */
static int usb_hcd_socle_remove (struct usb_hcd *hcd, struct platform_device *pdev)
{
	usb_remove_hcd(hcd);
	socle_stop_hc(pdev);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);

//	clk_put(fclk);
//	clk_put(iclk);
//	fclk = iclk = NULL;

	return 0;
}

/*-------------------------------------------------------------------------*/

static int __devinit
ohci_socle_start (struct usb_hcd *hcd)
{
//	struct socle_ohci_data	*board = hcd->self.controller->platform_data;
	struct ohci_hcd		*ohci = hcd_to_ohci (hcd);
	int			ret;

	if ((ret = ohci_init(ohci)) < 0)
		return ret;

	//printk("ohci_socle_start \n");
	if ((ret = ohci_run(ohci)) < 0) {
		err("can't start %s", hcd->self.bus_name);
		ohci_stop(hcd);
		return ret;
	}

//	hcd->self.root_hub->maxchild = board->ports;
	return 0;
}

/*-------------------------------------------------------------------------*/

static const struct hc_driver ohci_socle_hc_driver = {
	.description =		hcd_name,
	.product_desc =		"OHCI",
	.hcd_priv_size =	sizeof(struct ohci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq =			ohci_irq,
	.flags =		HCD_USB11 | HCD_MEMORY,

	/*
	 * basic lifecycle operations
	 */
	.start =		ohci_socle_start,
	.stop =			ohci_stop,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		ohci_urb_enqueue,
	.urb_dequeue =		ohci_urb_dequeue,
	.endpoint_disable =	ohci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number =	ohci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data =	ohci_hub_status_data,
	.hub_control =		ohci_hub_control,

#ifdef CONFIG_PM
	.bus_suspend =		ohci_bus_suspend,
	.bus_resume =		ohci_bus_resume,
#endif
	.start_port_reset =	ohci_start_port_reset,
};

/*-------------------------------------------------------------------------*/

static int ohci_hcd_socle_drv_probe(struct platform_device *dev)
{
	return usb_hcd_socle_probe(&ohci_socle_hc_driver, dev);
}

static int ohci_hcd_socle_drv_remove(struct platform_device *dev)
{
	return usb_hcd_socle_remove(platform_get_drvdata(dev), dev);
}

#ifdef CONFIG_PM

/* REVISIT suspend/resume look "too" simple here */


static int ohci_hcd_socle_drv_suspend(struct platform_device *dev)
{
	//disable clock
	printk("ohci_hcd_socle_drv_suspend \n");
	
	return 0;
}

static int ohci_hcd_socle_drv_resume(struct platform_device *dev)
{
	//enable clock
	printk("ohci_hcd_socle_drv_resume \n");
	return 0;
}
#else
#define ohci_hcd_socle_drv_suspend NULL
#define ohci_hcd_socle_drv_resume  NULL
#endif

MODULE_ALIAS("SQ OHCI");

static struct platform_driver ohci_hcd_socle_driver = {
	.probe		= ohci_hcd_socle_drv_probe,
	.remove		= ohci_hcd_socle_drv_remove,
	.suspend		= ohci_hcd_socle_drv_suspend,
	.resume		= ohci_hcd_socle_drv_resume,
	.driver		= {
		.name	= "SQ OHCI",
		.owner	= THIS_MODULE,
	},
};

static char __initdata banner[] = "SQ OHCI, (c) 2006 SQ Corp. \n";

static int __init ohci_hcd_socle_init (void)
{
	if (usb_disabled())
		return -ENODEV;

	printk(banner);
	
	return platform_driver_register(&ohci_hcd_socle_driver);
}

static void __exit ohci_hcd_socle_cleanup (void)
{
	platform_driver_unregister(&ohci_hcd_socle_driver);
}

module_init (ohci_hcd_socle_init);
module_exit (ohci_hcd_socle_cleanup);

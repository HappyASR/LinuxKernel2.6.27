/********************************************************************************
* File Name     : linux/arch/arm/mach-socle/pci.c 
* Author         : cyli
* Description   : Socle host bridge driver (A2PII)
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
*      1. 2006/08/25 cyli create this file 
*    
********************************************************************************/

#include <linux/pci.h>
#include <asm/mach/pci.h>
#include <asm/mach-types.h>
#include <linux/spinlock.h>

#include <mach/platform.h>
#include <mach/regs-pci.h>
#include <mach/irqs.h>

#ifdef CONFIG_PCI_DEBUG
#define DBG(fmt, args...) printk(KERN_DEBUG "SQ_PCI : " fmt, ## args)
#else
#define DBG(fmt, args...)
#endif

#define SOCLE_PCI_WRITE(ofs, data)  \
	__raw_writel(data, SOCLE_PCI_BR_VADDR + (unsigned int)(ofs))
#define SOCLE_PCI_READ(ofs, data)   \
	data = (__raw_readl(SOCLE_PCI_BR_VADDR + (unsigned int)(ofs)))

static DEFINE_SPINLOCK(socle_lock);

static void socle_config_access(unsigned int bus_nr, unsigned int devfn, int where)
{
	/*
	DBG("sq_config_access bus = %02x, devfun = %02x, where = %02x\n", bus_nr, devfn, where);
	DBG("Write CFGADDR at address %08x with %08x\n"
			, SOCLE_PCI_CFGADDR_OFS
			,(bus_nr         << SOCLE_PCI_CFGADDR_BUSNUM_SHF)   |
	   		 (devfn      << SOCLE_PCI_CFGADDR_FUNCTNUM_SHF) |
	   		 ((where / 4) << SOCLE_PCI_CFGADDR_REGNUM_SHF)   |
	   		 SOCLE_PCI_CFGADDR_CONFIGEN_BIT);
	*/
	   		 
	if((bus_nr > 1) || (PCI_SLOT(devfn) >= 32) || (PCI_FUNC(devfn) >=8))
		printk("Error: bus = %02x, slot = %02x, fun = %02x !!\n", bus_nr, PCI_SLOT(devfn), PCI_FUNC(devfn));

	SOCLE_PCI_WRITE(SOCLE_PCI_CFGADDR_OFS,
	   (bus_nr         << SOCLE_PCI_CFGADDR_BUSNUM_SHF)   |
	   (devfn      << SOCLE_PCI_CFGADDR_FUNCTNUM_SHF) |
	   ((where / 4) << SOCLE_PCI_CFGADDR_REGNUM_SHF)   |
	   SOCLE_PCI_CFGADDR_CONFIGEN_BIT);
}

/*
 * Mask table, bits to mask for quantity of size 1, 2 or 4 bytes.
 * 0 and 3 are not valid indexes...
 */
static u32 bytemask[] = {
	/*0*/	0,
	/*1*/	0xff,
	/*2*/	0xffff,
	/*3*/	0,
	/*4*/	0xffffffff,
};

static int socle_read_config(struct pci_bus *bus, unsigned int devfn, int where,
			  int size, u32 *val)
{
	unsigned long flags;
	u32 data = 0;

	/*
	DBG("In socle_read_config(%x) %x from dev %x:%x:%x ", size, where,
		bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn));
	*/
	
	spin_lock_irqsave(&socle_lock, flags);
	
	socle_config_access(bus->number, devfn, where);
	SOCLE_PCI_READ(SOCLE_PCI_CFGDATA_OFS, data);
	*val = (data >> ((where & 3) << 3)) & bytemask[size];
	
	/*
	DBG("val = 0x%08x\n", *val);
	*/

	spin_unlock_irqrestore(&socle_lock, flags);

	return PCIBIOS_SUCCESSFUL;
}

static int socle_write_config(struct pci_bus *bus, unsigned int devfn, int where,
			   int size, u32 val)
{
	unsigned long flags;
	u32 data = 0;

	/*
	DBG("In socle_write_config(%x) [%x]=%x from dev %x:%x:%x\n", size, where, val,
		bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn));
	*/
	
	spin_lock_irqsave(&socle_lock, flags);
	
	socle_config_access(bus->number, devfn, where);
	SOCLE_PCI_READ(SOCLE_PCI_CFGDATA_OFS, data);
	
	data = (data & ~(bytemask[size] << ((where & 3) << 3))) | 
			(val << ((where & 3) << 3));

	socle_config_access(bus->number, devfn, where);
	SOCLE_PCI_WRITE(SOCLE_PCI_CFGDATA_OFS, data);
	
	spin_unlock_irqrestore(&socle_lock, flags);

	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops pci_socle_ops = {
	.read	= socle_read_config,
	.write	= socle_write_config,
};

/**
 * Socle resources structure
 */
static struct resource socle_pci_mem = {
	.name	= "Socle PCI non-prefetchable memory",
	.start	= SOCLE_PCI_MEM_BASE,
	.end		= SOCLE_PCI_MEM_BASE + SOCLE_PCI_MEM_SIZE - 1,
	.flags	= IORESOURCE_MEM,
};

static struct resource socle_io_port  = {
	.name	= "Socle PCI ioports",
	.start	= SOCLE_PCI_IO_BASE,
	.end		= SOCLE_PCI_IO_BASE + SOCLE_PCI_IO_SIZE - 1,
	.flags	= IORESOURCE_IO,
};

struct pci_bus * __devinit pci_socle_scan_bus(int nr, struct pci_sys_data *sys)
{
	DBG("pci_socle_scan_bus %d: ...\n", sys->busnr);
	return pci_scan_bus(sys->busnr, &pci_socle_ops, sys);
}

static int __init pci_socle_setup_resources(struct resource **resource)
{
	if (request_resource(&iomem_resource, &socle_pci_mem)) {
		printk(KERN_ERR "PCI: unable to allocate non-prefetchable "
		       "memory region\n");
		return -EBUSY;
	}
	if (request_resource(&ioport_resource, &socle_io_port)) {
		release_resource(&socle_pci_mem);
		printk(KERN_ERR "PCI: unable to allocate prefetchable "
		       "memory region\n");
		return -EBUSY;
	}

	/*
	 * bus->resource[0] is the IO resource for this bus
	 * bus->resource[1] is the mem resource for this bus
	 * bus->resource[2] is the prefetch mem resource for this bus
	 */
	resource[0] = &socle_io_port;
	resource[1] = &socle_pci_mem;
	resource[2] = NULL;

/*
	resource[0] = &ioport_resource;
	resource[1] = &non_mem;
	resource[2] = &pre_mem;
*/
	
	return 1;
}

int __init pci_socle_setup(int nr, struct pci_sys_data *sys)
{
	int ret = 0;

	if (nr == 0) {
		sys->mem_offset = 0;
		ret = pci_socle_setup_resources(sys->resource);
	}

	return ret;
}

static u8 __init socle_swizzle(struct pci_dev *dev, u8 *pinp)
{
	return PCI_SLOT(dev->devfn);
}

static int irq_tab[2][4] __initdata = {
	{IRQ_PCI1_A,IRQ_PCI1_B,IRQ_PCI1_C,IRQ_PCI1_D},
	{IRQ_PCI2_A,IRQ_PCI2_B,IRQ_PCI2_C,IRQ_PCI2_D}
};

static int __init socle_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	DBG("sq_map_irq: slot=%d pin=%d irq=%d\n", slot, pin, irq_tab[slot-1][pin-1]);
	return(irq_tab[slot-1][pin-1]);
}

void __init pci_socle_preinit(void)
{
	unsigned long flags;

	DBG("pci_socle_preinit ...\n");
	
	spin_lock_irqsave(&socle_lock, flags);

   /* Setup PCI be Host Function.  */
	SOCLE_PCI_WRITE(SOCLE_PCI_CFGHOST_OFS, 0x00000001);
//#ifdef CONFIG_MMU
	SOCLE_PCI_WRITE(SOCLE_PCI_CFGUPATU_OFS, 0x00000004);//translate AHB address to 0x40000000
//#else
//	SOCLE_PCI_WRITE(SOCLE_PCI_CFGUPATU_OFS, 0x00000000);//translate AHB address to 0x00000000
//#endif
	SOCLE_PCI_WRITE(SOCLE_PCI_CFGBAR0_OFS, 0x00000008); //Enable 256MB

	spin_unlock_irqrestore(&socle_lock, flags);
}

void __init pci_socle_postinit(void)
{
	DBG("pci_socle_postinit ...\n");
}

static void __devinit pci_fixup_socle(struct pci_dev *dev)
{
	u32 tmp32 = 0;

	printk("PCI: pci_fixup_socle ");
	printk("Vendor: %04x, Device: %04x\n", 
					dev->vendor, dev->device);

	
	DBG("Fixup resource!\n");
	dev->resource[0].start = 0;
	dev->resource[0].end   = 0;
	dev->resource[0].flags = 0;
	
	dev->resource[1].start = 0;
	dev->resource[1].end   = 0;
	dev->resource[1].flags = 0;
	
	dev->resource[2].start = 0;
	dev->resource[2].end   = 0;
	dev->resource[2].flags = 0;
	
	dev->resource[6].start = 0;
	dev->resource[6].end   = 0;
	dev->resource[6].flags = 0;

	
	pci_write_config_word (dev, PCI_COMMAND, PCI_COMMAND_SERR | PCI_COMMAND_PARITY | 
							PCI_COMMAND_INVALIDATE | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY | PCI_COMMAND_IO);
			
	/*
	 * write the cacheline of host bridge
	 */
	//pci_write_config_byte (dev, PCI_CACHE_LINE_SIZE, 0x10);

	/*
	 * write the latency timer of host bridge
	 */
	//pci_write_config_byte (dev, PCI_LATENCY_TIMER, 0x10);

	/*
	 * write the BAR of host bridge
	 */
//#ifdef CONFIG_MMU
	pci_write_config_dword (dev, PCI_BASE_ADDRESS_0, 0x40000000);
//#else
//	pci_write_config_dword (dev, PCI_BASE_ADDRESS_0, 0x00000000);
//#endif
	pci_read_config_dword (dev, PCI_BASE_ADDRESS_0, &tmp32);
	printk ("SQ PCI Bridge Base: %08x\n", tmp32);
}

DECLARE_PCI_FIXUP_HEADER(PCI_VENDOR_ID_SOCLE, PCI_DEVICE_ID_SOCLE, pci_fixup_socle);

static struct hw_pci socle_pci __initdata = {
	.swizzle		= socle_swizzle,
	.map_irq		= socle_map_irq,
	.setup			= pci_socle_setup,
	.nr_controllers	= 1,
	.scan			= pci_socle_scan_bus,
	.preinit		= pci_socle_preinit,
	.postinit		= pci_socle_postinit,
};

static int __init socle_pci_init(void)
{
	if (machine_is_socle())
		pci_common_init(&socle_pci);

	return 0;
}

subsys_initcall(socle_pci_init);


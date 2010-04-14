/*======================================================================

    drivers/mtd/maps/socle-flash.c: Socle flash map driver

    Copyright (C) 2007 Socle Tech. Corp.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

======================================================================*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/init.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <asm/mach/flash.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/system.h>
#include <mach/platform.h>

#ifdef CONFIG_MTD_BAST_MAXSIZE
#define AREA_MAXSIZE (CONFIG_MTD_BAST_MAXSIZE * SZ_1M)
#else
#define AREA_MAXSIZE (16 * SZ_1M)
#endif

#define PFX "sq-flash: "

struct socle_flash_info {
	struct mtd_info		*mtd;
	struct map_info		 map;
	struct mtd_partition	*partitions;
	struct resource		*area;
};

//#ifdef CONFIG_MTD_PARTITIONS
//static const char *probes[] = { "u-boot", "kernel", "ramdisk", "data", NULL };
//#endif

static int socle_flash_remove(struct platform_device *pdev)
{
	struct socle_flash_info *info = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);

	if (info == NULL)
		return 0;

	if (info->map.virt != NULL)
		iounmap(info->map.virt);

	if (info->mtd) {
		del_mtd_partitions(info->mtd);
		map_destroy(info->mtd);
	}

	kfree(info->partitions);

	if (info->area) {
		release_resource(info->area);
		kfree(info->area);
	}

	kfree(info);

	return 0;
}

static int socle_flash_probe(struct platform_device *pdev)
{
	struct socle_flash_info *info;
	struct flash_platform_data *pdata = pdev->dev.platform_data;
	struct resource *res = pdev->resource;
	unsigned long size = res->end - res->start + 1;

	int err = 0;
	static int no_partitions;
	info = kmalloc(sizeof(struct socle_flash_info), GFP_KERNEL);
	if (info == NULL) {
		printk(KERN_ERR PFX "no memory for flash info\n");
		return -ENOMEM;
	}

	memzero(info, sizeof(struct socle_flash_info));

	if (!request_mem_region(res->start, size, "flash")) {
		err = -EBUSY;
	}
	
	
	platform_set_drvdata(pdev, info);

	info->map.virt = (void *) IO_ADDRESS(res->start);
//	info->map.virt = (void *) IO_ADDRESS(SOCLE_NOR_FLASH);		
	if (!info->map.virt) {
		err = -ENOMEM;
	}	
	info->map.phys = res->start;
	info->map.size = res->end - res->start + 1;
	info->map.name = pdev->dev.bus_id;
#ifdef CONFIG_MTD_MAP_BANK_WIDTH_1
		info->map.bankwidth = 1;
#endif
#ifdef CONFIG_MTD_MAP_BANK_WIDTH_2
		info->map.bankwidth = 2;
#endif	

//	printk("%s %x, %x, %s \n",__FUNCTION__,res->start,res->end,pdev->dev.bus_id);

	if (info->map.size > AREA_MAXSIZE)
		info->map.size = AREA_MAXSIZE;

//	printk("%s: area %08lx, size %lx\n", __FUNCTION__, info->map.phys, info->map.size);


//	printk("%s: virt at %08x, res->start %x \n", __FUNCTION__, (int)info->map.virt,res->start);

	if (info->map.virt == 0) {
		printk(KERN_ERR PFX "failed to ioremap() region\n");
		err = -EIO;
		goto exit_error;
	}

	info->partitions = pdata->parts;

	simple_map_init(&info->map);

	/* probe for the device(s) */

	info->mtd = do_map_probe(pdata->map_name, &info->map);
	if (!info->mtd) {
		err = -EIO;
		goto exit_error;
	}

	/* mark ourselves as the owner */
	info->mtd->owner = THIS_MODULE;

//	err = parse_mtd_partitions(info->mtd, probes, &info->partitions, 0);
//	if (err > 0) {
//		printk("parse error \n");
		no_partitions = pdata->nr_parts;//ARRAY_SIZE(nor_partitions);
//		info->partitions = nor_partitions;
		err = add_mtd_partitions(info->mtd, info->partitions, no_partitions);
		if (err)
			printk(KERN_ERR PFX "cannot add/parse partitions\n");
//	} else {
//		err = add_mtd_device(info->mtd);
//	}

	return 0;

	/* fall through to exit error */

 exit_error:
	kfree(info);
	socle_flash_remove(pdev);
	return err;
}

#ifdef CONFIG_PM
static int
socle_flash_suspend(struct platform_device *pdev, pm_message_t msg)
{
	pr_debug("sq_flash_suspend\n");

        return 0;
}

static int 
socle_flash_resume(struct platform_device *pdev)
{	
	pr_debug("sq_flash_resume\n");
	
  	return 0;
}
#else
#define socle_flash_suspend NULL
#define socle_flash_resume NULL
#endif
	
static struct platform_driver socle_flash_driver = {
	.probe		= socle_flash_probe,
	.remove		= socle_flash_remove,
	.suspend = socle_flash_suspend,
	.resume = socle_flash_resume,
	.driver		= {
		.name	= "sq-flash",
		.owner	= THIS_MODULE,
	},
};

static int __init socle_flash_init(void)
{
	printk("SQ NOR-Flash Driver, (c) 2007 SQ Tech \n");
	return platform_driver_register(&socle_flash_driver);
}

static void __exit socle_flash_exit(void)
{
	platform_driver_unregister(&socle_flash_driver);
}

module_init(socle_flash_init);
module_exit(socle_flash_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ryan Chen <ryanchen@socle-tech.com.tw>");
MODULE_DESCRIPTION("SQ MTD Map driver");


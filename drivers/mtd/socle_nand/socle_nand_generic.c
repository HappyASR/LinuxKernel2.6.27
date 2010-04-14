#include <asm/mach/flash.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include "socle_nand.h"
#include <linux/mtd/partitions.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

/*
 *  MTD structure for Socle
 *  */
static struct mtd_info *socle_mtd = NULL;

/*
 *  Define  partitions for flash device
 *  */
//static int nr_partitions;

#ifdef CONFIG_MTD_PARTITIONS
const char *part_probes[] = {"cmdlinepart", NULL};
#endif

/*
 *  Main initialization routine
 *  */
static int __init socle_nand_probe(struct platform_device *pdev) 
{
     struct nand_chip *chip;
	 	static int no_partitions;
     struct flash_platform_data *pdata = (struct flash_platform_data *)pdev->dev.platform_data;
//	struct mtd_partition socle_nand_default_partition_info[];

//     struct mtd_partition *socle_partition_info;
     int err = 0;

     /* Allocate memory for MTD device structure and private data */
     socle_mtd = kmalloc(sizeof(struct mtd_info)+sizeof(struct nand_chip), GFP_KERNEL);
     if (!socle_mtd) {
	  printk(KERN_ERR"Unable to allocate Socle NAND MTD device structure.\n");
	  return -ENOMEM;
     }

     /* Get pointer to private data */
     chip = (struct nand_chip *)(&socle_mtd[1]);

     /* Initialize structures */
     memset(socle_mtd, 0, sizeof(struct mtd_info));
     memset(chip, 0, sizeof(struct nand_chip));

     /* Link the private data with the MTD structure */
     socle_mtd->priv = chip;
     socle_mtd->owner = THIS_MODULE;

#ifdef CONFIG_MTD_SOCLE_NAND_USE_FLASH_BBT
     chip->options |= NAND_USE_FLASH_BBT;
#endif
#ifdef CONFIG_MTD_SOCLE_NAND_WITHOUT_HWECC
     chip->options |= NAND_RW_RAW
#endif

     /* Scan to find existence of the device */
     err = nand_scan(socle_mtd, 1);
     if (err) {
	  kfree(socle_mtd);
	  printk(KERN_INFO "nand_scan failed, err:%d\n", err);
	  return err;
     }

#if 0
     /* Adjust the first partition's start offset  */
     socle_nand_default_partition_info[0].offset = socle_mtd->erasesize + (128 * 1024);

     /* Register the partitions */
     socle_mtd->name = "socle-nand";
     nr_partitions = parse_mtd_partitions(socle_mtd, part_probes, &socle_partition_info, 0);
     if (nr_partitions <= 0) {
	  nr_partitions = DEFAULT_NUM_PARTITIONS;
	  socle_partition_info = socle_nand_default_partition_info;
     }
     add_mtd_partitions(socle_mtd, socle_partition_info, nr_partitions);
#else

	no_partitions = pdata->nr_parts;
	err = add_mtd_partitions(socle_mtd, pdata->parts, no_partitions);
#endif

     return 0;
}

/*
 *  socle_nand_remove
 *  */
static int __devexit
socle_nand_remove(struct platform_device *pdev)
{
     /* Release resources, unregister device */
     nand_release(socle_mtd);

     /* Free the MTD device structure */
     kfree(socle_mtd);
	return 0;
}

#ifdef CONFIG_PM
static int
socle_nand_suspend(struct platform_device *pdev, pm_message_t msg)
{
	pr_debug("sq_nand_suspend\n");

        return 0;
}

static int 
socle_nand_resume(struct platform_device *pdev)
{	
	pr_debug("sq_nand_resume\n");
	
  	return 0;
}
#else
#define socle_nand_suspend NULL
#define socle_nand_resume NULL
#endif

static struct platform_driver socle_nand_driver = {
	.probe		= socle_nand_probe,
	.remove		= __devexit_p(socle_nand_remove),
	.suspend	= socle_nand_suspend,
	.resume		= socle_nand_resume,
	.driver		= {
		.name	= "socle-nand",
		.owner	= THIS_MODULE,
	},
};

static int __init socle_nand_init(void)
{
	return platform_driver_register(&socle_nand_driver);
}

static void __exit socle_nand_exit(void)
{
	platform_driver_unregister(&socle_nand_driver);
}

module_init(socle_nand_init);
module_exit(socle_nand_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Obi Hsieh");
MODULE_DESCRIPTION("Device specific logic for generic NAND flash chips on Socle NAND flash controller");

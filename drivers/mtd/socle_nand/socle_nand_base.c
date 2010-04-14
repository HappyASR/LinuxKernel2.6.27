#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/mtd/mtd.h>
//#include <asm/arch/socle_nand_regs.h>
#include <mach/socle_nand_regs.h>
#include "socle_nand.h"
#include "rscode-1.0_socle/ecc.h"
#include <linux/bitops.h>
#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/ioport.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/time.h>

//#define TIMEING_TUNE

#ifdef CONFIG_MTD_PARTITIONS
#include <linux/mtd/partitions.h>
#endif

/* MTD function prototypes */
static int nand_erase(struct mtd_info *mtd, struct erase_info *instr);
static int nand_read(struct mtd_info *mtd, loff_t from, size_t len, 
		     size_t *retlen, u_char *buf);
static int nand_read_oob(struct mtd_info *mtd, loff_t from , struct mtd_oob_ops *ops);
static int nand_write(struct mtd_info *mtd, loff_t to, size_t len,
		      size_t *retlen, const u_char *buf);
static int nand_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);
static int nand_block_isbad(struct mtd_info *mtd, loff_t ofs);
static int nand_block_markbad(struct mtd_info *mtd, loff_t ofs);
static void nand_sync(struct mtd_info *mtd);
static int nand_suspend(struct mtd_info *mtd);
static void nand_resume(struct mtd_info *mtd);

/* Internal function prototypes */
static int nand_do_read_ops(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
static int nand_do_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
static void nand_transfer_oob(struct mtd_info *mtd, struct nand_chip *chip, u8 *oob,
			      u32 page, struct mtd_oob_ops *ops);
static int nand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
				u_char *buf);
static int nand_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
			      u_char *buf);
static int nand_do_write_ops(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);
static int nand_do_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);
static int nand_program_page(struct mtd_info *mtd, struct nand_chip *chip, const uint8_t *buf,
			     u32 page, int cached);
static int nand_verify_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
				  const u_char *buf);
static int nand_verify_page_raw(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
				const u_char *buf);
static int nand_fill_oob(struct mtd_info *mtd, struct nand_chip *chip, uint8_t *oob,
			 u32 page, struct mtd_oob_ops *ops);
static int nand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
				 const u_char *buf, int cached);
static int nand_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
			       const u_char *buf, int cached);
static int nand_block_checkbad(struct mtd_info *mtd, loff_t ofs, int getchip,
			       int allowbt);
static int nand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip);
static int nand_default_block_markbad(struct mtd_info *mtd, loff_t ofs);
static int nand_get_device(struct nand_chip *chip, int new_state);
static void nand_release_device(struct nand_chip *chip);
static void nand_select_chip(struct nand_chip *chip, int chipnr);
static int nand_check_wp(struct nand_chip *chip, int chipnr);
static int nand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip);
static void nand_read_buf(struct nand_chip *chip, u8 *buf, u32 len);
static void nand_write_buf(struct nand_chip *chip, const u8 *buf, u32 len);
static int nand_correct_data(struct nand_chip *chip, struct mtd_info *mtd, u8 *buf, 
			     int status , u32 page);
static int nand_chip_isscanned(struct mtd_info *mtd, int chipnr);
static int nand_chip_markscanned(struct mtd_info *mtd, int chipnr);
static int nand_scan_initial_badblock(struct mtd_info *mtd);
static int nand_scan_bad(struct mtd_info *mtd, struct nand_chip *chip, int chipnr);
static int scan_block_full(struct mtd_info *mtd, struct nand_bbt_descr *bd, loff_t offs,
			   uint8_t *buf, size_t readlen, int scanlen,
			   int len);
static int check_pattern(uint8_t *buf, int len, int paglen,
			 struct nand_bbt_descr *td);
static int scan_block_fast(struct mtd_info *mtd, struct nand_bbt_descr *bd, loff_t offs,
			   uint8_t *buf, int len);
static int check_short_pattern(uint8_t *buf, struct nand_bbt_descr *td);
static ssize_t socle_nand_write(struct file *filp, const char __user *buf, size_t count,
				loff_t *f_pos);
static int socle_nand_proc_open(struct inode *inode, struct file *file);
static void* socle_nand_seq_start(struct seq_file *s, loff_t *pos);
static void* socle_nand_seq_next(struct seq_file *s, void *v, loff_t *pos);
static void socle_nand_seq_stop(struct seq_file *s, void *v);
static int socle_nand_seq_show(struct seq_file *s, void *v);
static void socle_nand_timer_expiry(unsigned long data);

/* NAND controller low level function prototypes*/
static u32 nand_read_id(struct nand_chip *chip);
static int nand_erase_block_l(struct nand_chip *chip, u32 page_addr);
static int nand_read_l(struct nand_chip *chip, u32 column, u32 page,
		       u32 len, int ecc);
static int nand_program_l(struct nand_chip *chip, u32 column, u32 page,
			  u32 len, int ecc);
static int nand_cache_program_l(struct nand_chip *chip, u32 column, u32 page,
				u32 len, int ecc);
static int nand_erase_block_s(struct nand_chip *chip, u32 page);
static int nand_read_s(struct nand_chip *chip, u32 column, u32 page,
		       u32 len, int ecc);
static int nand_program_s(struct nand_chip *chip, u32 column, u32 page,
			  u32 len, int ecc);

/* Define default oob placement schemes for large nand small page devices */
#ifdef	CONFIG_NAND_15B	//leonid+
static struct nand_ecclayout nand_oob_16_raw = {
     .eccbytes = 0,
     .eccpos = {0},
     .oobfree = {
	   {.offset = 0,
	    .length = 16}
      }
};
static struct nand_ecclayout nand_oob_64_raw = {
     .eccbytes = 0,
     .eccpos = {0},
     .oobfree = {
	   {.offset = 0,
	    .length = 64}
      }
};
static struct nand_ecclayout nand_oob_16_hwecc = {
     .eccbytes = 15,
     .eccpos = {0},
     .oobfree = {
	   {.offset = 15,
	    .length = 1}
      }
};
static struct nand_ecclayout nand_oob_64_hwecc = {
     .eccbytes = 24,
     .eccpos = {0},
     .oobfree = {
	   {.offset = 60,
	    .length = 4}
      }
};
#else
static struct nand_ecclayout nand_oob_16_raw = {
     .eccbytes = 0,
     .eccpos = {0},
     .oobfree = {
	   {.offset = 0,
	    .length = 16}
      }
};
static struct nand_ecclayout nand_oob_64_raw = {
     .eccbytes = 0,
     .eccpos = {0},
     .oobfree = {
	   {.offset = 0,
	    .length = 64}
      }
};
static struct nand_ecclayout nand_oob_16_hwecc = {
     .eccbytes = 8,
     .eccpos = {0},
     .oobfree = {
	   {.offset = 8,
	    .length = 8}
      }
};
static struct nand_ecclayout nand_oob_64_hwecc = {
     .eccbytes = 32,
     .eccpos = {0},
     .oobfree = {
	   {.offset = 32,
	    .length = 32}
      }
};
#endif

#define AHB_MHZ(x) ((x) * 1000 * 1000)

#define BLK_CNT NAND_CHIP_SIZE_1G  
#define NAND_CHIP_SIZE_2G	2048	// 2G block numbers
#define NAND_CHIP_SIZE_1G	1024	// 1G block numbers

struct socle_nand_statistic {
     u32 wear_statistic[BLK_CNT];
     u32 erase_jiffies;
     u32 erase_count;
     u32 read_jiffies;
     u32 read_bytes;
     u32 write_jiffies;
     u32 write_bytes;
};

static struct socle_nand_statistic *socle_nand_statistic = NULL;

#ifdef CONFIG_MTD_SOCLE_NAND_DEBUG_VERBOSE
u32 socle_nand_debug_level = CONFIG_MTD_SOCLE_NAND_DEBUG_VERBOSE;
#else
u32 socle_nand_debug_level = 0;
#endif

#ifdef CONFIG_MTD_SOCLE_NAND_PERFORMANCE_MEASURE
unsigned long jiffies_before, jiffies_after, jiffies_diff;
#endif

/**
 * nand_erase - [MTD Interface] erase block(s)
 * @mtd:	MTD device structure
 * @instr:	erase instruction
 *
 * Erase one ore more blocks
 */
static int 
nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL0, "\nsocle_nand_host: nand_erase()\n");
     return nand_erase_nand(mtd, instr, 0);
}

#define BBT_PAGE_MASK 0xFFFFFF3F
/**
 * nand_erase_nand - [Internal] erase block(s)
 * @mtd:	MTD device structure
 * @instr:	erase instruction
 * @allowbbt:	allow erasing the bbt area
 *
 * Erase one ore more blocks
 */
extern int
nand_erase_nand(struct mtd_info *mtd, struct erase_info *instr, int allowbbt)
{
     int page, len, pages_per_block, ret, chipnr;
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     int rewrite_bbt[NAND_MAX_CHIPS] = {0};
     unsigned int bbt_masked_page = 0xffffffff;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL1, "\nsocle_nand_host: nand_erase_nand() start = 0x%08x, len = %d\n",
		      instr->addr, instr->len);

     /* Start address must align on block boundary */
     if (instr->addr & ((1 << chip->phys_erase_shift) - 1)) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_erase_nand(): Unaligned address\n");
	  return -EINVAL;
     }

     /* Length must align on block boundary */
     if (instr->len & ((1 << chip->phys_erase_shift) - 1)) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_erase_nand(): Length not block aligned\n");
	  return -EINVAL;
     }

     /* Do not allow erase past end of device */
     if ((instr->len + instr->addr) > mtd->size) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_erase_nand(): Erase past end of device\n");
	  return -EINVAL;
     }

     instr->fail_addr = 0xffffffff;

     /* Grab the lock and set if the device is available */
     nand_get_device(chip, FL_ERASING);

     /* Shift to get first page */
     page = (int)(instr->addr >> chip->page_shift);
     chipnr = (int)(instr->addr >> chip->chip_shift);

     /* Calculate pages in each block */
     pages_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);

     /* Select the NAND device */
     nand_select_chip(chip, chipnr);

     /* Check, if it is write protected */
     if (nand_check_wp(chip, chipnr)) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_erase_nand(): Device is write protected!!!\n");
	  instr->state = MTD_ERASE_FAILED;
	  goto erase_exit;
     }

     /*
      *  If BBT requires refresh, set the BBT page mask to see if the BBT
      *  should be rewritten, Otherwise the mask is set to 0xffffffff which
      *  can not be matched. This is also done when the bbt is actually
      *  erased to avoid recursive updates
      *  */
     if ((chip->options & BBT_AUTO_REFRESH) && !allowbbt)
	  bbt_masked_page = chip->bbt_td->pages[chipnr] & BBT_PAGE_MASK;

     /* Loop through the pages */
     len = instr->len;

     instr->state = MTD_ERASING;

     while(len) {
	  /*
	   *  Heck if we have a bad block, we do not erase bad blocks!
	   *  */
	  if (nand_block_checkbad(mtd, (loff_t)(page<<chip->page_shift), 0,
				  allowbbt)) {
	       printk(KERN_ERR"\nsocle_nand_host: nand_erase: attempt to erase a "
		      "bad block at page %d (0x%08x)\n", page, page<<chip->page_shift);
	       instr->state = MTD_ERASE_FAILED;
	       goto erase_exit;
	  }

	  /*
	   *  Invalidate the page cache, if we erase the block which
	   *  contains the current cached page
	   *  */
	  if ((page <= chip->pagebuf) && (chip->pagebuf < (page + pages_per_block)))
	       chip->pagebuf = -1;

#ifdef CONFIG_MTD_SOCLE_NAND_PERFORMANCE_MEASURE
	  jiffies_before = jiffies;
#endif
	  if (chip->erase_block(chip, page&chip->pagemask)) {
	       printk(KERN_ERR"\nsocle_nand_host: nand_erase_nand(): Failed erase, page %d (0x%08x)\n", page, page<<chip->page_shift); 
	       instr->state = MTD_ERASE_FAILED;
	       instr->fail_addr = page << chip->page_shift;
	       goto erase_exit;
	  }
#ifdef CONFIG_MTD_SOCLE_NAND_PERFORMANCE_MEASURE
	  jiffies_after = jiffies;
	  jiffies_diff = jiffies_after - jiffies_before;
	  socle_nand_statistic->erase_jiffies += jiffies_diff;
#endif
#ifdef CONFIG_MTD_SOCLE_NAND_WEAR_LEVELING_MEASURE
	  socle_nand_statistic->erase_count++;
	  socle_nand_statistic->wear_statistic[(page<<chip->page_shift)>>chip->phys_erase_shift]++;
#endif

	  /*
	   *  If BBT requires refresh, set the BBT rewrite flag to the
	   *  page being erased
	   *  */
	  if ((bbt_masked_page != 0xffffffff) &&
	      ((page & BBT_PAGE_MASK) == bbt_masked_page))
	       rewrite_bbt[chipnr] = (page << chip->page_shift);

	  /* Increment page address and decrement length */
	  len -= (1 << chip->phys_erase_shift);
	  page += pages_per_block;

	  /* Check, if we cross a chip boundary */
	  if (len && !(page & chip->pagemask)) {
	       chipnr++;
	       nand_select_chip(chip, -1);
	       nand_select_chip(chip, chipnr);

	       /*
		*  If BBT requires refresh and BBT-PERCHIP, set the BBT
		*  page mask to see if this BBT should be rewritten
		*  */
	       if ((bbt_masked_page != 0xffffffff) && 
		   (chip->bbt_td->options & NAND_BBT_PERCHIP))
		    bbt_masked_page = chip->bbt_td->pages[chipnr] &
			 BBT_PAGE_MASK;
	  }
     }
     instr->state = MTD_ERASE_DONE;

erase_exit:
     ret = instr->state == MTD_ERASE_DONE ? 0 : -EIO;

     /* Do call back function */
     if (!ret)
	  mtd_erase_callback(instr);

     /* Deselect and wake up anyone waiting on the device */
     nand_release_device(chip);

     /*
      *  If BBT requires refresh and erase was successful, rewrite any
      *  selected bad block tables
      *  */
     if ((0xffffffff == bbt_masked_page) || ret)
	  return ret;

     for (chipnr = 0; chipnr < chip->numchips; chipnr++) {
	  if (!rewrite_bbt[chipnr])
	       continue;

	  /* Update the BBT for chip */
	  SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL0, "\nsocle_nand_host: nand_erase_nand() nand_update_bbg"
			   "(%d: 0x%08x 0x%08x)\n", chipnr, rewrite_bbt[chipnr],
			   chip->bbt_td->pages[chipnr]);
	  nand_update_bbt(mtd, rewrite_bbt[chipnr]);
     }

     /* Return more or less happy */
     return ret;
}

/**
 * nand_read - [MTD Interface] MTD compability function for nand_do_read_ecc
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @len:	number of bytes to read
 * @retlen:	pointer to variable to store the number of read bytes
 * @buf:	the databuffer to put data
 *
 * Get hold of the chip and call nand_do_read
 */
static int
nand_read(struct mtd_info *mtd, loff_t from, size_t len,
	  size_t *retlen, uint8_t *buf)
{
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     int ret;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL0, "\nsocle_nand_host: nand_read()\n");

     /* Do not allow reads past end of device */
     if ((from + len) > mtd->size) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_read(): Attempt read beyond end of device\n");
	  return -EINVAL;
     }
     if (!len)
	  return 0;

     nand_get_device(chip, FL_READING);
     chip->ops.len = len;
     chip->ops.datbuf = buf;
     chip->ops.oobbuf = NULL;
     ret = nand_do_read_ops(mtd, from, &chip->ops);
     *retlen = chip->ops.retlen;
     nand_release_device(chip);
     return ret;
}

/**
 * nand_read_oob - [MTD Interface] NAND read data and/or out-of-band
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob operation description structure
 *
 * NAND read data and/or out-of-band data
 */
static int 
nand_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
     int (*read_page)(struct mtd_info*, struct nand_chip*, u32,
		      u8*) = NULL;
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     int ret = -ENOTSUPP;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL0, "\nsocle_nand_host: nand_read_oob()\n");

     ops->retlen = 0;

     /* Do not allow reads past end of device */
     if ((from + ops->len) > mtd->size) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_read_oob(): Attempt read beyond end of device\n");
	  return -EINVAL;
     }

     nand_get_device(chip, FL_READING);
     switch(ops->mode) {
     case MTD_OOB_PLACE:
     case MTD_OOB_AUTO:
	  break;
     case MTD_OOB_RAW:
	  /* Replace the read_page algotithm temporary */
	  read_page = chip->read_page;
	  chip->read_page = nand_read_page_raw;
	  break;
     default:
	  goto out;
     }
     if (!ops->datbuf)
	  ret = nand_do_read_oob(mtd, from, ops);
     else
	  ret = nand_do_read_ops(mtd, from, ops);
     if (unlikely(MTD_OOB_RAW == ops->mode))
	  chip->read_page = read_page;
out:
     nand_release_device(chip);
     return ret;
}

/**
 * nand_do_read_ops - [Internal] Read data with ECC
 *
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob ops structure
 *
 * Internal function. Called with chip held.
 */
static int 
nand_do_read_ops(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
     int chipnr, page, realpage, col, bytes, aligned;
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     struct mtd_ecc_stats stats;
     int ret = 0;
     u32 readlen = ops->len;
     u8 *bufpoi, *oob, *buf;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL1, "\nsocle_nand_host nand_do_read_ops() from = 0x%08x, len = %d, "
		      "ooboffs = %d, ooblen = %d\n", (unsigned int)from , (unsigned int)ops->len, 
		      (unsigned int)ops->ooboffs, (unsigned int)ops->ooblen);

     stats = mtd->ecc_stats;
     chipnr = (int)(from >> chip->chip_shift);
     nand_select_chip(chip, chipnr);
     realpage = (int)(from >> chip->page_shift);
     page = realpage & chip->pagemask;
     col = (int)(from & (mtd->writesize - 1));
     buf = ops->datbuf;
     oob = ops->oobbuf;
     while (1) {
	  bytes = min(mtd->writesize - col, readlen);
	  aligned = (bytes == mtd->writesize);

	  /* Is the current page in the buffer */
	  if ((realpage != chip->pagebuf) || oob) {
	       bufpoi = aligned ? buf : chip->buffers.databuf;
#ifdef CONFIG_MTD_SOCLE_NAND_PERFORMANCE_MEASURE	
	       jiffies_before = jiffies;
#endif
	       ret = chip->read_page(mtd, chip, page, bufpoi);
#ifdef CONFIG_MTD_SOCLE_NAND_PERFORMANCE_MEASURE
	       jiffies_after = jiffies;
	       jiffies_diff = jiffies_after - jiffies_before;
	       socle_nand_statistic->read_jiffies += jiffies_diff;
	       socle_nand_statistic->read_bytes += mtd->writesize;
#endif
	       if (ret < 0)
		    break;

	       /* Transfer not aligned data */
	       if (!aligned) {
		    chip->pagebuf = realpage;
		    memcpy(buf, chip->buffers.databuf+col, bytes);
	       }

	       buf += bytes;
	       if (unlikely(oob)) {
		    /* Raw mode does data:oob:data:oob */
		    if (ops->mode != MTD_OOB_RAW)
			 nand_transfer_oob(mtd, chip, oob, 
					   page, ops);
		    else
			 nand_transfer_oob(mtd, chip, buf, 
					   page, ops);
	       }
	  } else {
	       memcpy(buf, chip->buffers.databuf+col, bytes);
	       buf += bytes;
	  }

	  readlen -= bytes;
	  if (!readlen)
	       break;

	  /* For subsequent reads align to page boundary. */
	  col = 0;

	  /* Increment page address */
	  realpage++;

	  page = realpage & chip->pagemask;

	  /* Check, if we cross a chip boundary */
	  if (!page) {
	       chipnr++;
	       nand_select_chip(chip, -1);
	       nand_select_chip(chip, chipnr);
	  }
     }
     ops->retlen = ops->len - (size_t)readlen;
     if (ret)
	  return ret;
     if (mtd->ecc_stats.failed - stats.failed)
	  return -EBADMSG;
     return mtd->ecc_stats.corrected - stats.corrected ? -EUCLEAN : 0;
}

/**
 * nand_do_read_oob - [Intern] NAND read out-of-band
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob operations description structure
 *
 * NAND read out-of-band data from the spare area
 */
static int
nand_do_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
     int page, realpage, chipnr;
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     int readlen = ops->len;
     u8 *buf = ops->oobbuf;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL1, "\nsocle_nand_host: nand_do_read_oob() from = 0x%08x, len = %d, "
		      "ooboffs = %d, ooblen = %d\n", (unsigned int)from, (unsigned int)ops->len,
		      (unsigned int)ops->ooboffs, (unsigned int)ops->ooblen);

     chipnr = (int)(from >> chip->chip_shift);
     nand_select_chip(chip, chipnr);

     /* Shift to get page */
     realpage = (int)(from >> chip->page_shift);
     page = realpage & chip->pagemask;

     while (1) {
	  nand_transfer_oob(mtd, chip, buf, 
			    page, ops);
	  readlen -= ops->ooblen;
	  if (!readlen)
	       break;

	  /* Increment page address */
	  realpage++;

	  page = realpage & chip->pagemask;

	  /* Check, if we cross a chip boundary */
	  if (!page) {
	       chipnr++;
	       nand_select_chip(chip, -1);
	       nand_select_chip(chip, chipnr);
	  }
     }
     ops->retlen = ops->len;
     return 0;
}

/**
 * nand_transfer_oob - [Internal] Transfer oob to client buffer
 * @mtd:        MTD device structure
 * @chip:	nand chip structure
 * @oob:	oob destination address
 * @page:       page number to read
 * @ops:	oob ops structure
 */
static void
nand_transfer_oob(struct mtd_info *mtd, struct nand_chip *chip, u8 *oob, 
		  u32 page, struct mtd_oob_ops *ops)
{
     size_t len = ops->ooblen;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL2, "\nsocle_nand_host: nand_transfer_oob() page = %d (0x%08x), "
		      "mode = %d, ooboffs = %d, ooblen %d\n", page, page<<chip->page_shift,
		      ops->mode, ops->ooboffs, ops->ooblen);

     switch(ops->mode) {
     case MTD_OOB_PLACE:
     case MTD_OOB_RAW:
	  chip->read(chip, mtd->writesize+ops->ooboffs, page, len, 0);
	  nand_read_buf(chip, oob, len);
	  oob += len;
	  break;
     case MTD_OOB_AUTO: {
	  struct nand_oobfree *free = mtd->ecclayout->oobfree;
	  u32 boffs = 0, roffs = ops->ooboffs;
	  size_t bytes = 0;

	  for (; free->length && len; free++, len -= bytes) {
	       /* Read request not from offset 0 ? */
	       if (unlikely(roffs)) {
		    if (roffs >= free->length) {
			 roffs -= free->length;
			 continue;
		    }
		    boffs = free->offset + roffs;
		    bytes = min_t(size_t, len, (free->length-roffs));
		    roffs = 0;
	       } else {
		    bytes = min_t(size_t, len, free->length);
		    boffs = free->offset;
	       }
	       chip->read(chip, mtd->writesize+boffs, page, bytes, 0);
	       nand_read_buf(chip, oob, bytes);
	       oob += bytes;
	  }
	  break;
     }
     default:
	  BUG();
     }
}

/**
 * nand_read_page_hwecc - {REPLACABLE] hardware ecc based page read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:       page number to read
 * @buf:	buffer to store read data
 *
 * Not for syndrome calculating ecc controllers which need a special oob layout
 */
static int 
nand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
		     u_char *buf)
{
     int status;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL3, "\nsocle_nand_host: nand_read_page_hwecc() page = %d (0x%08x)\n",
		      page, page<<chip->page_shift);

     status = chip->read(chip, 0, page, 
			 mtd->writesize, 1);
     if (-EIO == status)
	  return -EIO;
     nand_read_buf(chip, buf, mtd->writesize);
     if (status)
	  return nand_correct_data(chip, mtd, buf, 
				   status, page);
     else
	  return 0;
}

/**
 * nand_read_page_raw - [Intern] read raw page data without ecc
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:       page number to read
 * @buf:	buffer to store read data
 */
static int
nand_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
		   u_char *buf)
{
     int status;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL3, "\nsocle_nand_host: nand_read_page_raw() page = %d (0x%08x)\n",
		      page, page<<chip->page_shift);

     status = chip->read(chip, 0, page,
			 mtd->writesize, 0);
     if (-EIO == status)
	  return -EIO;
     nand_read_buf(chip, buf, mtd->writesize);
     return 0;
}

/**
 * nand_write - [MTD Interface] NAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @len:	number of bytes to write
 * @retlen:	pointer to variable to store the number of written bytes
 * @buf:	the data to write
 *
 * NAND write with ECC
 */
static int
nand_write(struct mtd_info *mtd, loff_t to, size_t len,
	   size_t *retlen, const uint8_t *buf)
{
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     int ret;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL0, "sq_nand_host: nand_write()\n");

     /* Do not allow writes past end of device */
     if ((to + len) > mtd->size) {
	  printk(KERN_ERR"\nsq_nand_host: nand_write(): Attempt write beyond end of device\n");
	  return -EINVAL;
     }
     if (!len)
	  return 0;

     nand_get_device(chip, FL_WRITING);
     chip->ops.len = len;
     chip->ops.datbuf = (uint8_t *)buf;
     chip->ops.oobbuf = NULL;
     ret = nand_do_write_ops(mtd, to, &chip->ops);
     *retlen = chip->ops.retlen;
     nand_release_device(chip);
     return ret;
}

/**
 * nand_write_oob - [MTD Interface] NAND write data and/or out-of-band
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operation description structure
 */
static int 
nand_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
     int (*write_page)(struct mtd_info *mtd, struct nand_chip *chip , u32 page,
		       const u_char *buf, int cached) = NULL;
     int (*verify_page)(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
			const u_char *buf) = NULL;
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     int ret = -ENOTSUPP;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL0, "\nsocle_nand_host: nand_write_oob()\n");

     ops->retlen = 0;

     /* Do not allow writes past end of device */
     if ((to + ops->len) > mtd->size) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_write_oob(): Attemp write beyond end of device\n");
	  return -EINVAL;
     }

     nand_get_device(chip, FL_WRITING);
     switch(ops->mode) {
     case MTD_OOB_PLACE:
     case MTD_OOB_AUTO:
	  break;
     case MTD_OOB_RAW:
	  /* Replace the write_page and verify_pagealgorithm temporary */
	  write_page = chip->write_page;
	  verify_page = chip->verify_page;
	  chip->write_page = nand_write_page_raw;
	  chip->verify_page = nand_verify_page_raw;
	  break;
     default:
	  goto out;
     }
     if (!ops->datbuf)
	  ret = nand_do_write_oob(mtd, to, ops);
     else
	  ret = nand_do_write_ops(mtd, to, ops);
     if (unlikely(MTD_OOB_RAW == ops->mode)) {
	  chip->write_page = write_page;
	  chip->verify_page = verify_page;
     }
out:
     nand_release_device(chip);
     return ret;
}

#define NOTALIGNED(x) (x & (mtd->writesize-1)) != 0
/**
 * nand_do_write_ops - [Internal] NAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operations description structure
 *
 * NAND write with ECC
 */
static int 
nand_do_write_ops(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
     int chipnr, realpage, page, blockmask;
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     uint32_t writelen = ops->len;
     uint8_t *oob = ops->oobbuf;
     uint8_t *buf = ops->datbuf;
     int bytes = mtd->writesize;
     int status = 0;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL1, "\nsocle_nand_host: nand_do_write_ops() to = 0x%08x, len = %d, "
		      "ooboffs = %d, ooblen = %d\n", (unsigned int)to, (unsigned int)ops->len,
		      (unsigned int)ops->ooboffs, (unsigned int)ops->ooblen);

     ops->retlen = 0;

     /* Reject writes, which are not page aligned */
     if (NOTALIGNED(to) || NOTALIGNED(ops->len)) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_do_write_pos(): Attempt to write not page aligned data\n");
	  return -EINVAL;
     }

     if (!writelen)
	  return 0;
     chipnr = (int)(to >> chip->chip_shift);
     nand_select_chip(chip, chipnr);

     /* Check, if it is write protected */
     if (nand_check_wp(chip, chipnr)) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_do_write_ops(): Device is write protected!!!\n");	
	  return -EIO;
     }

     realpage = (int)(to >> chip->page_shift);
     page = realpage & chip->pagemask;
     blockmask = (1 << (chip->phys_erase_shift - chip->page_shift)) - 1;

     /* Invalidate the page cache, when we write to the cached page */
     if ((to <= (chip->pagebuf << chip->page_shift)) &&
	 ((chip->pagebuf << chip->page_shift) < (to + ops->len)))
	  chip->pagebuf = -1;

     while (1) {
	  int cached = (writelen > bytes) && (page != blockmask);

	  if (unlikely(oob))
	       status = nand_fill_oob(mtd, chip, oob, 
				      page, ops);
	  if (status)
	       break;
	  status = nand_program_page(mtd, chip, buf, page, cached);
	  if (status)
	       break;
	  writelen -= bytes;
	  if (!writelen)
	       break;
	  buf += bytes;
	  realpage++;
	  page = realpage & chip->pagemask;

	  /* Check, if we cross a chip boundary */
	  if (!page) {
	       chipnr++;
	       nand_select_chip(chip, -1);
	       nand_select_chip(chip, chipnr);
	  }
     }
     ops->retlen = ops->len - writelen;
     return status;
}

/**
 * nand_do_write_oob - [MTD Interface] NAND write out-of-band
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operation description structure
 *
 * NAND write out-of-band
 */
static int 
nand_do_write_oob(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
     int chipnr;
     u32 page;
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL1, "\nsocle_nand_host: nand_do_write_oob() to = 0x%08x, len = %d, "
		      "ooboffs = %d, ooblen = %d\n", (unsigned int)to, (unsigned int)ops->len,
		      (unsigned int)ops->ooboffs, (unsigned int)ops->ooblen);

     /* Do not allow write past end of page */
     if ((ops->ooboffs + ops->len) > mtd->oobsize) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_do_write_oob(): Attemp to write past end of page\n");
	  return -EINVAL;
     }

     chipnr = (int)(to >> chip->chip_shift);
     nand_select_chip(chip, chipnr);

     /* Shift to get page */
     page = (int)(to >> chip->page_shift);

     /* Check, if it is write protected */
     if (nand_check_wp(chip, chipnr)) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_do_write_oob(): Device is write protected!!!\n");
	  return -EROFS;
     }

     /* Invalidate the page cache, if we write to the cached page */
     if (page == chip->pagebuf)
	  chip->pagebuf = -1;

     nand_fill_oob(mtd, chip, ops->oobbuf, 
		   page&chip->pagemask, ops);
     ops->retlen = ops->len;
     return 0;
}

/**
 * nand_program_page - [INTERNAL] program one page
 * @mtd:	MTD device structure
 * @chip:	NAND chip descriptor
 * @buf:	the data to write
 * @page:	page number to write
 * @cached:	cached programming
 */
static int
nand_program_page(struct mtd_info *mtd, struct nand_chip *chip, const u_char *buf,
		  u32 page, int cached)
{
     int status;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL2, "\nsocle_nand_host: nand_program_page() page = %d (0x%08x), cached = %d\n", 
		      page, page<<chip->page_shift, cached);

     /*
      *  Cached programming disable for now, Not sure if its worth the
      *  trouble. The speed gain is not very impressive. (2.3 -> 2.6Mib/s)
      *  */
     cached = 0;
     if (!cached || !(chip->options & NAND_CACHEPRG)) {
#ifdef CONFIG_MTD_SOCLE_NAND_PERFORMANCE_MEASURE
	  jiffies_before = jiffies;
#endif
	  status = chip->write_page(mtd, chip, page, buf, 0);	  
#ifdef CONFIG_MTD_SOCLE_NAND_PERFORMANCE_MEASURE
	  jiffies_after = jiffies;
	  jiffies_diff = jiffies_after - jiffies_before;
	  socle_nand_statistic->write_jiffies += jiffies_diff;
	  socle_nand_statistic->write_bytes += mtd->writesize;
#endif
	  if (status)
	       return -EIO;
     } else 
	  status = chip->write_page(mtd, chip, page, buf, 1);


#ifdef CONFIG_MTD_SOCLE_NAND_VERIFY_WRITE
     if (chip->verify_page(mtd, chip, page, buf)) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_program_page(): Failed verification, page %d (0x%08x)\n",
		 page, page<<chip->page_shift); 
	  return -EIO;
     }
#endif
     return 0;
}

static int 
nand_verify_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
		       const u_char *buf)
{
     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL3, "\nsocle_nand_host: nand_verify_page_hwecc() page = %d (0x%08x)\n", page, page<<chip->page_shift);

     if (chip->read(chip, 0, page, mtd->writesize, 1))
	  return -EIO;
     return 0;
}

static int 
nand_verify_page_raw(struct mtd_info *mtd, struct nand_chip *chip, u32 page, 
		     const u_char *buf)
{
     int i;
     u32 nand_buf_pos = chip->controller->buffer;
     u32 nand_word;
     int status;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL3, "\nsocle_nand_host: nand_verify_page_raw() page = %d (0x%08x)\n", page, page<<chip->page_shift);

     status = chip->read(chip, 0, page, mtd->writesize, 0);
     if (-EIO == status)
	  return -EIO;
     for (i = 0; i < (mtd->writesize >> 2); i++) {
	  nand_word = ioread32(nand_buf_pos);
	  if (nand_word != *((u32 *)buf + i)) {
	       printk(KERN_ERR"\nsocle_nand_host: Pos = %d, NAND = 0x%08x, Buf = 0x%08x, Diff = %d\n",
		      i, nand_word, *((u32 *)buf + i), nand_word-*((u32 *)buf+i));
	       return -EIO;
	  }
	  nand_buf_pos += 4;
     }
     return 0;
}

/**
 * nand_fill_oob - [Internal] Transfer client buffer to oob
 * @mtd:        MTD device structure
 * @chip:	nand chip structure
 * @oob:	oob data buffer
 * @page:       page number to write
 * @ops:	oob ops structure
 */
static int
nand_fill_oob(struct mtd_info *mtd, struct nand_chip *chip, uint8_t *oob, 
	      u32 page, struct mtd_oob_ops *ops)
{
     int status;
     size_t len = ops->ooblen;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL2, "\nsocle_nand_host: nand_fill_oob() page = %d (0x%08x), "
		      "mode = %d, ooboffs = %d, ooblen = %d\n", page, page<<chip->page_shift,
		      ops->mode, ops->ooboffs, ops->ooblen);

     switch(ops->mode) {
     case MTD_OOB_PLACE:
     case MTD_OOB_RAW:
	  nand_write_buf(chip, oob, len);
	  status = chip->program(chip, mtd->writesize+ops->ooboffs, page, len, 0);
	  if (status)
	       return -EIO;
	  else {
	       oob += len;
	       return 0;
	  }
     case MTD_OOB_AUTO: {
	  struct nand_oobfree *free = mtd->ecclayout->oobfree;
	  uint32_t boffs = 0, woffs = ops->ooboffs;
	  size_t bytes = 0;

	  for (; free->length && len; free++, len -= bytes) {
	       /* Write request not from offset 0 ? */
	       if (unlikely(woffs)) {
		    if (woffs >= free->length) {
			 woffs -= free->length;
			 continue;
		    }
		    boffs = free->offset + woffs;
		    bytes = min_t(size_t, len, (free->length - woffs));
		    woffs = 0;
	       } else {
		    bytes = min_t(size_t, len, free->length);
		    boffs = free->offset;
	       }
	       nand_write_buf(chip, oob, bytes);
	       status = chip->program(chip, mtd->writesize+boffs, page, bytes, 0);
	       if (status)
		    return -1;
	       else 
		    oob += bytes;
	  }
	  return 0;
     }
     default:
	  BUG();
     }
     return -1;
}

/**
 * nand_write_page_hwecc - {REPLACABLE] hardware ecc based page write function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:       page number to write
 * @buf:	data buffer
 * @cached:     flag to indicate using cache program or not
 */
static int
nand_write_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
		      const u_char *buf, int cached)
{
     int status;
     
     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL3, "\nsocle_nand_host: nand_write_page_hwecc() page = %d (0x%08x) cached = %d\n", 
		      page, page<<chip->page_shift, cached);

     nand_write_buf(chip, buf, mtd->writesize);
     if (cached && chip->cache_program)
	  status = chip->cache_program(chip, 0, page, 
				       mtd->writesize, 1);
     else
	  status = chip->program(chip, 0, page, 
				 mtd->writesize, 1);
     if (status)
	  return -EIO;
     return 0;
}

/**
 * nand_write_page_raw - [Intern] raw page write function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:       page number to write
 * @buf:	data buffer
 * @cached:     flag to indicate using cache program or not
 */
static int
nand_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
		    const u_char *buf, int cached)
{
     int status;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL3, "\nsocle_nand_host: nand_write_page_raw() page = %d (0x%08x) cached = %d\n", 
		      page, page<<chip->page_shift, cached);

     nand_write_buf(chip, buf, mtd->writesize);
     if (cached && chip->cache_program)
	  status =  chip->cache_program(chip, 0, page,
					mtd->writesize, 0);
     else
	  status =  chip->program(chip, 0, page,
				  mtd->writesize, 0);
     return status;
}

/**
 * nand_block_isbad - [MTD Interface] Check whether the block at the given offset is bad
 * @mtd:	MTD device structure
 * @ofs:	offset relative to mtd start
 */
static int
nand_block_isbad(struct mtd_info *mtd, loff_t offs)
{
     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL0, "\nsocle_nand_host: nand_block_isbad()\n");

     /* Check for invalid offset */
     if (offs > mtd->size)
	  return -EINVAL;

     return nand_block_checkbad(mtd, offs, 1, 0);
}    

/**
 * nand_block_checkbad - [GENERIC] Check if a block is marked bad
 * @mtd:	MTD device structure
 * @ofs:	offset from device start
 * @getchip:	0, if the chip is already selected
 * @allowbbt:	1, if its allowed to access the bbt area
 *
 * Check, if the block is bad. Either by reading the bad block table or
 * calling of the scan function.
 */
static int
nand_block_checkbad(struct mtd_info *mtd, loff_t ofs, int getchip,
		    int allowbbt)
{
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL1, "\nsocle_nand_host: nand_block_checkbad() ofs = 0x%08x, getchip = %d, allowbbt = %d\n",
		      (unsigned int)ofs, (unsigned int)getchip, (unsigned int)allowbbt);

     if (!chip->bbt) {
	  return nand_block_bad(mtd, ofs, getchip);
     }

     /* Return info from the table */
     return nand_isbad_bbt(mtd, ofs, allowbbt);
}

/**
 * nand_block_bad - [DEFAULT] Read bad block marker from the chip
 * @mtd:	MTD device structure
 * @ofs:	offset from device start
 * @getchip:	0, if the chip is already selected
 *
 * Check, if the block is bad.
 */
static int
nand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
     u32 page, chipnr, res = 0;
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     u8 bad;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL2, "\nsocle_nand_host: nand_block_bad() ofs = 0x%08x, getchip = %d\n",
		      (unsigned int)ofs, (unsigned int)getchip);

     if (getchip) {
	  chipnr = (int)(ofs >> chip->chip_shift);

	  /* Grab the lock and see if the device is avaiable */
	  nand_get_device(chip, FL_READING);

	  /* Select the NAND device */
	  nand_select_chip(chip, chipnr);
     }
     
     /* We write the bad block marker into the 2nd page of the block */
     page = (int)(ofs >> chip->page_shift) + 1;

     chip->read(chip, mtd->writesize+chip->badblockpos, page&chip->pagemask, 
		1, 0);
     nand_read_buf(chip, &bad, 1);
     if (bad != 0xff)
	  res = 1;

     if (getchip) 
	  /* Deselect and wake up anyone waiting on the device */
	  nand_release_device(chip);

     return res;
}

/**
 * nand_block_markbad - [MTD Interface] Mark block at the given offset as bad
 * @mtd:	MTD device structure
 * @ofs:	offset relative to mtd start
 */
static int
nand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
     int ret;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL0, "\nsocle_nand_host: nand_block_markbad()\n");

     if ((ret = nand_block_isbad(mtd, ofs))) {
	  /* If it was bad already, return success and do nothing. */
	  if (ret > 0)
	       return 0;
	  return ret;
     }
     return nand_default_block_markbad(mtd, ofs);
}

/**
 * nand_default_block_markbad - [DEFAULT] mark a block bad
 * @mtd:	MTD device structure
 * @ofs:	offset from device start
 *
 * This is the default implementation, which can be overridden by
 * a hardware specific driver.
 */
static int
nand_default_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     u8 bad = 0;
     int block, ret;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL1, "\nsocle_nand_host: nand_default_block_markbad(): ofs = 0x%08x\n", (unsigned int)ofs);

     /* Get block number */
     block = ((int)ofs) >> chip->bbt_erase_shift;
     if (chip->bbt)
	  chip->bbt[block >> 2] |= 0x01 << ((block & 0x03) << 1);

     /* Do we have a flash based bad block table ? */
     if (chip->options & NAND_USE_FLASH_BBT)
	  ret = nand_update_bbt(mtd, ofs);
     else {
	  ofs += mtd->writesize;
	  chip->ops.len = 1;
	  chip->ops.datbuf = NULL;
	  chip->ops.oobbuf = &bad;
	  chip->ops.ooboffs = chip->badblockpos;
	  ret = nand_do_write_oob(mtd, ofs, &chip->ops);
     }
     if (!ret)
	  mtd->ecc_stats.badblocks++;
     return ret;
}

/**
 * nand_sync - [MTD Interface] sync
 * @mtd:	MTD device structure
 *
 * Sync is actually a wait for chip ready function
 */
static void
nand_sync(struct mtd_info *mtd)
{
     struct nand_chip *chip = mtd->priv;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL0, "\nsocle_nand_host: nand_sync()\n");

     /* Grab the lock and see if the device is available */
     nand_get_device(chip, FL_SYNCING);

     /* Release it and go back */
     nand_release_device(chip);
}

/**
 * nand_suspend - [MTD Interface] Suspend the NAND flash
 * @mtd:	MTD device structure
 */
static int 
nand_suspend(struct mtd_info *mtd)
{
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL0, "\nsocle_nand_host: nand_suspend()\n");

     return nand_get_device(chip, FL_PM_SUSPENDED);
}

/**
 * nand_resume - [MTD Interface] Resume the NAND flash
 * @mtd:	MTD device structure
 */
static void 
nand_resume(struct mtd_info *mtd)
{
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;

     SOCLE_NAND_DEBUG(SOCLE_NAND_DEBUG_LEVEL0, "\nsocle_nand_host: nand_resume()\n");

     if (FL_PM_SUSPENDED == chip->state)
	  nand_release_device(chip);
     else
	  printk(KERN_ERR"\nsocle_nand_host: resume() called for the chip which is not"
		 "in suspended state\n");
}


/*
 * Get the flash and manufacturer id and lookup if the type is supported
 */
static struct nand_flash_dev *
nand_get_flash_type(struct mtd_info *mtd, struct nand_chip *chip, u32 busw,
		    u32 *maf_id)
{
     struct nand_flash_dev *type = NULL;
     u32 id, dev_id, maf_idx;
     int i;

     /* Select the device */
     nand_select_chip(chip, 0);

     /* Send the command for reading device ID */
     id = nand_read_id(chip);

     /* Read manufacturer and device IDs */
     *maf_id = MAKER_CODE(id);
     dev_id = DEVICE_CODE(id);

     /* Lookup the flash id */
     for (i = 0; nand_flash_ids[i].name != NULL; i++) {
	  if (dev_id == nand_flash_ids[i].id) {
	       type = &nand_flash_ids[i];
	       break;
	  }
     }
     if (!type)
	  return ERR_PTR(-ENODEV);
     if (!mtd->name)
	  mtd->name = type->name;
     chip->chipsize = type->chipsize << 20;

     /* Newer devices have all the information in additional id bytes */
     if (!type->pagesize) {
	  int extid;

	  /* The 3rd id byte contains non relevant data ATM
	   * The 4th id byte is the important one*/
	  extid = id & 0xff;

	  /* Calc pagesize */
	  mtd->writesize = 1024 << (extid & 0x3);
	  extid >>= 2;

	  /* Calc oobsize */
	  mtd->oobsize = (8 << (extid & 0x01)) * (mtd->writesize >> 9);
	  extid >>= 2;

	  /* Calc blocksize. Blocksize is multiples of 64KiB */
	  mtd->erasesize = (64 * 1024) << (extid & 0x03);
	  extid >>= 2;

	  /* Get buswidth information */
	  busw = (extid & 0x01) ? NAND_BUSWIDTH_16 : 0;
     } else {
	  /*
	   *  Old devices have chip data hardcoded in the device id table
	   *  */
	  mtd->erasesize = type->erasesize;
	  mtd->writesize = type->pagesize;
	  mtd->oobsize = mtd->writesize / 32;
	  busw = type->options & NAND_BUSWIDTH_16;
     }

	 /*	leonid del for no this structure member	*/
     //mtd->eccsize = mtd->writesize;

     /* Try to identify manufacturer */
     for (maf_idx = 0; nand_manuf_ids[maf_idx].id != 0x0; maf_idx++) {
	  if (nand_manuf_ids[maf_idx].id == *maf_id)
	       break;
     }

     /*
      *  Check, if buswidth is correct. Hardware drivers should set
      *  chip correct !
      *  */
     if (busw != (chip->options & NAND_BUSWIDTH_16)) {
	  printk(KERN_INFO"NAND device: Manufacturer ID:"
		 "0x%02x, Chip ID: 0x%02x \n(%s %s)\n", *maf_id,
		 dev_id, nand_manuf_ids[maf_idx].name, mtd->name);
	  printk(KERN_WARNING"NAND bus width %d instead %d bit\n",
		 (chip->options & NAND_BUSWIDTH_16) ? 16 : 8,
		 busw ? 16 : 8);
	  return ERR_PTR(-EINVAL);
     }

     /* Calculate the address shift from the page size */
     chip->page_shift = ffs(mtd->writesize) - 1;

     /* Convert chipsize to number of pages per chip -1. */
     chip->pagemask = (chip->chipsize >> chip->page_shift) - 1;

     chip->bbt_erase_shift = chip->phys_erase_shift = 
	  ffs(mtd->erasesize) - 1;

     chip->chip_shift = ffs(chip->chipsize) - 1;

     /* Set the bad block position */
     chip->badblockpos = mtd->writesize > 512 ?
	  NAND_LARGE_BADBLOCK_POS : NAND_SMALL_BADBLOCK_POS;

     /* Get chip options, preserve non chip based options */
     chip->options &= ~NAND_CHIPOPTIONS_MSK;
     chip->options |= type->options & NAND_CHIPOPTIONS_MSK;

     /*
      *  Set chip as a default. Board drivers can override it, if necessary
      *  */
     chip->options |= NAND_NO_AUTOINCR;

     /* Check if chip is not a samsung device. Do not clear the
      * options for chips which are not having an extened id.x*/
     if ((*maf_id != NAND_MFR_SAMSUNG) && !type->pagesize)
	  chip->options &= ~NAND_SAMSUNG_LP_OPTIONS;

     printk(KERN_INFO"NAND device: Manufacturer ID: "
	    "0x%02x, Chip ID: 0x%02x \n(%s %s)\n", *maf_id, dev_id,
	    nand_manuf_ids[maf_idx].name, type->name);
     return type;
}


/**
 * nand_get_device - [GENERIC] Get chip for selected access
 * @this:	the nand chip descriptor
 * @new_state:	the state which is requested
 *
 * Get the device and lock it for exclusive access
 */
static int
nand_get_device(struct nand_chip *chip, int new_state)
{
     spinlock_t *lock = &chip->controller->lock;
     wait_queue_head_t *wq = &chip->controller->wq;
     DECLARE_WAITQUEUE(wait, current);

retry:
     spin_lock(lock);

     /* Hardware controller shared among indepentdend devices */
     if (!chip->controller->active)
	  chip->controller->active = chip;

     if ((chip->controller->active == chip) &&
	 (FL_READY == chip->state)) {
	  chip->state = new_state;
	  spin_unlock(lock);
	  return 0;
     }
     if (FL_PM_SUSPENDED == new_state) {
	  spin_unlock(lock);
	  return (FL_PM_SUSPENDED == chip->state) ? 0 : -EAGAIN;
     }
     set_current_state(TASK_UNINTERRUPTIBLE);
     add_wait_queue(wq, &wait);
     spin_unlock(lock);
     schedule();
     remove_wait_queue(wq, &wait);
     goto retry;
}

/**
 * nand_release_device - [GENERIC] release chip
 * @this:	the nand chip descriptor
 *
 * Deselect, release chip lock and wake up anyone waiting on the device
 */
static void
nand_release_device(struct nand_chip *chip)
{
     /* De-select the NAND device */
     nand_select_chip(chip, -1);

     /* Release the controller and the chip */
     spin_lock(&chip->controller->lock);
     chip->controller->active = NULL;
     chip->state = FL_READY;
     wake_up(&chip->controller->wq);
     spin_unlock(&chip->controller->lock);
}

/**
 * nand_select_chip - [DEFAULT] control CE line
 * @chipnr:	chipnumber to select, -1 for deselect
 *
 * Default select function for 1 chip devices.
 */
static void
nand_select_chip(struct nand_chip *chip, int chipnr)
{
     u32 io_base = chip->controller->io_base;
     u32 data;

     data = ioread32(NAND_FLSH_CE_WP(io_base));

     /* Clear all NAND flash device chip enable */
     data &= ~(0xf);

     if (chipnr != (-1))
	  data |= 1 << (chipnr & 0xf);
     iowrite32(data, NAND_FLSH_CE_WP(io_base));
}

/**
 * nand_check_wp - [GENERIC] check if the chip is write protected
 * @chipnr:	chip number
 * Check, if the device is write protected
 *
 * The function expects, that the device is already selected
 */
static int
nand_check_wp(struct nand_chip *chip, int chipnr)
{
     u32 io_base = chip->controller->io_base;
     u32 data;

     data = ioread32(NAND_FLSH_CE_WP(io_base));
     data &= (1 << (chipnr & 0xf)) << 4;
     if (data)
	  return 1;
     else 
	  return 0;
}

static void
nand_read_buf(struct nand_chip *chip, u8 *buf, u32 len)
{
     int i;
     u32 nand_buf_pos = chip->controller->buffer;
     u32 num_words = (len >> 2);
     u8 num_rem_bytes = len & (3);
     u8 rem_byte[4] = {0};

     /* Must use word access in writing data into nand buffer */     
     for (i = 0; i < num_words; i++) {
	  *((u32 *)buf + i) = ioread32(nand_buf_pos);
	  nand_buf_pos += 4;
     }
     if (num_rem_bytes) {
	  *((u32 *)rem_byte) = ioread32(nand_buf_pos);
	  for (i = 0; i < num_rem_bytes; i++)
	       buf[4*num_words+i] = rem_byte[i];
     }
}

static void 
nand_write_buf(struct nand_chip *chip, const u8 *buf, u32 len)
{
     int i;
     u32 nand_buf_pos = chip->controller->buffer;

     /* Must use word access in writing data into nand buffer */
     u32 num_words = len >> 2;
     u8 num_rem_bytes = len & 3;
     u8 rem_bytes[4] = {0};

     for (i = 0; i < num_words; i++) {
	  iowrite32(*((u32 *)buf+i), nand_buf_pos);
	  nand_buf_pos += 4;
     }
     if (num_rem_bytes) {
	  for (i = 0; i < num_rem_bytes; i++)
	       rem_bytes[i] = buf[4*num_words+i];
	  iowrite32(*((u32 *)rem_bytes), nand_buf_pos);
     }
}

#define ML (512 + NPAR)		/* data size + NPAR */

#if 1
static int 
nand_correct_data(struct nand_chip *chip, struct mtd_info *mtd, u_char *buf, 
		  int status, u32 page)
{
     u8 *codeword;
     u32 io_base = chip->controller->io_base;
     u32 tmp;
     int i, ret=0, loc;

	//printk("nand_correct_data : page 0x%08x\n", page);
	
	if(NPAR == 12)
		loc = 512 + 15;
	else		
		loc = 512 + 8;;

     /* Check if it is a erased range */
     if ((NAND_SYNDROME_ERROR_LOCATE4_MASK == (status & NAND_SYNDROME_ERROR_LOCATE4_MASK)) ||
	 (512 == mtd->writesize)) {

	  for (i = 0; i < (mtd->writesize >> 2); i++) {
	       if (*((u32 *)buf + i) != 0xffffffff)
		    goto data_correct;
	  }
	  return 0;
     }


data_correct:

#if 1	
	/*	20080325 leonid+ for nand syndrome bug sw work around	*/
	
	/* First 512 bytes */
	if(NAND_SYNDROME_ERROR_LOCATE1 == 
					(chip->read(chip, 0, page, 512, 1) & NAND_SYNDROME_ERROR_LOCATE1)){
		codeword = buf;
	  	tmp = ioread32(NAND_FLSH_1ST_SYNDR_1(io_base));
		printk("NAND_FLSH_1ST_SYNDR_1_1 : 0x%08x\n", tmp);
		synBytes[0] = NAND_SYNDROME_CODE_1(tmp);
		synBytes[1] = NAND_SYNDROME_CODE_2(tmp);
		synBytes[2] = NAND_SYNDROME_CODE_3(tmp);
	  	tmp = ioread32(NAND_FLSH_1ST_SYNDR_2(io_base));
		printk("NAND_FLSH_1ST_SYNDR_1_2 : 0x%08x\n", tmp);
		synBytes[3] = NAND_SYNDROME_CODE_1(tmp);
		synBytes[4] = NAND_SYNDROME_CODE_2(tmp);
		synBytes[5] = NAND_SYNDROME_CODE_3(tmp);
		if (NPAR == 12){	
	  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_3(io_base));
			printk("NAND_FLSH_1ST_SYNDR_1_3 : 0x%08x\n", tmp);
			synBytes[6] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[7] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[8] = NAND_SYNDROME_CODE_3(tmp);
	  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_4(io_base));
			printk("NAND_FLSH_1ST_SYNDR_1_4 : 0x%08x\n", tmp);
			synBytes[9] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[10] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[11] = NAND_SYNDROME_CODE_3(tmp);
		}
		if (correct_errors_hw_ecc(codeword, ML)) {
			printk("\nsocle_nand_host: data can not be corrected between 0 and 511 bytes in page %d\n", page);
			//return -1;
			ret = -1;
		} else 
			printk("\nsocle_nand_host: data is corrected between 0 and 511 bytes in page %d\n", page);			
	}
	
	if(2048 == mtd->writesize){
		/* Second 512 bytes */	
		if(NAND_SYNDROME_ERROR_LOCATE1 == 
					(chip->read(chip, loc, page, 512, 1) & NAND_SYNDROME_ERROR_LOCATE1)){
			codeword = buf + 512;
	  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_1(io_base));
			printk("NAND_FLSH_1ST_SYNDR_2_1 : 0x%08x\n", tmp);
			synBytes[0] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[1] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[2] = NAND_SYNDROME_CODE_3(tmp);
		  	tmp = ioread32(NAND_FLSH_1ST_SYNDR_2(io_base));
			printk("NAND_FLSH_1ST_SYNDR_2_2 : 0x%08x\n", tmp);
			synBytes[3] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[4] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[5] = NAND_SYNDROME_CODE_3(tmp);
			if (NPAR == 12){	
		  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_3(io_base));
			printk("NAND_FLSH_1ST_SYNDR_2_3 : 0x%08x\n", tmp);
				synBytes[6] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[7] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[8] = NAND_SYNDROME_CODE_3(tmp);
		  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_4(io_base));
			printk("NAND_FLSH_1ST_SYNDR_2_4 : 0x%08x\n", tmp);
				synBytes[9] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[10] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[11] = NAND_SYNDROME_CODE_3(tmp);
			}
			if (correct_errors_hw_ecc(codeword, ML)) {
				printk("\nsocle_nand_host: data can not be corrected between 512 and 1023 bytes in page %d\n", page);
				//return -1;
				ret = -1;
			} else 
				printk("\nsocle_nand_host: data is corrected between 512 and 1023 bytes in page %d\n", page);			
		}
		/* Third 512 bytes */
		if(NAND_SYNDROME_ERROR_LOCATE1 == 
					(chip->read(chip, loc*2, page, 512, 1) & NAND_SYNDROME_ERROR_LOCATE1)){
			codeword = buf + 1024;
	  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_1(io_base));
			printk("NAND_FLSH_1ST_SYNDR_3_1 : 0x%08x\n", tmp);
			synBytes[0] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[1] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[2] = NAND_SYNDROME_CODE_3(tmp);
		  	tmp = ioread32(NAND_FLSH_1ST_SYNDR_2(io_base));
			printk("NAND_FLSH_1ST_SYNDR_3_2 : 0x%08x\n", tmp);
			synBytes[3] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[4] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[5] = NAND_SYNDROME_CODE_3(tmp);
			if (NPAR == 12){	
		  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_3(io_base));
				printk("NAND_FLSH_1ST_SYNDR_3_3 : 0x%08x\n", tmp);
				synBytes[6] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[7] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[8] = NAND_SYNDROME_CODE_3(tmp);
		  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_4(io_base));
				printk("NAND_FLSH_1ST_SYNDR_3_4 : 0x%08x\n", tmp);
				synBytes[9] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[10] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[11] = NAND_SYNDROME_CODE_3(tmp);
			}
			if (correct_errors_hw_ecc(codeword, ML)) {
				printk("\nsocle_nand_host: data can not be corrected between 1024 and 1535 bytes in page %d\n", page);
				//return -1;
				ret = -1;
			} else 
				printk("\nsocle_nand_host: data is corrected between 1024 and 1535 bytes in page %d\n", page);
		}
		/* Forth 512 bytes */
		if(NAND_SYNDROME_ERROR_LOCATE1 == 
					(chip->read(chip, loc*3, page, 512, 1) & NAND_SYNDROME_ERROR_LOCATE1)){
			codeword = buf + 1536;
	  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_1(io_base));
			printk("NAND_FLSH_1ST_SYNDR_4_1 : 0x%08x\n", tmp);
			synBytes[0] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[1] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[2] = NAND_SYNDROME_CODE_3(tmp);
		  	tmp = ioread32(NAND_FLSH_1ST_SYNDR_2(io_base));
			printk("NAND_FLSH_1ST_SYNDR_4_2 : 0x%08x\n", tmp);
			synBytes[3] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[4] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[5] = NAND_SYNDROME_CODE_3(tmp);
			if (NPAR == 12){	
		  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_3(io_base));
				printk("NAND_FLSH_1ST_SYNDR_4_3 : 0x%08x\n", tmp);
				synBytes[6] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[7] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[8] = NAND_SYNDROME_CODE_3(tmp);
		  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_4(io_base));
				printk("NAND_FLSH_1ST_SYNDR_4_4 : 0x%08x\n", tmp);
				synBytes[9] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[10] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[11] = NAND_SYNDROME_CODE_3(tmp);
			}
			if (correct_errors_hw_ecc(codeword, ML)) {
				printk("\nsocle_nand_host: data can not be corrected between 1536 and 2047 bytes in page %d\n", page);
				//return -1;
				ret = -1;
			} else 
				printk("\nsocle_nand_host: data is corrected between 1536 and 2047 bytes in page %d\n", page);
		}
	}
	
     return ret;	 
#else
	
	/*	20080325 leonid+ for normal nand correct function	*/
	
	/* First 512 bytes */
	if(NAND_SYNDROME_ERROR_LOCATE1 == 
					(status & NAND_SYNDROME_ERROR_LOCATE1)){
		codeword = buf;
	  	tmp = ioread32(NAND_FLSH_1ST_SYNDR_1(io_base));
		printk("NAND_FLSH_1ST_SYNDR_1_1 : 0x%08x\n", tmp);
		synBytes[0] = NAND_SYNDROME_CODE_1(tmp);
		synBytes[1] = NAND_SYNDROME_CODE_2(tmp);
		synBytes[2] = NAND_SYNDROME_CODE_3(tmp);
	  	tmp = ioread32(NAND_FLSH_1ST_SYNDR_2(io_base));
		printk("NAND_FLSH_1ST_SYNDR_1_2 : 0x%08x\n", tmp);
		synBytes[3] = NAND_SYNDROME_CODE_1(tmp);
		synBytes[4] = NAND_SYNDROME_CODE_2(tmp);
		synBytes[5] = NAND_SYNDROME_CODE_3(tmp);
		if (NPAR == 12){	
	  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_3(io_base));
			printk("NAND_FLSH_1ST_SYNDR_1_3 : 0x%08x\n", tmp);
			synBytes[6] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[7] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[8] = NAND_SYNDROME_CODE_3(tmp);
	  		tmp = ioread32(NAND_FLSH_1ST_SYNDR_4(io_base));
			printk("NAND_FLSH_1ST_SYNDR_1_4 : 0x%08x\n", tmp);
			synBytes[9] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[10] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[11] = NAND_SYNDROME_CODE_3(tmp);
		}
		if (correct_errors_hw_ecc(codeword, ML)) {
			printk("\nsocle_nand_host: data can not be corrected between 0 and 511 bytes in page %d\n", page);
			return -1;
		} else 
			printk("\nsocle_nand_host: data is corrected between 0 and 511 bytes in page %d\n", page);			
	}
	
	if(2048 == mtd->writesize){
		/* Second 512 bytes */	
		if(NAND_SYNDROME_ERROR_LOCATE2 == 
					(status & NAND_SYNDROME_ERROR_LOCATE2)){
			codeword = buf + 512;
	  		tmp = ioread32(NAND_FLSH_2ND_SYNDR_1(io_base));
			printk("NAND_FLSH_1ST_SYNDR_2_1 : 0x%08x\n", tmp);
			synBytes[0] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[1] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[2] = NAND_SYNDROME_CODE_3(tmp);
		  	tmp = ioread32(NAND_FLSH_2ND_SYNDR_2(io_base));
			printk("NAND_FLSH_1ST_SYNDR_2_2 : 0x%08x\n", tmp);
			synBytes[3] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[4] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[5] = NAND_SYNDROME_CODE_3(tmp);
			if (NPAR == 12){	
		  		tmp = ioread32(NAND_FLSH_2ND_SYNDR_3(io_base));
			printk("NAND_FLSH_1ST_SYNDR_2_3 : 0x%08x\n", tmp);
				synBytes[6] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[7] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[8] = NAND_SYNDROME_CODE_3(tmp);
		  		tmp = ioread32(NAND_FLSH_2ND_SYNDR_4(io_base));
			printk("NAND_FLSH_1ST_SYNDR_2_4 : 0x%08x\n", tmp);
				synBytes[9] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[10] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[11] = NAND_SYNDROME_CODE_3(tmp);
			}
			if (correct_errors_hw_ecc(codeword, ML)) {
				printk("\nsocle_nand_host: data can not be corrected between 512 and 1023 bytes in page %d\n", page);
				return -1;
			} else 
				printk("\nsocle_nand_host: data is corrected between 512 and 1023 bytes in page %d\n", page);			
		}
		/* Third 512 bytes */
		if(NAND_SYNDROME_ERROR_LOCATE3 == 
					(status & NAND_SYNDROME_ERROR_LOCATE3)){
			codeword = buf + 1024;
	  		tmp = ioread32(NAND_FLSH_3RD_SYNDR_1(io_base));
			printk("NAND_FLSH_1ST_SYNDR_3_1 : 0x%08x\n", tmp);
			synBytes[0] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[1] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[2] = NAND_SYNDROME_CODE_3(tmp);
		  	tmp = ioread32(NAND_FLSH_3RD_SYNDR_2(io_base));
			printk("NAND_FLSH_1ST_SYNDR_3_2 : 0x%08x\n", tmp);
			synBytes[3] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[4] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[5] = NAND_SYNDROME_CODE_3(tmp);
			if (NPAR == 12){	
		  		tmp = ioread32(NAND_FLSH_3RD_SYNDR_3(io_base));
				printk("NAND_FLSH_1ST_SYNDR_3_3 : 0x%08x\n", tmp);
				synBytes[6] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[7] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[8] = NAND_SYNDROME_CODE_3(tmp);
		  		tmp = ioread32(NAND_FLSH_3RD_SYNDR_4(io_base));
				printk("NAND_FLSH_1ST_SYNDR_3_4 : 0x%08x\n", tmp);
				synBytes[9] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[10] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[11] = NAND_SYNDROME_CODE_3(tmp);
			}
			if (correct_errors_hw_ecc(codeword, ML)) {
				printk("\nsocle_nand_host: data can not be corrected between 1024 and 1535 bytes in page %d\n", page);
				return -1;
			} else 
				printk("\nsocle_nand_host: data is corrected between 1024 and 1535 bytes in page %d\n", page);
		}
		/* Forth 512 bytes */
		if(NAND_SYNDROME_ERROR_LOCATE1 == 
					(status & NAND_SYNDROME_ERROR_LOCATE1)){
			codeword = buf + 1536;
	  		tmp = ioread32(NAND_FLSH_4TH_SYNDR_1(io_base));
			printk("NAND_FLSH_1ST_SYNDR_4_1 : 0x%08x\n", tmp);
			synBytes[0] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[1] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[2] = NAND_SYNDROME_CODE_3(tmp);
		  	tmp = ioread32(NAND_FLSH_4TH_SYNDR_2(io_base));
			printk("NAND_FLSH_1ST_SYNDR_4_2 : 0x%08x\n", tmp);
			synBytes[3] = NAND_SYNDROME_CODE_1(tmp);
			synBytes[4] = NAND_SYNDROME_CODE_2(tmp);
			synBytes[5] = NAND_SYNDROME_CODE_3(tmp);
			if (NPAR == 12){	
		  		tmp = ioread32(NAND_FLSH_4TH_SYNDR_3(io_base));
				printk("NAND_FLSH_1ST_SYNDR_4_3 : 0x%08x\n", tmp);
				synBytes[6] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[7] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[8] = NAND_SYNDROME_CODE_3(tmp);
		  		tmp = ioread32(NAND_FLSH_4TH_SYNDR_4(io_base));
				printk("NAND_FLSH_1ST_SYNDR_4_4 : 0x%08x\n", tmp);
				synBytes[9] = NAND_SYNDROME_CODE_1(tmp);
				synBytes[10] = NAND_SYNDROME_CODE_2(tmp);
				synBytes[11] = NAND_SYNDROME_CODE_3(tmp);
			}
			if (correct_errors_hw_ecc(codeword, ML)) {
				printk("\nsocle_nand_host: data can not be corrected between 1536 and 2047 bytes in page %d\n", page);
				return -1;
			} else 
				printk("\nsocle_nand_host: data is corrected between 1536 and 2047 bytes in page %d\n", page);
		}
	}
	
     return 0;	 
#endif

}
#else
static int 
nand_correct_data(struct nand_chip *chip, struct mtd_info *mtd, u_char *buf, 
		  int status, u32 page)
{
     u8 *codeword = buf;
     u32 io_base = chip->controller->io_base;
     u32 tmp;
     u16 synbytes_1st[12], synbytes_2nd[12], synbytes_3rd[12], synbytes_4th[12];
     int syndrome_1st_err_flag = 0, syndrome_2nd_err_flag = 0, syndrome_3rd_err_flag = 0, syndrome_4th_err_flag = 0;
     int i;
     u32 nand_buf_pos;

     /* First 512 bytes */
     tmp = ioread32(NAND_FLSH_1ST_SYNDR_1(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_1ST_SYNDR_1 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_1ST_SYNDR_1(io_base), tmp);
#endif
     synbytes_1st[0] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_1st[1] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_1st[2] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_1ST_SYNDR_2(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_1ST_SYNDR_2 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_1ST_SYNDR_2(io_base), tmp);
#endif
     synbytes_1st[3] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_1st[4] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_1st[5] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_1ST_SYNDR_3(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_1ST_SYNDR_3 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_1ST_SYNDR_3(io_base), tmp);
#endif
     synbytes_1st[6] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_1st[7] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_1st[8] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_1ST_SYNDR_4(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_1ST_SYNDR_4 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_1ST_SYNDR_4(io_base), tmp);
#endif
     synbytes_1st[9] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_1st[10] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_1st[11] = NAND_SYNDROME_CODE_3(tmp);
     for (i = 0; i < 12; i++)
	  syndrome_1st_err_flag |= synbytes_1st[i];

     if (512 == mtd->writesize)
	  goto detect_erased_block;

     /* Second 512 bytes */
     tmp = ioread32(NAND_FLSH_2ND_SYNDR_1(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_2ND_SYNDR_1 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_2ND_SYNDR_1(io_base), tmp);
#endif
     synbytes_2nd[0] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_2nd[1] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_2nd[2] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_2ND_SYNDR_2(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_2ND_SYNDR_2 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_2ND_SYNDR_2(io_base), tmp);
#endif
     synbytes_2nd[3] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_2nd[4] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_2nd[5] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_2ND_SYNDR_3(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_2ND_SYNDR_3 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_2ND_SYNDR_3(io_base), tmp);
#endif
     synbytes_2nd[6] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_2nd[7] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_2nd[8] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_2ND_SYNDR_4(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_2ND_SYNDR_4 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_2ND_SYNDR_4(io_base), tmp);
#endif
     synbytes_2nd[9] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_2nd[10] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_2nd[11] = NAND_SYNDROME_CODE_3(tmp);
     for (i = 0; i < 12; i++)
	  syndrome_2nd_err_flag |= synbytes_2nd[i];

     /* Third 512 bytes */
     tmp = ioread32(NAND_FLSH_3RD_SYNDR_1(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_3RD_SYNDR_1 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_3RD_SYNDR_1(io_base), tmp);
#endif
     synbytes_3rd[0] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_3rd[1] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_3rd[2] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_3RD_SYNDR_2(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_3RD_SYNDR_2 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_3RD_SYNDR_2(io_base), tmp);
#endif
     synbytes_3rd[3] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_3rd[4] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_3rd[5] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_3RD_SYNDR_3(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_3RD_SYNDR_3 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_3RD_SYNDR_3(io_base), tmp);
#endif
     synbytes_3rd[6] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_3rd[7] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_3rd[8] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_3RD_SYNDR_4(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_3RD_SYNDR_4 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_3RD_SYNDR_4(io_base), tmp);
#endif
     synbytes_3rd[9] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_3rd[10] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_3rd[11] = NAND_SYNDROME_CODE_3(tmp);
     for (i = 0; i < 12; i++)
	  syndrome_3rd_err_flag |= synbytes_3rd[i];

     /* Forth 512 bytes */
     tmp = ioread32(NAND_FLSH_4TH_SYNDR_1(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_4TH_SYNDR_1 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_4TH_SYNDR_1(io_base), tmp);
#endif
     synbytes_4th[0] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_4th[1] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_4th[2] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_4TH_SYNDR_2(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_4TH_SYNDR_2 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_4TH_SYNDR_2(io_base), tmp);
#endif
     synbytes_4th[3] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_4th[4] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_4th[5] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_4TH_SYNDR_3(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_4TH_SYNDR_3 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_4TH_SYNDR_3(io_base), tmp);
#endif
     synbytes_4th[6] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_4th[7] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_4th[8] = NAND_SYNDROME_CODE_3(tmp);
     tmp = ioread32(NAND_FLSH_4TH_SYNDR_4(io_base));
#ifdef SYN_DEBUG
     printk("\nobi_debug: NAND_FLSH_4TH_SYNDR_4 = 0x%08x, addr = 0x%08x\n", 
	    NAND_FLSH_4TH_SYNDR_4(io_base), tmp);
#endif
     synbytes_4th[9] = NAND_SYNDROME_CODE_1(tmp);
     synbytes_4th[10] = NAND_SYNDROME_CODE_2(tmp);
     synbytes_4th[11] = NAND_SYNDROME_CODE_3(tmp);
     for (i = 0; i < 12; i++)
	  syndrome_4th_err_flag |= synbytes_4th[i];

#ifdef SYN_DEBUG
     printk("\nob_debug: syndrome_1st_err_flag = %d\n", syndrome_1st_err_flag);
     for (i = 0; i <12; i++)
	  printk("\nobi_debug: synbytes_1st[%d] = 0x%08x\n", i, synbytes_1st[i]);
     printk("\nob_debug: syndrome_2nd_err_flag = %d\n", syndrome_2nd_err_flag);
     for (i = 0; i <12; i++)
	  printk("\nobi_debug: synbytes_2nd[%d] = 0x%08x\n", i, synbytes_2nd[i]);
     printk("\nob_debug: syndrome_3rd_err_flag = %d\n", syndrome_3rd_err_flag);
     for (i = 0; i <12; i++)
	  printk("\nobi_debug: synbytes_3rd[%d] = 0x%08x\n", i, synbytes_3rd[i]);
     printk("\nob_debug: syndrome_4th_err_flag = %d\n", syndrome_4th_err_flag);
     for (i = 0; i <12; i++)
	  printk("\nobi_debug: synbytes_4th[%d] = 0x%08x\n", i, synbytes_4th[i]);
#endif

detect_erased_block:
     if (2048 == mtd->writesize) {
	  if (!(syndrome_1st_err_flag && syndrome_2nd_err_flag && 
		syndrome_3rd_err_flag && syndrome_4th_err_flag))
	       goto data_correct;
     }
#if 0
     printk("\nobi_debug: detect erased block\n");
     nand_buf_pos = NAND_FLSH_BUFFER(chip->controller->io_base);
     for (i = 0; i < (mtd->writesize >> 2); i += 4) {
#if 0
	  printk("\n0x%08x: 0x%08x ", i*4, ioread32(nand_buf_pos));
	  nand_buf_pos += 4;		 
	  printk("0x%08x ", ioread32(nand_buf_pos));
	  nand_buf_pos += 4;
	  printk("0x%08x ", ioread32(nand_buf_pos));
	  nand_buf_pos += 4;
	  printk("0x%08x ", ioread32(nand_buf_pos));
	  nand_buf_pos += 4;
#else
	  printk("\n0x%08x: 0x%08x ", i*4, *((u32 *)buf+i));
	  printk("0x%08x ", *((u32 *)buf+i+1));
	  printk("0x%08x ", *((u32 *)buf+i+2));
	  printk("0x%08x ", *((u32 *)buf+i+3));
#endif
     }
#endif
     for (i = 0; i < (mtd->writesize >> 2); i++) {
	  if (*((u32 *)buf + i) != 0xffffffff)
	       goto data_correct;
     }

     return 0;

data_correct:
     if (syndrome_1st_err_flag) {
	  for (i = 0; i < 12; i++)
	       synBytes[i] = synbytes_1st[i];
	  if (correct_errors_hw_ecc(codeword, ML)) {
	       printk(KERN_WARNING"\nsocle_nand_host: data can not be corrected between 0 and 511 bytes in " 
		      "page #%d (0x%08x)\n", page, page<<chip->page_shift);
	       return -EBADMSG;
	  } else 
	       printk(KERN_NOTICE"\nsocle_nand_host: data is corrected between 0 and 511 bytes in "
		      "page #%d (0x%08x)\n", page, page<<chip->page_shift);
     }
     codeword += 512;
     if (syndrome_2nd_err_flag) {
	  for (i = 0; i < 12; i++)
	       synBytes[i] = synbytes_2nd[i];
	  if (correct_errors_hw_ecc(codeword, ML)) {
	       printk(KERN_WARNING"\nsocle_nand_host: data can not be corrected between 512 and 1023 bytes in " 
		      "page #%d (0x%08x)\n", page, page<<chip->page_shift);
	       return -EBADMSG;
	  } else 
	       printk(KERN_NOTICE"\nsocle_nand_host: data is corrected between 512 and 1023 bytes in "
		      "page #%d (0x%08x)\n", page, page<<chip->page_shift);
     }
     codeword += 512;
     if (syndrome_3rd_err_flag) {
	  for (i = 0; i < 12; i++)
	       synBytes[i] = synbytes_3rd[i];
	  if (correct_errors_hw_ecc(codeword, ML)) {
	       printk(KERN_WARNING"\nsocle_nand_host: data can not be corrected between 1024 and 1535 bytes in " 
		      "page #%d (0x%08x)\n", page, page<<chip->page_shift);
	       return -EBADMSG;
	  } else 
	       printk(KERN_NOTICE"\nsocle_nand_host: data is corrected between 1024 and 1535 bytes in "
		      "page #%d (0x%08x)\n", page, page<<chip->page_shift);
     }
     codeword += 512;
     if (syndrome_4th_err_flag) {
	  for (i = 0; i < 12; i++)
	       synBytes[i] = synbytes_4th[i];
	  if (correct_errors_hw_ecc(codeword, ML)) {
	       printk(KERN_WARNING"\nsocle_nand_host: data can not be corrected between 1536 and 2047 bytes in " 
		      "page #%d (0x%08x)\n", page, page<<chip->page_shift);
	       return -EBADMSG;
	  } else 
	       printk(KERN_NOTICE"\nsocle_nand_host: data is corrected between 1536 and 2047 bytes in "
		      "page #%d (0x%08x)\n", page, page<<chip->page_shift);
     }
     codeword += 512;
     return 0;
}
#endif

/* Define some generic bad / good block scan pattern which are used
 * while scanning a device for factory marked good / bad blocks. */
static uint8_t scan_ff_pattern[] = { 0xff, 0xff };

static struct nand_bbt_descr smallpage_flashbased = {
     .options = NAND_BBT_SCAN2NDPAGE,
     .offs = 5,
     .len = 1,
     .pattern = scan_ff_pattern
};

static struct nand_bbt_descr largepage_flashbased = {
     .options = NAND_BBT_SCAN2NDPAGE,
     .offs = 0,
     .len = 2,
     .pattern = scan_ff_pattern
};

static int
nand_scan_initial_badblock(struct mtd_info *mtd)
{
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     int i;

     chip->bbt_td = NULL;
     chip->bbt_md = NULL;
     if (!chip->badblock_pattern)
	  chip->badblock_pattern = (mtd->writesize > 512) ?
	       &largepage_flashbased : &smallpage_flashbased;

     /* Loop through chip array */
     for (i = 0; i < chip->numchips; i++) {
	  /* Check if this chip has has been scanned */
	  if (nand_chip_isscanned(mtd, i))
	       continue;

	  /* Scan all pages in the chip */
	  if (nand_scan_bad(mtd, chip, i))
	       return -1;;

	  if (nand_chip_markscanned(mtd, i))
	       return -1;
     }

     return 0;
}

static int
nand_scan_bad(struct mtd_info *mtd, struct nand_chip *chip, int chipnr)
{
     struct nand_bbt_descr *bd = chip->badblock_pattern;
     int i, numblocks, len, scanlen;
     int startblock;
     loff_t from;
     size_t readlen;
     uint8_t * buf;

     /* Allocate a temporary buffer for one eraseblock incl. oob */
     len = (1 << chip->bbt_erase_shift);
     len += (len >> chip->page_shift) * mtd->oobsize;
     buf = kmalloc(len, GFP_KERNEL);
     if (!buf)
	  return -ENOMEM;

     if (bd->options & NAND_BBT_SCANALLPAGES)
	  len = 1 << (chip->bbt_erase_shift - chip->page_shift);
     else {
	  if (bd->options & NAND_BBT_SCAN2NDPAGE)
	       len = 2;
	  else
	       len = 1;
     }

     if (!(bd->options & NAND_BBT_SCANEMPTY)) {
	  /* We need only read few bytes from the OOB area */
	  scanlen = 0;
	  readlen = bd->len;
     } else {
	  /* Full page content should be read */
	  scanlen = mtd->writesize + mtd->oobsize;
	  readlen = len * mtd->writesize;
     }

     if (chipnr >= chip->numchips) {
	  printk(KERN_ERR"\nsocle_nand_host: nand_scan_bad(): chipnr (%d) > available chips (%d)\n",
		 chipnr+2, chip->numchips);
	  return -EINVAL;
     }

     /* Note that numblocks is 2 * (real numblocks) here, see i+=2
      * below as it makes shifting and masking less painful*/
     numblocks = chip->chipsize >> (chip->bbt_erase_shift - 1);
     startblock = chipnr * numblocks;
     numblocks += startblock;
     from = startblock << (chip->bbt_erase_shift - 1);

     for (i = startblock; i < numblocks;) {
	  int ret;

	  if (bd->options & NAND_BBT_SCANALLPAGES)
	       ret = scan_block_full(mtd, bd, from, buf, 
				     readlen, scanlen, len);
	  else
	       ret = scan_block_fast(mtd, bd, from, buf, len);
	  if (ret < 0)
	       return ret;
	  if (ret) {
	       printk(KERN_INFO"\nnand_scan_bad(): Bad eraseblock %d at 0x%08x\n",
		      i >> 1, (unsigned int)from);
	       mtd->block_markbad(mtd, from);
	       mtd->ecc_stats.badblocks++;
	  }
	  i += 2;
	  from += (1 << chip->bbt_erase_shift);
     }
     kfree(buf);
     return 0;
}

/*
 * Scan a given block full
 */
static int 
scan_block_full(struct mtd_info *mtd, struct nand_bbt_descr *bd, loff_t offs,
		uint8_t *buf, size_t readlen, int scanlen,
		int len)
{
     int ret, j;

     struct mtd_oob_ops ops;

     ops.mode = MTD_OOB_RAW;
     ops.ooboffs = 0;
     ops.ooblen = mtd->oobsize;
     ops.oobbuf = buf;
     ops.len = len;
     ret = mtd->read_oob(mtd, offs, &ops);
     if (ret)
	  return ret;

     for (j = 0; j < len; j++, buf += scanlen) {
	  if (check_pattern(buf, scanlen, mtd->writesize, bd))
	       return 1;
     }
     return 0;
}

/**
 * check_pattern - [GENERIC] check if a pattern is in the buffer
 * @buf:	the buffer to search
 * @len:	the length of buffer to search
 * @paglen:	the pagelength
 * @td:		search pattern descriptor
 *
 * Check for a pattern at the given place. Used to search bad block
 * tables and good / bad block identifiers.
 * If the SCAN_EMPTY option is set then check, if all bytes except the
 * pattern area contain 0xff
 *
 */
static int 
check_pattern(uint8_t *buf, int len, int paglen,
	      struct nand_bbt_descr *td)
{
     int i, end = 0;
     uint8_t *p = buf;

     end = paglen + td->offs;
     if (td->options & NAND_BBT_SCANEMPTY) {
	  for (i = 0; i < end; i++) {
	       if (p[i] != 0xff)
		    return -1;
	  }
     }
     p += end;

     /* Compare the pattern */
     for (i = 0; i < td->len; i++) {
	  if (p[i] != td->pattern[i])
	       return -1;
     }

     if (td->options & NAND_BBT_SCANEMPTY) {
	  p += td->len;
	  end += td->len;
	  for (i = end; i < len; i++) {
	       if (*p++ != 0xff)
		    return -1;
	  }
     }
     return 0;
}

/*
 * Scan a given block partially
 */
static int
scan_block_fast(struct mtd_info *mtd, struct nand_bbt_descr *bd, loff_t offs,
		uint8_t *buf, int len)
{
     struct mtd_oob_ops ops;
     int j, ret;

     ops.len = mtd->oobsize;
     ops.ooblen = mtd->oobsize;
     ops.oobbuf = buf;
     ops.datbuf = NULL;
     ops.mode = MTD_OOB_PLACE;
     for (j = 0; j < len; j++) {
	  /*
	   *  Read the full oob until read_oob is fixed to
	   *  handle single byte reads for 16 bit
	   *  buswidth
	   *  */
	  ret = mtd->read_oob(mtd, offs, &ops);
	  if (ret)
	       return ret;

	  if (check_short_pattern(buf, bd))
	       return 1;

	  offs += mtd->writesize;
     }
     return 0;
}

/**
 * check_short_pattern - [GENERIC] check if a pattern is in the buffer
 * @buf:	the buffer to search
 * @td:		search pattern descriptor
 *
 * Check for a pattern at the given place. Used to search bad block
 * tables and good / bad block identifiers. Same as check_pattern, but
 * no optional empty check
 *
 */
static int 
check_short_pattern(uint8_t *buf, struct nand_bbt_descr *td)
{
     int i;
     uint8_t *p = buf;

     /* Compare the pattern */
     for (i = 0; i < td->len; i++) {
	  if (p[td->offs + i] != td->pattern[i])
	       return -1;
     }
     return 0;
}

static int
nand_chip_isscanned(struct mtd_info *mtd, int chipnr)
{
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     u32 nand_buf_pos = chip->controller->buffer;
     u32 nand_word;
     int page, i, ret = 1;

     nand_get_device(chip, FL_READING);
     nand_select_chip(chip, chipnr);
     page = (int)(NAND_SCANNED_MARK_ADDR >> chip->page_shift);
     chip->read(chip, 0, page, mtd->writesize, 1);

     /* Check if the device has been scanned */
     for (i = 0; i < (mtd->writesize >> 2); i++) {
	  nand_word = ioread32(nand_buf_pos);
	  if (nand_word != NAND_DEVICE_SCANNED_MARKER) {
	       ret = 0;
	       goto out;
	  }
	  nand_buf_pos += 4;
     }

out:
     nand_release_device(chip);
     return ret;    
}

static int
nand_chip_markscanned(struct mtd_info *mtd, int chipnr)
{
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     u32 nand_buf_pos = chip->controller->buffer;
     struct erase_info instr;
     u32 page;     
     int status = 0, i;

     /* Erase block 0 to write the marker */
     instr.mtd = mtd;
     instr.addr = chip->chipsize * chipnr;
     instr.len = 1 << chip->phys_erase_shift;
     instr.callback = NULL;
     status = mtd->erase(mtd, &instr);
     if (status)
	  return -1;

     nand_get_device(chip, FL_WRITING);

     /* Select the chip */
     nand_select_chip(chip, chipnr);

     page = (int)(NAND_SCANNED_MARK_ADDR >> chip->page_shift);

     /* Write data into nand buffer */
     for (i = 0; i < (mtd->writesize >> 2); i++) {
	  iowrite32(NAND_DEVICE_SCANNED_MARKER, nand_buf_pos);
	  nand_buf_pos += 4;
     }

     status = chip->program(chip, 0, page, mtd->writesize, 1);
     if (!status)
	  status = chip->read(chip, 0, page, mtd->writesize, 1);
     nand_release_device(chip);
     return status;
}

static u32 
nand_read_id(struct nand_chip *chip)
{
     u32 id;
     u32 status;
     u32 io_base = chip->controller->io_base;

     /* Reset registers */
     NAND_DISABLE_FLSH_RESET(io_base);
     NAND_RESET_FLSH_INT_MASK(io_base);
     NAND_RESET_FLSH_INT_STATE(io_base);

     /* Config setting */
#ifdef TIMEING_TUNE
     iowrite32(NAND_tWC_4CYCLE |
	       NAND_tWP_2CYCLE |
	       NAND_tRC_4CYCLE |
	       NAND_tRP_2CYCLE |
	       NAND_PAGE_512BYTES,
	       NAND_FLSH_CONF(io_base));
#else
     iowrite32(NAND_tWC_15CYCLE |
	       NAND_tWP_7CYCLE |
	       NAND_tRC_15CYCLE |
	       NAND_tRP_7CYCLE |
	       NAND_PAGE_512BYTES,
	       NAND_FLSH_CONF(io_base));
#endif

     /* Command */
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_READID),
	       NAND_FLSH_COMM_1(io_base));
     iowrite32(NAND_CMD_NOTVALID,
	       NAND_FLSH_COMM_2(io_base));
     iowrite32(NAND_CMD_NOTVALID,
	       NAND_FLSH_STATE_COMM(io_base));

     /* Address */
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(0x00),
	       NAND_FLSH_ADDRESS_1(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_2(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_3(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_4(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_5(io_base));

     /* DMA setting */
     iowrite32(NAND_LITTLE_ENDING |
	       NAND_ECC_TESTMODE_DISABLE |
	       NAND_BUFFER_RW_DISABLE |
	       NAND_SECOND_CMD_NO |
	       NAND_AUTO_STATUS_CHECK_DISABLE |
	       NAND_ACCESS_DATA_REG_PORT |
	       NAND_DMA_READ |
	       NAND_DEVICE_NOT_WAIT_RB,
	       NAND_FLSH_DMA_SET(io_base));

     /* Set buffer count */
	iowrite32(NAND_RW_WITHOUT_ECC |
	       NAND_InterBuff_DATA_NUM(3),
	       NAND_FLSH_BUFF_CNT(io_base));
	 
     /* Start operation */
     iowrite32(NAND_DMA_ENABLE |
	       NAND_DATA_RW_ENABLE |
	       NAND_SEND_CMD_ADDR_ENABLE,
	       NAND_FLSH_CONTROL(io_base));

     /* Polling until data reading is done */
     while (1) {
	  status = ioread32(NAND_FLSH_INT_STATE(io_base));
	  if (status & NAND_READ_DATA_DONE)
	       break;
     }

     id = ioread32(NAND_FLSH_DATA(io_base));
     return id;
}

static int 
nand_erase_block_l(struct nand_chip *chip, u32 page)
{
     u32 io_base = chip->controller->io_base;
     u32 status;
     int ret;

	//printk("nand_erase_block_l : page = 0x%08x\n",page);

     /* Reset registers */
     NAND_DISABLE_FLSH_RESET(io_base);
     NAND_RESET_FLSH_INT_MASK(io_base);
     NAND_RESET_FLSH_INT_STATE(io_base);

     /* Config setting */
     iowrite32(chip->timing->tWC |
	       chip->timing->tWP |
	       chip->timing->tRC |
	       chip->timing->tRP |
	       NAND_PAGE_512BYTES,
	       NAND_FLSH_CONF(io_base));

     /* Command */
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_ERASE1),
	       NAND_FLSH_COMM_1(io_base));
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_ERASE2),
	       NAND_FLSH_COMM_2(io_base));
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_STATUS),
	       NAND_FLSH_STATE_COMM(io_base));

     /* Address */
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_1(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_2(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page),
	       NAND_FLSH_ADDRESS_3(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page >> 8),
	       NAND_FLSH_ADDRESS_4(io_base));
#if 0		
     if (chip->chipsize > (128 << 20))
	  iowrite32(NAND_ADDR_VALID |
		    NAND_ADDR_INPUT(page >> 16),
		    NAND_FLSH_ADDRESS_5(io_base));
     else
	  iowrite32(NAND_ADDR_NOTVALID,
		    NAND_FLSH_ADDRESS_5(io_base));
#else		//for P7DKv1 
	  iowrite32(NAND_ADDR_VALID |
	    NAND_ADDR_INPUT(page >> 16),
	    NAND_FLSH_ADDRESS_5(io_base));
#endif
     /* DMA setting */
     iowrite32(NAND_LITTLE_ENDING |
	       NAND_ECC_TESTMODE_DISABLE |
	       NAND_BUFFER_RW_DISABLE |
	       NAND_SECOND_CMD_YES |
	       NAND_AUTO_STATUS_CHECK_ENABLE |
	       NAND_ACCESS_DATA_INTER_BUF |
	       NAND_DMA_NOP |
	       NAND_DEVICE_WAIT_RB,
	       NAND_FLSH_DMA_SET(io_base));

     /* Start operation */
     iowrite32(NAND_DMA_ENABLE |
	       NAND_DATA_RW_DISABLE |
	       NAND_SEND_CMD_ADDR_ENABLE,
	       NAND_FLSH_CONTROL(io_base));

     chip->timeout_flag = 0;	/* Reset the timeout flag */
     //chip->timer.expires = jiffies + (2 * HZ / 100); /* 20msec */
     chip->timer.expires = jiffies + (20 * HZ / 100); /* 200msec */
     add_timer(&chip->timer);
     while (1) {
	  status = ioread32(NAND_FLSH_INT_STATE(io_base));
	  if (status & NAND_BLOCK_ERASE_FAIL) {
	       ret = -EIO;
	       goto out;
	  }
	  if (status & NAND_BLOCK_ERASE_DONE) {
	       ret = 0;
	       goto out;
	  }
	  if (chip->timeout_flag) {
	       printk(KERN_ERR"\nsocle_nand_host: nand_erase_block_l() is timeout\n");
	       return -EIO;
	  }
     }
out:
     del_timer(&chip->timer);
     return ret;
}

static int 
nand_read_l(struct nand_chip *chip, u32 column, u32 page,
	    u32 len, int ecc)
{
     u32 io_base = chip->controller->io_base;
     u32 status;
     int ret;

	//printk("nand_read_l : column = 0x%08x, page = 0x%08x, len = 0x%08x, ecc = %d\n", column, page, len, ecc);

     /* Reset registers */
     NAND_DISABLE_FLSH_RESET(io_base);
     NAND_RESET_FLSH_INT_MASK(io_base);
     NAND_RESET_FLSH_INT_STATE(io_base);

     /* Config setting */
     iowrite32(chip->timing->tWC |
	       chip->timing->tWP |
	       chip->timing->tRC |
	       chip->timing->tRP |
	       NAND_PAGE_2KBYTES,
	       NAND_FLSH_CONF(io_base));

     /* Command */
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_READ0),
	       NAND_FLSH_COMM_1(io_base));
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_READSTART),
	       NAND_FLSH_COMM_2(io_base));
     iowrite32(NAND_CMD_NOTVALID,
	       NAND_FLSH_STATE_COMM(io_base));

     /* Address */
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(column),
	       NAND_FLSH_ADDRESS_1(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(column >> 8),
	       NAND_FLSH_ADDRESS_2(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page),
	       NAND_FLSH_ADDRESS_3(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page >> 8),
	       NAND_FLSH_ADDRESS_4(io_base));
#if 0
     if (chip->chipsize > (128 << 20))
	  iowrite32(NAND_ADDR_VALID |
		    NAND_ADDR_INPUT(page >> 16),
		    NAND_FLSH_ADDRESS_5(io_base));
     else
	  iowrite32(NAND_ADDR_NOTVALID,
		    NAND_FLSH_ADDRESS_5(io_base));
#else
	  iowrite32(NAND_ADDR_VALID |
	    NAND_ADDR_INPUT(page >> 16),
	    NAND_FLSH_ADDRESS_5(io_base));
#endif

     /* DMA setting */
     iowrite32(NAND_LITTLE_ENDING |
	       NAND_ECC_TESTMODE_DISABLE |
	       NAND_BUFFER_RW_ENABLE |
	       NAND_SECOND_CMD_YES |
	       NAND_AUTO_STATUS_CHECK_DISABLE |
	       NAND_ACCESS_DATA_INTER_BUF |
	       NAND_DMA_READ |
	       NAND_DEVICE_WAIT_RB,
	       NAND_FLSH_DMA_SET(io_base));

     /* Set the start address of buffer */
     iowrite32(NAND_InterBuff_START_ADDR(0),
	       NAND_FLSH_BUFF_STADDR(io_base));

     /* Set buffer count and ecc mode */
	if (ecc)		
#ifdef CONFIG_NAND_TWO_ECC_MODEL

#ifdef CONFIG_NAND_15B
		iowrite32(NAND_RW_WITH_ECC | NAND_NPAR_LEN_15B |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
#else
		iowrite32(NAND_RW_WITH_ECC | NAND_NPAR_LEN_8B |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
#endif

#else
		iowrite32(NAND_RW_WITH_ECC |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
	
#endif
	else
		iowrite32(NAND_RW_WITHOUT_ECC |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
  
     /* Start operation */
     iowrite32(NAND_DMA_ENABLE |
	       NAND_DATA_RW_ENABLE |
	       NAND_SEND_CMD_ADDR_ENABLE,
	       NAND_FLSH_CONTROL(io_base));

     chip->timeout_flag = 0;	/* Reset the timeout flag */
     //chip->timer.expires = jiffies + (2 * HZ / 100); /* 20msec */
     chip->timer.expires = jiffies + (200 * HZ / 100); /* 200msec */
     add_timer(&chip->timer);
     while (1) {
	  status = ioread32(NAND_FLSH_INT_STATE(io_base));
	  if (status & NAND_READ_DATA_DONE)
	       break;
	  if (chip->timeout_flag) {
	       printk(KERN_ERR"\nsocle_nand_host: nand_read_l() is timeout\n");
	       return -EIO;
	  }
     }
     if (status & NAND_SYNDROME_ERROR_LOCATE4_MASK)
	  ret = status;
     else
	  ret = 0;
     del_timer(&chip->timer);
     return ret;
}

static int 
nand_program_l(struct nand_chip *chip, u32 column, u32 page, 
	       u32 len, int ecc)
{
     u32 io_base = chip->controller->io_base;
     u32 status;
     int ret;

	//printk("nand_program_l : column = 0x%08x, page = 0x%08x, len = 0x%08x, ecc = %d\n", column, page, len, ecc);

     /* Reset registers */
     NAND_DISABLE_FLSH_RESET(io_base);
     NAND_RESET_FLSH_INT_MASK(io_base);
     NAND_RESET_FLSH_INT_STATE(io_base);

     /* Config setting */
     iowrite32(chip->timing->tWC |
	       chip->timing->tWP |
	       chip->timing->tRC |
	       chip->timing->tRP |
	       NAND_PAGE_2KBYTES,
	       NAND_FLSH_CONF(io_base));

     /* Command */
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_SEQIN),
	       NAND_FLSH_COMM_1(io_base));
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_PAGEPROG),
	       NAND_FLSH_COMM_2(io_base));
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_STATUS),
	       NAND_FLSH_STATE_COMM(io_base));

     /* Address */
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(column),
	       NAND_FLSH_ADDRESS_1(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(column >> 8),
	       NAND_FLSH_ADDRESS_2(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page),
	       NAND_FLSH_ADDRESS_3(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page >> 8),
	       NAND_FLSH_ADDRESS_4(io_base));
#if 0
     if (chip->chipsize > (128 << 20))
	  iowrite32(NAND_ADDR_VALID |
		    NAND_ADDR_INPUT(page >> 16),
		    NAND_FLSH_ADDRESS_5(io_base));
     else
	  iowrite32(NAND_ADDR_NOTVALID,
		    NAND_FLSH_ADDRESS_5(io_base));
#else
	  iowrite32(NAND_ADDR_VALID |
	    NAND_ADDR_INPUT(page >> 16),
	    NAND_FLSH_ADDRESS_5(io_base));
#endif

     /* DMA setting */
     iowrite32(NAND_LITTLE_ENDING |
	       NAND_ECC_TESTMODE_DISABLE |
	       NAND_BUFFER_RW_ENABLE |
	       NAND_SECOND_CMD_YES |
	       NAND_AUTO_STATUS_CHECK_ENABLE |
	       NAND_ACCESS_DATA_INTER_BUF |
	       NAND_DMA_WRITE |
	       NAND_DEVICE_WAIT_RB,
	       NAND_FLSH_DMA_SET(io_base));

     /* Set the start address of buffer */
     iowrite32(NAND_InterBuff_START_ADDR(0),
	       NAND_FLSH_BUFF_STADDR(io_base));

     /* Set buffer count and ecc mode */
	if (ecc)		
#ifdef CONFIG_NAND_TWO_ECC_MODEL

#ifdef CONFIG_NAND_15B
		iowrite32(NAND_RW_WITH_ECC | NAND_NPAR_LEN_15B |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
#else
		iowrite32(NAND_RW_WITH_ECC | NAND_NPAR_LEN_8B |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
#endif

#else
		iowrite32(NAND_RW_WITH_ECC |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
	
#endif
	else
		iowrite32(NAND_RW_WITHOUT_ECC |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));

     /* Start operation */
     iowrite32(NAND_DMA_ENABLE |
	       NAND_DATA_RW_ENABLE |
	       NAND_SEND_CMD_ADDR_ENABLE,
	       NAND_FLSH_CONTROL(io_base));

     chip->timeout_flag = 0;	/* Reset the timeout flag */
     //chip->timer.expires = jiffies + (2 * HZ / 100); /* 20msec */
     chip->timer.expires = jiffies + (20 * HZ / 100); /* 200msec */
     add_timer(&chip->timer);
     while (1) {
	  status = ioread32(NAND_FLSH_INT_STATE(io_base));
	  if (status & NAND_WRITE_DATA_ERROR) {
	       ret = -EIO;
	       goto out;
	  }
	  if (status & NAND_WRITE_DATA_DONE) {
	       ret = 0;
	       goto out;
	  }
	  if (chip->timeout_flag) {
	       printk(KERN_ERR"\nsocle_nand_host: nand_program_l() is timeout\n");
	       return -EIO;
	  }
     }
out:
     del_timer(&chip->timer);
     return ret;
}

static int 
nand_cache_program_l(struct nand_chip *chip, u32 column, u32 page, 
		     u32 len, int ecc)
{
     u32 io_base = chip->controller->io_base;
     u32 status;
     int ret;

     /* Reset registers */
     NAND_DISABLE_FLSH_RESET(io_base);
     NAND_RESET_FLSH_INT_MASK(io_base);
     NAND_RESET_FLSH_INT_STATE(io_base);

     /* Config setting */
     iowrite32(chip->timing->tWC |
	       chip->timing->tWP |
	       chip->timing->tRC |
	       chip->timing->tRP |
	       NAND_PAGE_2KBYTES,
	       NAND_FLSH_CONF(io_base));

     /* Command */
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_SEQIN),
	       NAND_FLSH_COMM_1(io_base));
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_CACHEDPROG),
	       NAND_FLSH_COMM_2(io_base));
     iowrite32(NAND_CMD_NOTVALID,
	       NAND_FLSH_STATE_COMM(io_base));

     /* Address */
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(column),
	       NAND_FLSH_ADDRESS_1(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(column >> 8),
	       NAND_FLSH_ADDRESS_2(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page),
	       NAND_FLSH_ADDRESS_3(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page >> 8),
	       NAND_FLSH_ADDRESS_4(io_base));
     if (chip->chipsize > (128 << 20))
	  iowrite32(NAND_ADDR_VALID |
		    NAND_ADDR_INPUT(page >> 16),
		    NAND_FLSH_ADDRESS_5(io_base));
     else
	  iowrite32(NAND_ADDR_NOTVALID,
		    NAND_FLSH_ADDRESS_5(io_base));

     /* DMA setting */
     iowrite32(NAND_LITTLE_ENDING |
	       NAND_ECC_TESTMODE_DISABLE |
	       NAND_BUFFER_RW_ENABLE |
	       NAND_SECOND_CMD_YES |
	       NAND_AUTO_STATUS_CHECK_DISABLE |
	       NAND_ACCESS_DATA_INTER_BUF |
	       NAND_DMA_WRITE |
	       NAND_DEVICE_WAIT_RB,
	       NAND_FLSH_DMA_SET(io_base));

     /* Set the start address of buffer */
     iowrite32(NAND_InterBuff_START_ADDR(0),
	       NAND_FLSH_BUFF_STADDR(io_base));

     /* Set buffer count and ecc mode */
	if (ecc)		
#ifdef CONFIG_NAND_TWO_ECC_MODEL

#ifdef CONFIG_NAND_15B
		iowrite32(NAND_RW_WITH_ECC | NAND_NPAR_LEN_15B |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
#else
		iowrite32(NAND_RW_WITH_ECC | NAND_NPAR_LEN_8B |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
#endif

#else
		iowrite32(NAND_RW_WITH_ECC |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
	
#endif
	else
		iowrite32(NAND_RW_WITHOUT_ECC |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));


     /* Start operation */
     iowrite32(NAND_DMA_ENABLE |
	       NAND_DATA_RW_ENABLE |
	       NAND_SEND_CMD_ADDR_ENABLE,
	       NAND_FLSH_CONTROL(io_base));

     chip->timeout_flag = 0;	/* Reset the timeout flag */
     //chip->timer.expires = jiffies + (2 * HZ / 100); /* 20msec */
     chip->timer.expires = jiffies + (20 * HZ / 100); /* 200msec */
     add_timer(&chip->timer);
     while (1) {
	  status = ioread32(NAND_FLSH_INT_STATE(io_base));
	  if (status & NAND_WRITE_DATA_ERROR) {
	       ret = -EIO;
	       goto out;
	  }
	  if (status & NAND_WRITE_DATA_DONE) {
	       ret = 0;
	       goto out;
	  }
	  if (chip->timeout_flag) {
	       printk(KERN_ERR"\nsocle_nand_host: nand_cache_program_l() is timeout\n");
	       return -EIO;
	  }
     }
out:
     del_timer(&chip->timer);
     return ret;
}

static int 
nand_erase_block_s(struct nand_chip *chip, u32 page)
{
     u32 io_base = chip->controller->io_base;
     u32 status;
     int ret;

     /* Reset registers */
     NAND_DISABLE_FLSH_RESET(io_base);
     NAND_RESET_FLSH_INT_MASK(io_base);
     NAND_RESET_FLSH_INT_STATE(io_base);

     /* Config setting */
     iowrite32(chip->timing->tWC |
	       chip->timing->tWP |
	       chip->timing->tRC |
	       chip->timing->tRP |
	       NAND_PAGE_512BYTES,
	       NAND_FLSH_CONF(io_base));

     /* Command */
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_ERASE1),
	       NAND_FLSH_COMM_1(io_base));
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_ERASE2),
	       NAND_FLSH_COMM_2(io_base));
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_STATUS),
	       NAND_FLSH_STATE_COMM(io_base));

     /* Address */
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_1(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_2(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page),
	       NAND_FLSH_ADDRESS_3(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page >> 8),
	       NAND_FLSH_ADDRESS_4(io_base));
     if (chip->chipsize > (32 << 20))
	  iowrite32(NAND_ADDR_VALID |
		    NAND_ADDR_INPUT(page >> 16),
		    NAND_FLSH_ADDRESS_5(io_base));
     else
	  iowrite32(NAND_ADDR_NOTVALID,
		    NAND_FLSH_ADDRESS_5(io_base));

     /* DMA setting */
     iowrite32(NAND_LITTLE_ENDING |
	       NAND_ECC_TESTMODE_DISABLE |
	       NAND_BUFFER_RW_DISABLE |
	       NAND_SECOND_CMD_YES |
	       NAND_AUTO_STATUS_CHECK_ENABLE |
	       NAND_ACCESS_DATA_INTER_BUF |
	       NAND_DMA_NOP |
	       NAND_DEVICE_WAIT_RB,
	       NAND_FLSH_DMA_SET(io_base));

     /* Start operation */
     iowrite32(NAND_DMA_ENABLE |
	       NAND_DATA_RW_DISABLE |
	       NAND_SEND_CMD_ADDR_ENABLE,
	       NAND_FLSH_CONTROL(io_base));

     chip->timeout_flag = 0;	/* Reset the timeout flag */
     //chip->timer.expires = jiffies + (2 * HZ / 100); /* 20msec */
     chip->timer.expires = jiffies + (20 * HZ / 100); /* 200msec */
     add_timer(&chip->timer);
     while (1) {
	  status = ioread32(NAND_FLSH_INT_STATE(io_base));
	  if (status & NAND_BLOCK_ERASE_FAIL) {
	       ret = -EIO;
	       goto out;	   
	  }
	  if (status & NAND_BLOCK_ERASE_DONE) {
	       ret = 0;
	       goto out;
	  }
	  if (chip->timeout_flag) {
	       printk(KERN_ERR"\nsocle_nand_host: nand_erase_block_s() is timeout\n");
	       return -EIO;
	  }
     }
out:
     del_timer(&chip->timer);
     return ret;
}

static int 
nand_read_s(struct nand_chip *chip, u32 column, u32 page, 
	    u32 len, int ecc)
{
     u32 io_base = chip->controller->io_base;
     u32 status;
     int ret;

     /* Reset registers */
     NAND_DISABLE_FLSH_RESET(io_base);
     NAND_RESET_FLSH_INT_MASK(io_base);
     NAND_RESET_FLSH_INT_STATE(io_base);

     /* Config setting */
     iowrite32(chip->timing->tWC |
	       chip->timing->tWP |
	       chip->timing->tRC |
	       chip->timing->tRP |
	       NAND_PAGE_512BYTES,
	       NAND_FLSH_CONF(io_base));

     /* Command */
     if (column >= 512) {
	  /* OOB area */
	  column -= 512;
	  iowrite32(NAND_CMD_VALID |
		    NAND_CMD_INPUT(NAND_CMD_READOOB),
		    NAND_FLSH_COMM_1(io_base));
     } else if(column < 256) {
	  /* First 256 bytes --> READ0 */
	  iowrite32(NAND_CMD_VALID |
		    NAND_CMD_INPUT(NAND_CMD_READ0),
		    NAND_FLSH_COMM_1(io_base));
     } else {
	  column -= 256;
	  iowrite32(NAND_CMD_VALID |
		    NAND_CMD_INPUT(NAND_CMD_READ1),
		    NAND_FLSH_COMM_1(io_base));
     }
     iowrite32(NAND_CMD_NOTVALID,
	       NAND_FLSH_COMM_2(io_base));
     iowrite32(NAND_CMD_NOTVALID,
	       NAND_FLSH_STATE_COMM(io_base));

     /* Address */
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(column),
	       NAND_FLSH_ADDRESS_1(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_2(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page),
	       NAND_FLSH_ADDRESS_3(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page >> 8),
	       NAND_FLSH_ADDRESS_4(io_base));
     if (chip->chipsize > (32 << 20))
	  iowrite32(NAND_ADDR_VALID |
		    NAND_ADDR_INPUT(page >> 16),
		    NAND_FLSH_ADDRESS_5(io_base));
     else
	  iowrite32(NAND_ADDR_NOTVALID,
		    NAND_FLSH_ADDRESS_5(io_base));

     /* DMA setting */
     iowrite32(NAND_LITTLE_ENDING |
	       NAND_ECC_TESTMODE_DISABLE |
	       NAND_BUFFER_RW_ENABLE |
	       NAND_SECOND_CMD_NO |
	       NAND_AUTO_STATUS_CHECK_DISABLE |
	       NAND_ACCESS_DATA_INTER_BUF |
	       NAND_DMA_READ |
	       NAND_DEVICE_WAIT_RB,
	       NAND_FLSH_DMA_SET(io_base));

     /* Set the start address of buffer */
     iowrite32(NAND_InterBuff_START_ADDR(0),
	       NAND_FLSH_BUFF_STADDR(io_base));

     /* Set buffer count and ecc mode */
	if (ecc)		
#ifdef CONFIG_NAND_TWO_ECC_MODEL

#ifdef CONFIG_NAND_15B
		iowrite32(NAND_RW_WITH_ECC | NAND_NPAR_LEN_15B |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
#else
		iowrite32(NAND_RW_WITH_ECC | NAND_NPAR_LEN_8B |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
#endif

#else
		iowrite32(NAND_RW_WITH_ECC |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
	
#endif
	else
		iowrite32(NAND_RW_WITHOUT_ECC |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
     
     /* Start operation */
     iowrite32(NAND_DMA_ENABLE |
	       NAND_DATA_RW_ENABLE |
	       NAND_SEND_CMD_ADDR_ENABLE,
	       NAND_FLSH_CONTROL(io_base));

     chip->timeout_flag = 0;	/* Reset the timeout flag */
     //chip->timer.expires = jiffies + (2 * HZ / 100); /* 20msec */
     chip->timer.expires = jiffies + (20 * HZ / 100); /* 200msec */
     add_timer(&chip->timer);
     while (1) {
	  status = ioread32(NAND_FLSH_INT_STATE(io_base));
	  if (status & NAND_READ_DATA_DONE) 
	       break;
	  if (chip->timeout_flag) {
	       printk(KERN_ERR"\nsocle_nand_host: nand_read_l() is timeout\n");
	       return -EIO;
	  }
     }
     if (status & NAND_SYNDROME_ERROR_LOCATE4_MASK) 
	  ret = status;
     else
	  ret = 0;
     del_timer(&chip->timer);
     return ret;
}

static int 
nand_program_s(struct nand_chip *chip, u32 column, u32 page, 
	       u32 len, int ecc)
{
     u32 io_base = chip->controller->io_base;
     u32 status;
     int ret;

     /*
      *  Pointer Operation
      *  */

     /* Reset registers */
     NAND_DISABLE_FLSH_RESET(io_base);
     NAND_RESET_FLSH_INT_MASK(io_base);
     NAND_RESET_FLSH_INT_STATE(io_base);

     /* Config setting */
     iowrite32(chip->timing->tWC |
	       chip->timing->tWP |
	       chip->timing->tRC |
	       chip->timing->tRP |
	       NAND_PAGE_512BYTES,
	       NAND_FLSH_CONF(io_base));

     /* Command */
     if (column >= 512) {
	  /* OOB area */
	  column -= 512;
	  iowrite32(NAND_CMD_VALID |
		    NAND_CMD_INPUT(NAND_CMD_READOOB),
		    NAND_FLSH_COMM_1(io_base));
     } else if(column < 256) {
	  /* First 256 bytes --> READ0 */
	  iowrite32(NAND_CMD_VALID |
		    NAND_CMD_INPUT(NAND_CMD_READ0),
		    NAND_FLSH_COMM_1(io_base));
     } else {
	  column -= 256;
	  iowrite32(NAND_CMD_VALID |
		    NAND_CMD_INPUT(NAND_CMD_READ1),
		    NAND_FLSH_COMM_1(io_base));
     }
     iowrite32(NAND_CMD_NOTVALID,
	       NAND_FLSH_COMM_2(io_base));
     iowrite32(NAND_CMD_NOTVALID,
	       NAND_FLSH_STATE_COMM(io_base));

     /* Disable all address cycles */
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_1(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_2(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_3(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_4(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_5(io_base));

     /* DMA setting */
     iowrite32(NAND_LITTLE_ENDING |
	       NAND_ECC_TESTMODE_DISABLE |
	       NAND_BUFFER_RW_DISABLE |
	       NAND_SECOND_CMD_NO |
	       NAND_AUTO_STATUS_CHECK_DISABLE |
	       NAND_ACCESS_DATA_INTER_BUF |
	       NAND_DMA_READ |
	       NAND_DEVICE_NOT_WAIT_RB,
	       NAND_FLSH_DMA_SET(io_base));

     /* Start operation */
     iowrite32(NAND_DMA_ENABLE |
	       NAND_DATA_RW_ENABLE |
	       NAND_SEND_CMD_ADDR_ENABLE,
	       NAND_FLSH_CONTROL(io_base));

     chip->timeout_flag = 0;	/* Reset the timeout flag */
     //chip->timer.expires = jiffies + (2 * HZ / 100); /* 20msec */
     chip->timer.expires = jiffies + (20 * HZ / 100); /* 200msec */
     add_timer(&chip->timer);
     while (1) {
	  status = ioread32(NAND_FLSH_INT_STATE(io_base));
	  if (status & NAND_READ_DATA_DONE)
	       break;
	  if (chip->timeout_flag) {
	       printk(KERN_ERR"\nsocle_nand_host: nand_read_l() is timeout\n");
	       return -EIO;
	  }
     }
     del_timer(&chip->timer);

     /*
      *  Real page program sequence
      *  */
     /* Reset registers */
     NAND_DISABLE_FLSH_RESET(io_base);
     NAND_RESET_FLSH_INT_MASK(io_base);
     NAND_RESET_FLSH_INT_STATE(io_base);

     /* Config setting */
     iowrite32(chip->timing->tWC |
	       chip->timing->tWP |
	       chip->timing->tRC |
	       chip->timing->tRP |
	       NAND_PAGE_512BYTES,
	       NAND_FLSH_CONF(io_base));

     /* Command */
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_SEQIN),
	       NAND_FLSH_COMM_1(io_base));
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_PAGEPROG),
	       NAND_FLSH_COMM_2(io_base));
     iowrite32(NAND_CMD_VALID |
	       NAND_CMD_INPUT(NAND_CMD_STATUS),
	       NAND_FLSH_STATE_COMM(io_base));

     /* Address */
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(column),
	       NAND_FLSH_ADDRESS_1(io_base));
     iowrite32(NAND_ADDR_NOTVALID,
	       NAND_FLSH_ADDRESS_2(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page),
	       NAND_FLSH_ADDRESS_3(io_base));
     iowrite32(NAND_ADDR_VALID |
	       NAND_ADDR_INPUT(page >> 8),
	       NAND_FLSH_ADDRESS_4(io_base));
     if (chip->chipsize > (32 << 20))
	  iowrite32(NAND_ADDR_VALID |
		    NAND_ADDR_INPUT(page >> 16),
		    NAND_FLSH_ADDRESS_5(io_base));
     else
	  iowrite32(NAND_ADDR_NOTVALID, 
		    NAND_FLSH_ADDRESS_5(io_base));

     /* DMA setting */
     iowrite32(NAND_LITTLE_ENDING |
	       NAND_ECC_TESTMODE_DISABLE |
	       NAND_BUFFER_RW_ENABLE |
	       NAND_SECOND_CMD_YES |
	       NAND_AUTO_STATUS_CHECK_ENABLE |
	       NAND_ACCESS_DATA_INTER_BUF |
	       NAND_DMA_WRITE |
	       NAND_DEVICE_WAIT_RB,
	       NAND_FLSH_DMA_SET(io_base));

     /* Set the start address of buffer */
     iowrite32(NAND_InterBuff_START_ADDR(0),
	       NAND_FLSH_BUFF_STADDR(io_base));

     /* Set buffer count and ecc mode */
	if (ecc)		
#ifdef CONFIG_NAND_TWO_ECC_MODEL

#ifdef CONFIG_NAND_15B
		iowrite32(NAND_RW_WITH_ECC | NAND_NPAR_LEN_15B |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
#else
		iowrite32(NAND_RW_WITH_ECC | NAND_NPAR_LEN_8B |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
#endif

#else
		iowrite32(NAND_RW_WITH_ECC |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));
	
#endif
	else
		iowrite32(NAND_RW_WITHOUT_ECC |
			NAND_InterBuff_DATA_NUM(len-1),
			NAND_FLSH_BUFF_CNT(io_base));

     /* Start operation */
     iowrite32(NAND_DMA_ENABLE |
	       NAND_DATA_RW_ENABLE |
	       NAND_SEND_CMD_ADDR_ENABLE,
	       NAND_FLSH_CONTROL(io_base));

     chip->timeout_flag = 0;	/* Reset the timeout flag */
     //chip->timer.expires = jiffies + (2 * HZ / 100); /* 20msec */
     chip->timer.expires = jiffies + (20 * HZ / 100); /* 200msec */
     add_timer(&chip->timer);
     while (1) {
	  status = ioread32(NAND_FLSH_INT_STATE(io_base));
	  if (status & NAND_WRITE_DATA_ERROR) {
	       ret = -EIO;
	       goto out;
	  }
	  if (status & NAND_WRITE_DATA_DONE) {
	       ret = 0;
	       goto out;
	  }
	  if (chip->timeout_flag) {
	       printk(KERN_ERR"\nsocle_nand_host: nand_program_l() is timeout\n");
	       return -EIO;
	  }
     }
out:
     del_timer(&chip->timer);
     return ret;
}

/* module_text_address() isn't exported, and it's mostly a pointless
   test if this is a module _anyway_ -- they'd have to try _really_ hard
   to call us from in-kernel code if the core NAND support is modular. */
#ifdef MODULE
#define caller_is_module() (1)
#else
#define caller_is_module()						\
     module_text_address((unsigned long)__builtin_return_address(0))
#endif

/**
 * nand_scan - [NAND Interface] Scan for the NAND device
 * @mtd:	MTD device structure
 * @maxchips:	Number of chips to scan for
 *
 * This fills out all the uninitialized function pointers
 * with the defaults.
 * The flash ID is read and the mtd/chip structures are
 * filled with the appropriate values.
 * The mtd->owner field must be set to the module of the caller
 *
 */
extern int
nand_scan(struct mtd_info *mtd, int maxchips)
{
     int i, busw, nand_maf_id;
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;
     struct nand_flash_dev *type;
     u32 id;

     /* Many callers got this wrong, so check for it for a while... */
     if (!mtd->owner && caller_is_module()) {
	  printk(KERN_CRIT"\nsocle_nand_host: nand_scan() called with NULL mtd->owner!\n");
	  BUG();
     }

     /* Get buswidth to select the correct functions */
     busw = chip->options & NAND_BUSWIDTH_16;

     /* Initialize NAND flash controller */
     if (!chip->controller) {
	  chip->controller = &chip->hwcontrol;
	  chip->controller->io_base = NAND_VA_BASE;
	  chip->controller->buffer = NAND_FLSH_BUFFER(chip->controller->io_base);
	  chip->controller->active = NULL;
	  spin_lock_init(&chip->controller->lock);
	  init_waitqueue_head(&chip->controller->wq);
#if 0
	  /* Get our needed resources */
	  if (!request_mem_region(SOCLE_NAND_BASE, 128*1024, "NAND flash host")) {
	       printk(KERN_INFO "NAND flash host: cannot get I/O memory address 0x%08x ~ 0x%08x\n",
		      SOCLE_NAND_BASE, SOCLE_NAND_BASE+(128*1024)-1);
	       return -1;
	  }
#endif  
     }
     
     /* Initialize the timer to detect timeout event */
     init_timer(&chip->timer);
     chip->timer.function = &socle_nand_timer_expiry;
     chip->timer.data = (unsigned long)chip;

#ifdef TIMEING_TUNE
     if (!chip->timing) {
	  chip->timing = &chip->hwtiming;
	  chip->timing->tWC = NAND_tWC_4CYCLE;
          chip->timing->tWP = NAND_tWP_2CYCLE;
          chip->timing->tRC = NAND_tRC_4CYCLE;
          chip->timing->tRP = NAND_tRP_2CYCLE;
     }
#else
     if (!chip->timing) {
	  chip->timing = &chip->hwtiming;
	  chip->timing->tWC = NAND_tWC_15CYCLE;
          chip->timing->tWP = NAND_tWP_7CYCLE;
          chip->timing->tRC = NAND_tRC_15CYCLE;
          chip->timing->tRP = NAND_tRP_7CYCLE;
     }
#endif

     /* Read the flash type */
     type = nand_get_flash_type(mtd, chip, busw, &nand_maf_id);

     if (IS_ERR(type)) {
	  printk(KERN_WARNING"No NAND device found!!!\n");
	  nand_select_chip(chip, -1);
	  return PTR_ERR(type);
     }

     /* Check for a chip array */
     for (i = 1; i < maxchips; i++) {
	  nand_select_chip(chip, i);
	  
	  /* Send the command for reading device ID */
	  id = nand_read_id(chip);

	  /* Read manufacturer and device IDs */
	  if ((nand_maf_id != MAKER_CODE(id)) ||
	      type->id != DEVICE_CODE(id))
	       break;
     }
     if (i > 1)
	  printk(KERN_INFO"%d NAND chips detected\n", i);

     /* Store the number of chips and calc total size of mtd */
     chip->numchips = i;
     mtd->size = i * chip->chipsize;

     /* Setup low level operation for NAND chip */
     switch (mtd->writesize) {
     case 512:
	  chip->erase_block = nand_erase_block_s;
	  chip->read = nand_read_s;
	  chip->program = nand_program_s;
	  chip->cache_program = NULL;
	  break;
     case 2048:
	  chip->erase_block = nand_erase_block_l;
	  chip->read = nand_read_l;
	  chip->program = nand_program_l;
	  chip->cache_program = nand_cache_program_l;
	  break;
     default:
	  printk(KERN_WARNING"\nNAND device: Unknown page size\n");
	  BUG();
     }
     if (chip->options & NAND_RW_RAW) {
	  chip->read_page = nand_read_page_raw;
	  chip->write_page = nand_write_page_raw;
	  chip->verify_page = nand_verify_page_raw;
     } else {
	  chip->read_page = nand_read_page_hwecc;
	  chip->write_page = nand_write_page_hwecc;
	  chip->verify_page = nand_verify_page_hwecc;
     }

     /* Seelct the appropriate default oob placement scheme for
      * placement agnostic filesystems*/
     switch (mtd->oobsize) {
     case 16:
	  if (chip->options & NAND_RW_RAW)
	       mtd->ecclayout = &nand_oob_16_raw;
	  else
	       mtd->ecclayout = &nand_oob_16_hwecc;
	  break;
     case 64:
	  if (chip->options & NAND_RW_RAW)
	       mtd->ecclayout = &nand_oob_64_raw;
	  else
	       mtd->ecclayout = &nand_oob_64_hwecc;
	  break;
     default:
	  printk(KERN_WARNING"No oob scheme defined for oobsize %d\n", mtd->oobsize);
	  BUG();
     }

     /*
      *  The number of bytes available for a client to place data into
      *  the out of band area
      *  */
     mtd->ecclayout->oobavail = 0;
     for (i = 0; mtd->ecclayout->oobfree[i].length; i++)
	  mtd->ecclayout->oobavail += mtd->ecclayout->oobfree[i].length;

     /* Initialize state */
     chip->state = FL_READY;

     /* De-select the device */
     nand_select_chip(chip, -1);

     /* Invalidate the pagebuffer reference */
     chip->pagebuf = -1;

     /* Fill in remaining MTD driver data */
     mtd->type = MTD_NANDFLASH;
     mtd->flags = MTD_CAP_NANDFLASH;
	 /*	leonid del for no this structure member	*/
     //mtd->ecctype = MTD_ECC_SW;	
     mtd->erase = nand_erase;
     mtd->point = NULL;
     mtd->unpoint = NULL;
     mtd->read = nand_read;
     mtd->write = nand_write;
     mtd->read_oob = nand_read_oob;
     mtd->write_oob = nand_write_oob;
     mtd->sync = nand_sync;
     mtd->lock = NULL;
     mtd->unlock = NULL;
     mtd->suspend = nand_suspend;
     mtd->resume = nand_resume;
     mtd->block_isbad = nand_block_isbad;
     mtd->block_markbad = nand_block_markbad;

     if (chip->options & NAND_USE_FLASH_BBT) {
	  /* Check, if we should skip the bad block table scan */
	  if (chip->options & NAND_SKIP_BBTSCAN)
	       return 0;

	  /* Build bad block table */
	  return nand_default_bbt(mtd);
     } else
	  /* Scan initial bad blocks */
	  return nand_scan_initial_badblock(mtd);
}

/**
 * nand_release - [NAND Interface] Free resources held by the NAND device
 * @mtd:	MTD device structure
 */
extern void
nand_release(struct mtd_info *mtd)
{
     struct nand_chip *chip = (struct nand_chip *)mtd->priv;

#ifdef CONFIG_MTD_PARTITIONS
     /* Deregister partitions */
     del_mtd_partitions(mtd);
#endif

     /* Deregister the device */
     del_mtd_device(mtd);

     /* Free bad block table memory */
     if (chip->bbt)
	  kfree(chip->bbt);
}

static void
socle_nand_timer_expiry(unsigned long data)
{
     struct nand_chip *chip = (struct nand_chip *)data;

     chip->timeout_flag = 1;
}

static ssize_t
socle_nand_write(struct file *filp, const char __user *buf, size_t count,
		 loff_t *f_pos)
{
     socle_nand_debug_level = simple_strtoul(buf, NULL, 0);
     printk("\nThe debug level is %d now\n", socle_nand_debug_level);
     return count;

}

static u32 read_rate;

static void*
socle_nand_seq_start(struct seq_file *s, loff_t *pos)
{
     if (*pos > 0)
	  return NULL;
     else
	  return &read_rate;
}

static void*
socle_nand_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
     (*pos)++;
     return NULL;
}

static void 
socle_nand_seq_stop(struct seq_file *s, void *v)
{
     /* Actually, there's nothing to do here */
}

static int
socle_nand_seq_show(struct seq_file *s, void *v)
{

     seq_printf(s, "\n");

#ifdef CONFIG_MTD_SOCLE_NAND_WEAR_LEVELING_MEASURE
     do {
	  int i;
	  u32 index;
	  u32 total_erase_count = 0, avg_erase_count, max_erase_count, min_erase_count;

	  for (i = 0, index = 0; i < (BLK_CNT >> 2); i++, index += 4) {
	       seq_printf(s, "%10d %10d %10d %10d\n", 
			  socle_nand_statistic->wear_statistic[index],
			  socle_nand_statistic->wear_statistic[index+1],
			  socle_nand_statistic->wear_statistic[index+2],
			  socle_nand_statistic->wear_statistic[index+3]);
	  }
#if 1
	  max_erase_count = socle_nand_statistic->wear_statistic[131];
	  min_erase_count = socle_nand_statistic->wear_statistic[131];
	  for (i = 130; i < BLK_CNT; i++) {		//130 : block position of mtdblock1  
#else
	  max_erase_count = socle_nand_statistic->wear_statistic[148];
	  min_erase_count = socle_nand_statistic->wear_statistic[148];
	  for (i = 149; i < BLK_CNT; i++) {		//130 : block position of mtdblock1  
#endif
	       total_erase_count += socle_nand_statistic->wear_statistic[i];
	       if (max_erase_count < socle_nand_statistic->wear_statistic[i]) 
		    max_erase_count = socle_nand_statistic->wear_statistic[i];

	       if (min_erase_count > socle_nand_statistic->wear_statistic[i])
		    min_erase_count = socle_nand_statistic->wear_statistic[i];
	  }

	  avg_erase_count = total_erase_count / (BLK_CNT - 148);
	  seq_printf(s, "Average erase count: %10d\n", avg_erase_count);
	  seq_printf(s, "Maximum erase count: %10d\n", max_erase_count);
	  seq_printf(s, "Minimum erase count: %10d\n", min_erase_count);
     } while (0);
#endif
#ifdef CONFIG_MTD_SOCLE_NAND_PERFORMANCE_MEASURE
     do {
	  u32 time_per_erase, read_speed_rate, write_speed_rate;
	  struct timespec ts;
	  u32 u_sec, m_sec;

	  jiffies_to_timespec(socle_nand_statistic->erase_jiffies, &ts);
	  u_sec = ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
	  seq_printf(s, "Total time for erase: %10d us\n", u_sec);
	  seq_printf(s, "Total count for erase: %10d\n", socle_nand_statistic->erase_count);
	  if (0 == socle_nand_statistic->erase_count)
	       time_per_erase = 0;
	  else
	       time_per_erase = u_sec / socle_nand_statistic->erase_count;
	  seq_printf(s, "Time spending per erase %10d us\n", time_per_erase);
	  jiffies_to_timespec(socle_nand_statistic->read_jiffies, &ts);
	  m_sec = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	  seq_printf(s, "Total time for read: %10d ms\n", m_sec);
	  seq_printf(s, "Total bytes for read: %10d Bytes\n", socle_nand_statistic->read_bytes);
	  if (0 == m_sec)
	       read_speed_rate = 0;
	  else
	       read_speed_rate = socle_nand_statistic->read_bytes / m_sec;
	  seq_printf(s, "Speed rate of read: %10d B/ms\n", read_speed_rate);
	  jiffies_to_timespec(socle_nand_statistic->write_jiffies, &ts);
	  m_sec = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	  seq_printf(s, "Total time for write: %10d ms\n", u_sec);
	  seq_printf(s, "Total bytes for write: %10d Bytes\n", socle_nand_statistic->write_bytes);
	  if (0 == m_sec)
	       write_speed_rate = 0;
	  else
	       write_speed_rate = socle_nand_statistic->write_bytes / m_sec;
	  seq_printf(s, "Speed rate of write: %10d B/ms\n", write_speed_rate);
     } while (0);
#endif
     return 0;
}

/*
 *  Tie the sequence operators up.
 *  */
static struct seq_operations socle_nand_seq_ops = {
     .start = socle_nand_seq_start,
     .next = socle_nand_seq_next,
     .stop = socle_nand_seq_stop,
     .show = socle_nand_seq_show
};

/*
 *  Now to implement the /proc file we need only make an open
 *  method which sets up the sequence operators.
 *  */
static int socle_nand_proc_open(struct inode *inode, struct file *file)
{
     return seq_open(file, &socle_nand_seq_ops);
}

/*
 *  Create a set of file operations for our proc file.
 *  */
static struct file_operations socle_nand_proc_ops = {
     .owner = THIS_MODULE,
     .open = socle_nand_proc_open,
     .read = seq_read,
     .write = socle_nand_write,
     .llseek = seq_lseek,
     .release = seq_release
};

static struct proc_dir_entry *socle_nand_proc_entry;	//leonid+ for linux 2.6.27 fix

EXPORT_SYMBOL_GPL(nand_scan);
EXPORT_SYMBOL_GPL(nand_release);

extern struct proc_dir_entry proc_root;

static int __init nand_base_init(void)
{
     initialize_ecc();

     /* Install the proc_fs entry */
     socle_nand_proc_entry = create_proc_entry("sq_nand", 
					       S_IRUGO | S_IFREG,
					       &proc_root);
     if (socle_nand_proc_entry) {
	  socle_nand_proc_entry->proc_fops = &socle_nand_proc_ops;
	  socle_nand_proc_entry->data = NULL;
     } else
	  return -ENOMEM;

     socle_nand_statistic = kmalloc(sizeof(struct socle_nand_statistic), GFP_KERNEL);
     if (NULL == socle_nand_statistic)
	  return -ENOMEM;
     memset(socle_nand_statistic, 0, sizeof(struct socle_nand_statistic));
     return 0;
}

static void __exit 
nand_base_exit(void)
{
     remove_proc_entry("sq_nand", &proc_root);
     if (socle_nand_statistic)
	  kfree(socle_nand_statistic);
     return;
}

module_init(nand_base_init);
module_exit(nand_base_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Obi Hsieh");
MODULE_DESCRIPTION("SQ NAND flash driver code");

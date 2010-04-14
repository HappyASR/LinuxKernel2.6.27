#ifndef __NAND_H_INCLUDED
#define __NAND_H_INCLUDED

#include <linux/mtd/mtd.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/timer.h>

/* Scan and identify a NAND device */
extern int nand_scan(struct mtd_info *mtd, int max_chips);

/* Free resources held by the NAND device */
extern void nand_release(struct mtd_info *mtd);

extern int nand_erase_nand(struct mtd_info *mtd, struct erase_info *instr, 
			   int allowbbt);

/* The maximum number of NAND chips in an array */
#define NAND_MAX_CHIPS 4

/* This constant declares the max. oobsize / page, which
 * is supported now. If you add a chip with bigger oobsize/page
 * adjust this accordingly*/
#define NAND_MAX_OOBSIZE 64
#define NAND_MAX_PAGESIZE 2048

/*
 * Standard NAND flash commands
 */
#define NAND_CMD_READ0		0
#define NAND_CMD_READ1		1
#define NAND_CMD_RNDOUT		5
#define NAND_CMD_PAGEPROG	0x10
#define NAND_CMD_READOOB	0x50
#define NAND_CMD_ERASE1		0x60
#define NAND_CMD_STATUS		0x70
#define NAND_CMD_STATUS_MULTI	0x71
#define NAND_CMD_SEQIN		0x80
#define NAND_CMD_RNDIN		0x85
#define NAND_CMD_READID		0x90
#define NAND_CMD_ERASE2		0xd0
#define NAND_CMD_RESET		0xff

/* Extended commands for large page devices */
#define NAND_CMD_READSTART	0x30
#define NAND_CMD_RNDOUTSTART	0xE0
#define NAND_CMD_CACHEDPROG	0x15

/* Option constants for bizarre disfunctionality and real
*  features
*/
/* Chip can not auto increment pages */
#define NAND_NO_AUTOINCR	0x00000001
/* Buswitdh is 16 bit */
#define NAND_BUSWIDTH_16	0x00000002
/* Device supports partial programming without padding */
#define NAND_NO_PADDING		0x00000004
/* Chip has cache program function */
#define NAND_CACHEPRG		0x00000008
/* Chip has copy back function */
#define NAND_COPYBACK		0x00000010
/* AND Chip which has 4 banks and a confusing page / block
 * assignment. See Renesas datasheet for further information */
#define NAND_IS_AND		0x00000020
/* Chip has a array of 4 pages which can be read without
 * additional ready /busy waits */
#define NAND_4PAGE_ARRAY	0x00000040
/* Chip requires that BBT is periodically rewritten to prevent
 * bits from adjacent blocks from 'leaking' in altering data.
 * This happens with the Renesas AG-AND chips, possibly others.  */
#define BBT_AUTO_REFRESH	0x00000080
/* Chip does not require ready check on read. True
 * for all large page devices, as they do not support
 * autoincrement.*/
#define NAND_NO_READRDY		0x00000100

/* Options valid for Samsung large page devices */
#define NAND_SAMSUNG_LP_OPTIONS \
	(NAND_NO_PADDING | NAND_CACHEPRG | NAND_COPYBACK)

/* Macros to identify the above */
#define NAND_CANAUTOINCR(chip) (!(chip->options & NAND_NO_AUTOINCR))
#define NAND_MUST_PAD(chip) (!(chip->options & NAND_NO_PADDING))
#define NAND_HAS_CACHEPROG(chip) ((chip->options & NAND_CACHEPRG))
#define NAND_HAS_COPYBACK(chip) ((chip->options & NAND_COPYBACK))

/* Mask to zero out the chip options, which come from the id table */
#define NAND_CHIPOPTIONS_MSK	(0x0000ffff & ~NAND_NO_AUTOINCR)

/* Non chip related options */
/* Use a flash based bad block table. This option is passed to the
 * default bad block table function. */
#define NAND_USE_FLASH_BBT	0x00010000
/* This option skips the bbt scan during initialization. */
#define NAND_SKIP_BBTSCAN	0x00020000

/* Options set by nand scan */
/* Nand scan has allocated controller struct */
#define NAND_CONTROLLER_ALLOC	0x80000000
/* Nand scan disable the hardware ecc function */
#define NAND_RW_RAW 0x00040000

/*
 * nand_state_t - chip states
 * Enumeration for NAND flash chip state
 */
typedef enum {
	FL_READY,
	FL_READING,
	FL_WRITING,
	FL_ERASING,
	FL_SYNCING,
	FL_CACHEDPRG,
	FL_PM_SUSPENDED,
} nand_state_t;

/* Keep gcc happy */
struct nand_chip;

/**
 * struct nand_hw_control - Control structure for hardware controller (e.g ECC generator) shared among independent devices
 * @io_base             the io base for controller
 * @buffer              the buffer at controller
 * @active:		the mtd device which holds the controller currently
 *                      used instead of the per chip wait queue when a hw controller is available
 */
struct nand_hw_control {
     u32 io_base;
     u32 buffer;
     struct nand_chip *active;
     spinlock_t lock;
     wait_queue_head_t wq;
};

struct nand_timing {
     u32 tWC;			/* Write Cycle time */
     u32 tWP;			/* Write Pulse tiime, duration of write pulse */
     u32 tRC;			/* Read Cycle time */
     u32 tRP;			/* Read Pulse time, duration of read pulse */
};

/**
 * struct nand_buffers - buffer structure for read/write
 * @ecccalc:	buffer for calculated ecc
 * @ecccode:	buffer for ecc read from flash
 * @oobwbuf:	buffer for write oob data
 * @databuf:	buffer for data - dynamically sized
 * @oobrbuf:	buffer to read oob data
 *
 * Do not change the order of buffers. databuf and oobrbuf must be in
 * consecutive order.
 */
struct nand_buffers {
	uint8_t	ecccalc[NAND_MAX_OOBSIZE];
	uint8_t	ecccode[NAND_MAX_OOBSIZE];
	uint8_t	oobwbuf[NAND_MAX_OOBSIZE];
	uint8_t databuf[NAND_MAX_PAGESIZE];
	uint8_t	oobrbuf[NAND_MAX_OOBSIZE];
};

/**
 * struct nand_chip - NAND Private Flash Chip Data
 * @read
 * @program
 * @cache_program
 * @erase_block
 * @read_page
 * @program_page
 * @options:		[BOARDSPECIFIC] various chip options. They can partly be set to inform nand_scan about
 * @page_shift:		[INTERN] number of address bits in a page (column address bits)
 * @phys_erase_shift:	[INTERN] number of address bits in a physical eraseblock
 * @bbt_erase_shift:	[INTERN] number of address bits in a bbt entry
 * @chip_shift:		[INTERN] number of address bits in one chip
 * @numchips:		[INTERN] number of physical chips
 * @chipsize:		[INTERN] the size of one chip for multichip arrays
 * @pagemask:		[INTERN] page number mask = number of (pages / chip) - 1
 * @pagebuf:		[INTERN] holds the pagenumber which is currently in data_buf
 * @badblockpos:	[INTERN] position of the bad block marker in the oob area
 * @state:		[INTERN] the current state of the NAND device
 *			special functionality. See the defines for further explanation
 * @buffers:            buffer structure for read/write
 * @timing:
 * @controller:		[REPLACEABLE] a pointer to a hardware controller structure
 *			which is shared among multiple independend devices
 * @hwcontrol:		platform-specific hardware control structure
 * @ops:		oob operation operands
 * @bbt:		[INTERN] bad block table pointer
 * @bbt_td:		[REPLACEABLE] bad block table descriptor for flash lookup
 * @bbt_md:		[REPLACEABLE] bad block table mirror descriptor
 * @badblock_pattern:	[REPLACEABLE] bad block scan pattern used for initial bad block scan
 */
struct nand_chip {
     int (*read)(struct nand_chip *chip, u32 column, u32 page, 
		 u32 len, int ecc);
     int (*program)(struct nand_chip *chip, u32 column, u32 page,
		    u32 len, int ecc);
     int (*cache_program)(struct nand_chip *chip, u32 column, u32 page, 
			  u32 len, int ecc);
     int (*erase_block)(struct nand_chip *chip, u32 page_addr);
     int (*read_page)(struct mtd_info *mtd, struct nand_chip *chip, u32 page, 
		      u_char *buf);
     int (*write_page)(struct mtd_info *mtd, struct nand_chip *chip, u32 page, 
			 const u_char *buf, int cached);
     int (*verify_page)(struct mtd_info *mtd, struct nand_chip *chip, u32 page,
			const u_char *buf);
     unsigned int options;
     int page_shift;
     int phys_erase_shift;
     int bbt_erase_shift;
     int chip_shift;
     int numchips;
     unsigned long chipsize;
     int pagemask;
     int pagebuf;
     int badblockpos;
     nand_state_t state;
     struct timer_list timer;
     int timeout_flag;
     struct nand_buffers buffers;
     struct nand_timing *timing;
     struct nand_timing hwtiming;
     struct nand_hw_control *controller;
     struct nand_hw_control hwcontrol;
     struct mtd_oob_ops ops;
     u8 *bbt;
     struct nand_bbt_descr *bbt_td;
     struct nand_bbt_descr *bbt_md;
     struct nand_bbt_descr *badblock_pattern;
};

/*
 * NAND Flash Manufacturer ID Codes
 */
#define NAND_MFR_TOSHIBA	0x98
#define NAND_MFR_SAMSUNG	0xec
#define NAND_MFR_FUJITSU	0x04
#define NAND_MFR_NATIONAL	0x8f
#define NAND_MFR_RENESAS	0x07
#define NAND_MFR_STMICRO	0x20
#define NAND_MFR_HYNIX		0xad

/**
 * struct nand_flash_dev - NAND Flash Device ID Structure
 * @name:	Identify the device type
 * @id:		device ID code
 * @pagesize:	Pagesize in bytes. Either 256 or 512 or 0
 *		If the pagesize is 0, then the real pagesize
 *		and the eraseize are determined from the
 *		extended id bytes in the chip
 * @erasesize:	Size of an erase block in the flash device.
 * @chipsize:	Total chipsize in Mega Bytes
 * @options:	Bitfield to store chip relevant options
 */
struct nand_flash_dev {
	char *name;
	int id;
	unsigned long pagesize;
	unsigned long chipsize;
	unsigned long erasesize;
	unsigned long options;
};

/**
 * struct nand_manufacturers - NAND Flash Manufacturer ID Structure
 * @name:	Manufacturer name
 * @id:		manufacturer ID code of device.
*/
struct nand_manufacturers {
	int id;
	char * name;
};

extern struct nand_flash_dev nand_flash_ids[];
extern struct nand_manufacturers nand_manuf_ids[];

/**
 * struct nand_bbt_descr - bad block table descriptor
 * @options:	options for this descriptor
 * @pages:	the page(s) where we find the bbt, used with option BBT_ABSPAGE
 *		when bbt is searched, then we store the found bbts pages here.
 *		Its an array and supports up to 8 chips now
 * @offs:	offset of the pattern in the oob area of the page
 * @veroffs:	offset of the bbt version counter in the oob are of the page
 * @version:	version read from the bbt page during scan
 * @len:	length of the pattern, if 0 no pattern check is performed
 * @maxblocks:	maximum number of blocks to search for a bbt. This number of
 *		blocks is reserved at the end of the device where the tables are
 *		written.
 * @reserved_block_code: if non-0, this pattern denotes a reserved (rather than
 *              bad) block in the stored bbt
 * @pattern:	pattern to identify bad block table or factory marked good /
 *		bad blocks, can be NULL, if len = 0
 *
 * Descriptor for the bad block table marker and the descriptor for the
 * pattern which identifies good and bad blocks. The assumption is made
 * that the pattern and the version count are always located in the oob area
 * of the first block.
 */
struct nand_bbt_descr {
	int	options;
	int	pages[NAND_MAX_CHIPS];
	int	offs;
	int	veroffs;
	uint8_t	version[NAND_MAX_CHIPS];
	int	len;
	int	maxblocks;
	int	reserved_block_code;
	uint8_t	*pattern;
};

/* Options for the bad block table descriptors */

/* The number of bits used per block in the bbt on the device */
#define NAND_BBT_NRBITS_MSK	0x0000000F
#define NAND_BBT_1BIT		0x00000001
#define NAND_BBT_2BIT		0x00000002
#define NAND_BBT_4BIT		0x00000004
#define NAND_BBT_8BIT		0x00000008
/* The bad block table is in the last good block of the device */
#define	NAND_BBT_LASTBLOCK	0x00000010
/* The bbt is at the given page, else we must scan for the bbt */
#define NAND_BBT_ABSPAGE	0x00000020
/* The bbt is at the given page, else we must scan for the bbt */
#define NAND_BBT_SEARCH		0x00000040
/* bbt is stored per chip on multichip devices */
#define NAND_BBT_PERCHIP	0x00000080
/* bbt has a version counter at offset veroffs */
#define NAND_BBT_VERSION	0x00000100
/* Create a bbt if none axists */
#define NAND_BBT_CREATE		0x00000200
/* Search good / bad pattern through all pages of a block */
#define NAND_BBT_SCANALLPAGES	0x00000400
/* Scan block empty during good / bad block scan */
#define NAND_BBT_SCANEMPTY	0x00000800
/* Write bbt if neccecary */
#define NAND_BBT_WRITE		0x00001000
/* Read and write back block contents when writing bbt */
#define NAND_BBT_SAVECONTENT	0x00002000
/* Search good / bad pattern on the first and the second page */
#define NAND_BBT_SCAN2NDPAGE	0x00004000

/* The maximum number of blocks to scan for a bbt */
#define NAND_BBT_SCAN_MAXBLOCKS	4

extern int nand_scan_bbt(struct mtd_info *mtd, struct nand_bbt_descr *bd);
extern int nand_update_bbt(struct mtd_info *mtd, loff_t offs);
extern int nand_default_bbt(struct mtd_info *mtd);
extern int nand_isbad_bbt(struct mtd_info *mtd, loff_t offs, int allowbbt);

/*
 *  Constants for oob configuration
 *  */
#define NAND_SMALL_BADBLOCK_POS 15
#define NAND_LARGE_BADBLOCK_POS 60

#define NAND_DEVICE_SCANNED_MARKER 0x5343414E /* 'S' 'C' 'A' 'N' */
#define NAND_SCANNED_MARK_ADDR 8192
#define MAKER_CODE(id) ((id & 0xFF000000) >> 24)
#define DEVICE_CODE(id) ((id & 0x00FF0000) >> 16)

/*
 *  Debugging macro and defines
 *  */
extern u32 socle_nand_debug_level;

#define SOCLE_NAND_DEBUG_LEVEL0 0 /* Quiet */
#define SOCLE_NAND_DEBUG_LEVEL1 1 /* Audible */
#define SOCLE_NAND_DEBUG_LEVEL2 2 /* Loud */
#define SOCLE_NAND_DEBUG_LEVEL3 3 /* Noisy */

#ifdef CONFIG_MTD_SOCLE_NAND_DEBUG
#define SOCLE_NAND_DEBUG(n, args...)				\
     do {					\
	  if (n < socle_nand_debug_level)	\
	       printk(KERN_INFO args);		\
     } while(0)
#else
#define SOCLE_NAND_DEBUG(n, args...)
#endif

#endif

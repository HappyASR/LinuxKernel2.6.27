/********************************************************************************
* File Name     : arch/arm/mach-socle/socle-cache.c
* Author        : ryan chen
* Description   : Socle Cache
*
* Copyright (C) SQ Tech. Corp.
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by the Free Software Foundation;
* either version 2 of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*   Version      : 2,0,0,1
*   History      :
*      1. 2008/03/04 ryan chen create this file
*
********************************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <mach/platform.h>
#include <mach/regs-cache.h>

static inline void
socle_cache_write(u32 value,u32 reg) 
{
	iowrite32(value, (SOCLE_CC_BASE + reg));
}

static inline u32
socle_cache_read(u32 reg)
{
	return ioread32(SOCLE_CC_BASE + reg);
}

int cacheop_wait_status(void)
{     
       int count=0;


       while(count<1000){
               if((socle_cache_read(CACHE_CACHEOP)& 0x3) == Inval_autoclr)
               	{
			  //printk("return 0\n");
                       return 0;
               	}
               else{
                       count++;
                       if(count == 1000){
                               printk("CacheOP Autocleared error");
                               return -1;
                       }
               }
       }

       return -1;

}


extern void cache_invalidate_way(int invalid_way)
{
#ifndef CONFIG_SOCLE_CACHE
	return;
#endif
       socle_cache_write((CACHE_INVALIDATE_WAY(invalid_way) | InvalWay),CACHE_CACHEOP);
       cacheop_wait_status();
}

EXPORT_SYMBOL(cache_invalidate_way);

void socle_cache_enable(void)
{
#ifndef CONFIG_SOCLE_CACHE
	return;
#endif
	int i;
//	printk("SQ_cache_enable \n");
	for(i=0;i<2;i++)
		cache_invalidate_way(i);

	socle_cache_write(0x80000000,CACHE_DEVID);
}

void socle_cache_disable(void)
{
#ifndef CONFIG_SOCLE_CACHE
	return;
#endif
//	printk("SQ_cache_disable \n");
	socle_cache_write(0x0,CACHE_DEVID);
}

extern void socle_cache_switch(int change)
{
#ifndef CONFIG_SOCLE_CACHE
	return;
#endif
	int i;
	if(change == 0) 
	{
		if((socle_cache_read(CACHE_DEVID) & 0x80000000) == 0x80000000)
		{
//			printk("cache off \n");
			socle_cache_write(0x0,CACHE_DEVID);
		}
		else
			return;
	}
	else
	{
		if((socle_cache_read(CACHE_DEVID) & 0x80000000) == 0x80000000)
			return;
		else
		{
//			printk("cache on \n");	
			for(i=0;i<2;i++)
				cache_invalidate_way(i);
			
			socle_cache_write(0x80000000,CACHE_DEVID);
		}
	}
}

EXPORT_SYMBOL(socle_cache_switch);

extern void cache_invalidate(void)
{
#ifndef CONFIG_SOCLE_CACHE
	return;
#endif
	int i;

	for(i=0;i<2;i++)
		cache_invalidate_way(i);
	
}

EXPORT_SYMBOL(cache_invalidate);

static struct proc_dir_entry *socle_cache_proc_entry;

struct socle_cache_info
{
	u32 socle_cache_stutas;
	u32 socle_cache_mapa;
	u32 socle_cache_mapb;
	u32 socle_cache_mapc;
	u32 socle_cache_mapd;
};

static struct socle_cache_info cache_info;

static int socle_cache_write_proc(struct file *file, const char __user *buffer,unsigned long count, void *data)
{
	char cmd;
//	printk("SQ_cache_write_proc \n");
	copy_from_user(&cmd, buffer, 1);
	switch (cmd) {
		case 'E': /* Enable the Cache */
		case 'e':
			socle_cache_enable();
		break;
		case 'D': /* Disable the Cache */
		case 'd':
			socle_cache_disable();
		break;
	}
	return 1;
}

static int socle_cache_read_proc(char *page, char **start,off_t off, int count, int *eof, void *data)
{
	char *buffer = page;
	int copyCount = 0;
//	printk("SQ_cache_read_proc \n");

	cache_info.socle_cache_stutas = socle_cache_read(CACHE_DEVID) >> 31;
	cache_info.socle_cache_mapa = socle_cache_read(CACHE_MEMMAPA) & 0xffffff00;
	cache_info.socle_cache_mapb = socle_cache_read(CACHE_MEMMAPB) & 0xffffff00;
	cache_info.socle_cache_mapc = socle_cache_read(CACHE_MEMMAPC) & 0xffffff00;
	cache_info.socle_cache_mapd = socle_cache_read(CACHE_MEMMAPD) & 0xffffff00;

	copyCount += snprintf(buffer+copyCount, count, "Cache Status: %s\n", (cache_info.socle_cache_stutas) ? "ON" : "OFF");
	copyCount += snprintf(buffer+copyCount, count, "Cache Map A: %x\n", cache_info.socle_cache_mapa);
	copyCount += snprintf(buffer+copyCount, count, "Cache Map B: %x\n", cache_info.socle_cache_mapb);
	copyCount += snprintf(buffer+copyCount, count, "Cache Map C: %x\n", cache_info.socle_cache_mapc);
	copyCount += snprintf(buffer+copyCount, count, "Cache Map D: %x\n", cache_info.socle_cache_mapd);

	*eof = 1;

	return copyCount;
}

static int __init socle_cache_init(void)
{
	int ret = 0;
	/* Install the proc_fs entry */
	socle_cache_proc_entry = create_proc_entry("sq_cache", S_IRUGO | S_IFREG, &proc_root);
	if (socle_cache_proc_entry)
	{
		socle_cache_proc_entry->read_proc = socle_cache_read_proc;
		socle_cache_proc_entry->write_proc = socle_cache_write_proc;
	}
	else
		return -ENOMEM;

	return ret;
}

core_initcall(socle_cache_init);

#ifndef __ASM_ARCH_SOCLE_REGS_CACHE_H
#define __ASM_ARCH_SOCLE_REGS_CACHE_H	1

#define CACHE_DEVID        	0x00
#define CACHE_CACHEOP      	0x04
#define CACHE_LKDN         		0x08
#define CACHE_MEMMAP_BASE  	0x10
#define CACHE_MEMMAPA     	0x10
#define CACHE_MEMMAPB      	0x14
#define CACHE_MEMMAPC      	0x18
#define CACHE_MEMMAPD      	0x1C
#define CACHE_MEMMAP_OFFSET     0x4      

#define CACHE_INVALIDATE_ENTRY_ADDR(x) (x & ~3)

#define Inval_autoclr           0x0
#define InvalEntry              0x1
#define CACHELINESIZE             32                            // 8 WORDS Cache Line Size

#define CACHE_INVALIDATE_WAY(x) ((x & 1) << 31)
#define InvalWay 0x2

#endif	//__CACHE_REG_H

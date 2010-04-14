/********************************************************************************
* File Name     : drivers/usb/gadget/socle_udc.c
* Author         : Ryan Chen
* Description   : driver for socle-series USB peripheral controller (UDC Driver)
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
    
*   Version      : 1,0,0,0
*   History      : 
*      1. 2008/08/01 ryan chen create this file 
*    
********************************************************************************/
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/clk.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/dma-mapping.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/mach-types.h>

#include <asm/arch/irqs.h>
#include <asm/arch/platform.h>
#include <asm/arch/regs-udc.h>

#define	DRIVER_VERSION	"28 AUG 2008"

static const char driver_name [] = "sq_udc";
static const char ep0name[] = "ep0";

#define	NUM_ENDPOINTS		4

struct socle_request {
	struct usb_request		req;
	struct list_head		queue;
	unsigned			dma_bytes;
	unsigned			mapped:1;
};


struct socle_ep {
	struct usb_ep			ep;
	struct list_head		queue;
	struct socle_udc		*dev;	
	const struct usb_endpoint_descriptor
					*desc;
	u8				int_mask;
	unsigned			is_pingpong:1;
	u8				addr;
	unsigned			stopped:1;
	unsigned			is_in:1;
	u8				complete;
	struct socle_udc			*udc;	
};

struct socle_udc {
	struct usb_gadget		gadget;
	struct socle_ep		ep[NUM_ENDPOINTS];
	struct usb_gadget_driver	*driver;

	struct platform_device		*pdev;
	struct proc_dir_entry		*pde;
	u8				hs;
	u8				suspended;
	u8				enable;
	u8				vbus;
	u32				udc_base;	
	int				udc_irq;	
	u8				config;	
	spinlock_t 		lock;	
	struct 	timer_list 			timer;				/* check configuration timer. */	
	u8				timer_en;
	u8				resetflag;
// 1: reset, 2:set addr , 3configuration 	
        void                                    *ctrl_out_buf;
        dma_addr_t                      ctrl_out_dma;

};

struct socle_ctrl_out {
	u32				total_len;
	u32				cur_ptr;
	u32				dma_addr;
};

#define DMA_ADDR_INVALID (~(dma_addr_t)0)

static struct socle_udc controller;

//#define CONFIG_UDC_DATA_DEBUG
//#define CONFIG_UDC_DEBUG

#ifdef CONFIG_UDC_DEBUG
	#define SOCLE_UDC_DBG(fmt, args...) printk("UDC: " fmt, ## args)
#else
	#define SOCLE_UDC_DBG(fmt, args...)
#endif

//#define	DMA_ADDR_INVALID	(~(dma_addr_t)0)

//static u32 socle_udc_base = IO_ADDRESS(SOCLE_AHB0_UDC);

static inline u32 socle_udc_read(u32 reg)
{
	u32 val;
	val = ioread32(controller.udc_base + reg);
	return val;
}

static inline void socle_udc_write(u32 val, u32 reg)
{
	iowrite32(val, controller.udc_base + reg);
}

#define SOFT_DISCONNECT()	socle_udc_write(socle_udc_read(SOCLE_UDC_DEV_CTRL) |DEV_CTL_SELF_PWR ,SOCLE_UDC_DEV_CTRL)
#define SOFT_CONNECT()	socle_udc_write((socle_udc_read(SOCLE_UDC_DEV_CTRL) | DEV_CTL_SOFT_CN | DEV_CTL_SELF_PWR | DEV_CTL_CSR_DONE ),SOCLE_UDC_DEV_CTRL)

//function list
static void stop_activity(struct socle_udc *udc);
static void done(struct socle_ep *ep, struct socle_request *req, int status);
static void udc_reinit(struct socle_udc *udc);
static int write_fifo(struct socle_ep *ep, struct socle_request *req);
static int read_fifo (struct socle_ep *ep, struct socle_request *req);

#if 0
static void socle_udc_stat_show(void)
{
	u8	socle_ep_num;	
        int ep_num = 0;
        u32 devinfo = socle_udc_read(SOCLE_UDC_DEV_INFO);

        printk("UDC Device Addr = %x \n",(devinfo & DEV_INFO_DEV_ADDR));
	 printk("UDC Device En = %x (config) \n",(devinfo & DEV_INFO_DEV_EN) >> 7);
        printk("UDC Config Num = %x \n",(devinfo & DEV_INFO_CFG_NUM)>>8);
        printk("UDC VBus State = %x \n",(devinfo & DEV_INFO_VBUS_STS)>>20);
        printk("UDC Enum Speed = %x (0:HS, 3:FS)  \n", (devinfo & DEV_INFO_SPEED_MASK)>>DEV_INFO_SPEED);

        ep_num = devinfo & DEV_INFO_EP_NUM_MASK;

        if (ep_num == EP_NUM_10) {
                socle_ep_num = 10;
                printk("The Socle UDC has 10 end-points\n");
        } else if (ep_num == EP_NUM_16) {
                socle_ep_num = 16;
                printk("The Socle UDC has 16 end-points\n");
        } else if (ep_num == EP_NUM_4) {
                socle_ep_num = 4;
                printk("The Socle UDC has 4 end-points\n");
        } else {
                printk("Error end-point numbers!\n");
                printk("DEV_INFO = 0x%08x\n", ep_num);
        }


}
#endif

static void socle_udc_irq_init(void)
{
	// interrupt enable
	socle_udc_write( INT_FLAG_SETUP_INTR | INT_FLAG_IN0_INTR | INT_FLAG_OUT0_INTR | SOCLE_EN_BOUTALL_INTR |
			SOCLE_EN_BINALL_INTR | SOCLE_EN_IINALL_INTR | INT_FLAG_USBRST_INTR | INT_FLAG_RSUME_INTR |
			INT_FLAG_SUSP_INTR, SOCLE_UDC_EN_INT );

	// INT 0,1,2 enable, level-trigger(default), high-active
	socle_udc_write( INTCON_INTEN | INTCON_INTHIGH_ACT, SOCLE_UDC_INTCON);

}

static void socle_ii_enable(u32 ep_num)
{
	SOCLE_UDC_DBG("II ep_num %x, base = %x  \n",ep_num,SOCLE_UDC_II_BASE(ep_num));
	socle_udc_write((ep_num  << 8) | TXCON_ACK_EN | TXCON_VOID_EN | TXCON_ERR_EN | TXCON_EP_EN , SOCLE_UDC_II_BASE(ep_num) + UDC_TXCON);
}

static void socle_bi_enable(u32 ep_num)
{
	SOCLE_UDC_DBG("sq_bi_enable ep : %x \n",ep_num);
	socle_udc_write((ep_num  << 8) |TXCON_DMA_DN_EN | TXCON_EP_EN ,SOCLE_UDC_BI_BASE(ep_num) + UDC_TXCON);

//	socle_udc_write(socle_udc_read(SOCLE_UDC_BO_BASE(ep_num) + UDC_TXCON) | TXCON_CLR ,SOCLE_UDC_BO_BASE(ep_num) + UDC_TXCON);	
//	socle_udc_write(socle_udc_read(SOCLE_UDC_BO_BASE(ep_num) + UDC_TXCON) & ~TXCON_CLR ,SOCLE_UDC_BO_BASE(ep_num) + UDC_TXCON);	
	
}

static void socle_bo_enable(u32 ep_num)
{
	SOCLE_UDC_DBG("sq_bo_enable ep : %x  \n",ep_num);
	socle_udc_write((ep_num  << 8) |RXCON_ACK_EN | RXCON_EP_EN ,SOCLE_UDC_BO_BASE(ep_num) + UDC_RXCON);

//	socle_udc_write(socle_udc_read(SOCLE_UDC_BO_BASE(ep_num) + UDC_RXCON) | RXCON_CLR ,SOCLE_UDC_BO_BASE(ep_num) + UDC_RXCON);	
//	socle_udc_write(socle_udc_read(SOCLE_UDC_BO_BASE(ep_num) + UDC_RXCON) & ~RXCON_CLR ,SOCLE_UDC_BO_BASE(ep_num) + UDC_RXCON);	

}

static void socle_ep0_enable(void)
{
	struct socle_udc	*udc = &controller;
	//rxcon
	socle_udc_write( RXCON_ACK_EN |  RXCON_EP_EN  , SOCLE_UDC_RX0CON );
	//txcon
	socle_udc_write( TXCON_ACK_EN | TXCON_NAK  , SOCLE_UDC_TX0CON );

       socle_udc_write( udc->ctrl_out_dma, SOCLE_UDC_DMA0LM_OADDR );
       socle_udc_write( UDC_DMA_START ,  SOCLE_UDC_DMA0CTLO);


}

static void socle_udc_init(void)
{
//	int i;
	
#ifdef	SOCLE_UDC_FS_MODE
 	// set UDC is Full-Speed mode, default is High-Speed mode
 	socle_udc_write(socle_udc_read(SOCLE_UDC_DEV_CTRL)|FULL_SPEED, SOCLE_UDC_DEV_CTRL);
#endif

	socle_udc_irq_init();

	socle_ep0_enable();

//	for(i = 0; i<socle_ep_num; i++)
//		socle_ep_enable(i);
}

static void socle_usb_phy_reset(void)
{
	//phy reset
//	socle_udc_write(((socle_udc_read(SOCLE_UDC_DEV_CTRL) | DEV_CTL_SOFT_POR) & ~DEV_CTL_SELF_PWR),SOCLE_UDC_DEV_CTRL);
	socle_udc_write((socle_udc_read(SOCLE_UDC_DEV_CTRL) | DEV_CTL_SOFT_POR) ,SOCLE_UDC_DEV_CTRL);
//	MSDELAY(10);
	socle_udc_write((socle_udc_read(SOCLE_UDC_DEV_CTRL) & ~DEV_CTL_SOFT_POR), SOCLE_UDC_DEV_CTRL);
}

// 3,6,9,12,15 --> Interrupt IN 
static void socle_ii_ep_intr(struct socle_udc *udc, u8 ep_num)
{
	u32 status = socle_udc_read(SOCLE_UDC_II_BASE(2) + UDC_TXSTAT);
	if (status & TXSTAT_ERR)
	{
		SOCLE_UDC_DBG("II TX Error \n");
	}
	else if (status & TXSTAT_ACK)
	{
		SOCLE_UDC_DBG("II TXSTAT_ACK  \n");	
	}
}


//   2,5,8,11,14 --> Bulk IN
static void socle_bi_ep_intr(struct socle_udc *udc, u8 ep_num)
{
	u32 txstat = socle_udc_read(SOCLE_UDC_BI_BASE(ep_num) + UDC_TXSTAT);
	if(txstat & TXSTAT_DMA_DN)
	{
		struct socle_ep		*ep = &udc->ep[ep_num];
		struct socle_request	*req;

		SOCLE_UDC_DBG("BI ep %x dma down \n",ep_num);

		if (list_empty(&ep->queue)) {
			req = NULL;
			printk("SQ_bi_ep_intr BI req = NULL ERROR !! \n");
		} else {
			req = list_entry(ep->queue.next, struct socle_request, queue);
		}
		
		if(req->req.length == req->req.actual) {
			SOCLE_UDC_DBG("BI DONE \n");
			done(ep, req, 0);
		} else {
//			printk("next BI PACK (%d / %d)\n", req->req.actual, req->req.length);
			write_fifo(ep, req);
		}

	
	}
	else if (txstat & TXSTAT_ERR)
	{
		SOCLE_UDC_DBG("BI TX Error \n");
	}
	else if (txstat & TXSTAT_TC_CF_INT)
	{
		SOCLE_UDC_DBG("BI TXSTAT_TC_CF_INT \n");	
	}
	else
		SOCLE_UDC_DBG("BI  Error !!!!!!!!!!!!!!!\n");	
}


//1,4,7,10,13 --> Bulk OUT
static void socle_bo_ep_intr(struct socle_udc *udc, u8 ep_num)
{
	u32 	rxstat;
	u32	len;
	u32	ret;

	socle_udc_write( socle_udc_read(SOCLE_UDC_BO_BASE(ep_num) + UDC_RXCON) | RXCON_NAK ,SOCLE_UDC_BO_BASE(ep_num) + UDC_RXCON);

	rxstat = socle_udc_read(SOCLE_UDC_BO_BASE(ep_num) + UDC_RXSTAT);

	if(rxstat & RXSTAT_ACK) {
		struct socle_ep		*ep = &udc->ep[ep_num];
		struct socle_request	*req;
		SOCLE_UDC_DBG("BO RX ACK %x intr \n",ep_num);

//		printk("&req->queue : %x ,&ep->queue : %x \n",&req->queue,&ep->queue);

		if (list_empty(&ep->queue)) {
			req = NULL;
			printk("SQ_bo_ep_intr BO req = NULL !!!!!!!!!!!!!"); 
			return;
		} else {
			req = list_entry(ep->queue.next, struct socle_request, queue);
//			printk("SQ_bo_ep_intr BO req = %x \n",&req);
		}

		len = rxstat & RXSTAT_LEN;
		if(req) {
#ifdef CONFIG_UDC_DATA_DEBUG
	int 	i;
                for(i=0;i<len ;i++ )
                        printk("%02x",*(u8 * )(req->req.buf+i));
                printk("\n");
#endif				

//			printk("BO done ep len = %d  \n", rxstat & RXSTAT_LEN );

//			printk("BO OLD RX len = %d \n",req->req.actual);
			
			req->req.actual = req->req.actual + len;
			if(len > 512)
				printk("RX BO > 512 \n");
			if(req->req.actual > req->req.length)
				printk("RX BO ERROR !! \n");
			
//			printk("BO RX len = %d \n",req->req.actual);

//socle_udc_write( socle_udc_read(SOCLE_UDC_BO_BASE(ep_num) + UDC_RXCON) | RXCON_EP_EN ,SOCLE_UDC_BO_BASE(ep_num) + UDC_RXCON);

			if((len < 512) || (req->req.actual  == req->req.length) ){
//				printk("done : %d: %d : %d : %x\n",req->req.length,req->req.actual,len,&ep->queue);
				done(ep, req, 0);
			}else {
//				printk("NEXT BO %d, / %d = %d \n",req->req.actual,req->req.length);
				ret = read_fifo(ep, req);
			}
//			done(ep, req, 0);
		}
	}
	else if (rxstat & RXSTAT_ERR)
	{
		printk("bulk out (%d) error \n", ep_num);

		socle_udc_write(socle_udc_read(SOCLE_UDC_BO_BASE(ep_num) + UDC_TXCON) | TXCON_CLR ,SOCLE_UDC_BO_BASE(ep_num) + UDC_TXCON);	
		socle_udc_write(socle_udc_read(SOCLE_UDC_BO_BASE(ep_num) + UDC_TXCON) & ~TXCON_CLR ,SOCLE_UDC_BO_BASE(ep_num) + UDC_TXCON);	
	}
	else if (rxstat & RXSTAT_CF_INT)
	{
		printk("bulk out (%d) clear feature \n", ep_num);	
	}
	else
		printk("BO  Error !!!!!!!!!!!!!!!\n");	
	
		
} 

/* ----------------- interface processing fucntions:entry functions  ---------- */
/* 1,4,7,10,13 --> Bulk OUT
   2,5,8,11,14 --> Bulk IN
   3,6,9,12,15 --> Interrupt IN */

static void proc_ep_intr(struct socle_udc *udc, u8 ep_num)
{
	/* TODO: please change the mapping of endpoint number to d_interface
	   here to reduce three copy of code */

	SOCLE_UDC_DBG("ep %x intr \n",ep_num);
	
	switch ((ep_num-1) % 3) {
	case 0:
		socle_bo_ep_intr(udc, ep_num);
		break;
	case 1:
		socle_bi_ep_intr(udc, ep_num);
		break;
	case 2:
		socle_ii_ep_intr(udc, ep_num);
		break;
	}
	return;
}

static void proc_ctl_out_intr(struct socle_udc *udc)
{
	u32 stats = socle_udc_read(SOCLE_UDC_RX0STAT);  

        struct socle_ep         *ep0 = &udc->ep[0];
        struct socle_request    *req;
	
	SOCLE_UDC_DBG("proc_ctl_out_intr \n");
//	printk("proc_ctl_out_intr \n");
	
	if(stats & RXSTAT_ACK)
	{
		SOCLE_UDC_DBG("RXSTAT_ACK \n");

		if (list_empty(&ep0->queue)) {
			req = NULL;
//			printk("!! list_empty ========ERROR =======\n");
//			return;
			goto ctrl_out_dma;
		} else {
			req = list_entry(ep0->queue.next, struct socle_request, queue);
//			printk("req = %x \n",req);
		}

		if(stats&RXSTAT_LEN)
			printk("ctrl rx = %d ================\n",stats&RXSTAT_LEN);

              done(ep0, req, 0);
ctrl_out_dma:		
		socle_udc_write( udc->ctrl_out_dma, SOCLE_UDC_DMA0LM_OADDR );
		socle_udc_write( UDC_DMA_START ,  SOCLE_UDC_DMA0CTLO);
	}
	else if(stats & RXSTAT_FULL)
	{
		printk("RXSTAT_FULL \n");
	}
	else if(stats & RXSTAT_OVF)
	{
		printk("RXSTAT_OVF \n");		
	}
	else if(stats & RXSTAT_ERR)
	{
		printk("RXSTAT_ERR \n");		
	}
	else
	{
		printk("ERROR =====\n ");
	}
	
}

static void proc_ctl_in_intr(struct socle_udc *udc)
{
	u32 stats = socle_udc_read(SOCLE_UDC_TX0STAT);  // FIXME : must read

        struct socle_ep         *ep0 = &udc->ep[0];
        struct socle_request    *req;

	SOCLE_UDC_DBG("proc_ctl_in_intr\n");

	if(stats & TXSTAT_ACK)
	{
#if 1
tx_ack:

//////////////////////////////////////////////////////////
//		struct socle_ep		*ep0 = &udc->ep[0];
//		struct socle_request	*req;

		if (list_empty(&ep0->queue)) {
			req = NULL;
//			printk("!! list_empty ========ERROR =======\n");
			return;
		} else {
			req = list_entry(ep0->queue.next, struct socle_request, queue);
//			printk("req = %x \n",req);
		}
/*
		if(req) {
			SOCLE_UDC_DBG("ctrl out done ep0 \n");
			done(ep0, req, 0);
		}
*/
                //ryan FIXME sned zero length package for indicate end package
                if((req->req.length == req->req.actual) && (req->req.length%ep0->ep.maxpacket == 0)) {
//                        printk("next ctrl_in 1 \n");
                        write_fifo(ep0, req);
                } else {
//                        printk("next ctrl_in 1\n");
                        write_fifo(ep0, req);
                }
#endif
	}
	else if(stats & TXSTAT_ERR)
	{
		printk("TX0 ERROR \n");
	}
	else if(stats & TXSTAT_FULL)
	{
		SOCLE_UDC_DBG("TX0 FULL \n ");
	}
	else if(stats & TXSTAT_VOID)
	{
		SOCLE_UDC_DBG("TX0 VOID \n ");
	}
	else if(stats & TXSTAT_OVF)
	{
		SOCLE_UDC_DBG("TX0 OVF \n ");	
	}
	else
	{
		printk("ERROR \n");		
		goto tx_ack;
	}
		


	
}

static void proc_setup_intr(struct socle_udc *udc)
{
//	struct socle_request	*req;

	struct usb_ctrlrequest 	setup_request;
	u32	setup_request_data[2];
	u32 	ret;
  
/*
	if (list_empty(&ep0->queue)) {
		printk
		req = NULL;
       } else {
		req = list_entry(ep0->queue.next, struct socle_request, queue);
       }
*/
	SOCLE_UDC_DBG("proc_setup_intr\n");

	if(udc->driver == NULL) {
		printk("return setup \n");
		return;
	}

	if(udc->resetflag == 1) {
		
		//Speed has been recornage at setup stage
		if (((socle_udc_read(SOCLE_UDC_DEV_INFO) & DEV_INFO_SPEED_MASK)>>DEV_INFO_SPEED) == FULL_SPEED) {

			udc->gadget.ep0->maxpacket = FULL_SPEED_CTRL_PACKET_SIZE;
			udc->ep[0].ep.maxpacket = FULL_SPEED_CTRL_PACKET_SIZE;
			udc->ep[1].ep.maxpacket = FULL_SPEED_BULK_PACKET_SIZE;
			udc->ep[2].ep.maxpacket = FULL_SPEED_BULK_PACKET_SIZE;
			udc->ep[3].ep.maxpacket = FULL_SPEED_INTR_PACKET_SIZE;
			udc->gadget.speed = USB_SPEED_FULL;
			udc->hs = 0;
			
			SOCLE_UDC_DBG("**full speed ===  ** \n");

		}
		else {

			udc->gadget.ep0->maxpacket = HI_SPEED_CTRL_PACKET_SIZE;
			udc->ep[0].ep.maxpacket = HI_SPEED_CTRL_PACKET_SIZE;
			
			udc->ep[1].ep.maxpacket = HI_SPEED_BULK_PACKET_SIZE;
			udc->ep[2].ep.maxpacket = HI_SPEED_BULK_PACKET_SIZE;
			udc->ep[3].ep.maxpacket = HI_SPEED_INTR_PACKET_SIZE;
			udc->gadget.speed = USB_SPEED_HIGH;
			udc->hs = 1;
			SOCLE_UDC_DBG("**high speed ====** \n");
			
		}
//		udc_reinit(udc);	
		udc->resetflag = 2;
                if (udc->config == 0) {
			if(udc->timer_en == 0) {
				//add timer check configuration
				udc->timer.expires = jiffies + 1;
//				printk("setup add timer \n");
				add_timer(&udc->timer);
				udc->timer_en = 1;
			}
                }

		
	}

	if (udc->gadget.dev.driver == NULL) {
//		printk("setup structure null \n");
		return;
	}

	setup_request_data[0] = socle_udc_read(SOCLE_UDC_SETUP1);
	setup_request_data[1] = socle_udc_read(SOCLE_UDC_SETUP2 );


	memcpy(&setup_request, setup_request_data, sizeof(setup_request));
	
//	printk( "bRequestType = %d ,bRequest = %d bRequestType %d wLength = %d\n",
//		setup_request.bRequestType, setup_request.bRequest, setup_request.bRequestType, setup_request.wLength);

	udc->ep[0].is_in = 1;

	ret = udc->driver->setup(&udc->gadget, &setup_request);

	if (ret < 0) {
		printk("setup fail set ep0 STALL ========\n");

		
	}

} /* end proc_setup */

static void proc_resume_intr(void)
{
	SOCLE_UDC_DBG("proc_resume_intr \n");
//	sdev->bIsSuspend = 0;

	// clear all buffer
	socle_udc_write(socle_udc_read(SOCLE_UDC_TX0CON) | TXCON_CLR, SOCLE_UDC_TX0CON);
	socle_udc_write(socle_udc_read(SOCLE_UDC_TX0CON) & ~TXCON_CLR, SOCLE_UDC_TX0CON);
	socle_udc_write(socle_udc_read(SOCLE_UDC_RX0CON) | RXCON_CLR, SOCLE_UDC_RX0CON);
	socle_udc_write(socle_udc_read(SOCLE_UDC_RX0CON) & ~RXCON_CLR, SOCLE_UDC_RX0CON);


} /* end proc_resume */

static void proc_suspend_intr(void)
{
	SOCLE_UDC_DBG("proc_suspend_intr \n");
//	sdev->bIsSuspend = 1;

	/* TODO: since host is now suspend, free out all queue,
	   if you don't consider on doing so, at least reset the d_size field 
	   just in case for any error un-finish transition */

	/* TODO: after the bus is suspend, tell upper layer to reset state machine
	   in case the upper layer pending in un-finish state */

	return;
}

static void proc_reset_intr(struct socle_udc *udc)
{
	//initial udc controller timing issue
	SOCLE_UDC_DBG("proc_reset_intr ==============\n");
	
	socle_udc_init();
	udc->config = 0;
	udc->resetflag = 1;
	udc->suspended = 0;

	stop_activity(udc);
//	udc_reinit(udc);
//	socle_udc_con_show();
}

static void proc_vbus_intr(struct socle_udc *udc)
{
	//soft connect
	if(socle_udc_read(SOCLE_UDC_DEV_INFO) & DEV_INFO_VBUS_STS) {
		SOCLE_UDC_DBG(" -> USB VBus connect\n");
		udc->vbus = 1;
		//reset phy
		socle_usb_phy_reset();
		socle_udc_init();		
		SOFT_CONNECT();	
	} else {
		SOCLE_UDC_DBG(" -> USB VBus disconnect \n");
		//soft disconnect
		SOFT_DISCONNECT();	
		stop_activity(udc);
		udc->vbus = 0;	
	}
}

static irqreturn_t socle_udc_irq(int irq, void *dev)
{
	u32 int_mask;
	u32 ep_num;
	u32	irq_flag;
       unsigned long   flags;

	struct socle_udc *socle_dev = dev;	

	spin_lock_irqsave(&socle_dev->lock, flags);		

	irq_flag = socle_udc_read(SOCLE_UDC_INTFLG);

	if (irq_flag & INT_FLAG_VBUS_INTR) {
		proc_vbus_intr(socle_dev);
        	spin_unlock_irqrestore(&socle_dev->lock, flags);
		return IRQ_HANDLED;
	}

	if (irq_flag & INT_FLAG_USBRST_INTR) {
		proc_reset_intr(socle_dev);
        	spin_unlock_irqrestore(&socle_dev->lock, flags);
		return IRQ_HANDLED;
	}

	if (irq_flag & INT_FLAG_SUSP_INTR) {
		proc_suspend_intr();
	}

	if (irq_flag & INT_FLAG_RSUME_INTR) {
		proc_resume_intr();
	}
/*
	if (irq_flag & INT_FLAG_SETUP_INTR) {
		proc_setup_intr(socle_dev);
	}
*/

        if (irq_flag & INT_FLAG_OUT0_INTR) {
                proc_ctl_out_intr(socle_dev);
        }

	if (irq_flag & INT_FLAG_IN0_INTR) {
		proc_ctl_in_intr(socle_dev);
	}

	//must move check setup intr here
       if (irq_flag & INT_FLAG_SETUP_INTR) {
                proc_setup_intr(socle_dev);
        }

	/* process endpoint interrupt */
	for (irq_flag >>= 8, int_mask = 1, ep_num = 1;
	     irq_flag; ep_num++, int_mask <<= 1) {
		
		if (irq_flag & int_mask) {
			irq_flag ^= int_mask;
			proc_ep_intr(socle_dev, ep_num);
		}
	}

	spin_unlock_irqrestore(&socle_dev->lock, flags);		
	return IRQ_HANDLED;
}

/*-------------------------------------------------------------------------*/

static void done(struct socle_ep *ep, struct socle_request *req, int status)
{
	unsigned	stopped = ep->stopped;

	list_del_init(&req->queue);
	if (req->req.status == -EINPROGRESS)
		req->req.status = status;
	else
		status = req->req.status;
	
	if (status && status != -ESHUTDOWN)
		printk("%s done %p, status %d\n", ep->ep.name, req, status);

//	printk("done &ep->queue : %x : next: %x, %s \n",&ep->queue, ep->queue.next ,ep->ep.name);
	ep->stopped = 1;
	spin_unlock(&ep->udc->lock);		//new
	req->req.complete(&ep->ep, &req->req);
	spin_lock(&ep->udc->lock);		//new
	ep->stopped = stopped;
	
	if (!list_empty(&ep->queue)) {
		printk("ERROR Done \n");
	}
}

/*-------------------------------------------------------------------------*/

static int read_fifo (struct socle_ep *ep, struct socle_request *req)
{

//	printk("%s : read fifo : len : %d, short : %d , actual : %d\n", ep->ep.name, req->req.length,req->req.short_not_ok,req->req.actual);

//	printk("read fifo len = %d, actual =%d \n", req->req.length, req->req.actual);
//	printk("read fifo buff = %x, dma =%x \n", req->req.buf, req->req.dma);


	if(ep->ep.name == "ep0") {
		printk("ep 0 read fifo error !!!!!!!!!!!!!!!!!!!!!!!!\n");
	} else if (ep->ep.name == "ep1out-bulk") {
//		printk("ep 1 read fifo Kick Bout RX \n");
//		printk("BO DMA = + %d",req->req.actual);		
//		read buffer and put to up layer 
//		printk("BO req->req.dma = %x \n",req->req.dma);
//socle_udc_write( socle_udc_read(SOCLE_UDC_BO_BASE(ep_num) + UDC_RXCON) & ~RXCON_NAK ,SOCLE_UDC_BO_BASE(ep_num) + UDC_RXCON)a;
		socle_udc_write(req->req.dma + req->req.actual, SOCLE_UDC_BO_BASE(1) + UDC_DMALM_OADDR);
		socle_udc_write(UDC_DMA_START,SOCLE_UDC_BO_BASE(1) + UDC_DMACTRLO);
		socle_udc_write( socle_udc_read(SOCLE_UDC_BO_BASE(1) + UDC_RXCON) & ~RXCON_NAK ,SOCLE_UDC_BO_BASE(1) + UDC_RXCON);

	} else if (ep->ep.name == "ep2in-bulk") {
		//bulk endpoint
		printk("ep 2 read fifo error !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		
	} else if (ep->ep.name == "ep3") {
		printk("ep 3 read fifo \n");
	}

//	req->req.actual += count;
//	req->req.length = count;

//	printk("%s %p out/%d%s\n", ep->ep.name, &req->req, count,
//			);

	/*
	 * avoid extra trips through IRQ logic for packets already in
	 * the fifo ... maybe preventing an extra (expensive) OUT-NAK
	 */
//	if (is_done)
//		done(ep, req, 0);
//	else {
//		printk("wait .BO intr..........\n");
//	}


	return 0;

}

static int write_fifo(struct socle_ep *ep, struct socle_request *req)
{
	unsigned	total, count;

//	printk("%s : write_fifo , len = %d \n",ep->ep.name,req->req.length);
//	printk("ep->ep.maxpacket = %x >>>>\n", ep->ep.maxpacket);
//	if(req->req.length == 525)
//		printk("%s : write_fifo , len = %d \n",ep->ep.name,req->req.length);

	total = req->req.length - req->req.actual;
	
	if (ep->ep.maxpacket < total) {
//		printk(">>>>>> maxpackage ======len : %x \n",req->req.length);
		count = ep->ep.maxpacket;
	} else {
		count = total;
	}

#ifdef CONFIG_UDC_DATA_DEBUG
	int i;
	for(i=0;i< count;i++ )
		printk("%02x",*(u8 * )(req->req.buf+i));
	
	printk("\n");
#endif
	if(ep->ep.name == "ep0") {
		if(!(socle_udc_read(SOCLE_UDC_TX0BUF) & TXBUF_FULL)) 
		{
/*			printk("write fifo count = %d ,buff = %x, dma =%x \n",count, req->req.buf, req->req.dma);		

	int i;
	for(i=0;i< count;i++ )
		printk("%02x",*(u8 * )(req->req.buf+i));
	
	printk("\n");
*/
			socle_udc_write(count, SOCLE_UDC_TX0STAT);
			socle_udc_write(req->req.dma + req->req.actual, SOCLE_UDC_DMA0LM_IADDR);
			socle_udc_write( UDC_DMA_START , SOCLE_UDC_DMA0CTLI);
			socle_udc_write(socle_udc_read( SOCLE_UDC_TX0CON ) & ~TXCON_NAK , SOCLE_UDC_TX0CON );
			req->req.actual += count;
		}
		else
			printk("EP0 FIFO FULL error !!!!!!!!!!!!!!!!!!!!!!!!\n");
	} else if(ep->ep.name == "ep2in-bulk") {
		if(!(socle_udc_read(SOCLE_UDC_BI_BASE(2) + UDC_TXBUF) &TXBUF_FULL))
		{

//			printk("BI write fifo count = %d ,buff = %x, dma =%x \n",count, req->req.buf, req->req.dma);
			socle_udc_write(count,SOCLE_UDC_BI_BASE(2) + UDC_TXSTAT);
			socle_udc_write(req->req.dma + req->req.actual,SOCLE_UDC_BI_BASE(2) + UDC_DMALM_IADDR);
			socle_udc_write(UDC_DMA_START,SOCLE_UDC_BI_BASE(2) + UDC_DMACTRLI);
			req->req.actual += count;
		}
		else {
			while((socle_udc_read(SOCLE_UDC_BI_BASE(2) + UDC_TXBUF) &TXBUF_FULL));
//			   printk("EP2 FIFO FULL END req->req.actual = %d count = %d \n",req->req.actual,count);
                        socle_udc_write(count,SOCLE_UDC_BI_BASE(2) + UDC_TXSTAT);
                        socle_udc_write(req->req.dma + req->req.actual,SOCLE_UDC_BI_BASE(2) + UDC_DMALM_IADDR);
                        socle_udc_write(UDC_DMA_START,SOCLE_UDC_BI_BASE(2) + UDC_DMACTRLI);
                        req->req.actual += count;

		}
	} else {
		printk("write fifo error !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	}

	return 0;
}
static void nuke(struct socle_ep *ep, int status)
{
	struct socle_request *req;
//	printk("nuke \n");
	// terminer chaque requete dans la queue
	ep->stopped = 1;
	if (list_empty(&ep->queue)) {
//		printk("list_empty \n");
		return;
	}

//	printk("%s %s\n", __FUNCTION__, ep->ep.name);
	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct socle_request, queue);
		done(ep, req, status);
	}
}

/*-------------------------------------------------------------------------*/

static int socle_ep_enable(struct usb_ep *_ep,
				const struct usb_endpoint_descriptor *desc)
{
	struct socle_ep	*ep = container_of(_ep, struct socle_ep, ep);
//	struct socle_udc	*dev = ep->udc;

//	printk("%s : socle_ep_enable \n", _ep->name);	

	u16		maxpacket;
	unsigned long	flags;

	if (!_ep || !ep
//			|| !desc || ep->desc
//			|| _ep->name == ep0name
//			|| desc->bDescriptorType != USB_DT_ENDPOINT
			|| (maxpacket = le16_to_cpu(desc->wMaxPacketSize)) == 0
			|| maxpacket > ep->ep.maxpacket) {
		printk("bad ep or descriptor ============\n");
		return -EINVAL;
	}

/*
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN) {
		printk("bogus device state\n");
		return -ESHUTDOWN;
	}
*/
//	printk("ep maxpacket = %d \n",maxpacket);

	local_irq_save(flags);

	/* initialize endpoint to match this descriptor */
	ep->is_in = (desc->bEndpointAddress & USB_DIR_IN) != 0;
	ep->stopped = 0;

	//enable endpoint
	if(_ep->name == "ep1out-bulk") {
		socle_bo_enable(1);
	} else if(_ep->name == "ep2in-bulk") {
		socle_bi_enable(2);
	} else if(_ep->name == "ep3in-int") {
		socle_ii_enable(3);
	} else
		printk("error endpoint \n");

	ep->desc = desc;
//	ep->ep.maxpacket = maxpacket;

	local_irq_restore(flags);
	return 0;
}

static int socle_ep_disable (struct usb_ep * _ep)
{

	struct socle_ep	*ep = container_of(_ep, struct socle_ep, ep);
	unsigned long	flags;

	if (ep == &ep->udc->ep[0])
		return -EINVAL;

	local_irq_save(flags);

	nuke(ep, -ESHUTDOWN);

	/* restore the endpoint's pristine config */
	ep->desc = NULL;
	ep->ep.maxpacket = HI_SPEED_BULK_PACKET_SIZE;

	/* reset fifos and endpoint */
//	if (ep->udc->clocked) {
		//socle_udp_write(AT91_UDP_RST_EP, ep->int_mask);
		//socle_udp_write(AT91_UDP_RST_EP, 0);
		//__raw_writel(0, ep->creg);
//	}

	local_irq_restore(flags);
	return 0;

}

/*
 * this is a PIO-only driver, so there's nothing
 * interesting for request or buffer allocation.
 */

static struct usb_request *
socle_ep_alloc_request(struct usb_ep *_ep, unsigned int gfp_flags)
{
	struct socle_request *req;

	req = kcalloc(1, sizeof (struct socle_request), gfp_flags);
/*
	printk("%s : socle_ep_alloc_request \n",_ep->name);	
	printk("req : %x \n",req);
	printk("&req->queue %x \n",&req->queue);
*/	
	if (req) {
		INIT_LIST_HEAD (&req->queue);
	}
	else
		return NULL;
	
	return &req->req;
}

static void socle_ep_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
//	printk("%s : socle_ep_free_request \n",_ep->name);

	struct socle_request *req = container_of(_req, struct socle_request, req);
	if (_req)
		kfree (req);
}

//leonid fix for delete ocle_otg_alloc_buffer & socle_otg_free_buffer function
#if 0
static void *socle_ep_alloc_buffer(
	struct usb_ep *_ep,
	unsigned bytes,
	dma_addr_t *dma,
	gfp_t gfp_flags)
{
	struct socle_ep	*ep;
	
//	printk("%s : socle_ep_alloc_buffer \n",_ep->name);
	ep = container_of(_ep, struct socle_ep, ep);
	//use dma
	return dma_alloc_coherent(ep->udc->gadget.dev.parent,
				bytes, dma, gfp_flags);
}

static void socle_ep_free_buffer(
	struct usb_ep *_ep,
	void *buf,
	dma_addr_t dma,
	unsigned bytes)
{
	struct socle_ep	*ep;
	ep = container_of(_ep, struct socle_ep, ep);
	if (!_ep) {
		printk("SQ_ep_free_buffer error return \n");
		return;
	}
	dma_free_coherent(ep->udc->gadget.dev.parent, bytes, buf, dma);
}
#endif

static int socle_ep_queue(struct usb_ep *_ep,
			struct usb_request *_req, gfp_t gfp_flags)
{

	struct socle_request	*req;
	struct socle_ep		*ep;
	struct socle_udc		*dev;
	int			status;
	unsigned long		flags;
	int 			i=0;

//	printk("%s : socle_ep_queue : actual = %d , short_not_ok = %d, status = %x ,  len = %d ,_req = %x \n",_ep->name,_req->actual,_req->short_not_ok,_req->status,  _req->length, _req);
//	printk("zero = %d , no_interrupt = %d \n",_req->zero,_req->no_interrupt);	

	//test
/*
	if(_req->length > 100)
	{
		printk("ERROR NOT SEND ----------len : %d \n",_req->length); 
                _req->status = 0;
		_req->actual = 0;
		_req->complete(_ep, _req);
		return 0;
	}
*/
	req = container_of(_req, struct socle_request, req);
	ep = container_of(_ep, struct socle_ep, ep);


	if (_req->length == 0) {

//		printk("send ZLP \n"); 
		spin_lock_irqsave(&ep->udc->lock, flags);		
		done(ep, req, 0);
		spin_unlock_irqrestore(&ep->udc->lock, flags);		
		return 0;
	}

	if (!_req || !_req->complete
			|| !_req->buf || !list_empty(&req->queue)) {
		printk("invalid request\n");
		if(!_req) printk("!_req \n");
		if(!_req->complete) printk("!_req->complete \n");
		if(!_req->buf) printk("!_req->buf \n");
		if(!list_empty(&req->queue)) printk("!list_empty(&req->queue) \n");		
		return -EINVAL;
	}

//	printk("ep->desc %x \n",ep->desc);
//	printk("ep->ep.name = %s \n",ep->ep.name);
	/*
	if (!_ep || (ep->ep.name != ep0name)) {
		printk("invalid ep\n");
		return -EINVAL;
	}
*/
	dev = ep->udc;

	if (!dev || !dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN) {
		printk("invalid device\n");
		return -EINVAL;
	}
re_scan:
	spin_lock_irqsave(&ep->udc->lock, flags);	
	

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	/* map virtual address to hardware */
	if (req->req.dma == 0) {
		req->req.dma = dma_map_single(ep->udc->gadget.dev.parent,
					req->req.buf,
					req->req.length, ep->is_in
						? DMA_TO_DEVICE
						: DMA_FROM_DEVICE);
		req->mapped = 1;
	} else {
		dma_sync_single_for_device(ep->udc->gadget.dev.parent,
					req->req.dma, req->req.length,
					ep->is_in
						? DMA_TO_DEVICE
						: DMA_FROM_DEVICE);
		req->mapped = 0;
	}

//	if(bo_count > 0)
//		printk("ERROR!!! %d \n",bo_count);

	/* try to kickstart any empty and idle queue */
	if (list_empty(&ep->queue) && !ep->stopped) {
//		printk("next = %x , head = %x \n",ep->queue.next, &ep->queue);
		if (ep->is_in) {
			status = write_fifo(ep, req);
		} else {
			status = read_fifo(ep, req);
		}
	} else {
//		printk("next = %x , head = %x \n",ep->queue.next, &ep->queue);
//		printk("EEEEEEEEE ep->stopped : %d, &ep->queue : %x : %s : next = %x , head : %x \n",ep->stopped, &ep->queue, _ep->name,ep->queue.next, ep->queue);
//		printk("bo_count = %d \n",bo_count);
//		if (list_empty(&ep->queue) ){
//			printk("empty \n");
//		}
//		if(!ep->stopped) {
//			printk("stop !! \n");	
//		}
//		local_irq_restore(flags);
		spin_unlock_irqrestore(&ep->udc->lock, flags);	


		for(i=0;i<10000;i++);

//		printk("re_scan \n");
		goto re_scan;
	}

//	printk("status = %x \n",status);
	if (req && !status) {
//		printk("req  list_add_tail &req->queue : %x ,&ep->queue : %x \n",&req->queue,&ep->queue);
		list_add_tail (&req->queue, &ep->queue);
	}

	spin_unlock_irqrestore(&ep->udc->lock, flags);		

	
	return (status < 0) ? status : 0;
}

static int socle_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct socle_ep	*ep;
	struct socle_request	*req;
	unsigned long		flags;
	
	printk("SQ_ep_dequeue \n");
	ep = container_of(_ep, struct socle_ep, ep);
	if (!_ep || ep->ep.name == ep0name) {
		printk("ep0 retrun ============ \n");
		return -EINVAL;
	}
	
        spin_lock_irqsave(&ep->udc->lock, flags);

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry (req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req) {
		spin_unlock_irqrestore(&ep->udc->lock, flags);
		return -EINVAL;
	}

	done(ep, req, -ECONNRESET);


	spin_unlock_irqrestore(&ep->udc->lock, flags);		

	return 0;

}

static int socle_ep_set_halt(struct usb_ep *_ep, int value)
{
	struct socle_ep	*ep = container_of(_ep, struct socle_ep, ep);
	unsigned long	flags;
	int		status = 0;
	printk("%s : socle_ep_set_halt set stall\n",_ep->name);
	

	spin_lock_irqsave(&ep->udc->lock, flags);		

	/*
	 * fail with still-busy IN endpoints, ensuring correct sequencing
	 * of data tx then stall.  note that the fifo rx bytecount isn't
	 * completely accurate as a tx bytecount.
	 */
	if (ep->is_in && (!list_empty(&ep->queue) != 0))
		status = -EAGAIN;
	else {
		//set stall
		if(_ep->name == "ep1out-bulk") {
			printk("set ep1out-bulk stall\n");
			socle_udc_write(socle_udc_read(SOCLE_UDC_BO_BASE(1) + UDC_RXCON) | RXCON_CF_EN | RXCON_STALL_AUTO_CLR ,SOCLE_UDC_BO_BASE(1) + UDC_RXCON);
//			socle_udc_write(socle_udc_read(SOCLE_UDC_BO_BASE(1) + UDC_RXCON) | RXCON_STALL | RXCON_STALL_AUTO_CLR ,SOCLE_UDC_BO_BASE(2) + UDC_RXCON);
			
		} else if(_ep->name == "ep2in-bulk") {
			printk("set ep2in-bulk stall\n");
			socle_udc_write(socle_udc_read(SOCLE_UDC_BI_BASE(2) + UDC_TXCON) | TXCON_CF_INT_EN | TXCON_STALL_AUTO_CLR ,SOCLE_UDC_BI_BASE(2) + UDC_TXCON);
//			socle_udc_write(socle_udc_read(SOCLE_UDC_BI_BASE(2) + UDC_TXCON) | TXCON_STALL | TXCON_STALL_AUTO_CLR ,SOCLE_UDC_BI_BASE(1) + UDC_TXCON);
			
		}
	}


	spin_unlock_irqrestore(&ep->udc->lock, flags);		

	return status;
}


static struct usb_ep_ops socle_ep_ops = {
	.enable		= socle_ep_enable,
	.disable		= socle_ep_disable,

	.alloc_request	= socle_ep_alloc_request,
	.free_request	= socle_ep_free_request,

//leonid fix for delete ocle_otg_alloc_buffer & socle_otg_free_buffer function
	//.alloc_buffer	= socle_ep_alloc_buffer,
	//.free_buffer	= socle_ep_free_buffer,

	.queue		= socle_ep_queue,
	.dequeue		= socle_ep_dequeue,

	.set_halt		= socle_ep_set_halt,
};


/*-------------------------------------------------------------------------*/

static int socle_get_frame(struct usb_gadget *gadget)
{
	printk("SQ_get_frame ======================\n");
	return 0;
}

static int socle_wakeup(struct usb_gadget *gadget)
{
	printk("SQ_wakeup =======================\n");
#if 0
	struct socle_udc	*udc = to_udc(gadget);
	u32		glbstate;
	int		status = -EINVAL;
	unsigned long	flags;

	DBG("%s\n", __FUNCTION__ );
	local_irq_save(flags);

	if (!udc->clocked || !udc->suspended)
		goto done;

	/* NOTE:  some "early versions" handle ESR differently ... */

	glbstate = socle_udp_read(AT91_UDP_GLB_STAT);
	if (!(glbstate & AT91_UDP_ESR))
		goto done;
	glbstate |= AT91_UDP_ESR;
	socle_udp_write(AT91_UDP_GLB_STAT, glbstate);

done:
	local_irq_restore(flags);
	return status;
#endif
	return 0;
}

/* reinit == restore inital software state */
static void udc_reinit(struct socle_udc *udc)
{
	u32 i;
	INIT_LIST_HEAD(&udc->gadget.ep_list);
	INIT_LIST_HEAD(&udc->gadget.ep0->ep_list);

	for (i = 0; i < NUM_ENDPOINTS; i++) {
		struct socle_ep *ep = &udc->ep[i];

		if (i != 0)
			list_add_tail(&ep->ep.ep_list, &udc->gadget.ep_list);

		
		ep->desc = NULL;
		ep->stopped = 0;

		if(i == 0)
			ep->ep.maxpacket = HI_SPEED_CTRL_PACKET_SIZE;
		else
			ep->ep.maxpacket = HI_SPEED_BULK_PACKET_SIZE;
		
		// initialiser une queue par endpoint
		INIT_LIST_HEAD(&ep->queue);
	}
}

static void stop_activity(struct socle_udc *udc)
{

	struct usb_gadget_driver *driver = udc->driver;
	int i;

//	printk("stop_activity \n");

	if (udc->gadget.speed == USB_SPEED_UNKNOWN)
		driver = NULL;

	udc->gadget.speed = USB_SPEED_UNKNOWN;
	udc->suspended = 0;

	for (i = 0; i < NUM_ENDPOINTS; i++) {
		struct socle_ep *ep = &udc->ep[i];
		ep->stopped = 1;
		nuke(ep, -ESHUTDOWN);
	}
	
	if (driver && (udc->vbus == 1))
//	if (driver)
		driver->disconnect(&udc->gadget);

	udc_reinit(udc);

}


/*
 * activate/deactivate link with host; minimize power usage for
 * inactive links by cutting clocks and transceiver power.
 */
static void pullup(struct socle_udc *udc, int is_on)
{
	printk("pullup ==================\n");
#if 0
	if (!udc->enabled || !udc->vbus)
		is_on = 0;
	printk("%sactive\n", is_on ? "" : "in");
#endif
}

/* vbus is here!  turn everything on that's ready */
static int socle_vbus_session(struct usb_gadget *gadget, int is_active)
{
	printk("SQ_vbus_session =================\n");
/*
	struct socle_udc	*udc = to_udc(gadget);
	unsigned long	flags;

	// VDBG("vbus %s\n", is_active ? "on" : "off");
	local_irq_save(flags);
	udc->vbus = (is_active != 0);
	pullup(udc, is_active);
	local_irq_restore(flags);
	*/
	return 0;
}

static int socle_pullup(struct usb_gadget *gadget, int is_on)
{
	printk("SQ_pullup ===================\n");
/*
	struct socle_udc	*udc = to_udc(gadget);
	unsigned long	flags;

	local_irq_save(flags);
	udc->enabled = is_on = !!is_on;
	pullup(udc, is_on);
	local_irq_restore(flags);
	*/
	return 0;
}

static int socle_set_selfpowered(struct usb_gadget *gadget, int is_on)
{
//	printk("SQ_set_selfpowered ==============\n");

	return 0;
}

static const struct usb_gadget_ops socle_udc_ops = {
	.get_frame		= socle_get_frame,
	.wakeup			= socle_wakeup,
	.set_selfpowered	= socle_set_selfpowered,
	.vbus_session		= socle_vbus_session,
	.pullup			= socle_pullup,
};

/*-------------------------------------------------------------------------*/

static struct socle_udc controller = {
	.gadget = {
		.ops	= &socle_udc_ops,
		.ep0	= &controller.ep[0].ep,
		.name	= driver_name,
		.dev	= {
			.bus_id = "gadget",
		}
	},
	.ep[0] = {
		.ep = {
			.name	= "ep0",
			.ops	= &socle_ep_ops,
		},
		.udc		= &controller,
	},
	.ep[1] = {
		.ep = {
			.name	= "ep1out-bulk",
			.ops	= &socle_ep_ops,
		},
		.udc		= &controller,
		.is_pingpong	= 1,
	},
	.ep[2] = {
		.ep = {
			.name	= "ep2in-bulk",
			.ops	= &socle_ep_ops,
		},
		.udc		= &controller,
		.is_pingpong	= 1,
	},
	.ep[3] = {
		.ep = {
			.name	= "ep3in-int",
			.ops	= &socle_ep_ops,
		},
		.udc		= &controller,
	},
	/* ep6 and ep7 are also reserved (custom silicon might use them) */
};

int usb_gadget_register_driver (struct usb_gadget_driver *driver)
{
	struct socle_udc	*udc = &controller;
	int		retval;

//	printk("usb_gadget_register_driver \n");
	if (udc->driver) {
		printk("UDC already has a gadget driver\n");
		return -EBUSY;
	}

	udc->driver = driver;
	udc->gadget.dev.driver = &driver->driver;
	udc->gadget.dev.driver_data = &driver->driver;
//	udc->enabled = 1;
//	udc->selfpowered = 1;

	retval = driver->bind(&udc->gadget);
	if (retval) {
		printk("driver->bind() returned %d\n", retval);
		udc->driver = NULL;
		return retval;
	}
//	local_irq_disable();
//	pullup(udc, 1);
//	local_irq_enable();

//	printk("bound to %s\n", driver->driver.name);
	return 0;
}
EXPORT_SYMBOL (usb_gadget_register_driver);

int usb_gadget_unregister_driver (struct usb_gadget_driver *driver)
{
	struct socle_udc *udc = &controller;

	if (!driver || driver != udc->driver)
		return -EINVAL;

	local_irq_disable();
	stop_activity(udc);
	local_irq_enable();

	driver->unbind(&udc->gadget);
	udc->driver = NULL;

//	device_del (&udc->gadget.dev);
//	driver_unregister (&driver->driver);

//	printk("unbound from %s\n", driver->driver.name);
	return 0;


}
EXPORT_SYMBOL (usb_gadget_unregister_driver);

/*-------------------------------------------------------------------------*/

static void socle_udc_shutdown(struct platform_device *dev)
{
	/* force disconnect on reboot */
	pullup(platform_get_drvdata(dev), 0);
}

static void socle_udc_timer(unsigned long data)
{
	u32 ret;
	struct usb_ctrlrequest  setup_request;	
	struct socle_udc *udc = (struct socle_udc *)data;

//       unsigned long   flags;

//	printk("SQ_udc_timer \n");
       u32 devinfo = socle_udc_read(SOCLE_UDC_DEV_INFO);

        if (udc->config == 0) {
                if((devinfo & DEV_INFO_DEV_EN)) {
                        udc->config = 1;
                        udc->ep[0].is_in = 1;
                        setup_request.bRequestType = 0;
                        setup_request.bRequest = 0x09;
                        setup_request.wValue = 0x01;
                        setup_request.wIndex = 0;
                        setup_request.wLength = 0;
//                       printk("timer set configuration 1 \n");
//        spin_lock_irqsave(&udc->lock, flags);

			   udc->timer_en = 0;
                        ret = udc->driver->setup(&udc->gadget, &setup_request);

//	 spin_unlock_irqrestore(&udc->lock, flags);

							
                        if (ret < 0) {
                                printk("setup config fail !!!!!!!!!!!!!!!!!!!!\n");
                        }
                } else {
                		if(udc->vbus == 0) {
					udc->timer.expires = jiffies + 30 * HZ;
//					printk("add_timer1 \n");
					add_timer(&udc->timer);
					udc->timer_en = 1;
                		} else {
					udc->timer.expires = jiffies + 1;
//					printk("add_timer + 1 \n");
					add_timer(&udc->timer);
					udc->timer_en = 1;					
                		
                		}
					
                }
			
        }

} /* socle_udc_timer() */


static int __devinit socle_udc_probe(struct platform_device *pdev)
{

	struct device	*dev = &pdev->dev;
	struct socle_udc	*udc;
	int				retval;
	struct resource 	*io_area;	

	/* init software state */
	udc = &controller;
	udc->gadget.dev.parent = dev;
	udc->pdev = pdev;
	udc_reinit(udc);

	udc->gadget.is_dualspeed = 1;
	retval = device_register(&udc->gadget.dev);
	if (retval < 0) {
		printk("device_register fail !! \n");
		goto fail0; 
	}

	spin_lock_init (&udc->lock);

        //initial Ctrl Out DMA data
        udc->ctrl_out_buf = dma_alloc_coherent(udc->gadget.dev.parent,
                        64, &udc->ctrl_out_dma, GFP_KERNEL);

	io_area = request_mem_region(pdev->resource[0].start, pdev->resource[0].end - pdev->resource[0].start + 1, pdev->name);
	if (NULL == io_area) {
		dev_err(&pdev->dev, "cannot reserved UDC region\n");
		retval = -ENXIO;
		goto fail0;
	}
	
	udc->udc_base = IO_ADDRESS(io_area->start);

	/* request UDC and maybe VBUS irqs */
	udc->udc_irq = platform_get_irq(pdev, 0);

	//leonid fix from SA_INTERRUPT to IRQF_DISABLED
	if (request_irq(udc->udc_irq, socle_udc_irq,
			IRQF_DISABLED, driver_name, udc)) {
		printk("request irq %d failed\n", udc->udc_irq);
		retval = -EBUSY;
		goto fail1;
	}

	init_timer(&udc->timer);	

	udc->timer.data = (unsigned long)udc;
	udc->timer.function = socle_udc_timer;
	udc->timer_en = 0;

	proc_vbus_intr(udc);
		
	dev_set_drvdata(dev, udc);


//	platform_set_drvdata(pdev, udc);

//	create_debug_file(udc);

	printk("%s version %s\n", driver_name, DRIVER_VERSION);
	return 0;

fail1:
	device_unregister(&udc->gadget.dev);
fail0:
	return retval;
	

////////////////////////////////////////////////////////////////////////////
}

static int __devexit socle_udc_remove(struct platform_device *pdev)
{
//	struct device	*dev = &pdev->dev;
	struct socle_udc *udc = platform_get_drvdata(pdev);
	free_irq(udc->udc_irq, udc);
	dma_free_coherent(udc->gadget.dev.parent, 64, udc->ctrl_out_buf, udc->ctrl_out_dma);
	device_unregister(&udc->gadget.dev);
	del_timer_sync (&udc->timer);
	platform_set_drvdata(pdev, NULL);
//	printk("remove\n");

	return 0;
}

#ifdef CONFIG_PM
static int socle_udc_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	return 0;
}

static int socle_udc_resume(struct platform_device *pdev)
{

	return 0;
}
#else
#define	socle_udc_suspend	NULL
#define	socle_udc_resume	NULL
#endif

static struct platform_driver socle_udc_driver = {
	.probe		= socle_udc_probe,
	.remove		= __devexit_p(socle_udc_remove),
	.shutdown	= socle_udc_shutdown,
	.suspend	= socle_udc_suspend,
	.resume		= socle_udc_resume,
	.driver		= {
		.name	= (char *) driver_name,
		.owner	= THIS_MODULE,
	},
};

static int __devinit udc_init_module(void)
{
	return platform_driver_register(&socle_udc_driver);
}
module_init(udc_init_module);

static void __devexit udc_exit_module(void)
{
	platform_driver_unregister(&socle_udc_driver);
}
module_exit(udc_exit_module);

MODULE_DESCRIPTION("SQ udc driver");
MODULE_AUTHOR("Ryan Chen");
MODULE_LICENSE("GPL");

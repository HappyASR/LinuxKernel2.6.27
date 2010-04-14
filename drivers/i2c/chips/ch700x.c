#ifdef CONFIG_I2C_DEBUG_CHIP
#endif

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <asm/arch/ch700x.h>
#include <linux/delay.h>

//#define USE_DSPIN
#ifdef	USE_DSPIN
	#define DEFAULT_SAV		(0x1c)
#else
	#define DEFAULT_SAV		(64*2+18)
#endif
#define MPEG4_VOUT_MASTER_MODE
//#define DEFAULT_HP			(0x2d)
#define DEFAULT_HP			(0x06)
//#define DEFAULT_VP			(0xf4)
#define DEFAULT_VP			(0x00)

/* Addresses to scan */
static unsigned short normal_i2c[] = {
	CH7007A_I2C_CLIENT_ADDR,
	I2C_CLIENT_END,
};

static struct i2c_client *ch700x_client;

I2C_CLIENT_INSMOD_1(CH700X);

static int ch700x_command(struct i2c_client *client, unsigned int cmd, void *arg);
static int ch700x_attach_adapter(struct i2c_adapter *adap);
static int ch700x_detect(struct i2c_adapter *adap, int addr, int kind);
static int ch700x_detach_client(struct i2c_client *client);

/* This is the driver that will be inserted */
static struct i2c_driver ch700x_driver = {
	.driver = {
		.name = "CH700X TV Encoder CODEC"
	},
//	.id = I2C_DRIVERID_CH700X,
	.attach_adapter = ch700x_attach_adapter,
	.detach_client = ch700x_detach_client,
	.command = ch700x_command,
};


static int inline
ch700x_write(struct i2c_client *client, u8 reg, u8 val)
{
	int ret;
	u8 buf[2];

	buf[0] = reg;
	buf[1] = val;
	ret = i2c_master_send(client, buf, 2);
	if (ret != 2)
		return -1;
	else
		return 0;
}

static int inline
ch700x_read(struct i2c_client *client, u8 reg)
{
	int ret;
	struct i2c_msg msg[2];
	u8 buf = reg;
	u8 ret_buf;
	int ret_val;      
	memset((void *)msg, 0x00, 2*sizeof(struct i2c_msg));
	msg[0].addr = client->addr;
	msg[0].buf = &buf;
	msg[0].len = 1;
	
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = &ret_buf;
	msg[1].len = 1;
	ret = i2c_transfer(client->adapter, msg, 2);
	//if (ret != 2)
	//	return -1;
	ret_val = ret_buf;
	return ret_val;
}

const char		ch7007a_initset[] =		// Reg Addr, set value
{
	CH700X_PMR, CH700X_PMR_RESET | CH700X_PMR_PD_PIN
, CH700X_PMR, CH700X_PMR_NORMAL | CH700X_PMR_PD_PIN
, CH700X_PMR, CH700X_PMR_NORMAL | CH700X_PMR_PD_PIN
, CH700X_PLLC, CH700X_PLLC_MEM33V | CH700X_PLLC_PLL5VA | CH700X_PLLC_PLL33VD | CH700X_PLLC_PLLS5 | CH700X_PLLC_720x480_858x525_NTSC
//mark by bacon to test interlace mode
/*
, CH700X_MNE, PLLMNE (CH700X_PLLM_640x480_784x600_NTSC,CH700X_PLLN_640x480_784x600_NTSC)
, CH700X_PLLM, PLLM_7_0 (CH700X_PLLM_640x480_784x600_NTSC)
, CH700X_PLLN, PLLN_7_0 (CH700X_PLLN_640x480_784x600_NTSC)
, CH700X_DMR, CH700X_DMR_640x480_784x600_NTSC
*/    
, CH700X_MNE, PLLMNE (CH700X_PLLM_720x480_858x525_NTSC,CH700X_PLLN_720x480_858x525_NTSC)
, CH700X_PLLM, PLLM_7_0 (CH700X_PLLM_720x480_858x525_NTSC)
, CH700X_PLLN, PLLN_7_0 (CH700X_PLLN_720x480_858x525_NTSC)
, CH700X_DMR, CH700X_DMR_720x480_858x525_NTSC

//End of mark by bacon to test interlace mode
, CH700X_FFR, CH700X_FFR_LUMA_MAX | CH700X_FFR_TEXT_MAX | CH700X_FFR_CHROMA_MAX
, CH700X_VBW, CH700X_VBW_YCV_HIGH | CH700X_VBW_YSV_HIGH | CH700X_VBW_YPEAK_ENABLE | CH700X_VBW_CBW_HIGH | CH700X_VBW_CVBW_DISABLE | CH700X_VBW_FLFF_DISABLE
, CH700X_IDF, CH700X_IDF_CCIR656_YCBCR | CH700X_IDF_DACG_NTSC_PALm
#ifdef CONFIG_ARCH_LDK3V21
, CH700X_CM, CH700X_CM_PCM_2 | CH700X_CM_XCM_2 | CH700X_CM_MCP_POS |CH700X_CM_MS_MASTER | CH700X_CM_CFRB_LOOK
#else
, CH700X_CM, CH700X_CM_PCM_2 | CH700X_CM_XCM_2 | CH700X_CM_MCP_NEG |CH700X_CM_MS_MASTER | CH700X_CM_CFRB_LOOK
#endif
, CH700X_CE, CH700X_CE_CONTRAST_NORMAL
//This have order problem must put after PLL
, CH700X_PO, CH700X_PO_VALUE (DEFAULT_SAV, DEFAULT_HP, DEFAULT_VP)
, CH700X_SAV, CH700X_SAV_VALUE (DEFAULT_SAV)
, CH700X_HPR, CH700X_HPR_VALUE (DEFAULT_HP)
, CH700X_VPR, CH700X_VPR_VALUE (DEFAULT_VP)
#ifdef 	MPEG4_VOUT_MASTER_MODE
, CH700X_SPR, CH700X_SPR_HSP_HIGH | CH700X_SPR_VSP_HIGH | CH700X_SPR_SYO_SLAVE |CH700X_SPR_DES_PIN
#else
, CH700X_SPR, CH700X_SPR_HSP_HIGH | CH700X_SPR_VSP_HIGH | CH700X_SPR_SYO_MASTER |CH700X_SPR_DES_PIN
#endif
, CH700X_CIVC, CH700X_CIVC_ACIV_NON
//mark by bacon to test interlace mode

#ifdef 	MPEG4_VOUT_MASTER_MODE
, CH700X_SPR, CH700X_SPR_HSP_HIGH | CH700X_SPR_VSP_HIGH | CH700X_SPR_SYO_SLAVE | CH700X_SPR_DES_PIN
#else
, CH700X_SPR, CH700X_SPR_HSP_HIGH | CH700X_SPR_VSP_HIGH | CH700X_SPR_SYO_MASTER |CH700X_SPR_DES_PIN
#endif
/*
, CH700X_FSCIN (0), CH700X_FSCIN_VALUE (0, CH700X_DMR_640x480_784x600_NTSC_FCSI)
, CH700X_FSCIN (1), CH700X_FSCIN_VALUE (1,CH700X_DMR_640x480_784x600_NTSC_FCSI)
, CH700X_FSCIN (2), CH700X_FSCIN_VALUE (2,CH700X_DMR_640x480_784x600_NTSC_FCSI)
, CH700X_FSCIN (3), CH700X_FSCIN_VALUE (3,CH700X_DMR_640x480_784x600_NTSC_FCSI) | CH700X_FSCI3_GPIOIN1_HIGH | CH700X_FSCI3_GPIOIN0_HIGH | CH700X_FSCI3_DVDD2_33V | CH700X_FSCI3_POUTP_LOW
#ifdef	USE_DSPIN
, CH700X_FSCIN (4), CH700X_FSCIN_VALUE (4, CH700X_DMR_640x480_784x600_NTSC_FCSI) | CH700X_FSCI4_GOENB1_OD | CH700X_FSCI4_GOENB0_OD | CH700X_FSCI4_DSM_DIS | CH700X_FSCI4_DSEN_EN
#else
, CH700X_FSCIN (4), CH700X_FSCIN_VALUE (4, CH700X_DMR_640x480_784x600_NTSC_FCSI) | CH700X_FSCI4_GOENB1_OD | CH700X_FSCI4_GOENB0_OD | CH700X_FSCI4_DSM_DIS | CH700X_FSCI4_DSEN_DIS
#endif
, CH700X_FSCIN (5), CH700X_FSCIN_VALUE (5, CH700X_DMR_640x480_784x600_NTSC_FCSI)
, CH700X_FSCIN (6), CH700X_FSCIN_VALUE (6, CH700X_DMR_640x480_784x600_NTSC_FCSI)
, CH700X_FSCIN (7), CH700X_FSCIN_VALUE (7, CH700X_DMR_640x480_784x600_NTSC_FCSI)
*/
#ifdef 	MPEG4_VOUT_MASTER_MODE
, CH700X_SPR, CH700X_SPR_HSP_HIGH | CH700X_SPR_VSP_HIGH | CH700X_SPR_SYO_SLAVE | CH700X_SPR_DES_PIXEL
#else
, CH700X_SPR, CH700X_SPR_HSP_HIGH | CH700X_SPR_VSP_HIGH | CH700X_SPR_SYO_MASTER | CH700X_SPR_DES_PIXEL
#endif
, CH700X_FSCIN (0), CH700X_FSCIN_VALUE (0, CH700X_DMR_720x480_858x525_NTSC_FCSI)
, CH700X_FSCIN (1), CH700X_FSCIN_VALUE (1, CH700X_DMR_720x480_858x525_NTSC_FCSI)
, CH700X_FSCIN (2), CH700X_FSCIN_VALUE (2, CH700X_DMR_720x480_858x525_NTSC_FCSI)
, CH700X_FSCIN (3), CH700X_FSCIN_VALUE (3, CH700X_DMR_720x480_858x525_NTSC_FCSI) | CH700X_FSCI3_GPIOIN1_HIGH | CH700X_FSCI3_GPIOIN0_HIGH | CH700X_FSCI3_DVDD2_33V | CH700X_FSCI3_POUTP_LOW
#ifdef	USE_DSPIN
, CH700X_FSCIN (4), CH700X_FSCIN_VALUE (4, CH700X_DMR_720x480_858x525_NTSC_FCSI) | CH700X_FSCI4_GOENB1_OD | CH700X_FSCI4_GOENB0_OD | CH700X_FSCI4_DSM_DIS | CH700X_FSCI4_DSEN_EN
#else
, CH700X_FSCIN (4), CH700X_FSCIN_VALUE (4, CH700X_DMR_720x480_858x525_NTSC_FCSI) | CH700X_FSCI4_GOENB1_OD | CH700X_FSCI4_GOENB0_OD | CH700X_FSCI4_DSM_DIS | CH700X_FSCI4_DSEN_DIS
#endif
, CH700X_FSCIN (5), CH700X_FSCIN_VALUE (5, CH700X_DMR_720x480_858x525_NTSC_FCSI)
, CH700X_FSCIN (6), CH700X_FSCIN_VALUE (6, CH700X_DMR_720x480_858x525_NTSC_FCSI)
, CH700X_FSCIN (7), CH700X_FSCIN_VALUE (7, CH700X_DMR_720x480_858x525_NTSC_FCSI)
//End of mark by bacon to test interlace mode

, CH700X_PMR, CH700X_PMR_NORMAL | CH700X_PMR_PD_ON
};

static int
ch700x_setup(void)
{
	int ret=0,i;
	u8 buf;
	//i2c_master_initialize(SOCLE_APB0_I2C0);
	//ch700x_client.addr = CH7007A_I2C_CLIENT_ADDR;

	buf=ch700x_read(ch700x_client, CH700X_VID);
	if(buf != CH7007A_VID)
	{
		printk("!!!!! Error could not find the CH7007A verison ID !!!!! get VID = %x\n",buf);
		return -1;
	}
	printk("I2C Channel get CH7007A verison ID.\n");
		
	printk("CH7007A : Try initial!\n");
	for(i=0;i<sizeof(ch7007a_initset);i=i+2)
	{		
		ret |= ch700x_write(ch700x_client, CH700X_REG_PREFIX|ch7007a_initset[i],ch7007a_initset[i+1]);
		msleep(50);
#if 0
		printk("CH7007A Reg(");
		printk("%x",ch7007a_initset[i]);
		printk(") Set Data(");
		printk("%x",ch7007a_initset[i+1]);	
		printk(")\n");
#endif		
	}

//for read back regs
#if 0
	for(i=2;i<sizeof(ch7007a_initset);i=i+2)
	{
		printk("CH7007A Reg(");
		printk("%x",ch7007a_initset[i]);		
		printk(") Read Back Data(");
		printk("%x",ch700x_read(ch700x_client, ch7007a_initset[i]));
		printk(")\n");		
	}
#endif	
	printk("CH7007A : initial end!\n");
	return ret;
}

static int 
ch700x_command(struct i2c_client *client, unsigned int cmd, void *arg)
{
	return 0;
}


static int
ch700x_attach_adapter(struct i2c_adapter *adap)
{
	int ret=0;
	ret=i2c_probe(adap, &addr_data, ch700x_detect);
  ret |= ch700x_setup();
  if(ret!=0)
  	printk("CH700X: setup error\n");
	return ret;
}

/* This function is called by i2c_probe */
static int
ch700x_detect(struct i2c_adapter *adap, int addr, int kind)
{
 	struct i2c_client *new_client;
	int err = 0;
	if (!i2c_check_functionality(adap, I2C_FUNC_I2C))
		goto exit;
	new_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
	if (NULL == new_client) {
		printk("CH700X: cannot allocate memory to new client\n");
		err = -ENOMEM;
		goto exit;
	}
	new_client->addr = addr;
	new_client->driver = &ch700x_driver;
	new_client->adapter = adap;
	new_client->flags = 0;

	/* Fill in the remaining client fields */
	strlcpy(new_client->name, "CH700X TV Encoder CODEC", I2C_NAME_SIZE);
	ch700x_client = new_client;

	/* Tell the I2C layer a new client has arrived */
	err = i2c_attach_client(new_client);
	if (err) {
		printk("CH700X: cannot attach the new client\n");
		goto exit_kfree;
	}
	return 0;
exit_kfree:
	kfree(new_client);
exit:
	return err;
}

static int
ch700x_detach_client(struct i2c_client *client)
{
	int err;

	err = i2c_detach_client(client);
	if (err)
		return err;
	kfree(client);
	return 0;
}

static int __init
ch700x_init(void)
{
	int ret=0;
	ret |= i2c_add_driver(&ch700x_driver);
	return ret;
}

static void __exit
ch700x_exit(void)
{
	i2c_del_driver(&ch700x_driver);
}

module_init(ch700x_init);
module_exit(ch700x_exit);

MODULE_AUTHOR("JS Ho");
MODULE_DESCRIPTION("CH700X TV Encoder CODEC");
MODULE_LICENSE("GPL");

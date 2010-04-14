#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include <mach/platform.h>

#include <mach/regs-msmv-scu.h>
#include <mach/msmv-scu.h>

#include <linux/delay.h>


//#define SOCLE_SCU_DEBUG

#ifdef SOCLE_SCU_DEBUG
#define SCUDBUG(fmt, args...) printk("%s() " fmt, __FUNCTION__, ## args)
#else
#define SCUDBUG(fmt, args...)
#endif

static struct socle_clock_st {
	unsigned long cpu_clock;
	unsigned long ahb_clock;
	unsigned long apb_clock;
	unsigned long uart_clock;
}socle_clock;

#define SOCLE_SCU_BASE	SOCLE_APB0_SCU

static inline void
socle_scu_write(u32 value, u32 reg) 
{
	SCUDBUG("value = %x, addr = %x\n", value, (IO_ADDRESS(SOCLE_SCU_BASE)+reg));

	iowrite32(value, IO_ADDRESS(SOCLE_SCU_BASE)+reg);
}

static inline u32
socle_scu_read(u32 reg)
{
	u32 tmp;

	tmp = ioread32(IO_ADDRESS(SOCLE_SCU_BASE)+reg);

	SCUDBUG("value = %x, addr = %x\n", tmp, (IO_ADDRESS(SOCLE_SCU_BASE)+reg));	

	return tmp;
}

static inline void
socle_scu_show(char *info)
{
	printk("%s", info);
}

static int socle_scu_pll_formula (int n, int m, int od, int type);

static int 
socle_scu_pll_formula (int n, int m, int od, int type)
{
	int clock;
	
	SCUDBUG("n = %d, m = %d, od = %d, type = %d\n", n, m, od, type);

	if(1 == type){		//CPLL
		clock = EXT_OSC * (m+1) / (n+1) / (od+1) ;
		SCUDBUG("cpu clock = %d\n", clock);
	}
	
	return clock;
}


/*	SCU_MPLLCOM	*/
	/* the configuration value of MPLL for system clock usage */
extern void 
socle_scu_saturation_behavior_enable ()
{	
	u32 tmp;

	tmp = socle_scu_read(SCU_MPLLCON) | SCU_MPLLCOM_SATUR_BEHAV_EN;

	socle_scu_write(tmp, SCU_MPLLCON);
	
	return ;
}

EXPORT_SYMBOL(socle_scu_saturation_behavior_enable);

extern void 
socle_scu_saturation_behavior_disable  ()
{	
	u32 tmp;

	tmp = socle_scu_read(SCU_MPLLCON) & ~SCU_MPLLCOM_SATUR_BEHAV_EN;

	socle_scu_write(tmp, SCU_MPLLCON);
	
	return ;
}

EXPORT_SYMBOL(socle_scu_saturation_behavior_disable);

extern int 
socle_scu_saturation_behavior_status ()		//return 1:enable	0:disable	
{	
	u32 tmp;

	tmp = socle_scu_read(SCU_MPLLCON) & SCU_MPLLCOM_SATUR_BEHAV_EN;

	if(SCU_MPLLCOM_SATUR_BEHAV_EN == tmp)
		return 1;
	else
		return 0;
}

EXPORT_SYMBOL(socle_scu_saturation_behavior_status);


extern void 
socle_scu_fast_locking_circuit_enable ()
{	
	u32 tmp;

	tmp = socle_scu_read(SCU_MPLLCON) | SCU_MPLLCOM_FAST_LOCK_EN;

	socle_scu_write(tmp, SCU_MPLLCON);
	
	return ;
}

extern void 
socle_scu_fast_locking_circuit_disable  ()
{	
	u32 tmp;

	tmp = socle_scu_read(SCU_MPLLCON) & ~SCU_MPLLCOM_FAST_LOCK_EN;

	socle_scu_write(tmp, SCU_MPLLCON);
	
	return ;
}

extern int socle_scu_fast_locking_circuit_status ()		//return 1:enable	0:disable	
{	
	u32 tmp;

	tmp = socle_scu_read(SCU_MPLLCON) & SCU_MPLLCOM_FAST_LOCK_EN;

	if(SCU_MPLLCOM_FAST_LOCK_EN == tmp)
		return 1;
	else
		return 0;
}

extern void 
socle_scu_pll_reset ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_MPLLCON) | SCU_MPLLCOM_PLL_RESET;

	socle_scu_write(tmp, SCU_MPLLCON);
	
	return ;	
}

extern void 
socle_scu_pll_power_down ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_MPLLCON) | SCU_MPLLCOM_PLL_ROWER_DOWN;

	socle_scu_write(tmp, SCU_MPLLCON);
	
	return ;	
}
extern void 
socle_scu_pll_active ()
{	
	u32 tmp;

	tmp = socle_scu_read(SCU_MPLLCON) & ~SCU_MPLLCOM_PLL_ROWER_DOWN;

	socle_scu_write(tmp, SCU_MPLLCON);
	
	return ;
}
extern int socle_scu_pll_power_down_status ()			//return 1:power down	0:active
{
	u32 tmp;

	tmp = socle_scu_read(SCU_MPLLCON) & SCU_MPLLCOM_PLL_ROWER_DOWN;

	if(SCU_MPLLCOM_PLL_ROWER_DOWN == tmp)
		return 1;
	else
		return 0;	
}

extern int 
socle_scu_pll_set (int clock)				//input CPU clock
{	
	u32 tmp, cpll;
			
	switch(clock){
		case SOCLE_SCU_CPU_CLOCK_33 :			
			tmp = SCU_CPU_CLOCK_33;
			break;
		case SOCLE_SCU_CPU_CLOCK_66 :			
			tmp = SCU_CPU_CLOCK_66;
			break;
		case SOCLE_SCU_CPU_CLOCK_80 :			
			tmp = SCU_CPU_CLOCK_80;
			break;
		case SOCLE_SCU_CPU_CLOCK_100 :			
			tmp = SCU_CPU_CLOCK_100;
			break;
		case SOCLE_SCU_CPU_CLOCK_133 :			
			tmp = SCU_CPU_CLOCK_133;
			break;
		case SOCLE_SCU_CPU_CLOCK_200 :			
			tmp = SCU_CPU_CLOCK_200;
			break;
		case SOCLE_SCU_CPU_CLOCK_264 :			
			tmp = SCU_CPU_CLOCK_264;
			break;
		case SOCLE_SCU_CPU_CLOCK_266 :			
			tmp = SCU_CPU_CLOCK_266;
			break;
		case SOCLE_SCU_CPU_CLOCK_280 :			
			tmp = SCU_CPU_CLOCK_280;
			break;
		case SOCLE_SCU_CPU_CLOCK_300 :			
			tmp = SCU_CPU_CLOCK_300;
			break;
		case SOCLE_SCU_CPU_CLOCK_320 :			
			tmp = SCU_CPU_CLOCK_320;
			break;
		case SOCLE_SCU_CPU_CLOCK_340 :			
			tmp = SCU_CPU_CLOCK_340;
			break;
		case SOCLE_SCU_CPU_CLOCK_350 :			
			tmp = SCU_CPU_CLOCK_350;
			break;
		case SOCLE_SCU_CPU_CLOCK_360 :			
			tmp = SCU_CPU_CLOCK_360;
			break;
		case SOCLE_SCU_CPU_CLOCK_400 :			
			tmp = SCU_CPU_CLOCK_400;
			break;
		case SOCLE_SCU_CPU_CLOCK_252 :			
			tmp = SCU_CPU_CLOCK_252;
			break;
		case SOCLE_SCU_CPU_CLOCK_240 :			
			tmp = SCU_CPU_CLOCK_240;
			break;
		case SOCLE_SCU_CPU_CLOCK_212 :			
			tmp = SCU_CPU_CLOCK_212;
			break;
		case SOCLE_SCU_CPU_CLOCK_292 :			
			tmp = SCU_CPU_CLOCK_292;
			break;
		case SOCLE_SCU_CPU_CLOCK_160 :			
			tmp = SCU_CPU_CLOCK_160;
			break;
		default :		
			socle_scu_show("unknow upll clock !!\n");
			return -1;
			break;
	}

	SCUDBUG("tmp = %x\n", tmp);

	cpll = ((socle_scu_read(SCU_MPLLCON) & ~SCU_MPLLCOM_CPLL_M) 
			| (tmp << SCU_MPLLCOM_CPLL_S));

	SCUDBUG("cpll = 0x%08x\n", cpll);

	socle_scu_write(cpll, SCU_MPLLCON);

	mdelay(10);

	tmp = socle_get_cpu_clock();

	SCUDBUG("sq_get_cpu_clock() = %d\n", tmp);	

	return 0;
}

extern int 
socle_scu_pll_get ()					//return cpll clock value
{	
	int m,n,od;
	u32 clk;

	clk = socle_scu_read(SCU_MPLLCON);

	SCUDBUG("clk = %x\n", clk);
	
	n = (clk & SCU_MPLLCOM_REF_DIVIDER_M) >> SCU_MPLLCOM_REF_DIVIDER_S;
	m = (clk & SCU_MPLLCOM_MUL_FACTOR_M) >> SCU_MPLLCOM_MUL_FACTOR_S;
	od = (clk & SCU_MPLLCOM_OUTPUT_DIVIDER_M) >> SCU_MPLLCOM_OUTPUT_DIVIDER_S;

	SCUDBUG("n = %d, m = %d, od = %d\n", n, m, od);
	
	clk = socle_scu_pll_formula(n, m, od, 1);

	return clk;
}


/*	SCU_PLLLOCK		*/
	/* the lock time value of MPLL */
extern void 
socle_scu_mpll_lock_time_counter_set (int cnt)	//input PLL lock time counter for MPLL
{
	u32 tmp;

	SCUDBUG("cnt = 0x%x\n", cnt);

	tmp = cnt & SCU_PLLLOCK_MPLL_LOCK_TIME_CNT_M;

	SCUDBUG("tmp = 0x%x, SCU_PLLLOCK = 0x%x\n", tmp, SCU_PLLLOCK);
	socle_scu_write(tmp, SCU_PLLLOCK);
	
	return ;	
}

extern int socle_scu_mpll_lock_time_counter_get ()		//return PLL lock time counter for MPLL
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLLLOCK) & SCU_PLLLOCK_MPLL_LOCK_TIME_CNT_M;
	
	return tmp;		
}


/*	SCU_CLKSRC	*/
	/* the clock source control */
extern void socle_scu_clk_src_ext_input_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CLKSRC) & ~SCU_CLKSRC_CLK_SRC_MPLL_OUTPUT;

	socle_scu_write(tmp, SCU_CLKSRC);
	
	return ;	
}

extern void socle_scu_clk_src_mpll_output_set()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CLKSRC) | SCU_CLKSRC_CLK_SRC_MPLL_OUTPUT;

	socle_scu_write(tmp, SCU_CLKSRC);
	
	return ;	
}

extern int socle_scu_clk_src_status ()					//return 1:MPLL output clock  0:External input clock	
{	
	u32 tmp;

	tmp = socle_scu_read(SCU_CLKSRC) & SCU_CLKSRC_CLK_SRC_MPLL_OUTPUT;

	if(SCU_CLKSRC_CLK_SRC_MPLL_OUTPUT == tmp)
		return 1;
	else
		return 0;
}

/*	SCU_CLKDIV	*/
	/* the clock divisor control */
extern int socle_scu_clock_ratio_set (int ratio)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CLKDIV) & ~SCU_CLKDIV_RATIO_CCLK_HCLK_M;
	
	switch(ratio){
		case SOCLE_SCU_CLOCK_RATIO_2_1 :
			tmp = tmp | SCU_CLKDIV_RATIO_CCLK_HCLK_2;
			break;
		case SOCLE_SCU_CLOCK_RATIO_4_1 :
			tmp = tmp | SCU_CLKDIV_RATIO_CCLK_HCLK_4;
			break;
		case SOCLE_SCU_CLOCK_RATIO_8_1 :
			tmp = tmp | SCU_CLKDIV_RATIO_CCLK_HCLK_8;
			break;
		default :
			socle_scu_show("unknow ratio value\n");
			return -1;
			break;
	}				
	socle_scu_write(tmp, SCU_CLKDIV);
	
	mdelay(10);
	
	return 0;
}

extern int 
socle_scu_clock_ratio_get ()
{
	u32 tmp;
	int ratio;

	tmp = socle_scu_read(SCU_CLKDIV) & SCU_CLKDIV_RATIO_CCLK_HCLK_M;

	switch(tmp){
		case SCU_CLKDIV_RATIO_CCLK_HCLK_2 :
			ratio = 2;
			break;
		case SCU_CLKDIV_RATIO_CCLK_HCLK_4 :
			ratio = 4;
			break;
		case SCU_CLKDIV_RATIO_CCLK_HCLK_8 :
			ratio = 8;
			break;
		default :
			socle_scu_show("unknow ratio value\n");
			return -1;
			break;
	}		
	return ratio;
}

EXPORT_SYMBOL(socle_scu_clock_ratio_get);


/*	SCU_CLKOUT	*/
	/* the clock output configuration */
extern void 
socle_scu_clkout_src_mpll_output_set()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CLKOUT) & ~SCU_CLKOUT_CLK_SRC_CPUCLK;

	socle_scu_write(tmp, SCU_CLKOUT); 
	
	return ;
}		

extern void 
socle_scu_clkout_src_cpuclk_set()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CLKOUT) | SCU_CLKOUT_CLK_SRC_CPUCLK;

	socle_scu_write(tmp, SCU_CLKOUT); 
	
	return ;
}		

extern int 
socle_scu_clkout_src_status ()				//return 1:CPUCLK clock  0:MPLL output clock
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CLKOUT) & SCU_CLKOUT_CLK_SRC_CPUCLK;

	if(SCU_CLKOUT_CLK_SRC_CPUCLK == tmp)
		return 1;
	else
		return 0;
}

extern int 
socle_scu_clkout_divide_set (int div)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CLKOUT) & ~SCU_CLKOUT_CLKOUT_DIV_CLKOUT_M;
	
	switch(div){
		case SOCLE_SCU_CLKOUT_CLK_DIV_1 :
			tmp = tmp | SCU_CLKOUT_CLKOUT_DIV_CLKOUT;
			break;
		case SOCLE_SCU_CLKOUT_CLK_DIV_2 :
			tmp = tmp | SCU_CLKOUT_CLKOUT_DIV_CLKOUT_2;
			break;
		case SOCLE_SCU_CLKOUT_CLK_DIV_4 :
			tmp = tmp | SCU_CLKOUT_CLKOUT_DIV_CLKOUT_4;
			break;
		case SOCLE_SCU_CLKOUT_CLK_DIV_8 :
			tmp = tmp | SCU_CLKOUT_CLKOUT_DIV_CLKOUT_8;
			break;
		case SOCLE_SCU_CLKOUT_CLK_DIV_16 :
			tmp = tmp | SCU_CLKOUT_CLKOUT_DIV_CLKOUT_16;
			break;
		case SOCLE_SCU_CLKOUT_CLK_DIV_32 :
			tmp = tmp | SCU_CLKOUT_CLKOUT_DIV_CLKOUT_32;
			break;
		default :
			socle_scu_show("unknow divide value\n");
			return -1;
			break;
	}				
	socle_scu_write(tmp, SCU_CLKOUT);	
	
	return 0;
}

extern int 
socle_scu_clkout_divide_get ()
{
	u32 tmp;
	int div;

	tmp = socle_scu_read(SCU_CLKOUT) & SCU_CLKOUT_CLKOUT_DIV_CLKOUT_M;
	
	switch(tmp){
		case SCU_CLKOUT_CLKOUT_DIV_CLKOUT :
			div = SOCLE_SCU_CLKOUT_CLK_DIV_1;
			break;
		case SCU_CLKOUT_CLKOUT_DIV_CLKOUT_2 :
			div = SOCLE_SCU_CLKOUT_CLK_DIV_2;
			break;
		case SCU_CLKOUT_CLKOUT_DIV_CLKOUT_4 :
			div = SOCLE_SCU_CLKOUT_CLK_DIV_4;
			break;
		case SCU_CLKOUT_CLKOUT_DIV_CLKOUT_8 :
			div = SOCLE_SCU_CLKOUT_CLK_DIV_8;
			break;
		case SCU_CLKOUT_CLKOUT_DIV_CLKOUT_16 :
			div = SOCLE_SCU_CLKOUT_CLK_DIV_16;
			break;
		case SCU_CLKOUT_CLKOUT_DIV_CLKOUT_32 :
			div = SOCLE_SCU_CLKOUT_CLK_DIV_32;
			break;
		default :
			socle_scu_show("unknow divide value\n");
			return -1;
			break;
	}				
	
	return div;
}


/*	SCU_HCLKEN	*/
	/*  the enable bit for HCLK clock of individual peripheral */
extern int 
socle_scu_hclk_enable (int ip)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_HCLKEN);
		
	switch(ip){
		case SOCLE_SCU_HCLKEN_UHC :
			tmp = tmp | SCU_HCLKEN_UHC_EN;
			break;
		case SOCLE_SCU_HCLKEN_DMA :
			tmp = tmp | SCU_HCLKEN_DMA_EN;
			break;
		case SOCLE_SCU_HCLKEN_CLCD :
			tmp = tmp | SCU_HCLKEN_CLCD_EN;
			break;
		case SOCLE_SCU_HCLKEN_NFC :
			tmp = tmp | SCU_HCLKEN_NFC_EN;
			break;
		case SOCLE_SCU_HCLKEN_SRAM :
			tmp = tmp | SCU_HCLKEN_EMB_SRAM_EN;
			break;
		default :
			socle_scu_show("unknow peripheral\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_HCLKEN); 
		
	return 0;
}

extern int 
socle_scu_hclk_disable (int ip)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_HCLKEN);
		
	switch(ip){
		case SOCLE_SCU_HCLKEN_UHC :
			tmp = tmp & ~SCU_HCLKEN_UHC_EN;
			break;
		case SOCLE_SCU_HCLKEN_DMA :
			tmp = tmp & ~SCU_HCLKEN_DMA_EN;
			break;
		case SOCLE_SCU_HCLKEN_CLCD :
			tmp = tmp & ~SCU_HCLKEN_CLCD_EN;
			break;
		case SOCLE_SCU_HCLKEN_NFC :
			tmp = tmp & ~SCU_HCLKEN_NFC_EN;
			break;
		case SOCLE_SCU_HCLKEN_SRAM :
			tmp = tmp & ~SCU_HCLKEN_EMB_SRAM_EN;
			break;
		default :
			socle_scu_show("unknow peripheral\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_HCLKEN); 
	
	return 0;
}

extern int 
socle_scu_hclk_status (int ip)			//return 1:enable	0:disable	
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_HCLKEN);
		
	switch(ip){
		case SOCLE_SCU_HCLKEN_UHC :
			tmp = tmp & SCU_HCLKEN_UHC_EN;
			break;
		case SOCLE_SCU_HCLKEN_DMA :
			tmp = tmp & SCU_HCLKEN_DMA_EN;
			break;
		case SOCLE_SCU_HCLKEN_CLCD :
			tmp = tmp & SCU_HCLKEN_CLCD_EN;
			break;
		case SOCLE_SCU_HCLKEN_NFC :
			tmp = tmp & SCU_HCLKEN_NFC_EN;
			break;
		case SOCLE_SCU_HCLKEN_SRAM :
			tmp = tmp & SCU_HCLKEN_EMB_SRAM_EN;
			break;
		default :
			socle_scu_show("unknow peripheral\n");
			return -1;
			break;
	}
	if(tmp)
		return 1;
	else
		return 0;
}

/*	SCU_PCLKEN	*/
	/*  the enable bit for PCLK clock of individual peripheral*/
extern int 
socle_scu_pclk_enable (int ip)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PCLKEN);
		
	switch(ip){
		case SOCLE_SCU_PCLKEN_UART0 :
			tmp = tmp | SCU_PCLKEN_UART0_EN;
			break;
		case SOCLE_SCU_PCLKEN_UART1 :
			tmp = tmp | SCU_PCLKEN_UART1_EN;
			break;
		case SOCLE_SCU_PCLKEN_SPI :
			tmp = tmp | SCU_PCLKEN_SPI_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2C0 :
			tmp = tmp | SCU_PCLKEN_I2C0_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2C1 :
			tmp = tmp | SCU_PCLKEN_I2C1_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2C2 :
			tmp = tmp | SCU_PCLKEN_I2C2_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2S :
			tmp = tmp | SCU_PCLKEN_I2S_EN;
			break;
		case SOCLE_SCU_PCLKEN_SDMMC :
			tmp = tmp | SCU_PCLKEN_SDMMC_EN;
			break;
		case SOCLE_SCU_PCLKEN_TIMER :
			tmp = tmp | SCU_PCLKEN_TIMER_EN;
			break;
		case SOCLE_SCU_PCLKEN_GPIO :
			tmp = tmp | SCU_PCLKEN_GPIO_EN;
			break;
		case SOCLE_SCU_PCLKEN_RTC :
			tmp = tmp | SCU_PCLKEN_RTC_EN;
			break;
		case SOCLE_SCU_PCLKEN_WDT :
			tmp = tmp | SCU_PCLKEN_WDT_EN;
			break;
		case SOCLE_SCU_PCLKEN_PWM :
			tmp = tmp | SCU_PCLKEN_PWM_EN;
			break;
		default :
			socle_scu_show("unknow peripheral\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_PCLKEN); 
	
	return 0;
}

extern int 
socle_scu_pclk_disable (int ip)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PCLKEN);
		
	switch(ip){
		case SOCLE_SCU_PCLKEN_UART0 :
			tmp = tmp & ~SCU_PCLKEN_UART0_EN;
			break;
		case SOCLE_SCU_PCLKEN_UART1 :
			tmp = tmp & ~SCU_PCLKEN_UART1_EN;
			break;
		case SOCLE_SCU_PCLKEN_SPI :
			tmp = tmp & ~SCU_PCLKEN_SPI_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2C0 :
			tmp = tmp & ~SCU_PCLKEN_I2C0_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2C1 :
			tmp = tmp & ~SCU_PCLKEN_I2C1_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2C2 :
			tmp = tmp & ~SCU_PCLKEN_I2C2_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2S :
			tmp = tmp & ~SCU_PCLKEN_I2S_EN;
			break;
		case SOCLE_SCU_PCLKEN_SDMMC :
			tmp = tmp & ~SCU_PCLKEN_SDMMC_EN;
			break;
		case SOCLE_SCU_PCLKEN_TIMER :
			tmp = tmp & ~SCU_PCLKEN_TIMER_EN;
			break;
		case SOCLE_SCU_PCLKEN_GPIO :
			tmp = tmp & ~SCU_PCLKEN_GPIO_EN;
			break;
		case SOCLE_SCU_PCLKEN_RTC :
			tmp = tmp & ~SCU_PCLKEN_RTC_EN;
			break;
		case SOCLE_SCU_PCLKEN_WDT :
			tmp = tmp & ~SCU_PCLKEN_WDT_EN;
			break;
		case SOCLE_SCU_PCLKEN_PWM :
			tmp = tmp & ~SCU_PCLKEN_PWM_EN;
			break;
		default :
			socle_scu_show("unknow peripheral\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_PCLKEN); 
	
	return 0;
}


extern int 
socle_scu_pclk_status (int ip)			//return 1:enable	0:disable	
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PCLKEN);
		
	switch(ip){
		case SOCLE_SCU_PCLKEN_UART0 :
			tmp = tmp & SCU_PCLKEN_UART0_EN;
			break;
		case SOCLE_SCU_PCLKEN_UART1 :
			tmp = tmp & SCU_PCLKEN_UART1_EN;
			break;
		case SOCLE_SCU_PCLKEN_SPI :
			tmp = tmp & SCU_PCLKEN_SPI_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2C0 :
			tmp = tmp & SCU_PCLKEN_I2C0_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2C1 :
			tmp = tmp & SCU_PCLKEN_I2C1_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2C2 :
			tmp = tmp & SCU_PCLKEN_I2C2_EN;
			break;
		case SOCLE_SCU_PCLKEN_I2S :
			tmp = tmp & SCU_PCLKEN_I2S_EN;
			break;
		case SOCLE_SCU_PCLKEN_SDMMC :
			tmp = tmp & SCU_PCLKEN_SDMMC_EN;
			break;
		case SOCLE_SCU_PCLKEN_TIMER :
			tmp = tmp & SCU_PCLKEN_TIMER_EN;
			break;
		case SOCLE_SCU_PCLKEN_GPIO :
			tmp = tmp & SCU_PCLKEN_GPIO_EN;
			break;
		case SOCLE_SCU_PCLKEN_RTC :
			tmp = tmp & SCU_PCLKEN_RTC_EN;
			break;
		case SOCLE_SCU_PCLKEN_WDT :
			tmp = tmp & SCU_PCLKEN_WDT_EN;
			break;
		case SOCLE_SCU_PCLKEN_PWM :
			tmp = tmp & SCU_PCLKEN_PWM_EN;
			break;
		default :
			socle_scu_show("unknow peripheral\n");
			return -1;
			break;
	}
	if(tmp)
		return 1;
	else
		return 0;
}



/*	SCU_SCLKEN	*/
	/*  the enable bit for special clock of individual peripheral */
extern int 
socle_scu_sclk_enable (int ip)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_SCLKEN);
		
	switch(ip){
		case SOCLE_SCU_SCLKEN_UART0 :
			tmp = tmp | SCU_SCLKEN_UCLK_UART0_EN;
			break;
		case SOCLE_SCU_SCLKEN_UART1 :
			tmp = tmp | SCU_SCLKEN_UCLK_UART1_EN;
			break;
		case SOCLE_SCU_SCLKEN_I2S :
			tmp = tmp | SCU_SCLKEN_I2SCLK_I2S_EN;
			break;
		case SOCLE_SCU_SCLKEN_UHC :
			tmp = tmp | SCU_SCLKEN_UHCCLK_UHC_EN;
			break;
		case SOCLE_SCU_SCLKEN_LCD :
			tmp = tmp | SCU_SCLKEN_LCDCLK_LCD_EN;
			break;
		case SOCLE_SCU_SCLKEN_RTC :
			tmp = tmp | SCU_SCLKEN_RTCCLK_RTC_EN;
			break;
		default :
			socle_scu_show("unknow peripheral\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_SCLKEN); 
	
	return 0;
}

extern int 
socle_scu_sclk_disable (int ip)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_SCLKEN);
		
	switch(ip){
		case SOCLE_SCU_SCLKEN_UART0 :
			tmp = tmp & ~SCU_SCLKEN_UCLK_UART0_EN;
			break;
		case SOCLE_SCU_SCLKEN_UART1 :
			tmp = tmp & ~SCU_SCLKEN_UCLK_UART1_EN;
			break;
		case SOCLE_SCU_SCLKEN_I2S :
			tmp = tmp & ~SCU_SCLKEN_I2SCLK_I2S_EN;
			break;
		case SOCLE_SCU_SCLKEN_UHC :
			tmp = tmp & ~SCU_SCLKEN_UHCCLK_UHC_EN;
			break;
		case SOCLE_SCU_SCLKEN_LCD :
			tmp = tmp & ~SCU_SCLKEN_LCDCLK_LCD_EN;
			break;
		case SOCLE_SCU_SCLKEN_RTC :
			tmp = tmp & ~SCU_SCLKEN_RTCCLK_RTC_EN;
			break;
		default :
			socle_scu_show("unknow peripheral\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_SCLKEN); 
	
	return 0;
}

extern int 
socle_scu_sclk_status (int ip)			//return 1:enable	0:disable
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_SCLKEN);
		
	switch(ip){
		case SOCLE_SCU_SCLKEN_UART0 :
			tmp = tmp & SCU_SCLKEN_UCLK_UART0_EN;
			break;
		case SOCLE_SCU_SCLKEN_UART1 :
			tmp = tmp & SCU_SCLKEN_UCLK_UART1_EN;
			break;
		case SOCLE_SCU_SCLKEN_I2S :
			tmp = tmp & SCU_SCLKEN_I2SCLK_I2S_EN;
			break;
		case SOCLE_SCU_SCLKEN_UHC :
			tmp = tmp & SCU_SCLKEN_UHCCLK_UHC_EN;
			break;
		case SOCLE_SCU_SCLKEN_LCD :
			tmp = tmp & SCU_SCLKEN_LCDCLK_LCD_EN;
			break;
		case SOCLE_SCU_SCLKEN_RTC :
			tmp = tmp & SCU_SCLKEN_RTCCLK_RTC_EN;
			break;
		default :
			socle_scu_show("unknow peripheral\n");
			return -1;
			break;
	}
	if(tmp)
		return 1;
	else
		return 0;

}	


/*	SCU_SWRST	*/
	/*	generate software reset	*/
extern void 
socle_scu_sw_reset (void)
{
	u32 tmp;

	tmp = socle_scu_chip_id_get () ;
	
	socle_scu_write(tmp, SCU_SWRST);
	
	return ;	
}


/*	SCU_REMAP	*/
	/*	generate software reset	*/
extern void 
socle_scu_sw_remap (void)
{
	u32 tmp;

	tmp = socle_scu_chip_id_get () ;
	
	socle_scu_write(tmp, SCU_REMAP);
	
	return ;	
}

/*	SCU_PWRMODE	*/
	/*	the power mode control bit	*/
extern void 
socle_scu_power_mode_idle_set (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PWRMODE) | SCU_PWRMODE_IDLE_EN;

	socle_scu_write(tmp, SCU_PWRMODE); 
	
	return ;	
}

EXPORT_SYMBOL(socle_scu_power_mode_idle_set);

extern void 
socle_scu_power_mode_stop_set (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PWRMODE) | SCU_PWRMODE_STOP_EN;

	SCUDBUG("\n\ntmp = %x, SCU_PWRMODE = %x\n\n", tmp, SCU_PWRMODE);

	socle_scu_write(tmp, SCU_PWRMODE); 
	
	return ;	
}

EXPORT_SYMBOL(socle_scu_power_mode_stop_set);

extern void 
socle_scu_power_mode_sleep_set (void)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PWRMODE) & ~SCU_PWRMODE_SLEEP_M;
	tmp = tmp | SCU_PWRMODE_SLEEP_EN;

	socle_scu_write(tmp, SCU_PWRMODE); 
	
	return ;	
}

EXPORT_SYMBOL(socle_scu_power_mode_sleep_set);



/*	SCU_PWRCFG	*/
	/*	the power management configuration bit	*/
extern int 
socle_scu_pwrcfg_filter_enable (int ip)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG);
		
	switch(ip){
		case SOCLE_SCU_PWRCFG_EXT_INT0 :
			tmp = tmp | SCU_PWRCFG_FILTER_EXT_INT0_EN;
			break;
		case SOCLE_SCU_PWRCFG_EXT_INT1 :
			tmp = tmp | SCU_PWRCFG_FILTER_EXT_INT1_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO0 :
			tmp = tmp | SCU_PWRCFG_FILTER_GPIO0_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO1 :
			tmp = tmp | SCU_PWRCFG_FILTER_GPIO1_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO2 :
			tmp = tmp | SCU_PWRCFG_FILTER_GPIO2_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO3 :
			tmp = tmp | SCU_PWRCFG_FILTER_GPIO3_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO4 :
			tmp = tmp | SCU_PWRCFG_FILTER_GPIO4_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO5 :
			tmp = tmp | SCU_PWRCFG_FILTER_GPIO5_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO6 :
			tmp = tmp | SCU_PWRCFG_FILTER_GPIO6_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO7 :
			tmp = tmp | SCU_PWRCFG_FILTER_GPIO7_EN;
			break;
		default :
			socle_scu_show("unknow configuration bit\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_PWRCFG); 
	
	return 0;
}


extern int 
socle_scu_pwrcfg_filter_disable (int ip)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG);
		
	switch(ip){
		case SOCLE_SCU_PWRCFG_EXT_INT0 :
			tmp = tmp & ~SCU_PWRCFG_FILTER_EXT_INT0_EN;
			break;
		case SOCLE_SCU_PWRCFG_EXT_INT1 :
			tmp = tmp & ~SCU_PWRCFG_FILTER_EXT_INT1_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO0 :
			tmp = tmp & ~SCU_PWRCFG_FILTER_GPIO0_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO1 :
			tmp = tmp & ~SCU_PWRCFG_FILTER_GPIO1_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO2 :
			tmp = tmp & ~SCU_PWRCFG_FILTER_GPIO2_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO3 :
			tmp = tmp & ~SCU_PWRCFG_FILTER_GPIO3_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO4 :
			tmp = tmp & ~SCU_PWRCFG_FILTER_GPIO4_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO5 :
			tmp = tmp & ~SCU_PWRCFG_FILTER_GPIO5_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO6 :
			tmp = tmp & ~SCU_PWRCFG_FILTER_GPIO6_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO7 :
			tmp = tmp & ~SCU_PWRCFG_FILTER_GPIO7_EN;
			break;
		default :
			socle_scu_show("unknow configuration bit\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_PWRCFG); 
	
	return 0;
}

extern int 
socle_scu_pwrcfg_filter_status (int ip)			//return 1:enable	0:disable	
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG);
		
	switch(ip){
		case SOCLE_SCU_PWRCFG_EXT_INT0 :
			tmp = tmp & SCU_PWRCFG_FILTER_EXT_INT0_EN;
			break;
		case SOCLE_SCU_PWRCFG_EXT_INT1 :
			tmp = tmp & SCU_PWRCFG_FILTER_EXT_INT1_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO0 :
			tmp = tmp & SCU_PWRCFG_FILTER_GPIO0_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO1 :
			tmp = tmp & SCU_PWRCFG_FILTER_GPIO1_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO2 :
			tmp = tmp & SCU_PWRCFG_FILTER_GPIO2_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO3 :
			tmp = tmp & SCU_PWRCFG_FILTER_GPIO3_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO4 :
			tmp = tmp & SCU_PWRCFG_FILTER_GPIO4_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO5 :
			tmp = tmp & SCU_PWRCFG_FILTER_GPIO5_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO6 :
			tmp = tmp & SCU_PWRCFG_FILTER_GPIO6_EN;
			break;
		case SOCLE_SCU_PWRCFG_GPIO7 :
			tmp = tmp & SCU_PWRCFG_FILTER_GPIO7_EN;
			break;
		default :
			socle_scu_show("unknow configuration bit\n");
			return -1;
			break;
	}
	if(tmp)
		return 1;
	else	
		return 0;
}


extern int 
socle_scu_ext_int0_trigger_type_set (int type)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG) & ~SCU_PWRCFG_EXT_INT0_TRIG_TYPE_M;
		
	switch(type){
		case SOCLE_SCU_PWRCFG_HIGH_LEVEL :
			tmp = tmp | SCU_PWRCFG_EXT_INT0_TRIG_TYPE_HI_LV;
			break;
		case SOCLE_SCU_PWRCFG_LOW_LEVEL :
			tmp = tmp | SCU_PWRCFG_EXT_INT0_TRIG_TYPE_LO_LV;
			break;
		case SOCLE_SCU_PWRCFG_RISING_LEVEL :
			tmp = tmp | SCU_PWRCFG_EXT_INT0_TRIG_TYPE_RIS_EG;
			break;
		case SOCLE_SCU_PWRCFG_FALLING_LEVEL :
			tmp = tmp | SCU_PWRCFG_EXT_INT0_TRIG_TYPE_FAL_EG;
			break;
		default :
			socle_scu_show("unknow trigger type\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_PWRCFG); 
	
	return 0;
}

extern int 
socle_scu_ext_int0_trigger_type_get ()
{	
	u32 tmp;
	int type;
	
	tmp = socle_scu_read(SCU_PWRCFG) & SCU_PWRCFG_EXT_INT0_TRIG_TYPE_M;
		
	switch(tmp){
		case SCU_PWRCFG_EXT_INT0_TRIG_TYPE_HI_LV :
			type = SOCLE_SCU_PWRCFG_HIGH_LEVEL;
			break;
		case SCU_PWRCFG_EXT_INT0_TRIG_TYPE_LO_LV :
			type = SOCLE_SCU_PWRCFG_LOW_LEVEL;
			break;
		case SCU_PWRCFG_EXT_INT0_TRIG_TYPE_RIS_EG:
			type = SOCLE_SCU_PWRCFG_RISING_LEVEL;
			break;
		case SCU_PWRCFG_EXT_INT0_TRIG_TYPE_FAL_EG:
			type = SOCLE_SCU_PWRCFG_FALLING_LEVEL;
			break;
		default :
			socle_scu_show("unknow trigger type\n");
			return -1;
			break;
	}
	
	return type;
}

extern int 
socle_scu_ext_int1_trigger_type_set (int type)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG) & ~SCU_PWRCFG_EXT_INT1_TRIG_TYPE_M;
		
	switch(type){
		case SOCLE_SCU_PWRCFG_HIGH_LEVEL :
			tmp = tmp | SCU_PWRCFG_EXT_INT1_TRIG_TYPE_HI_LV;
			break;
		case SOCLE_SCU_PWRCFG_LOW_LEVEL :
			tmp = tmp | SCU_PWRCFG_EXT_INT1_TRIG_TYPE_LO_LV;
			break;
		case SOCLE_SCU_PWRCFG_RISING_LEVEL :
			tmp = tmp | SCU_PWRCFG_EXT_INT1_TRIG_TYPE_RIS_EG;
			break;
		case SOCLE_SCU_PWRCFG_FALLING_LEVEL :
			tmp = tmp | SCU_PWRCFG_EXT_INT1_TRIG_TYPE_FAL_EG;
			break;
		default :
			socle_scu_show("unknow trigger type\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_PWRCFG); 
	
	return 0;
}

extern int 
socle_scu_ext_int1_trigger_type_get ()
{	
	u32 tmp;
	int type;
	
	tmp = socle_scu_read(SCU_PWRCFG) & SCU_PWRCFG_EXT_INT1_TRIG_TYPE_M;
		
	switch(tmp){
		case SCU_PWRCFG_EXT_INT1_TRIG_TYPE_HI_LV :
			type = SOCLE_SCU_PWRCFG_HIGH_LEVEL;
			break;
		case SCU_PWRCFG_EXT_INT1_TRIG_TYPE_LO_LV :
			type = SOCLE_SCU_PWRCFG_LOW_LEVEL;
			break;
		case SCU_PWRCFG_EXT_INT1_TRIG_TYPE_RIS_EG:
			type = SOCLE_SCU_PWRCFG_RISING_LEVEL;
			break;
		case SCU_PWRCFG_EXT_INT1_TRIG_TYPE_FAL_EG:
			type = SOCLE_SCU_PWRCFG_FALLING_LEVEL;
			break;
		default :
			socle_scu_show("unknow trigger type\n");
			return -1;
			break;
	}
	
	return type;
}


extern int 
socle_scu_stabdbywifi_set (int type)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG)  & ~SCU_PWRCFG_STANDBYWIFI_M;
		
	switch(type){
		case SOCLE_SCU_PWRCFG_STANDBYWIFI_IGNORE :
			tmp = tmp | SCU_PWRCFG_STANDBYWIFI_IGNORE;
			break;
		case SOCLE_SCU_PWRCFG_STANDBYWIFI_IDLE :
			tmp = tmp | SCU_PWRCFG_STANDBYWIFI_IDLE;
			break;
		case SOCLE_SCU_PWRCFG_STANDBYWIFI_STOP :
			tmp = tmp | SCU_PWRCFG_STANDBYWIFI_STOP;
			break;
		default :
			socle_scu_show("unknow standbywifi configuration\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_PWRCFG); 
	
	return 0;
}

extern int 
socle_scu_stabdbywifi_get ()
{	
	u32 tmp;
	int type;
	
	tmp = socle_scu_read(SCU_PWRCFG)  & SCU_PWRCFG_STANDBYWIFI_M;
		
	switch(tmp){
		case SCU_PWRCFG_STANDBYWIFI_IGNORE :
			type = SOCLE_SCU_PWRCFG_STANDBYWIFI_IGNORE;
			break;
		case SCU_PWRCFG_STANDBYWIFI_IDLE :
			type = SOCLE_SCU_PWRCFG_STANDBYWIFI_IDLE;
			break;
		case SCU_PWRCFG_STANDBYWIFI_STOP:
			type = SOCLE_SCU_PWRCFG_STANDBYWIFI_STOP;
			break;
		default :
			socle_scu_show("unknow standbywifi configuration\n");
			return -1;
			break;
	}
	
	return type;
}

extern void 
socle_scu_rtc_alarm_int_wakeup_mask ()
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG) |SCU_PWRCFG_MASK_WAKE_SRC_RTC_MASK;
		
	socle_scu_write(tmp, SCU_PWRCFG); 
	
	return ;	
}

extern void 
socle_scu_rtc_alarm_int_wakeup_unmask ()
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG) & ~SCU_PWRCFG_MASK_WAKE_SRC_RTC_MASK;
		
	socle_scu_write(tmp, SCU_PWRCFG); 
	
	return ;	
}

extern int 
socle_scu_rtc_alarm_int_wakeup_mask_status ()		//return 1:masked  0:no mask
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG) & SCU_PWRCFG_MASK_WAKE_SRC_RTC_MASK;

	if(SCU_PWRCFG_MASK_WAKE_SRC_RTC_MASK == tmp)
		return 1;
	else
		return 0;
}

extern void 
socle_scu_gpio_inf_wakeup_mask ()
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG) |SCU_PWRCFG_MASK_WAKE_SRC_GPIO_MASK;
		
	socle_scu_write(tmp, SCU_PWRCFG); 
	
	return ;	
}

extern void 
socle_scu_gpio_inf_wakeup_unmask ()
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG) & ~SCU_PWRCFG_MASK_WAKE_SRC_GPIO_MASK;
		
	socle_scu_write(tmp, SCU_PWRCFG); 
	
	return ;	
}
	
extern int 
socle_scu_gpio_inf_wakeup_mask_status ()			//return 1:masked  0:no mask
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG) & SCU_PWRCFG_MASK_WAKE_SRC_GPIO_MASK;

	if(SCU_PWRCFG_MASK_WAKE_SRC_GPIO_MASK == tmp)
		return 1;
	else
		return 0;
}

extern int 
socle_scu_ext_int_wakeup_mask_set (int set)
{	
	u32 tmp;
	
	tmp = socle_scu_read(SCU_PWRCFG) & ~SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT_M;
		
	switch(set){
		case SOCLE_SCU_PWRCFG_MASK_NO :
			tmp = tmp | SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT_NO;
			break;
		case SOCLE_SCU_PWRCFG_MASK_EXT_INT0 :
			tmp = tmp | SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT0;
			break;
		case SOCLE_SCU_PWRCFG_MASK_EXT_INT1 :
			tmp = tmp | SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT1;
			break;
		case SOCLE_SCU_PWRCFG_MASK_EXT_INT01 :
			tmp = tmp | SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT01;
			break;
		default :
			socle_scu_show("unknow interrupt source\n");
			return -1;
			break;
	}
	socle_scu_write(tmp, SCU_PWRCFG); 
	
	return 0;
}

extern int 
socle_scu_ext_int_wakeup_mask_get ()
{	
	u32 tmp;
	int set;
	
	tmp = socle_scu_read(SCU_PWRCFG) & SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT_M;
		
	switch(tmp){
		case SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT_NO :
			set = SOCLE_SCU_PWRCFG_MASK_NO;
			break;
		case SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT0 :
			set = SOCLE_SCU_PWRCFG_MASK_EXT_INT0;
			break;
		case SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT1 :
			set = SOCLE_SCU_PWRCFG_MASK_EXT_INT1;
			break;
		case SCU_PWRCFG_MASK_WAKE_SRC_EXT_INT01 :
			set = SOCLE_SCU_PWRCFG_MASK_EXT_INT01;
			break;
		default :
			socle_scu_show("unknow interrupt source\n");
			return -1;
			break;
	}
	
	return set;
}



/*	SCU_PWREN	*/
	/*	the internal core power gating control	*/	
extern void 
socle_scu_embedded_sram_retent_enable ()
{
	u32 tmp;
	u32 tmp2;	

	tmp2 = socle_scu_read(SCU_PWREN);
	tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
	tmp = tmp | SCU_PWREN_EMB_SRAM1_EN;	
	
	socle_scu_write(tmp, SCU_PWREN); 

	return ;
}

EXPORT_SYMBOL(socle_scu_embedded_sram_retent_enable);

extern void 
socle_scu_embedded_sram_retent_disable ()
{
	u32 tmp;
	u32 tmp2;

        tmp2 = socle_scu_read(SCU_PWREN);
        tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
	tmp = tmp & ~SCU_PWREN_EMB_SRAM1_EN;
	
	socle_scu_write(tmp, SCU_PWREN); 

	return ;
}
		
extern int 
socle_scu_embedded_sram_retent_status ()			//return 1:enable  0:disable
{
	u32 tmp;
	u32 tmp2;

        tmp2 = socle_scu_read(SCU_PWREN);
        tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
	tmp = tmp & SCU_PWREN_EMB_SRAM1_EN;

	if(SCU_PWREN_EMB_SRAM1_EN == tmp)
		return 1;
	else
		return 0;
}

extern void 
socle_scu_embedded_sram_block1_reset ()
{
	u32 tmp;
	u32 tmp2;

        tmp2 = socle_scu_read(SCU_PWREN);
        tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
        tmp = tmp | SCU_PWREN_EMB_SRAM1_RESET;
	
	socle_scu_write(tmp, SCU_PWREN); 

	return ;
}

extern void 
socle_scu_embedded_sram_block0_reset ()
{
	u32 tmp;
	u32 tmp2;

        tmp2 = socle_scu_read(SCU_PWREN);
        tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
        tmp = tmp | SCU_PWREN_EMB_SRAM0_RESET;
	
	socle_scu_write(tmp, SCU_PWREN); 

	return ;
}

extern void 
socle_scu_uhc_block_reset ()
{
	u32 tmp;
	u32 tmp2;

        tmp2 = socle_scu_read(SCU_PWREN);
        tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
        tmp = tmp | SCU_PWREN_UHC_RESET;
	
	socle_scu_write(tmp, SCU_PWREN); 

	return ;
}

EXPORT_SYMBOL(socle_scu_uhc_block_reset);


extern void 
socle_scu_embedded_sram_core_power_enable ()
{
	u32 tmp;
	u32 tmp2;

        tmp2 = socle_scu_read(SCU_PWREN);
        tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
        tmp = tmp | SCU_PWREN_EMB_SRAM_CORR_PWR_EN;
	
	socle_scu_write(tmp, SCU_PWREN); 

	return ;
}

EXPORT_SYMBOL(socle_scu_embedded_sram_core_power_enable);

extern void 
socle_scu_embedded_sram_core_power_disable ()
{
	u32 tmp;
	u32 tmp2;

        tmp2 = socle_scu_read(SCU_PWREN);
        tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
        tmp = tmp & ~SCU_PWREN_EMB_SRAM_CORR_PWR_EN;
	
	socle_scu_write(tmp, SCU_PWREN); 

	return ;
}

EXPORT_SYMBOL(socle_scu_embedded_sram_core_power_disable);
		
extern int 
socle_scu_embedded_sram_core_power_status ()			//return 1:enable  0:disable
{
	u32 tmp;
	u32 tmp2;

        tmp2 = socle_scu_read(SCU_PWREN);
        tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
        tmp = tmp & SCU_PWREN_EMB_SRAM_CORR_PWR_EN;

	if(SCU_PWREN_EMB_SRAM_CORR_PWR_EN == tmp)
		return 1;
	else
		return 0;
}

extern void 
socle_scu_uhc_core_power_enable ()
{
	u32 tmp;
	u32 tmp2;

        tmp2 = socle_scu_read(SCU_PWREN);
        tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
        tmp = tmp | SCU_PWREN_UHC_CORR_PWR_EN;
	
	printk("SQ_scu_uhc_core_power_enable : tmp = %x, SCU_PWREN = %x\n", tmp, SCU_PWREN);
	
	socle_scu_write(tmp, SCU_PWREN); 

	return ;
}

EXPORT_SYMBOL(socle_scu_uhc_core_power_enable);
	
extern void 
socle_scu_uhc_core_power_disable ()
{
	u32 tmp;
	u32 tmp2;

        tmp2 = socle_scu_read(SCU_PWREN);
        tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
        tmp = tmp & ~SCU_PWREN_UHC_CORR_PWR_EN;
	
	printk("SQ_scu_uhc_core_power_disable : tmp = %x, SCU_PWREN = %x\n", tmp, SCU_PWREN);
	
	socle_scu_write(tmp, SCU_PWREN); 

	return ;
}

EXPORT_SYMBOL(socle_scu_uhc_core_power_disable);

extern int 
socle_scu_uhc_core_power_status ()			//return 1:enable  0:disable
{
	u32 tmp;
	u32 tmp2;

        tmp2 = socle_scu_read(SCU_PWREN);
        tmp = ((tmp2 & 0x38) << 2) | ((tmp2 & 0x4) << 1) | (tmp2 & 0x3);
        tmp = tmp & SCU_PWREN_UHC_CORR_PWR_EN;

	if(SCU_PWREN_UHC_CORR_PWR_EN == tmp)
		return 1;
	else
		return 0;
}


/*	SCU_RSTCNT	*/
	/*	reset status bits	*/
extern void 
socle_scu_rstcnt_pclk_counter_set (int cnt)		//input internal reset counter 
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_RSTCNT) & ~SCU_RSTCNT_PCLK_CNT_M;
	tmp = tmp | (cnt << SCU_RSTCNT_PCLK_CNT_S);

	socle_scu_write(tmp, SCU_RSTCNT); 

	return ;
}

extern int 
socle_scu_rstcnt_pclk_counter_get ()			//return input internal reset counter 
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_RSTCNT) & SCU_RSTCNT_PCLK_CNT_M;
	
	tmp = tmp >> SCU_RSTCNT_PCLK_CNT_S;

	return tmp;
}

extern void 
socle_scu_rstcnt_ext_counter_set (int cnt)			//input external power source reset counter 
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_RSTCNT) & ~SCU_RSTCNT_EXT_CNT_M;
	tmp = tmp | (cnt << SCU_RSTCNT_EXT_CNT_S);

	socle_scu_write(tmp, SCU_RSTCNT); 

	return ;
}

extern int 
socle_scu_rstcnt_ext_counter_get ()			//return external power source reset counter 
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_RSTCNT) & SCU_RSTCNT_EXT_CNT_M;
	
	tmp = tmp >> SCU_RSTCNT_EXT_CNT_S;

	return tmp;
}

/*	SCU_RSTSTAT	*/
	/*	the power mode control bit	*/
extern int 
socle_scu_rststat_sw_reset_status ()			//return 1:sw reset  0:other reset
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_RSTSTAT) & SCU_RSTSTAT_SW_RESET;

	if(SCU_RSTSTAT_SW_RESET == tmp)
		return 1;
	else
		return 0;
}

extern int 
socle_scu_rststat_wakeup_reset_status ()		//return 1:wakeup reset  0:other reset
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_RSTSTAT) & SCU_RSTSTAT_WAKEUP_RESET;

	if(SCU_RSTSTAT_WAKEUP_RESET == tmp)
		return 1;
	else
		return 0;
}

extern int 
socle_scu_rststat_wdt_reset_status ()			//return 1:watch-dog reset  0:other reset
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_RSTSTAT) & SCU_RSTSTAT_WDT_RESET;

	if(SCU_RSTSTAT_WDT_RESET == tmp)
		return 1;
	else
		return 0;
}

extern int 
socle_scu_rststat_hw_reset_status ()			//return 1:hw reset  0:other reset
{
	u32 tmp;
	
	tmp = socle_scu_read(SCU_RSTSTAT) & SCU_RSTSTAT_HW_RESET;

	if(SCU_RSTSTAT_HW_RESET == tmp)
		return 1;
	else
		return 0;
}


/*	SCU_WKUPSTAT	*/
	/*	the trigger source for exiting SLEEP mode	*/
extern int 
socle_scu_wkupstat_status ()
{	
	u32 tmp;
	int status;

	status = 0;
	
	tmp = socle_scu_read(SCU_WKUPSTAT);

	if((tmp & SCU_WKUPSTAT_RTC) == SCU_WKUPSTAT_RTC){
		SCUDBUG("sleep mode wake up from RTC\n");
		 status |= SOCLE_SCU_WKUPSTAT_RTC;
	}
	if((tmp & SCU_WKUPSTAT_GPIO) == SCU_WKUPSTAT_GPIO){
		SCUDBUG("sleep mode wake up from GPIO\n");
		 status |= SOCLE_SCU_WKUPSTAT_GPIO;
	}

	if((tmp & SCU_WKUPSTAT_EXT_INT) == SCU_WKUPSTAT_EXT_INT){
		SCUDBUG("sleep mode wake up from external interrupt\n");
		 status |= SOCLE_SCU_WKUPSTAT_EXT_INT;
	}

	if(status == 0)
		SCUDBUG("NO sleep mode wake up source\n");		

	return status;
}

EXPORT_SYMBOL(socle_scu_wkupstat_status);

extern int 
socle_scu_wkupstat_clear (int src)
{	
	u32 tmp;

	switch(src){
		case SOCLE_SCU_WKUPSTAT_RTC :
			tmp = SCU_WKUPSTAT_RTC;
			break;
		case SOCLE_SCU_WKUPSTAT_GPIO:
			tmp = SCU_WKUPSTAT_GPIO;
			break;
		case SOCLE_SCU_WKUPSTAT_EXT_INT:
			tmp = SOCLE_SCU_WKUPSTAT_EXT_INT;
			break;
		default :
			socle_scu_show("unknow wake-up source\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_WKUPSTAT);
			
	return 0;
}		

/*	SCU_MEMISLP		*/
	/*	the output pin value of ST/SDR memory interface in SLEEP mode	*/
extern int 
socle_scu_memislp_ouput0_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_MEMISLP);

	switch(pin){
		case SOCLE_SCU_MEMISLP_ST_WEN :
			tmp = (tmp & ~SCU_MEMISLP_ST_WEN_M) | SCU_MEMISLP_ST_WEN_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_ST_OEN:
			tmp = (tmp & ~SCU_MEMISLP_ST_OEN_M) | SCU_MEMISLP_ST_OEN_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_ST_CSN:
			tmp = (tmp & ~SCU_MEMISLP_ST_CSN_M) | SCU_MEMISLP_ST_CSN_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_SD_CLKOUT :
			tmp = (tmp & ~SCU_MEMISLP_SD_CLKOUT_M) | SCU_MEMISLP_SD_CLKOUT_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_SD_CKE:
			tmp = (tmp & ~SCU_MEMISLP_SD_CKE_M) | SCU_MEMISLP_SD_CKE_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_SD_BA:
			tmp = (tmp & ~SCU_MEMISLP_SD_BA_M) | SCU_MEMISLP_SD_BA_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_SD_DQM :
			tmp = (tmp & ~SCU_MEMISLP_SD_DQM_M) | SCU_MEMISLP_SD_DQM_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_SD_WEN:
			tmp = (tmp & ~SCU_MEMISLP_SD_WEN_M) | SCU_MEMISLP_SD_WEN_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_SD_RASN:
			tmp = (tmp & ~SCU_MEMISLP_SD_RASN_M) | SCU_MEMISLP_SD_RASN_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_SD_CASN :
			tmp = (tmp & ~SCU_MEMISLP_SD_CASN_M) | SCU_MEMISLP_SD_CASN_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_SD_CEN:
			tmp = (tmp & ~SCU_MEMISLP_SD_CEN_M) | SCU_MEMISLP_SD_CEN_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_SD_DQ:
			tmp = (tmp & ~SCU_MEMISLP_SD_DQ_M) | SCU_MEMISLP_SD_DQ_OUTPUT0;
			break;
		case SOCLE_SCU_MEMISLP_SD_ADDR:
			tmp = (tmp & ~SCU_MEMISLP_SD_ADDR_M) | SCU_MEMISLP_SD_ADDR_OUTPUT0;
			break;
		default :
			socle_scu_show("unknow MEMISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_MEMISLP);
			
	return 0;
}

extern int 
socle_scu_memislp_ouput1_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_MEMISLP);

	switch(pin){
		case SOCLE_SCU_MEMISLP_ST_WEN :
			tmp = (tmp & ~SCU_MEMISLP_ST_WEN_M) | SCU_MEMISLP_ST_WEN_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_ST_OEN:
			tmp = (tmp & ~SCU_MEMISLP_ST_OEN_M) | SCU_MEMISLP_ST_OEN_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_ST_CSN:
			tmp = (tmp & ~SCU_MEMISLP_ST_CSN_M) | SCU_MEMISLP_ST_CSN_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_SD_CLKOUT :
			tmp = (tmp & ~SCU_MEMISLP_SD_CLKOUT_M) | SCU_MEMISLP_SD_CLKOUT_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_SD_CKE:
			tmp = (tmp & ~SCU_MEMISLP_SD_CKE_M) | SCU_MEMISLP_SD_CKE_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_SD_BA:
			tmp = (tmp & ~SCU_MEMISLP_SD_BA_M) | SCU_MEMISLP_SD_BA_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_SD_DQM :
			tmp = (tmp & ~SCU_MEMISLP_SD_DQM_M) | SCU_MEMISLP_SD_DQM_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_SD_WEN:
			tmp = (tmp & ~SCU_MEMISLP_SD_WEN_M) | SCU_MEMISLP_SD_WEN_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_SD_RASN:
			tmp = (tmp & ~SCU_MEMISLP_SD_RASN_M) | SCU_MEMISLP_SD_RASN_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_SD_CASN :
			tmp = (tmp & ~SCU_MEMISLP_SD_CASN_M) | SCU_MEMISLP_SD_CASN_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_SD_CEN:
			tmp = (tmp & ~SCU_MEMISLP_SD_CEN_M) | SCU_MEMISLP_SD_CEN_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_SD_DQ:
			tmp = (tmp & ~SCU_MEMISLP_SD_DQ_M) | SCU_MEMISLP_SD_DQ_OUTPUT1;
			break;
		case SOCLE_SCU_MEMISLP_SD_ADDR:
			tmp = (tmp & ~SCU_MEMISLP_SD_ADDR_M) | SCU_MEMISLP_SD_ADDR_OUTPUT1;
			break;
		default :
			socle_scu_show("unknow MEMISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_MEMISLP);
			
	return 0;
}

extern int 
socle_scu_memislp_ouput_dis_input_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_MEMISLP);

	switch(pin){
		case SOCLE_SCU_MEMISLP_ST_WEN :
			tmp = (tmp & ~SCU_MEMISLP_ST_WEN_M) | SCU_MEMISLP_ST_WEN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_MEMISLP_ST_OEN:
			tmp = (tmp & ~SCU_MEMISLP_ST_OEN_M) | SCU_MEMISLP_ST_OEN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_MEMISLP_ST_CSN:
			tmp = (tmp & ~SCU_MEMISLP_ST_CSN_M) | SCU_MEMISLP_ST_CSN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_MEMISLP_SD_CLKOUT :
			tmp = (tmp & ~SCU_MEMISLP_SD_CLKOUT_M) | SCU_MEMISLP_SD_CLKOUT_OUTPUT_DIS;
			break;
		case SOCLE_SCU_MEMISLP_SD_CKE:
			tmp = (tmp & ~SCU_MEMISLP_SD_CKE_M) | SCU_MEMISLP_SD_CKE_OUTPUT_DIS;
			break;
		case SOCLE_SCU_MEMISLP_SD_BA:
			tmp = (tmp & ~SCU_MEMISLP_SD_BA_M) | SCU_MEMISLP_SD_BA_OUTPUT_DIS;
			break;
		case SOCLE_SCU_MEMISLP_SD_DQM :
			tmp = (tmp & ~SCU_MEMISLP_SD_DQM_M) | SCU_MEMISLP_SD_DQM_OUTPUT_DIS;
			break;
		case SOCLE_SCU_MEMISLP_SD_WEN:
			tmp = (tmp & ~SCU_MEMISLP_SD_WEN_M) | SCU_MEMISLP_SD_WEN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_MEMISLP_SD_RASN:
			tmp = (tmp & ~SCU_MEMISLP_SD_RASN_M) | SCU_MEMISLP_SD_RASN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_MEMISLP_SD_CASN :
			tmp = (tmp & ~SCU_MEMISLP_SD_CASN_M) | SCU_MEMISLP_SD_CASN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_MEMISLP_SD_CEN:
			tmp = (tmp & ~SCU_MEMISLP_SD_CEN_M) | SCU_MEMISLP_SD_CEN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_MEMISLP_SD_DQ:
			tmp = (tmp & ~SCU_MEMISLP_SD_DQ_M) | SCU_MEMISLP_SD_DQ_INPUT;
			break;
		case SOCLE_SCU_MEMISLP_SD_ADDR:
			tmp = (tmp & ~SCU_MEMISLP_SD_ADDR_M) | SCU_MEMISLP_SD_ADDR_OUTPUT_DIS;
			break;
		default :
			socle_scu_show("unknow MEMISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_MEMISLP);
			
	return 0;
}

extern int 
socle_scu_memislp_status (int pin)					//return 0:output0  1:output1  2:output disable or input
{
	u32 tmp;

	tmp = socle_scu_read(SCU_MEMISLP);

	switch(pin){
		case SOCLE_SCU_MEMISLP_ST_WEN :
			tmp = (tmp & SCU_MEMISLP_ST_WEN_M) >> SCU_MEMISLP_ST_WEN_S;
			break;
		case SOCLE_SCU_MEMISLP_ST_OEN:
			tmp = (tmp & SCU_MEMISLP_ST_OEN_M) >> SCU_MEMISLP_ST_OEN_S;
			break;
		case SOCLE_SCU_MEMISLP_ST_CSN:
			tmp = (tmp & SCU_MEMISLP_ST_OEN_M) >> SCU_MEMISLP_ST_OEN_S;
			break;
		case SOCLE_SCU_MEMISLP_SD_CLKOUT :
			tmp = (tmp & SCU_MEMISLP_SD_CLKOUT_M) >> SCU_MEMISLP_SD_CLKOUT_S;
			break;
		case SOCLE_SCU_MEMISLP_SD_CKE:
			tmp = (tmp & SCU_MEMISLP_SD_CKE_M) >> SCU_MEMISLP_SD_CKE_S;
			break;
		case SOCLE_SCU_MEMISLP_SD_BA:
			tmp = (tmp & SCU_MEMISLP_SD_BA_M) >> SCU_MEMISLP_SD_BA_S;
			break;
		case SOCLE_SCU_MEMISLP_SD_DQM :
			tmp = (tmp & SCU_MEMISLP_SD_DQM_M) >> SCU_MEMISLP_SD_DQM_S;
			break;
		case SOCLE_SCU_MEMISLP_SD_WEN:
			tmp = (tmp & SCU_MEMISLP_SD_WEN_M) >> SCU_MEMISLP_SD_WEN_S;
			break;
		case SOCLE_SCU_MEMISLP_SD_RASN:
			tmp = (tmp & SCU_MEMISLP_SD_RASN_M) >> SCU_MEMISLP_SD_RASN_S;
			break;
		case SOCLE_SCU_MEMISLP_SD_CASN :
			tmp = (tmp & SCU_MEMISLP_SD_CASN_M) >> SCU_MEMISLP_SD_CASN_S;
			break;
		case SOCLE_SCU_MEMISLP_SD_CEN:
			tmp = (tmp & SCU_MEMISLP_SD_CEN_M) >> SCU_MEMISLP_SD_CEN_S;
			break;
		case SOCLE_SCU_MEMISLP_SD_DQ:
			tmp = (tmp & SCU_MEMISLP_SD_DQ_M) >> SCU_MEMISLP_SD_DQ_S;
			break;
		case SOCLE_SCU_MEMISLP_SD_ADDR:
			tmp = (tmp & SCU_MEMISLP_SD_ADDR_M) >> SCU_MEMISLP_SD_ADDR_S;
			break;
		default :
			socle_scu_show("unknow MEMISLP pin\n");
			return -1;
			break;			
	}
	if(tmp > 2)
		return -1;
			
	return tmp;
}


/*	SCU_NFISLP	*/
	/*	the output pin value of NAND-Flash memory interface in SLEEP mode	*/
extern int 
socle_scu_nfislp_ouput0_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_NFISLP);

	switch(pin){
		case SOCLE_SCU_NFISLP_NF_IO :
			tmp = (tmp & ~SCU_NFISLP_NF_IO_M) | SCU_NFISLP_NF_IO_OUTPUT0;
			break;
		case SOCLE_SCU_NFISLP_NF_CLE:
			tmp = (tmp & ~SCU_NFISLP_NF_CLE_M) | SCU_NFISLP_NF_CLE_OUTPUT0;
			break;
		case SOCLE_SCU_NFISLP_NF_ALE:
			tmp = (tmp & ~SCU_NFISLP_NF_ALE_M) | SCU_NFISLP_NF_ALE_OUTPUT0;
			break;
		case SOCLE_SCU_NFISLP_NF_CEN :
			tmp = (tmp & ~SCU_NFISLP_NF_CEN_M) | SCU_NFISLP_NF_CEN_OUTPUT0;
			break;
		case SOCLE_SCU_NFISLP_NF_REN:
			tmp = (tmp & ~SCU_NFISLP_NF_REN_M) | SCU_NFISLP_NF_REN_OUTPUT0;
			break;
		case SOCLE_SCU_NFISLP_NF_WEN:
			tmp = (tmp & ~SCU_NFISLP_NF_WEN_M) | SCU_NFISLP_NF_WEN_OUTPUT0;
			break;
		case SOCLE_SCU_NFISLP_NF_WPN :
			tmp = (tmp & ~SCU_NFISLP_NF_WPN_M) | SCU_NFISLP_NF_WPN_OUTPUT0;
			break;
		default :
			socle_scu_show("unknow NFISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_NFISLP);
			
	return 0;
}

extern int 
socle_scu_nfislp_ouput1_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_NFISLP);

	switch(pin){
		case SOCLE_SCU_NFISLP_NF_IO :
			tmp = (tmp & ~SCU_NFISLP_NF_IO_M) | SCU_NFISLP_NF_IO_OUTPUT1;
			break;
		case SOCLE_SCU_NFISLP_NF_CLE:
			tmp = (tmp & ~SCU_NFISLP_NF_CLE_M) | SCU_NFISLP_NF_CLE_OUTPUT1;
			break;
		case SOCLE_SCU_NFISLP_NF_ALE:
			tmp = (tmp & ~SCU_NFISLP_NF_ALE_M) | SCU_NFISLP_NF_ALE_OUTPUT1;
			break;
		case SOCLE_SCU_NFISLP_NF_CEN :
			tmp = (tmp & ~SCU_NFISLP_NF_CEN_M) | SCU_NFISLP_NF_CEN_OUTPUT1;
			break;
		case SOCLE_SCU_NFISLP_NF_REN:
			tmp = (tmp & ~SCU_NFISLP_NF_REN_M) | SCU_NFISLP_NF_REN_OUTPUT1;
			break;
		case SOCLE_SCU_NFISLP_NF_WEN:
			tmp = (tmp & ~SCU_NFISLP_NF_WEN_M) | SCU_NFISLP_NF_WEN_OUTPUT1;
			break;
		case SOCLE_SCU_NFISLP_NF_WPN :
			tmp = (tmp & ~SCU_NFISLP_NF_WPN_M) | SCU_NFISLP_NF_WPN_OUTPUT1;
			break;
		default :
			socle_scu_show("unknow NFISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_NFISLP);
			
	return 0;
}

extern int 
socle_scu_nfislp_ouput_dis_input_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_NFISLP);

	switch(pin){
		case SOCLE_SCU_NFISLP_NF_IO :
			tmp = (tmp & ~SCU_NFISLP_NF_IO_M) | SCU_NFISLP_NF_IO_OUTPUT_DIS;
			break;
		case SOCLE_SCU_NFISLP_NF_CLE:
			tmp = (tmp & ~SCU_NFISLP_NF_CLE_M) | SCU_NFISLP_NF_CLE_OUTPUT_DIS;
			break;
		case SOCLE_SCU_NFISLP_NF_ALE:
			tmp = (tmp & ~SCU_NFISLP_NF_ALE_M) | SCU_NFISLP_NF_ALE_OUTPUT_DIS;
			break;
		case SOCLE_SCU_NFISLP_NF_CEN :
			tmp = (tmp & ~SCU_NFISLP_NF_CEN_M) | SCU_NFISLP_NF_CEN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_NFISLP_NF_REN:
			tmp = (tmp & ~SCU_NFISLP_NF_REN_M) | SCU_NFISLP_NF_REN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_NFISLP_NF_WEN:
			tmp = (tmp & ~SCU_NFISLP_NF_WEN_M) | SCU_NFISLP_NF_WEN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_NFISLP_NF_WPN :
			tmp = (tmp & ~SCU_NFISLP_NF_WPN_M) | SCU_NFISLP_NF_WPN_OUTPUT_DIS;
			break;
		default :
			socle_scu_show("unknow NFISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_NFISLP);
			
	return 0;
}

extern int 
socle_scu_nfislp_status (int pin)						//return 0:output0  1:output1  2:output disable or input
{
	u32 tmp;

	tmp = socle_scu_read(SCU_NFISLP);

	switch(pin){
		case SOCLE_SCU_NFISLP_NF_IO :
			tmp = (tmp & SCU_NFISLP_NF_IO_M) >> SCU_NFISLP_NF_IO_S;
			break;
		case SOCLE_SCU_NFISLP_NF_CLE:
			tmp = (tmp & SCU_NFISLP_NF_CLE_M) >> SCU_NFISLP_NF_CLE_S;
			break;
		case SOCLE_SCU_NFISLP_NF_ALE:
			tmp = (tmp & SCU_NFISLP_NF_ALE_M) >> SCU_NFISLP_NF_ALE_S;
			break;
		case SOCLE_SCU_NFISLP_NF_CEN :
			tmp = (tmp & SCU_NFISLP_NF_CEN_M) >> SCU_NFISLP_NF_CEN_S;
			break;
		case SOCLE_SCU_NFISLP_NF_REN:
			tmp = (tmp & SCU_NFISLP_NF_REN_M) >> SCU_NFISLP_NF_REN_S;
			break;
		case SOCLE_SCU_NFISLP_NF_WEN:
			tmp = (tmp & SCU_NFISLP_NF_WEN_M) >> SCU_NFISLP_NF_WEN_S;
			break;
		case SOCLE_SCU_NFISLP_NF_WPN :
			tmp = (tmp & SCU_NFISLP_NF_WPN_M) >> SCU_NFISLP_NF_WPN_S;
			break;
		default :
			socle_scu_show("unknow NFISLP pin\n");
			return -1;
			break;			
	}
	if(tmp > 2)
		return -1;
			
	return tmp;
}

/*	SCU_USBISLP	*/
	/*	the output pin value of USB interface in SLEEP mode	*/
extern int 
socle_scu_usbislp_ouput0_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_USBISLP);

	switch(pin){
		case SOCLE_SCU_USBISLP_VBUS_UHC :
			tmp = (tmp & ~SCU_USBISLP_VBUS_UHC_M) | SCU_USBISLP_VBUS_UHC_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_SUSPENDM:
			tmp = (tmp & ~SCU_USBISLP_USB_SUSPENDM_M) | SCU_USBISLP_USB_SUSPENDM_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_POR:
			tmp = (tmp & ~SCU_USBISLP_USB_POR_M) | SCU_USBISLP_USB_POR_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_XCVR_OWN :
			tmp = (tmp & ~SCU_USBISLP_USB_FS_XCVR_OWN_M) | SCU_USBISLP_USB_FS_XCVR_OWN_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_TX_ENB:
			tmp = (tmp & ~SCU_USBISLP_USB_TX_ENB_M) | SCU_USBISLP_USB_TX_ENB_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_SE0_EXT:
			tmp = (tmp & ~SCU_USBISLP_USB_FS_SE0_EXT_M) | SCU_USBISLP_USB_FS_SE0_EXT_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_DATA_EXT :
			tmp = (tmp & ~SCU_USBISLP_USB_FS_DATA_EXT_M) | SCU_USBISLP_USB_FS_DATA_EXT_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_TX_VALIDH :
			tmp = (tmp & ~SCU_USBISLP_USB_TX_VALIDH_M) | SCU_USBISLP_USB_TX_VALIDH_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_TX_VALID:
			tmp = (tmp & ~SCU_USBISLP_USB_TX_VALID_M) | SCU_USBISLP_USB_TX_VALID_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_DATAI:
			tmp = (tmp & ~SCU_USBISLP_USB_DATAI_M) | SCU_USBISLP_USB_DATAI_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_DM_PULLDOWN:
			tmp = (tmp & ~SCU_USBISLP_USB_DM_PULLDOWN_M) | SCU_USBISLP_USB_DM_PULLDOWN_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_DP_PULLDOWN:
			tmp = (tmp & ~SCU_USBISLP_USB_DP_PULLDOWN_M) | SCU_USBISLP_USB_DP_PULLDOWN_OUTPUT0;
			break;			
		case SOCLE_SCU_USBISLP_USB_XCVR_SEL:
			tmp = (tmp & ~SCU_USBISLP_USB_XCVR_SEL_M) | SCU_USBISLP_USB_XCVR_SEL_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_TERM_SEL:
			tmp = (tmp & ~SCU_USBISLP_USB_TERM_SEL_M) | SCU_USBISLP_USB_TERM_SEL_OUTPUT0;
			break;
		case SOCLE_SCU_USBISLP_USB_OPMODE :
			tmp = (tmp & ~SCU_USBISLP_USB_OPMODE_M) | SCU_USBISLP_USB_OPMODE_OUTPUT0;
			break;
		default :
			socle_scu_show("unknow USBISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_USBISLP);
			
	return 0;
}

extern int 
socle_scu_usbislp_ouput1_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_USBISLP);

	switch(pin){
		case SOCLE_SCU_USBISLP_VBUS_UHC :
			tmp = (tmp & ~SCU_USBISLP_VBUS_UHC_M) | SCU_USBISLP_VBUS_UHC_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_SUSPENDM:
			tmp = (tmp & ~SCU_USBISLP_USB_SUSPENDM_M) | SCU_USBISLP_USB_SUSPENDM_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_POR:
			tmp = (tmp & ~SCU_USBISLP_USB_POR_M) | SCU_USBISLP_USB_POR_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_XCVR_OWN :
			tmp = (tmp & ~SCU_USBISLP_USB_FS_XCVR_OWN_M) | SCU_USBISLP_USB_FS_XCVR_OWN_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_TX_ENB:
			tmp = (tmp & ~SCU_USBISLP_USB_TX_ENB_M) | SCU_USBISLP_USB_TX_ENB_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_SE0_EXT:
			tmp = (tmp & ~SCU_USBISLP_USB_FS_SE0_EXT_M) | SCU_USBISLP_USB_FS_SE0_EXT_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_DATA_EXT :
			tmp = (tmp & ~SCU_USBISLP_USB_FS_DATA_EXT_M) | SCU_USBISLP_USB_FS_DATA_EXT_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_TX_VALIDH :
			tmp = (tmp & ~SCU_USBISLP_USB_TX_VALIDH_M) | SCU_USBISLP_USB_TX_VALIDH_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_TX_VALID:
			tmp = (tmp & ~SCU_USBISLP_USB_TX_VALID_M) | SCU_USBISLP_USB_TX_VALID_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_DATAI:
			tmp = (tmp & ~SCU_USBISLP_USB_DATAI_M) | SCU_USBISLP_USB_DATAI_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_DM_PULLDOWN:
			tmp = (tmp & ~SCU_USBISLP_USB_DM_PULLDOWN_M) | SCU_USBISLP_USB_DM_PULLDOWN_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_DP_PULLDOWN:
			tmp = (tmp & ~SCU_USBISLP_USB_DP_PULLDOWN_M) | SCU_USBISLP_USB_DP_PULLDOWN_OUTPUT1;
			break;			
		case SOCLE_SCU_USBISLP_USB_XCVR_SEL:
			tmp = (tmp & ~SCU_USBISLP_USB_XCVR_SEL_M) | SCU_USBISLP_USB_XCVR_SEL_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_TERM_SEL:
			tmp = (tmp & ~SCU_USBISLP_USB_TERM_SEL_M) | SCU_USBISLP_USB_TERM_SEL_OUTPUT1;
			break;
		case SOCLE_SCU_USBISLP_USB_OPMODE :
			tmp = (tmp & ~SCU_USBISLP_USB_OPMODE_M) | SCU_USBISLP_USB_OPMODE_OUTPUT1;
			break;
		default :
			socle_scu_show("unknow USBISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_USBISLP);
			
	return 0;
}

extern int 
socle_scu_usbislp_ouput_dis_input_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_USBISLP);

	switch(pin){
		case SOCLE_SCU_USBISLP_VBUS_UHC :
			tmp = (tmp & ~SCU_USBISLP_VBUS_UHC_M) | SCU_USBISLP_VBUS_UHC_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_SUSPENDM:
			tmp = (tmp & ~SCU_USBISLP_USB_SUSPENDM_M) | SCU_USBISLP_USB_SUSPENDM_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_POR:
			tmp = (tmp & ~SCU_USBISLP_USB_POR_M) | SCU_USBISLP_USB_POR_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_XCVR_OWN :
			tmp = (tmp & ~SCU_USBISLP_USB_FS_XCVR_OWN_M) | SCU_USBISLP_USB_FS_XCVR_OWN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_TX_ENB:
			tmp = (tmp & ~SCU_USBISLP_USB_TX_ENB_M) | SCU_USBISLP_USB_TX_ENB_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_SE0_EXT:
			tmp = (tmp & ~SCU_USBISLP_USB_FS_SE0_EXT_M) | SCU_USBISLP_USB_FS_SE0_EXT_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_DATA_EXT :
			tmp = (tmp & ~SCU_USBISLP_USB_FS_DATA_EXT_M) | SCU_USBISLP_USB_FS_DATA_EXT_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_TX_VALIDH :
			tmp = (tmp & ~SCU_USBISLP_USB_TX_VALIDH_M) | SCU_USBISLP_USB_TX_VALIDH_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_TX_VALID:
			tmp = (tmp & ~SCU_USBISLP_USB_TX_VALID_M) | SCU_USBISLP_USB_TX_VALID_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_DATAI:
			tmp = (tmp & ~SCU_USBISLP_USB_DATAI_M) | SCU_USBISLP_USB_DATAI_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_DM_PULLDOWN:
			tmp = (tmp & ~SCU_USBISLP_USB_DM_PULLDOWN_M) | SCU_USBISLP_USB_DM_PULLDOWN_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_DP_PULLDOWN:
			tmp = (tmp & ~SCU_USBISLP_USB_DP_PULLDOWN_M) | SCU_USBISLP_USB_DP_PULLDOWN_OUTPUT_DIS;
			break;			
		case SOCLE_SCU_USBISLP_USB_XCVR_SEL:
			tmp = (tmp & ~SCU_USBISLP_USB_XCVR_SEL_M) | SCU_USBISLP_USB_XCVR_SEL_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_TERM_SEL:
			tmp = (tmp & ~SCU_USBISLP_USB_TERM_SEL_M) | SCU_USBISLP_USB_TERM_SEL_OUTPUT_DIS;
			break;
		case SOCLE_SCU_USBISLP_USB_OPMODE :
			tmp = (tmp & ~SCU_USBISLP_USB_OPMODE_M) | SCU_USBISLP_USB_OPMODE_OUTPUT_DIS;
			break;
		default :
			socle_scu_show("unknow USBISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_USBISLP);
			
	return 0;
}

					
extern int 
socle_scu_usbislp_status (int pin)						//return 0:output0  1:output1  2:output disable or input
{
	u32 tmp;

	tmp = socle_scu_read(SCU_USBISLP);

	switch(pin){
		case SOCLE_SCU_USBISLP_VBUS_UHC :
			tmp = (tmp & SCU_USBISLP_VBUS_UHC_M) >> SCU_USBISLP_VBUS_UHC_S;
			break;
		case SOCLE_SCU_USBISLP_USB_SUSPENDM:
			tmp = (tmp & SCU_USBISLP_USB_SUSPENDM_M) >> SCU_USBISLP_USB_SUSPENDM_S;
			break;
		case SOCLE_SCU_USBISLP_USB_POR:
			tmp = (tmp & SCU_USBISLP_USB_POR_M) >> SCU_USBISLP_USB_POR_S;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_XCVR_OWN :
			tmp = (tmp & SCU_USBISLP_USB_FS_XCVR_OWN_M) >> SCU_USBISLP_USB_FS_XCVR_OWN_S;
			break;			
		case SOCLE_SCU_USBISLP_USB_TX_ENB:
			tmp = (tmp & SCU_USBISLP_USB_TX_ENB_M) >> SCU_USBISLP_USB_TX_ENB_S;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_SE0_EXT:
			tmp = (tmp & SCU_USBISLP_USB_FS_SE0_EXT_M) >> SCU_USBISLP_USB_FS_SE0_EXT_S;
			break;
		case SOCLE_SCU_USBISLP_USB_FS_DATA_EXT :
			tmp = (tmp & SCU_USBISLP_USB_FS_DATA_EXT_M) >> SCU_USBISLP_USB_FS_DATA_EXT_S;
			break;			
		case SOCLE_SCU_USBISLP_USB_TX_VALIDH :
			tmp = (tmp & SCU_USBISLP_USB_TX_VALIDH_M) >> SCU_USBISLP_USB_TX_VALIDH_S;
			break;
		case SOCLE_SCU_USBISLP_USB_TX_VALID:
			tmp = (tmp & SCU_USBISLP_USB_TX_VALID_M) >> SCU_USBISLP_USB_TX_VALID_S;
			break;
		case SOCLE_SCU_USBISLP_USB_DATAI :
			tmp = (tmp & SCU_USBISLP_USB_DATAI_M) >> SCU_USBISLP_USB_DATAI_S;
			break;
		case SOCLE_SCU_USBISLP_USB_DM_PULLDOWN:
			tmp = (tmp & SCU_USBISLP_USB_DM_PULLDOWN_M) >> SCU_USBISLP_USB_DM_PULLDOWN_S;
			break;			
		case SOCLE_SCU_USBISLP_USB_DP_PULLDOWN:
			tmp = (tmp & SCU_USBISLP_USB_DP_PULLDOWN_M) >> SCU_USBISLP_USB_DP_PULLDOWN_S;
			break;
		case SOCLE_SCU_USBISLP_USB_XCVR_SEL :
			tmp = (tmp & SCU_USBISLP_USB_XCVR_SEL_M) >> SCU_USBISLP_USB_XCVR_SEL_S;
			break;			
		case SOCLE_SCU_USBISLP_USB_TERM_SEL:
			tmp = (tmp & SCU_USBISLP_USB_TERM_SEL_M) >> SCU_USBISLP_USB_TERM_SEL_S;
			break;
		case SOCLE_SCU_USBISLP_USB_OPMODE :
			tmp = (tmp & SCU_USBISLP_USB_OPMODE_M) >> SCU_USBISLP_USB_OPMODE_S;
			break;
		default :
			socle_scu_show("unknow USBISLP pin\n");
			return -1;
			break;			
	}
	if(tmp > 2)
		return -1;
			
	return tmp;
}




/*	SCU_LCDISLP	*/
	/*	the output pin value of LCD interface in SLEEP mode	*/
extern int 
socle_scu_lcdislp_ouput0_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_LCDISLP);

	switch(pin){
		case SOCLE_SCU_LCDISLP_LCD_POWER :
			tmp = (tmp & ~SCU_LCDISLP_LCD_POWER_M) | SCU_LCDISLP_LCD_POWER_OUTPUT0;
			break;
		case SOCLE_SCU_LCDISLP_LCD_LP:
			tmp = (tmp & ~SCU_LCDISLP_LCD_LP_M) | SCU_LCDISLP_LCD_LP_OUTPUT0;
			break;
		case SOCLE_SCU_LCDISLP_LCD_LE:
			tmp = (tmp & ~SCU_LCDISLP_LCD_LE_M) | SCU_LCDISLP_LCD_LE_OUTPUT0;
			break;
		case SOCLE_SCU_LCDISLP_LCD_FP :
			tmp = (tmp & ~SCU_LCDISLP_LCD_FP_M) | SCU_LCDISLP_LCD_FP_OUTPUT0;
			break;
		case SOCLE_SCU_LCDISLP_LCD_DATA:
			tmp = (tmp & ~SCU_LCDISLP_LCD_DATA_M) | SCU_LCDISLP_LCD_DATA_OUTPUT0;
			break;
		case SOCLE_SCU_LCDISLP_LCD_CP:
			tmp = (tmp & ~SCU_LCDISLP_LCD_CP_M) | SCU_LCDISLP_LCD_CP_OUTPUT0;
			break;
		case SOCLE_SCU_LCDISLP_LCD_AC :
			tmp = (tmp & ~SCU_LCDISLP_LCD_AC_M) | SCU_LCDISLP_LCD_AC_OUTPUT0;
			break;
		default :
			socle_scu_show("unknow LCDISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_LCDISLP);
			
	return 0;
}

extern int 
socle_scu_lcdislp_ouput1_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_LCDISLP);

	switch(pin){
		case SOCLE_SCU_LCDISLP_LCD_POWER :
			tmp = (tmp & ~SCU_LCDISLP_LCD_POWER_M) | SCU_LCDISLP_LCD_POWER_OUTPUT1;
			break;
		case SOCLE_SCU_LCDISLP_LCD_LP:
			tmp = (tmp & ~SCU_LCDISLP_LCD_LP_M) | SCU_LCDISLP_LCD_LP_OUTPUT1;
			break;
		case SOCLE_SCU_LCDISLP_LCD_LE:
			tmp = (tmp & ~SCU_LCDISLP_LCD_LE_M) | SCU_LCDISLP_LCD_LE_OUTPUT1;
			break;
		case SOCLE_SCU_LCDISLP_LCD_FP :
			tmp = (tmp & ~SCU_LCDISLP_LCD_FP_M) | SCU_LCDISLP_LCD_FP_OUTPUT1;
			break;
		case SOCLE_SCU_LCDISLP_LCD_DATA:
			tmp = (tmp & ~SCU_LCDISLP_LCD_DATA_M) | SCU_LCDISLP_LCD_DATA_OUTPUT1;
			break;
		case SOCLE_SCU_LCDISLP_LCD_CP:
			tmp = (tmp & ~SCU_LCDISLP_LCD_CP_M) | SCU_LCDISLP_LCD_CP_OUTPUT1;
			break;
		case SOCLE_SCU_LCDISLP_LCD_AC :
			tmp = (tmp & ~SCU_LCDISLP_LCD_AC_M) | SCU_LCDISLP_LCD_AC_OUTPUT1;
			break;
		default :
			socle_scu_show("unknow LCDISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_LCDISLP);
			
	return 0;
}

extern int 
socle_scu_lcdislp_ouput_dis_input_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_LCDISLP);

	switch(pin){
		case SOCLE_SCU_LCDISLP_LCD_POWER :
			tmp = (tmp & ~SCU_LCDISLP_LCD_POWER_M) | SCU_LCDISLP_LCD_POWER_OUTPUT_DIS;
			break;
		case SOCLE_SCU_LCDISLP_LCD_LP:
			tmp = (tmp & ~SCU_LCDISLP_LCD_LP_M) | SCU_LCDISLP_LCD_LP_OUTPUT_DIS;
			break;
		case SOCLE_SCU_LCDISLP_LCD_LE:
			tmp = (tmp & ~SCU_LCDISLP_LCD_LE_M) | SCU_LCDISLP_LCD_LE_OUTPUT_DIS;
			break;
		case SOCLE_SCU_LCDISLP_LCD_FP :
			tmp = (tmp & ~SCU_LCDISLP_LCD_FP_M) | SCU_LCDISLP_LCD_FP_OUTPUT_DIS;
			break;
		case SOCLE_SCU_LCDISLP_LCD_DATA:
			tmp = (tmp & ~SCU_LCDISLP_LCD_DATA_M) | SCU_LCDISLP_LCD_DATA_OUTPUT_DIS;
			break;
		case SOCLE_SCU_LCDISLP_LCD_CP:
			tmp = (tmp & ~SCU_LCDISLP_LCD_CP_M) | SCU_LCDISLP_LCD_CP_OUTPUT_DIS;
			break;
		case SOCLE_SCU_LCDISLP_LCD_AC :
			tmp = (tmp & ~SCU_LCDISLP_LCD_AC_M) | SCU_LCDISLP_LCD_AC_OUTPUT_DIS;
			break;
		default :
			socle_scu_show("unknow LCDISLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_LCDISLP);
			
	return 0;
}

extern int 
socle_scu_lcdislp_status (int pin)						//return 0:output0  1:output1  2:output disable or input
{
	u32 tmp;

	tmp = socle_scu_read(SCU_LCDISLP);

	switch(pin){
		case SOCLE_SCU_LCDISLP_LCD_POWER :
			tmp = (tmp & SCU_LCDISLP_LCD_POWER_M) >> SCU_LCDISLP_LCD_POWER_S;
			break;
		case SOCLE_SCU_LCDISLP_LCD_LP:
			tmp = (tmp & SCU_LCDISLP_LCD_LP_M) >> SCU_LCDISLP_LCD_LP_S;
			break;
		case SOCLE_SCU_LCDISLP_LCD_LE:
			tmp = (tmp & SCU_LCDISLP_LCD_LE_M) >> SCU_LCDISLP_LCD_LE_S;
			break;
		case SOCLE_SCU_LCDISLP_LCD_FP :
			tmp = (tmp & SCU_LCDISLP_LCD_FP_M) >> SCU_LCDISLP_LCD_FP_S;
			break;			
		case SOCLE_SCU_LCDISLP_LCD_DATA:
			tmp = (tmp & SCU_LCDISLP_LCD_DATA_M) >> SCU_LCDISLP_LCD_DATA_S;
			break;
		case SOCLE_SCU_LCDISLP_LCD_CP :
			tmp = (tmp & SCU_LCDISLP_LCD_CP_M) >> SCU_LCDISLP_LCD_CP_S;
			break;			
		case SOCLE_SCU_LCDISLP_LCD_AC:
			tmp = (tmp & SCU_LCDISLP_LCD_AC_M) >> SCU_LCDISLP_LCD_AC_S;
			break;
		default :
			socle_scu_show("unknow LCDISLP pin\n");
			return -1;
			break;			
	}
	if(tmp > 2)
		return -1;
			
	return tmp;
}


/*	SCU_PERI0SLP	*/
	/*	the output pin value of peripherals interface including UART, SPI, I2C, I2S, and PWM in SLEEP mode	*/
extern int 
socle_scu_peri0slp_ouput0_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PERI0SLP);

	switch(pin){
		case SOCLE_SCU_PERI0SLP_PWM :
			tmp = (tmp & ~SCU_PERI0SLP_PWM_M) | SCU_PERI0SLP_PWM_OUTPUT0;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_SDO:
			tmp = (tmp & ~SCU_PERI0SLP_I2S_SDO_M) | SCU_PERI0SLP_I2S_SDO_OUTPUT0;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_TXLRCK:
			tmp = (tmp & ~SCU_PERI0SLP_I2S_TXLRCK_M) | SCU_PERI0SLP_I2S_TXLRCK_OUTPUT0;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_TXSCLK :
			tmp = (tmp & ~SCU_PERI0SLP_I2S_TXSCLK_M) | SCU_PERI0SLP_I2S_TXSCLK_OUTPUT0;
			break;			
		case SOCLE_SCU_PERI0SLP_I2C2_SDA:
			tmp = (tmp & ~SCU_PERI0SLP_I2C2_SDA_M) | SCU_PERI0SLP_I2C2_SDA_OUTPUT0;
			break;
		case SOCLE_SCU_PERI0SLP_I2C2_SCL :
			tmp = (tmp & ~SCU_PERI0SLP_I2C2_SCL_M) | SCU_PERI0SLP_I2C2_SCL_OUTPUT0;
			break;			
		case SOCLE_SCU_PERI0SLP_I2C1_SDA:
			tmp = (tmp & ~SCU_PERI0SLP_I2C1_SDA_M) | SCU_PERI0SLP_I2C1_SDA_OUTPUT0;
			break;
		case SOCLE_SCU_PERI0SLP_I2C1_SCL:
			tmp = (tmp & ~SCU_PERI0SLP_I2C1_SCL_M) | SCU_PERI0SLP_I2C1_SCL_OUTPUT0;
			break;
		case SOCLE_SCU_PERI0SLP_I2C0_SDA:
			tmp = (tmp & ~SCU_PERI0SLP_I2C0_SDA_M) | SCU_PERI0SLP_I2C0_SDA_OUTPUT0;
			break;
		case SOCLE_SCU_PERI0SLP_I2C0_SCL :
			tmp = (tmp & ~SCU_PERI0SLP_I2C0_SCL_M) | SCU_PERI0SLP_I2C0_SCL_OUTPUT0;
			break;			
		case SOCLE_SCU_PERI0SLP_SPI_MOSI:
			tmp = (tmp & ~SCU_PERI0SLP_SPI_MOSI_M) | SCU_PERI0SLP_SPI_MOSI_OUTPUT0;
			break;
		case SOCLE_SCU_PERI0SLP_SPI_SSN :
			tmp = (tmp & ~SCU_PERI0SLP_SPI_SSN_M) | SCU_PERI0SLP_SPI_SSN_OUTPUT0;
			break;			
		case SOCLE_SCU_PERI0SLP_SPI_SCK:
			tmp = (tmp & ~SCU_PERI0SLP_SPI_SCK_M) | SCU_PERI0SLP_SPI_SCK_OUTPUT0;
			break;
		case SOCLE_SCU_PERI0SLP_UART1_TXD :
			tmp = (tmp & ~SCU_PERI0SLP_UART1_TXD_M) | SCU_PERI0SLP_UART1_TXD_OUTPUT0;
			break;			
		case SOCLE_SCU_PERI0SLP_UART0_TXD:
			tmp = (tmp & ~SCU_PERI0SLP_UART0_TXD_M) | SCU_PERI0SLP_UART0_TXD_OUTPUT0;
			break;		
		case SOCLE_SCU_PERI0SLP_GPIO:
			tmp = (tmp & ~SCU_PERI0SLP_GPIO_M) | SCU_PERI0SLP_GPIO_OUTPUT0;
			break;
		default :
			socle_scu_show("unknow PERI0SLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_PERI0SLP);
			
	return 0;
}


extern int 
socle_scu_peri0slp_ouput1_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PERI0SLP);

	switch(pin){
		case SOCLE_SCU_PERI0SLP_PWM :
			tmp = (tmp & ~SCU_PERI0SLP_PWM_M) | SCU_PERI0SLP_PWM_OUTPUT1;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_SDO:
			tmp = (tmp & ~SCU_PERI0SLP_I2S_SDO_M) | SCU_PERI0SLP_I2S_SDO_OUTPUT1;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_TXLRCK:
			tmp = (tmp & ~SCU_PERI0SLP_I2S_TXLRCK_M) | SCU_PERI0SLP_I2S_TXLRCK_OUTPUT1;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_TXSCLK :
			tmp = (tmp & ~SCU_PERI0SLP_I2S_TXSCLK_M) | SCU_PERI0SLP_I2S_TXSCLK_OUTPUT1;
			break;			
		case SOCLE_SCU_PERI0SLP_I2C2_SDA:
			tmp = (tmp & ~SCU_PERI0SLP_I2C2_SDA_M) | SCU_PERI0SLP_I2C2_SDA_OUTPUT1;
			break;
		case SOCLE_SCU_PERI0SLP_I2C2_SCL :
			tmp = (tmp & ~SCU_PERI0SLP_I2C2_SCL_M) | SCU_PERI0SLP_I2C2_SCL_OUTPUT1;
			break;			
		case SOCLE_SCU_PERI0SLP_I2C1_SDA:
			tmp = (tmp & ~SCU_PERI0SLP_I2C1_SDA_M) | SCU_PERI0SLP_I2C1_SDA_OUTPUT1;
			break;
		case SOCLE_SCU_PERI0SLP_I2C1_SCL:
			tmp = (tmp & ~SCU_PERI0SLP_I2C1_SCL_M) | SCU_PERI0SLP_I2C1_SCL_OUTPUT1;
			break;
		case SOCLE_SCU_PERI0SLP_I2C0_SDA:
			tmp = (tmp & ~SCU_PERI0SLP_I2C0_SDA_M) | SCU_PERI0SLP_I2C0_SDA_OUTPUT1;
			break;
		case SOCLE_SCU_PERI0SLP_I2C0_SCL :
			tmp = (tmp & ~SCU_PERI0SLP_I2C0_SCL_M) | SCU_PERI0SLP_I2C0_SCL_OUTPUT1;
			break;			
		case SOCLE_SCU_PERI0SLP_SPI_MOSI:
			tmp = (tmp & ~SCU_PERI0SLP_SPI_MOSI_M) | SCU_PERI0SLP_SPI_MOSI_OUTPUT1;
			break;
		case SOCLE_SCU_PERI0SLP_SPI_SSN :
			tmp = (tmp & ~SCU_PERI0SLP_SPI_SSN_M) | SCU_PERI0SLP_SPI_SSN_OUTPUT1;
			break;			
		case SOCLE_SCU_PERI0SLP_SPI_SCK:
			tmp = (tmp & ~SCU_PERI0SLP_SPI_SCK_M) | SCU_PERI0SLP_SPI_SCK_OUTPUT1;
			break;
		case SOCLE_SCU_PERI0SLP_UART1_TXD :
			tmp = (tmp & ~SCU_PERI0SLP_UART1_TXD_M) | SCU_PERI0SLP_UART1_TXD_OUTPUT1;
			break;			
		case SOCLE_SCU_PERI0SLP_UART0_TXD:
			tmp = (tmp & ~SCU_PERI0SLP_UART0_TXD_M) | SCU_PERI0SLP_UART0_TXD_OUTPUT1;
			break;		
		case SOCLE_SCU_PERI0SLP_GPIO:
			tmp = (tmp & ~SCU_PERI0SLP_GPIO_M) | SCU_PERI0SLP_GPIO_OUTPUT1;
			break;
		default :
			socle_scu_show("unknow PERI0SLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_PERI0SLP);
			
	return 0;
}

extern int 
socle_scu_peri0slp_ouput_dis_input_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PERI0SLP);

	switch(pin){
		case SOCLE_SCU_PERI0SLP_PWM :
			tmp = (tmp & ~SCU_PERI0SLP_PWM_M) | SCU_PERI0SLP_PWM_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_SDO:
			tmp = (tmp & ~SCU_PERI0SLP_I2S_SDO_M) | SCU_PERI0SLP_I2S_SDO_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_TXLRCK:
			tmp = (tmp & ~SCU_PERI0SLP_I2S_TXLRCK_M) | SCU_PERI0SLP_I2S_TXLRCK_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_TXSCLK :
			tmp = (tmp & ~SCU_PERI0SLP_I2S_TXSCLK_M) | SCU_PERI0SLP_I2S_TXSCLK_OUTPUT_DIS;
			break;			
		case SOCLE_SCU_PERI0SLP_I2C2_SDA:
			tmp = (tmp & ~SCU_PERI0SLP_I2C2_SDA_M) | SCU_PERI0SLP_I2C2_SDA_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI0SLP_I2C2_SCL :
			tmp = (tmp & ~SCU_PERI0SLP_I2C2_SCL_M) | SCU_PERI0SLP_I2C2_SCL_OUTPUT_DIS;
			break;			
		case SOCLE_SCU_PERI0SLP_I2C1_SDA:
			tmp = (tmp & ~SCU_PERI0SLP_I2C1_SDA_M) | SCU_PERI0SLP_I2C1_SDA_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI0SLP_I2C1_SCL:
			tmp = (tmp & ~SCU_PERI0SLP_I2C1_SCL_M) | SCU_PERI0SLP_I2C1_SCL_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI0SLP_I2C0_SDA:
			tmp = (tmp & ~SCU_PERI0SLP_I2C0_SDA_M) | SCU_PERI0SLP_I2C0_SDA_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI0SLP_I2C0_SCL :
			tmp = (tmp & ~SCU_PERI0SLP_I2C0_SCL_M) | SCU_PERI0SLP_I2C0_SCL_OUTPUT_DIS;
			break;			
		case SOCLE_SCU_PERI0SLP_SPI_MOSI:
			tmp = (tmp & ~SCU_PERI0SLP_SPI_MOSI_M) | SCU_PERI0SLP_SPI_MOSI_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI0SLP_SPI_SSN :
			tmp = (tmp & ~SCU_PERI0SLP_SPI_SSN_M) | SCU_PERI0SLP_SPI_SSN_OUTPUT_DIS;
			break;			
		case SOCLE_SCU_PERI0SLP_SPI_SCK:
			tmp = (tmp & ~SCU_PERI0SLP_SPI_SCK_M) | SCU_PERI0SLP_SPI_SCK_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI0SLP_UART1_TXD :
			tmp = (tmp & ~SCU_PERI0SLP_UART1_TXD_M) | SCU_PERI0SLP_UART1_TXD_OUTPUT_DIS;
			break;			
		case SOCLE_SCU_PERI0SLP_UART0_TXD:
			tmp = (tmp & ~SCU_PERI0SLP_UART0_TXD_M) | SCU_PERI0SLP_UART0_TXD_OUTPUT_DIS;
			break;		
		case SOCLE_SCU_PERI0SLP_GPIO:
			tmp = (tmp & ~SCU_PERI0SLP_GPIO_M) | SCU_PERI0SLP_GPIO_OUTPUT_DIS;
			break;
		default :
			socle_scu_show("unknow PERI0SLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_PERI0SLP);
			
	return 0;
}


extern int 
socle_scu_peri0slp_status (int pin)						//return 0:output0  1:output1  2:output disable or input
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PERI0SLP);

	switch(pin){
		case SOCLE_SCU_PERI0SLP_PWM :
			tmp = (tmp & SCU_PERI0SLP_PWM_M) >> SCU_PERI0SLP_PWM_S;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_SDO:
			tmp = (tmp & SCU_PERI0SLP_PWM_M) >> SCU_PERI0SLP_PWM_S;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_TXLRCK:
			tmp = (tmp & SCU_PERI0SLP_I2S_TXLRCK_M) >> SCU_PERI0SLP_I2S_TXLRCK_S;
			break;
		case SOCLE_SCU_PERI0SLP_I2S_TXSCLK :
			tmp = (tmp & SCU_PERI0SLP_I2S_TXSCLK_M) >> SCU_PERI0SLP_I2S_TXSCLK_S;
			break;			
		case SOCLE_SCU_PERI0SLP_I2C2_SDA:
			tmp = (tmp & SCU_PERI0SLP_I2C2_SDA_M) >> SCU_PERI0SLP_I2C2_SDA_S;
			break;
		case SOCLE_SCU_PERI0SLP_I2C2_SCL :
			tmp = (tmp & SCU_PERI0SLP_I2C2_SCL_M) >> SCU_PERI0SLP_I2C2_SCL_S;
			break;			
		case SOCLE_SCU_PERI0SLP_I2C1_SDA:
			tmp = (tmp & SCU_PERI0SLP_I2C1_SDA_M) >> SCU_PERI0SLP_I2C1_SDA_S;
			break;
		case SOCLE_SCU_PERI0SLP_I2C1_SCL:
			tmp = (tmp & SCU_PERI0SLP_I2C1_SCL_M) >> SCU_PERI0SLP_I2C1_SCL_S;
			break;
		case SOCLE_SCU_PERI0SLP_I2C0_SDA:
			tmp = (tmp & SCU_PERI0SLP_I2C0_SDA_M) >> SCU_PERI0SLP_I2C0_SDA_S;
			break;
		case SOCLE_SCU_PERI0SLP_I2C0_SCL :
			tmp = (tmp & SCU_PERI0SLP_I2C0_SCL_M) >> SCU_PERI0SLP_I2C0_SCL_S;
			break;			
		case SOCLE_SCU_PERI0SLP_SPI_MOSI:
			tmp = (tmp & SCU_PERI0SLP_SPI_MOSI_M) >> SCU_PERI0SLP_SPI_MOSI_S;
			break;
		case SOCLE_SCU_PERI0SLP_SPI_SSN :
			tmp = (tmp & SCU_PERI0SLP_SPI_SSN_M) >> SCU_PERI0SLP_SPI_SSN_S;
			break;			
		case SOCLE_SCU_PERI0SLP_SPI_SCK:
			tmp = (tmp & SCU_PERI0SLP_SPI_SCK_M) >> SCU_PERI0SLP_SPI_SCK_S;
			break;
		case SOCLE_SCU_PERI0SLP_UART1_TXD :
			tmp = (tmp & SCU_PERI0SLP_UART1_TXD_M) >> SCU_PERI0SLP_UART1_TXD_S;
			break;			
		case SOCLE_SCU_PERI0SLP_UART0_TXD:
			tmp = (tmp & SCU_PERI0SLP_UART0_TXD_M) >> SCU_PERI0SLP_UART0_TXD_S;
			break;		
		case SOCLE_SCU_PERI0SLP_GPIO:
			tmp = (tmp & SCU_PERI0SLP_GPIO_M) >> SCU_PERI0SLP_GPIO_S;
			break;
		default :
			socle_scu_show("unknow PERI0SLP pin\n");
			return -1;
			break;			
	}
	if(tmp > 2)
		return -1;
			
	return tmp;
}


/*	SCU_PERI1SLP	*/
	/*	the output pin value of peripherals interface including SD/MMC in SLEEP mode	*/
extern int 
socle_scu_peri1slp_ouput0_set (int pin)

{
	u32 tmp;

	tmp = socle_scu_read(SCU_PERI1SLP);

	switch(pin){
		case SOCLE_SCU_PERI1SLP_SDC_CLK :
			tmp = (tmp & ~SCU_PERI1SLP_SDC_CLK_M) | SCU_PERI1SLP_SDC_CLK_OUTPUT0;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_PWR:
			tmp = (tmp & ~SCU_PERI1SLP_SDC_PWR_M) | SCU_PERI1SLP_SDC_PWR_OUTPUT0;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_DATA:
			tmp = (tmp & ~SCU_PERI1SLP_SDC_DATA_M) | SCU_PERI1SLP_SDC_DATA_OUTPUT0;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_CMD :
			tmp = (tmp & ~SCU_PERI1SLP_SDC_CMD_M) | SCU_PERI1SLP_SDC_CMD_OUTPUT0;
			break;
		default :
			socle_scu_show("unknow PERI1SLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_PERI1SLP);
			
	return 0;
}

extern int 
socle_scu_peri1slp_ouput1_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PERI1SLP);

	switch(pin){
		case SOCLE_SCU_PERI1SLP_SDC_CLK :
			tmp = (tmp & ~SCU_PERI1SLP_SDC_CLK_M) | SCU_PERI1SLP_SDC_CLK_OUTPUT1;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_PWR:
			tmp = (tmp & ~SCU_PERI1SLP_SDC_PWR_M) | SCU_PERI1SLP_SDC_PWR_OUTPUT1;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_DATA:
			tmp = (tmp & ~SCU_PERI1SLP_SDC_DATA_M) | SCU_PERI1SLP_SDC_DATA_OUTPUT1;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_CMD :
			tmp = (tmp & ~SCU_PERI1SLP_SDC_CMD_M) | SCU_PERI1SLP_SDC_CMD_OUTPUT1;
			break;
		default :
			socle_scu_show("unknow PERI1SLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_PERI1SLP);
			
	return 0;
}

extern int 
socle_scu_peri1slp_ouput_dis_input_set (int pin)
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PERI1SLP);

	switch(pin){
		case SOCLE_SCU_PERI1SLP_SDC_CLK :
			tmp = (tmp & ~SCU_PERI1SLP_SDC_CLK_M) | SCU_PERI1SLP_SDC_CLK_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_PWR:
			tmp = (tmp & ~SCU_PERI1SLP_SDC_PWR_M) | SCU_PERI1SLP_SDC_PWR_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_DATA:
			tmp = (tmp & ~SCU_PERI1SLP_SDC_DATA_M) | SCU_PERI1SLP_SDC_DATA_OUTPUT_DIS;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_CMD :
			tmp = (tmp & ~SCU_PERI1SLP_SDC_CMD_M) | SCU_PERI1SLP_SDC_CMD_OUTPUT_DIS;
			break;
		default :
			socle_scu_show("unknow PERI1SLP pin\n");
			return -1;
			break;			
	}
	socle_scu_write(tmp, SCU_PERI1SLP);
			
	return 0;
}

extern int 
socle_scu_peri1slp_status (int pin)						//return 0:output0  1:output1  2:output disable or input
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PERI1SLP);

	switch(pin){
		case SOCLE_SCU_PERI1SLP_SDC_CLK :
			tmp = (tmp & SCU_PERI1SLP_SDC_CLK_M) >> SCU_PERI1SLP_SDC_CLK_S;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_PWR:
			tmp = (tmp & SCU_PERI1SLP_SDC_PWR_M) >> SCU_PERI1SLP_SDC_PWR_S;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_DATA:
			tmp = (tmp & SCU_PERI1SLP_SDC_DATA_M) >> SCU_PERI1SLP_SDC_DATA_S;
			break;
		case SOCLE_SCU_PERI1SLP_SDC_CMD :
			tmp = (tmp & SCU_PERI1SLP_SDC_CMD_M) >> SCU_PERI1SLP_SDC_CMD_S;
			break;			
		default :
			socle_scu_show("unknow SCU_PERI1SLP pin\n");
			return -1;
			break;			
	}
	if(tmp > 2)
		return -1;
			
	return tmp;
}


/*	SCU_SLPEN	*/
	/*	SLEEP mode IO configuration register	*/
extern void 
socle_scu_sleep_io_conf_slpen_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_SLPEN) & ~SCU_SLPEN_IO_STAT_CONF_AUTO;

	socle_scu_write(tmp, SCU_SLPEN);
			
	return ;
}

extern void 
socle_scu_sleep_io_conf_auto_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_SLPEN) | SCU_SLPEN_IO_STAT_CONF_AUTO;

	socle_scu_write(tmp, SCU_SLPEN);
			
	return ;
}

extern int 
socle_scu_sleep_io_conf_status ()				//return 1:automatically by SLEEP mode	0:set by SCU_SLPEN[0] bit	
{
	u32 tmp;

	tmp = socle_scu_read(SCU_SLPEN) & SCU_SLPEN_IO_STAT_CONF_AUTO;

	if(SCU_SLPEN_IO_STAT_CONF_AUTO == tmp)
		return 1;
	else
		return 0;
}

extern void 
socle_scu_sleep_io_en_ext_output_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_SLPEN) | SCU_SLPEN_IO_STAT_EN_SCU_REGS;

	socle_scu_write(tmp, SCU_SLPEN);
			
	return ;
}

extern void 
socle_scu_sleep_io_en_normal_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_SLPEN) & ~SCU_SLPEN_IO_STAT_EN_SCU_REGS;

	socle_scu_write(tmp, SCU_SLPEN);
			
	return ;
}

extern int 
socle_scu_sleep_io_en_status ()				//return 1:external output pins are controlled by SCU_xxx	0:change output into normal function
{
	u32 tmp;

	tmp = socle_scu_read(SCU_SLPEN) & SCU_SLPEN_IO_STAT_EN_SCU_REGS;

	if(SCU_SLPEN_IO_STAT_EN_SCU_REGS == tmp)
		return 1;
	else
		return 0;
}

/*	SCU_IOMODE	*/
	/*	GPIO function register	*/
extern void 
socle_scu_iomode_gpio7_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) & ~SCU_IOMODE_GPIO7_I2C2_SDA;

	socle_scu_write(tmp, SCU_IOMODE);
			
	return ;
}

extern void 
socle_scu_iomode_i2c2_sda_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) | SCU_IOMODE_GPIO7_I2C2_SDA;

	socle_scu_write(tmp, SCU_IOMODE);
			
	return ;
}

extern int 
socle_scu_iomode_gpio7_status ()				//return 1:I2C2_SDA function	0:GPIO[7] function
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) & SCU_IOMODE_GPIO7_I2C2_SDA;

	if(SCU_IOMODE_GPIO7_I2C2_SDA == tmp)
		return 1;
	else
		return 0;
}

extern void 
socle_scu_iomode_gpio6_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) & ~SCU_IOMODE_GPIO6_I2C2_SCL;

	socle_scu_write(tmp, SCU_IOMODE);
			
	return ;
}

extern void 
socle_scu_iomode_i2c2_scl_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) | SCU_IOMODE_GPIO6_I2C2_SCL;

	socle_scu_write(tmp, SCU_IOMODE);
			
	return ;
}

extern int 
socle_scu_iomode_gpio6_status ()				//return 1:I2C2_SCL function	0:GPIO[6] function
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) & SCU_IOMODE_GPIO6_I2C2_SCL;

	if(SCU_IOMODE_GPIO6_I2C2_SCL == tmp)
		return 1;
	else
		return 0;
}

extern void 
socle_scu_iomode_gpio5_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) & ~SCU_IOMODE_GPIO5_I2C1_SDA;

	socle_scu_write(tmp, SCU_IOMODE);
			
	return ;
}

extern void 
socle_scu_iomode_i2c1_sda_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) | SCU_IOMODE_GPIO5_I2C1_SDA;

	socle_scu_write(tmp, SCU_IOMODE);
			
	return ;
}

extern int 
socle_scu_iomode_gpio5_status ()				//return 1:I2C1_SDA function	0:GPIO[5] function
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) & SCU_IOMODE_GPIO5_I2C1_SDA;

	if(SCU_IOMODE_GPIO5_I2C1_SDA == tmp)
		return 1;
	else
		return 0;
}

extern void 
socle_scu_iomode_gpio4_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) & ~SCU_IOMODE_GPIO4_I2C1_SCL;

	socle_scu_write(tmp, SCU_IOMODE);
			
	return ;
}

extern void 
socle_scu_iomode_i2c1_scl_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) | SCU_IOMODE_GPIO4_I2C1_SCL;

	socle_scu_write(tmp, SCU_IOMODE);
			
	return ;
}

extern int 
socle_scu_iomode_gpio4_status ()				//return 1:I2C1_SCL function	0:GPIO[4] function
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) & SCU_IOMODE_GPIO4_I2C1_SCL;

	if(SCU_IOMODE_GPIO4_I2C1_SCL == tmp)
		return 1;
	else
		return 0;
}

extern void 
socle_scu_iomode_gpio0_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) & ~SCU_IOMODE_GPIO0_I2C1_PLLOUT;

	socle_scu_write(tmp, SCU_IOMODE);
			
	return ;
}

extern void 
socle_scu_iomode_pllout_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) | SCU_IOMODE_GPIO0_I2C1_PLLOUT;

	socle_scu_write(tmp, SCU_IOMODE);
			
	return ;
}

extern int 
socle_scu_iomode_gpio0_status ()				//return 1:PLLOUT function	0:GPIO[0] function
{
	u32 tmp;

	tmp = socle_scu_read(SCU_IOMODE) & SCU_IOMODE_GPIO0_I2C1_PLLOUT;

	if(SCU_IOMODE_GPIO0_I2C1_PLLOUT == tmp)
		return 1;
	else
		return 0;
}	


/*	SCU_CHIPID	*/
	/*	Chip ID register	*/
extern u32 
socle_scu_chip_id_get ()							//return chip ID	
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPID);
			
	return tmp;
}

/*	SCU_INFORM0	*/
	/*	User defined information register	*/
extern void 
socle_scu_info0_set (u32 inf)	
{
	socle_scu_write(inf, SCU_INFORM0);
			
	return ;
}		

extern u32 
socle_scu_info0_get ()						//return information0 value
{
	u32 tmp;

	tmp = socle_scu_read(SCU_INFORM0);
			
	return tmp;
}		

/*	SCU_INFORM1	*/
	/*	User defined information register	*/
extern void 
socle_scu_info1_set (u32 inf)
{
	socle_scu_write(inf, SCU_INFORM1);
			
	return ;
}		

extern u32 
socle_scu_info1_get ()						//return information1 value		
{
	u32 tmp;

	tmp = socle_scu_read(SCU_INFORM1);
			
	return tmp;
}		

/*	SCU_INFORM2	*/
	/*	User defined information register	*/
extern void 
socle_scu_info2_set (u32 inf)
{
	socle_scu_write(inf, SCU_INFORM2);
			
	return ;
}		

extern u32 
socle_scu_info2_get ()						//return information2 value
{
	u32 tmp;

	tmp = socle_scu_read(SCU_INFORM2);
			
	return tmp;
}				

/*	SCU_INFORM3	*/
	/*	User defined information register	*/
extern void 
socle_scu_info3_set (u32 inf)
{
	socle_scu_write(inf, SCU_INFORM3);
			
	return ;
}		
		
extern u32 
socle_scu_info3_get ()						//return information3 value	
{
	u32 tmp;

	tmp = socle_scu_read(SCU_INFORM3);
			
	return tmp;
}					

/*	SCU_CHIPMD	*/
	/*	PLL related setting	*/
extern int 
socle_scu_dcm_mode_status ()				//return 1:DCM test mode  0:Normal mode	
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPMD) & SCU_CHIPMD_DCM_MODE_DCM;

	if(SCU_CHIPMD_DCM_MODE_DCM == tmp)
		return 1;
	else
		return 0;
}	

extern void 
socle_scu_pmu_mode_pmu_debug_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPMD) | SCU_CHIPMD_PMU_DEBUG_MODE_DEBUG;

	socle_scu_write(tmp, SCU_CHIPMD);
			
	return ;
}

extern void 
socle_scu_pmu_mode_normal_set ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPMD) & ~SCU_CHIPMD_PMU_DEBUG_MODE_DEBUG;

	socle_scu_write(tmp, SCU_CHIPMD);
			
	return ;
}

extern int 
socle_scu_pmu_mode_status ()				//return 1:Debug and test mode  0:Normal function mode	
{
	u32 tmp;

	tmp = socle_scu_read(SCU_CHIPMD) & SCU_CHIPMD_PMU_DEBUG_MODE_DEBUG;

	if(SCU_CHIPMD_PMU_DEBUG_MODE_DEBUG == tmp)
		return 1;
	else
		return 0;
}	


/*	SCU_PLL	*/
	/*	PLL related setting	*/
extern int 
socle_scu_pll_relock_status ()				//return 0:PLL is locked  1:PLL is start to re-configure 2:pPLL is on re-configuring and unlocked	
{
	u32 tmp;

	tmp = (socle_scu_read(SCU_PLL) & SCU_PLL_RELOCK_STAT_M) >>SCU_PLL_RELOCK_STAT_S;

	if(tmp >2)
		return -1;
	
	return tmp;
}	

extern void 
socle_scu_pll_lock_counter_enable ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLL) & ~SCU_PLL_LOCK_COUNTER_DIS;

	socle_scu_write(tmp, SCU_PLL);
			
	return ;
}

extern void 
socle_scu_pll_lock_counter_disable ()
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLL) | SCU_PLL_LOCK_COUNTER_DIS;

	socle_scu_write(tmp, SCU_PLL);
			
	return ;
}

extern int 
socle_scu_pll_lock_counter_status ()		//return 1:disable  0:enable	
{
	u32 tmp;

	tmp = socle_scu_read(SCU_PLL) & SCU_PLL_LOCK_COUNTER_DIS;

	if(SCU_PLL_LOCK_COUNTER_DIS == tmp)
		return 1;
	else
		return 0;
}	


/*	SCU_DBCT	*/
	/*	PLL related setting	*/
extern void 
socle_scu_debounce_time_set (int time)	
{
	u32 tmp;

	tmp = socle_scu_read(SCU_DBCT) & ~SCU_DBCT_DEBOUNCE_TIME_M;
	tmp = tmp | (time << SCU_DBCT_DEBOUNCE_TIME_S);

	socle_scu_write(tmp, SCU_DBCT); 

	return ;
}	

extern int 
socle_scu_debounce_time_get ()				//return Debounce time for GPIO and EXT_INT noise filter
{
	u32 tmp;

	tmp = socle_scu_read(SCU_DBCT) & SCU_DBCT_DEBOUNCE_TIME_M;

	tmp = tmp >> SCU_DBCT_DEBOUNCE_TIME_S;
	
	return tmp;
}	


extern unsigned long
socle_get_cpu_clock (void)
{	
	/*	get power mode */
	if(1 == socle_scu_pll_power_down_status()){
		SCUDBUG("sq_scu_pll_power_down\n");
		socle_clock.cpu_clock = EXT_OSC;		/* power down */
	}else{
		SCUDBUG("sq_scu_pll_power_up\n");
		socle_clock.cpu_clock = socle_scu_pll_get() ;
	}
	SCUDBUG("sq_clock.cpu_clock = %ld\n", socle_clock.cpu_clock);
				
	return socle_clock.cpu_clock ;
}

EXPORT_SYMBOL(socle_get_cpu_clock);

extern unsigned long
socle_get_ahb_clock (void)
{
	int ratio;
	
	ratio = socle_scu_clock_ratio_get();

	SCUDBUG("ratio = %d\n", ratio);
	
	//20080407 leonid fix for ahb clock
	socle_clock.ahb_clock = socle_get_cpu_clock() / ratio;

	SCUDBUG("sq_clock.ahb_clock = %ld\n", socle_clock.ahb_clock);	

	return socle_clock.ahb_clock;
}

extern unsigned long
socle_get_apb_clock (void)
{
	//20080407 leonid fix for apb clock
	socle_clock.apb_clock = socle_get_ahb_clock() / 2;	
	SCUDBUG("sq_clock.apb_clock = %ld\n", socle_clock.apb_clock);	

	return socle_clock.apb_clock;
}
EXPORT_SYMBOL(socle_get_apb_clock);

//
extern unsigned long  
get_pll_clock(void)
{
	unsigned long scu_clock, scu_ratio, apb_clock;
	
	scu_clock = socle_get_cpu_clock();
//	 socle_scu_show("scu_clock = %x \n",scu_clock);
	/*      get scu ratio */
	scu_ratio = socle_scu_clock_ratio_get();

	SCUDBUG("MSMV CPU = %ld MHz , HCLCK = %ld MHz\n",scu_clock/1000000, (scu_clock/scu_ratio)/1000000);

	apb_clock = scu_clock/scu_ratio/2;
	return (apb_clock);
}



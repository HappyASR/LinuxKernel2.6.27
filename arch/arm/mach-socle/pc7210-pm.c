#include <asm/io.h>
#include <linux/kernel.h>
#include <mach/platform.h>
#include <mach/pc7210-scu.h>
#include <mach/regs-pwmt.h>
#include <mach/reg-mac.h>
#include <linux/netdevice.h>

//#define SOCLE_PS_DEBUG

#ifdef SOCLE_PS_DEBUG
#define PSDBUG(fmt, args...) printk(KERN_DEBUG "PS_DEBUG: %s" fmt, __FUNCTION__, ## args)
#else
#define PSDBUG(fmt, args...)
#endif

extern void stop_mode_test(void);
extern void sleep_mode_test(void);
extern void slow_mode_test(void);
void ps_ip_clock_test (int mode, int swh, int ip);
void ps_mac_phy_clock_on_test(void);
void ps_mac_phy_clock_off_test(void);
void ps_usb_phy_clock_on_test(void);
void ps_usb_phy_clock_off_test(void);
void ps_i2s_clock_on_test(void);
void ps_i2s_clock_off_test(void);

extern void 
stop_mode_test(void)
{
	printk(" Enter to stop mode (press fiq button to wake up)\n");
	
	PSDBUG("FIQ enable\n");
	socle_scu_fast_irq_enable();
	PSDBUG("Disable I2S external clock\n");
	ps_i2s_clock_off_test();
	PSDBUG("Disable MAC PHY clock\n");	
	ps_mac_phy_clock_off_test();
	PSDBUG("Disable USB PHY clock\n");
	ps_usb_phy_clock_off_test();
	/*	Disable UPLL	*/
	socle_scu_upll_power_down();
	/*	Enter SLOW mode	*/
	socle_scu_normal_mode_disable();
	//PSDBUG("sq_scu_normal_mode_disablek\n");
	/*	Disable CPLL	*/
	socle_scu_cpll_power_down();
	//PSDBUG("sq_scu_cpll_power_down\n");
	/*	Enter STOP mode	*/
	socle_scu_stop_mode_enable();
	
	/*	wait for firq wake up	*/	
	/*	Enable UPLL	*/
	socle_scu_upll_normal();
	PSDBUG("sq_scu_upll_power_up\n");
	/*	Enable CPLL	*/
	socle_scu_cpll_normal();
	PSDBUG("sq_scu_cpll_power_up\n");
	/*	Enter Normal mode	*/
	socle_scu_normal_mode_enable();
	PSDBUG("sq_scu_normal_mode_enablek\n");
	ps_usb_phy_clock_on_test();
	PSDBUG("Enable USB PHY clock\n");
	ps_mac_phy_clock_on_test();
	PSDBUG("Enable MAC PHY clock\n");
	ps_i2s_clock_on_test();
	PSDBUG("Enable I2S external clock\n");
	
	//printk("\nStop mode test success!!!\n");
	
	return ;	
}

extern void 
sleep_mode_test(void)
{
	printk(" Enter to sleep mode (press fiq button to wake up)\n");
	
	PSDBUG("FIQ enable\n");
	socle_scu_fast_irq_enable();
	PSDBUG("Disable I2S external clock\n");
	ps_i2s_clock_off_test();
	PSDBUG("Disable MAC PHY clock\n");
	ps_mac_phy_clock_off_test();
	PSDBUG("Disable USB PHY clock\n");
	ps_usb_phy_clock_off_test();
	/*	Disable UPLL	*/
	socle_scu_upll_power_down();
	/*	Disable all IP clock	*/
	ps_ip_clock_test(0, 0, 0);	
	/*	Enter SLEEP mode	*/
	socle_scu_sleep_mode_enable();
	
	/*	wait for firq wake up	*/
	/*	Enable all IP clock	*/
	ps_ip_clock_test(0, 1, 0);	
	//PSDBUG("Enable all IP clock\n");	
	/*	Enable UPLL	*/
	socle_scu_upll_normal();
	PSDBUG("sq_scu_upll_power_up\n");
	ps_usb_phy_clock_on_test();
	PSDBUG("Enable USB PHY clock\n");
	ps_mac_phy_clock_on_test();
	PSDBUG("Enable MAC PHY clock\n");
	ps_i2s_clock_on_test();
	PSDBUG("Enable I2S external clock\n");
	
	//printk("\nSleep mode test success!!!\n");
	
	return ;	
}

extern void 
slow_mode_test(void)
{
	int tmp;
	
	PSDBUG("FIQ enable\n");
	socle_scu_fast_irq_enable();
	PSDBUG("Disable I2S external clock\n");
	ps_i2s_clock_off_test();
	PSDBUG("Disable MAC PHY clock\n");
	ps_mac_phy_clock_off_test();
	PSDBUG("Disable USB PHY clock\n");
	ps_usb_phy_clock_off_test();
	/*	Disable UPLL	*/
	socle_scu_upll_power_down();
	/*	Enter SLOW mode	*/
	socle_scu_normal_mode_disable();
	//PSDBUG("sq_scu_normal_mode_disable\n");
	/*	Disable CPLL	*/
	socle_scu_cpll_power_down();
	//PSDBUG("sq_scu_cpll_power_down\n");	
	/*	Disable all IP clock	*/
	ps_ip_clock_test(0, 0, 0);
	//PSDBUG("Disable all IP clock\n");

	for(tmp=0;tmp<0x80000;tmp++){}
				
	/*	Enable all IP clock	*/
	ps_ip_clock_test(0, 1, 0);	
	/*	ENable UPLL	*/
	socle_scu_upll_normal();
	PSDBUG("sq_scu_upll_power_up\n");
	/*	Enable CPLL	*/
	socle_scu_cpll_normal();
	PSDBUG("sq_scu_cpll_power_up\n");
	/*	Enter Normal mode	*/
	socle_scu_normal_mode_enable();
	PSDBUG("sq_scu_normal_mode_enablek\n");
	ps_usb_phy_clock_on_test();
	PSDBUG("Enable USB PHY clock\n");
	ps_mac_phy_clock_on_test();
	PSDBUG("Enable MAC PHY clock\n");
	ps_i2s_clock_on_test();
	PSDBUG("Enable I2S external clock\n");
		
	PSDBUG("Slow mode test success!!!\n");	

	return ;
}
		
void ps_ip_clock_test (int mode, int swh, int ip)	//mode 0 : auto all IP, 1: auto one IP, 2: not auto
{	
	int ch=0, tmp=0;	
		
	if(mode == 0){
		for(ch='a';ch<='u';ch++){
			//printk("%c\n", ch);
			goto ON_SWITCH;	 
MODE_0:
			;
		}
		return ;
	}else{
		tmp = ip;
		goto CLK_SWITCH;	 
MODE_1:			
		return ;
	}
	
ON_SWITCH:						
			switch(ch){	
				case 'a' :
					tmp = SOCLE_SCU_ADCCLK;
					break;
				case 'b' :
					tmp = SOCLE_SCU_PCLK_UART3;
					break;
				case 'c' :
					tmp = SOCLE_SCU_PCLK_UART2;
					break;
				case 'd' :
					tmp = SOCLE_SCU_PCLK_UART1;
					//PSDBUG("not power off UART1 clock\n");
					break;
				case 'e' :
					tmp = SOCLE_SCU_PCLK_UART0;
					break;
				case 'f' :
					tmp = SOCLE_SCU_PCLK_ADC;
					break;
				case 'g' :
					tmp = SOCLE_SCU_PCLK_PWM;
					break;
				case 'h' :
					tmp = SOCLE_SCU_PCLK_SDC;
					break;
				case 'i' :
					tmp = SOCLE_SCU_PCLK_I2S;
					break;
				case 'j' :
					tmp = SOCLE_SCU_PCLK_I2C;
					break;
				case 'k' :
					tmp = SOCLE_SCU_PCLK_SPI;
					break;
				case 'l' :
					tmp = SOCLE_SCU_PCLK_GPIO;
					break;
				case 'm' :
					tmp = SOCLE_SCU_PCLK_WDT;
					break;
				case 'n' :
					tmp = SOCLE_SCU_PCLK_RTC;
					break;
				case 'o' :
					tmp = SOCLE_SCU_PCLK_TMR;
					break;
				case 'p' :
					tmp = SOCLE_SCU_PCLK_LCD;
					break;
				case 'q' :
					tmp = SOCLE_SCU_PCLK_NFC;
					break;
				case 'r' :
					tmp = SOCLE_SCU_PCLK_UDC;
					break;
				case 's' :
					tmp = SOCLE_SCU_PCLK_UHC;
					break;
				case 't' :
					tmp = SOCLE_SCU_PCLK_MAC;
					break;
				case 'u' :
					tmp = SOCLE_SCU_PCLK_HDMA;
					break;
				case 'v' :
					tmp = SOCLE_SCU_PCLK_SDRSTMC;
					//PSDBUG("not power off SDRSTMC clock\n");
					break;
				case 'x':
					printk("exit IP clock on test\n");
					return ;
					break;
				default:
					break;
			}
CLK_SWITCH :
			if(swh == 0)
				socle_scu_pclk_disable(tmp);
			else				
				socle_scu_pclk_enable(tmp);
				
			if(mode == 0)
				goto MODE_0;
			else
				goto MODE_1;
			
	return ;			
}

extern void socle_mac_phy_clock_on_test(void);
extern void socle_mac_phy_clock_off_test(void);

/* MAC PHY clock test */	
void ps_mac_phy_clock_on_test(void)
{	
	socle_mac_phy_clock_on_test();

	return ;
}
	
void ps_mac_phy_clock_off_test(void)
{	
	socle_mac_phy_clock_off_test();

	return ;
}

/* USB PHY clock test */
void ps_usb_phy_clock_on_test(void)
{		
	socle_scu_usb_tranceiver_upstream();	//Set PHY common block power down register in SCU 
	socle_scu_usb_pll_power_save();			//Set UDC mode register in SCU 
	iowrite32((ioread32(0x18060008) & 0xffffffef), 0x18060008);				//disable suspend register in UDC controller
			
	return ;
}
	
void ps_usb_phy_clock_off_test(void)
{		
	socle_scu_usb_tranceiver_upstream();	//Set PHY common block power down register in SCU 
	socle_scu_usb_pll_power_save();			//Set UDC mode register in SCU 
	iowrite32((ioread32(0x18060008) | 0x10), 0x18060008);				//enable suspend register in UDC controller 	 
		
	return ;
}

#define I2S_PHY_PWM_INDEX	2

/* I2S clock test */
void ps_i2s_clock_on_test(void)
{
	struct socle_pwmt *p;

	p = get_socle_pwmt_structure(I2S_PHY_PWM_INDEX);

	p->drv->i2s_phy_clock_on(p);

	release_socle_pwmt_structure(I2S_PHY_PWM_INDEX);

	return ;	
}

void ps_i2s_clock_off_test(void)
{
	struct socle_pwmt *p;

	p = get_socle_pwmt_structure(I2S_PHY_PWM_INDEX);

	p->drv->i2s_phy_clock_off(p);

	release_socle_pwmt_structure(I2S_PHY_PWM_INDEX);

	return ;	
}


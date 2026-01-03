#ifndef __CLOCK_17XX_40XX_H_
#define __CLOCK_17XX_40XX_H_
   
#include <stdint.h>
   
   

#ifdef __cplusplus
extern "C" {
#endif

extern const uint32_t OscRateIn;
extern const uint32_t RTCOscRateIn;

/** @defgroup CLOCK_17XX_40XX CHIP: LPC17xx/40xx Clock Driver
 * @ingroup CHIP_17XX_40XX_Drivers
 * @{
 */

#define SYSCTL_OSCRANGE_15_25 (1 << 4)	/*!< SCS register - main oscillator range 15 to 25MHz */
#define SYSCTL_OSCEC          (1 << 5)	/*!< SCS register - main oscillator enable */
#define SYSCTL_OSCSTAT        (1 << 6)	/*!< SCS register - main oscillator is ready status */

/*!< Internal oscillator frequency */
#define SYSCTL_IRC_FREQ (12000000)

#define SYSCTL_PLL_ENABLE   (1 << 0)/*!< PLL enable flag */

//=============================================================================
/**
 * @brief	Enables or connects a PLL
 * @param	PLLNum:	PLL number
 * @param	flags:	SYSCTL_PLL_ENABLE or SYSCTL_PLL_CONNECT
 * @return	Nothing
 * @note	This will also perform a PLL feed sequence. Connect only applies to the
 * LPC175x/6x devices.
 */
void Chip_Clock_EnablePLL(CHIP_SYSCTL_PLL_T PLLNum, uint32_t flags);
//=============================================================================
/**
 * @brief	Disables or disconnects a PLL
 * @param	PLLNum:	PLL number
 * @param	flags:	SYSCTL_PLL_ENABLE or SYSCTL_PLL_CONNECT
 * @return	Nothing
 * @note	This will also perform a PLL feed sequence. Connect only applies to the
 * LPC175x/6x devices.
 */
void Chip_Clock_DisablePLL(CHIP_SYSCTL_PLL_T PLLNum, uint32_t flags);

/**
 * @brief	Sets up a PLL
 * @param	PLLNum:		PLL number
 * @param	msel:	PLL Multiplier value (Must be pre-decremented)
 * @param	psel:	PLL Divider value (Must be pre-decremented)
 * @note		See the User Manual for limitations on these values for stable PLL
 * operation. Be careful with these values - they must be safe values for the
 * msl, nsel, and psel registers so must be already decremented by 1 or the
 * the correct value for psel (0 = div by 1, 1 = div by 2, etc.).
 * @return	Nothing
 */
void Chip_Clock_SetupPLL(CHIP_SYSCTL_PLL_T PLLNum, uint32_t msel, uint32_t psel);

#define SYSCTL_PLLSTS_ENABLED   (1 << 8)	/*!< PLL enable flag */
#define SYSCTL_PLLSTS_LOCKED    (1 << 10)	/*!< PLL connect flag */

/**
 * @brief	Returns PLL status
 * @param	PLLNum:		PLL number
 * @return	Current enabled flags, Or'ed SYSCTL_PLLSTS_* states
 * @note	Note flag positions for PLL0 and PLL1 differ on the LPC175x/6x devices.
 */
#define Chip_Clock_GetPLLStatus(PLLNum) { 	(uint32_t) LPC_SC->PLL[ (CHIP_SYSCTL_PLL_T) PLLNum].PLLSTAT; }


/**
 * @brief	Read PLL0 enable status
 * @return	true of the PLL0 is enabled. false if not enabled
 */
#define Chip_Clock_IsMainPLLEnabled() ((LPC_SC->PLL[0].PLLSTAT & SYSCTL_PLLSTS_ENABLED) != 0)


/**
 * @brief	Read PLL1 enable status
 * @return	true of the PLL1 is enabled. false if not enabled
 */
#define Chip_Clock_IsUSBPLLEnabled() ((LPC_SC->PLL[1].PLLSTAT & SYSCTL_PLLSTS_ENABLED) != 0)

/**
 * @brief	Read PLL0 lock status
 * @return	true of the PLL0 is locked. false if not locked
 */
#define Chip_Clock_IsMainPLLLocked() ((LPC_SC->PLL[0].PLLSTAT & SYSCTL_PLLSTS_LOCKED) != 0)

/**
 * @brief	Read PLL1 lock status
 * @return	true of the PLL1 is locked. false if not locked
 */
#define Chip_Clock_IsUSBPLLLocked()  ((LPC_SC->PLL[1].PLLSTAT & SYSCTL_PLLSTS_LOCKED) != 0)

/**
 * @brief	Enables the external Crystal oscillator
 * @return	Nothing
 */
#define Chip_Clock_EnableCrystal() { LPC_SC->SCS |= SYSCTL_OSCEC; }

/**
 * @brief	Checks if the external Crystal oscillator is enabled
 * @return	true if enabled, false otherwise
 */
#define Chip_Clock_IsCrystalEnabled() { (LPC_SC->SCS & SYSCTL_OSCSTAT) != 0; }

/**
 * @brief Sets the external crystal oscillator range to 15Mhz - 25MHz
 * @return	Nothing
 */
#define Chip_Clock_SetCrystalRangeHi() { LPC_SC->SCS |= SYSCTL_OSCRANGE_15_25; }

/**
 * @brief Sets the external crystal oscillator range to 1Mhz - 20MHz
 * @return	Nothing
 */
#define Chip_Clock_SetCrystalRangeLo() { LPC_SC->SCS &= ~SYSCTL_OSCRANGE_15_25; }

/**
 * @brief	Feeds a PLL
 * @param	PLLNum:	PLL number
 * @return	Nothing
 */
#define Chip_Clock_FeedPLL(PLLNum) {  LPC_SC->PLL[PLLNum].PLLFEED = 0xAA;   LPC_SC->PLL[PLLNum].PLLFEED = 0x55; }

/**
 * Power control for peripherals
 */
typedef enum CHIP_SYSCTL_CLOCK {
	SYSCTL_CLOCK_LCD,				/*!< LCD clock */
	SYSCTL_CLOCK_TIMER0,			/*!< Timer 0 clock */
	SYSCTL_CLOCK_TIMER1,			/*!< Timer 1 clock */
	SYSCTL_CLOCK_UART0,				/*!< UART 0 clock */
	SYSCTL_CLOCK_UART1,				/*!< UART 1 clock */
	SYSCTL_CLOCK_PWM0,				/*!< PWM0 clock */
	SYSCTL_CLOCK_PWM1,				/*!< PWM1 clock */
	SYSCTL_CLOCK_I2C0,				/*!< I2C0 clock */
	SYSCTL_CLOCK_UART4,				/*!< UART 4 clock */
	SYSCTL_CLOCK_RTC,				/*!< RTC clock */
	SYSCTL_CLOCK_SSP1,				/*!< SSP1 clock */
	SYSCTL_CLOCK_EMC,				/*!< EMC clock */
	SYSCTL_CLOCK_ADC,				/*!< ADC clock */
	SYSCTL_CLOCK_CAN1,				/*!< CAN1 clock */
	SYSCTL_CLOCK_CAN2,				/*!< CAN2 clock */
	SYSCTL_CLOCK_GPIO,				/*!< GPIO clock */
	SYSCTL_CLOCK_SPIFI,				/*!< SPIFI clock */
	SYSCTL_CLOCK_MCPWM,				/*!< MCPWM clock */
	SYSCTL_CLOCK_QEI,				/*!< QEI clock */
	SYSCTL_CLOCK_I2C1,				/*!< I2C1 clock */
	SYSCTL_CLOCK_SSP2,				/*!< SSP2 clock */
	SYSCTL_CLOCK_SSP0,				/*!< SSP0 clock */
	SYSCTL_CLOCK_TIMER2,			/*!< Timer 2 clock */
	SYSCTL_CLOCK_TIMER3,			/*!< Timer 3 clock */
	SYSCTL_CLOCK_UART2,				/*!< UART 2 clock */
	SYSCTL_CLOCK_UART3,				/*!< UART 3 clock */
	SYSCTL_CLOCK_I2C2,				/*!< I2C2 clock */
	SYSCTL_CLOCK_I2S,				/*!< I2S clock */
	SYSCTL_CLOCK_SDC,				/*!< SD Card interface clock */
	SYSCTL_CLOCK_GPDMA,				/*!< GP DMA clock */
	SYSCTL_CLOCK_ENET,				/*!< EMAC/Ethernet clock */
	SYSCTL_CLOCK_USB,				/*!< USB clock */
	SYSCTL_CLOCK_RSVD32,
	SYSCTL_CLOCK_RSVD33,
	SYSCTL_CLOCK_RSVD34,
	SYSCTL_CLOCK_CMP,				/*!< Comparator clock (PCONP1) */
} CHIP_SYSCTL_CLOCK_T;

/**
 * @brief	Enables power and clocking for a peripheral
 * @param	clk:	Clock to enable
 * @return	Nothing
 * @note	Only peripheral clocks that are defined in the PCONP registers of the clock
 * and power controller can be enabled and disabled with this function.
 * Some clocks need to be enabled elsewhere (ie, USB) and will return
 * false to indicate it can't be enabled with this function.
 */
void Chip_Clock_EnablePeriphClock(CHIP_SYSCTL_CLOCK_T clk);

/**
 * @brief	Disables power and clocking for a peripheral
 * @param	clk:	Clock to disable
 * @return	Nothing
 * @note	Only peripheral clocks that are defined in the PCONP registers of the clock
 * and power controller can be enabled and disabled with this function.
 * Some clocks need to be disabled elsewhere (ie, USB) and will return
 * false to indicate it can't be disabled with this function.
 */
void Chip_Clock_DisablePeriphClock(CHIP_SYSCTL_CLOCK_T clk);

/**
 * @brief	Returns power enables state for a peripheral
 * @param	clk:	Clock to check
 * @return	true if the clock is enabled, false if disabled
 */
bool Chip_Clock_IsPeripheralClockEnabled(CHIP_SYSCTL_CLOCK_T clk);

/**
 * EMC clock divider values
 */
typedef enum CHIP_SYSCTL_EMC_DIV {
	SYSCTL_EMC_DIV1 = 0,
	SYSCTL_EMC_DIV2 = 1,
} CHIP_SYSCTL_EMC_DIV_T;

/**
 * @brief	Selects a EMC divider rate
 * @param	emcDiv:	Source clock for PLL
 * @return	Nothing
 * @note	This function controls division of the clock before it is used by the EMC.
 * The EMC uses the same base clock as the CPU and the APB peripherals. The
 * EMC clock can tun at half or the same as the CPU clock. This is intended to
 * be used primarily when the CPU is running faster than the external bus can
 * support.
 */
#define Chip_Clock_SetEMCClockDiv( emcDiv) { LPC_SC->EMCCLKSEL = (uint32_t) emcDiv; }


/**
 * @brief	Get EMC divider rate
 * @return	divider value
 */
#define Chip_Clock_GetEMCClockDiv() { ((uint32_t) LPC_SC->EMCCLKSEL) + 1; }

/**
 * Selectable CPU clock sources
 */
typedef enum CHIP_SYSCTL_CCLKSRC {
	SYSCTL_CCLKSRC_SYSCLK,		/*!< Select Sysclk as the input to the CPU clock divider. */
	SYSCTL_CCLKSRC_MAINPLL,		/*!< Select the output of the Main PLL as the input to the CPU clock divider. */
} CHIP_SYSCTL_CCLKSRC_T;

/**
 * @brief	Sets the current CPU clock source
 * @param   src     Source selected
 * @return	Nothing
 * @note	When setting the clock source to the PLL, it should
 * be enabled and locked.
 */
void Chip_Clock_SetCPUClockSource(CHIP_SYSCTL_CCLKSRC_T src);

/**
 * @brief	Returns the current CPU clock source
 * @return	CPU clock source
 * @note	On 177x/8x and 407x/8x devices, this is also the peripheral
 * clock source.
 */
CHIP_SYSCTL_CCLKSRC_T Chip_Clock_GetCPUClockSource(void);

/**
 * @brief	Sets the CPU clock divider
 * @param	div:	CPU clock divider, between 1 and divider max
 * @return	Nothing
 * @note	The maximum divider for the 175x/6x is 256. The maximum divider for
 * the 177x/8x and 407x/8x is 32. Note on 175x/6x devices, the divided CPU
 * clock rate is used as the input to the peripheral clock dividers,
 * while 177x/8x and 407x/8x devices use the undivided CPU clock rate.
 */
void Chip_Clock_SetCPUClockDiv(uint32_t div);

/**
 * @brief	Gets the CPU clock divider
 * @return	CPU clock divider, between 1 and divider max
 * @note	The maximum divider for the 175x/6x is 256. The maximum divider for
 * the 177x/8x and 407x/8x is 32. Note on 175x/6x devices, the divided CPU
 * clock rate is used as the input to the peripheral clock dividers,
 * while 177x/8x and 407x/8x devices use the undivided CPU clock rate.
 */
uint32_t Chip_Clock_GetCPUClockDiv(void);

/**
 * Clock sources for the USB divider. On 175x/6x devices, only the USB
 * PLL1 can be used as an input for the USB divider
 */
typedef enum CHIP_SYSCTL_USBCLKSRC {
	SYSCTL_USBCLKSRC_SYSCLK,		/*!< SYSCLK clock as USB divider source */
	SYSCTL_USBCLKSRC_MAINPLL,		/*!< PLL0 clock as USB divider source */
	SYSCTL_USBCLKSRC_USBPLL,		/*!< PLL1 clock as USB divider source */
	SYSCTL_USBCLKSRC_RESERVED
} CHIP_SYSCTL_USBCLKSRC_T;

/**
 * @brief	Sets the USB clock divider source
 * @param	src:	USB clock divider source clock
 * @return	Nothing
 * @note	This function doesn't apply for LPC175x/6x devices. The divider must be
 * be selected with the selected source to give a valid USB clock with a
 * rate of 48MHz.
 */
void Chip_Clock_SetUSBClockSource(CHIP_SYSCTL_USBCLKSRC_T src);

/**
 * @brief	Gets the USB clock divider source
 * @return	USB clock divider source clock
 */
#define Chip_Clock_GetUSBClockSource() ( (CHIP_SYSCTL_USBCLKSRC_T) ((LPC_SC->USBCLKSEL >> 8) & 0x3) )

/**
 * @brief	Sets the USB clock divider
 * @param	div:	USB clock divider to generate 48MHz from USB source clock
 * @return	Nothing
 * @note	Divider values are between 1 and 32 (16 max for 175x/6x)
 */
void Chip_Clock_SetUSBClockDiv(uint32_t div);

/**
 * @brief	Gets the USB clock divider
 * @return	USB clock divider
 * @note	Divider values are between 1 and 32 (16 max for 175x/6x)
 */
uint32_t Chip_Clock_GetUSBClockDiv(void);

/**
 * PLL source clocks
 */
typedef enum CHIP_SYSCTL_PLLCLKSRC {
	SYSCTL_PLLCLKSRC_IRC,			/*!< PLL is sourced from the internal oscillator (IRC) */
	SYSCTL_PLLCLKSRC_MAINOSC,		/*!< PLL is sourced from the main oscillator */
	SYSCTL_PLLCLKSRC_RESERVED1,
	SYSCTL_PLLCLKSRC_RESERVED2
} CHIP_SYSCTL_PLLCLKSRC_T;

/**
 * @brief	Selects a input clock source for SYSCLK
 * @param	src:	input clock source for SYSCLK
 * @return	Nothing
 * @note	SYSCLK is used for sourcing PLL0, SPIFI FLASH, the USB clock
 * divider, and the CPU clock divider.
 */
#define Chip_Clock_SetMainPLLSource( src) { LPC_SC->CLKSRCSEL = (CHIP_SYSCTL_PLLCLKSRC_T) src };


/**
 * @brief	Returns the input clock source for SYSCLK
 * @return	input clock source for SYSCLK
 */
#define Chip_Clock_GetMainPLLSource() ( (CHIP_SYSCTL_PLLCLKSRC_T) (LPC_SC->CLKSRCSEL & 0x3) )

/**
 * @brief	Sets a clock divider for all peripherals
 * @param	div:	Divider for all peripherals, 0 = disable
 * @return	Nothing
 * @note	All the peripherals in the device use the same clock divider. The
 * divider is based on the CPU's clock rate. Use 0 to disable all
 * peripheral clocks or a divider of 1 to 15. (LPC177X/8X and 407X/8X)
 */
#define Chip_Clock_SetPCLKDiv( div) {	LPC_SC->PCLKSEL = (uint32_t)div; }

/**
 * @brief	Gets the clock divider for all peripherals
 * @return	Divider for all peripherals, 0 = disabled
 * @note	All the peripherals in the device use the same clock divider. The
 * divider is based on the CPU's clock rate. (LPC177X/8X and 407X/8X)
 */
#define Chip_Clock_GetPCLKDiv() ((uint32_t) LPC_SC->PCLKSEL & 0x1F)

/**
 * Clock sources for the SPIFI clock divider
 */
typedef enum CHIP_SYSCTL_SPIFICLKSRC {
	SYSCTL_SPIFICLKSRC_SYSCLK,		/*!< SYSCLK clock as SPIFI divider source */
	SYSCTL_SPIFICLKSRC_MAINPLL,		/*!< PLL0 clock as SPIFI divider source */
	SYSCTL_SPIFICLKSRC_USBPLL,		/*!< PLL1 clock as SPIFI divider source */
	SYSCTL_SPIFICLKSRC_RESERVED
} CHIP_SYSCTL_SPIFICLKSRC_T;

/**
 * @brief	Sets the SPIFI clock divider source
 * @param	src:	SPIFI clock divider source clock
 * @return	Nothing
 */
void Chip_Clock_SetSPIFIClockSource(CHIP_SYSCTL_SPIFICLKSRC_T src);

/**
 * @brief	Gets the SPIFI clock divider source
 * @return	SPIFI clock divider source clock
 */

#define  Chip_Clock_GetSPIFIClockSource() ((CHIP_SYSCTL_SPIFICLKSRC_T) ((LPC_SC->SPIFICLKSEL >> 8) & 0x3))

/**
 * @brief	Sets the SPIFI clock divider
 * @param	div:	SPIFI clock divider, 0 to disable
 * @return	Nothing
 * @note	Divider values are between 1 and 31
 */
void Chip_Clock_SetSPIFIClockDiv(uint32_t div);

/**
 * @brief	Gets the SPIFI clock divider
 * @return	SPIFI clock divider
 * @note	Divider values are between 1 and 31, 0 is disabled
 */
#define Chip_Clock_GetSPIFIClockDiv() ((uint32_t) LPC_SC->SPIFICLKSEL & 0x1F) 

/**
 * @brief	Set the LCD clock prescaler
 * @param	div:	Divider value, minimum of 1
 * @return	Nothing
 */
#define Chip_Clock_SetLCDClockDiv(div)  { LPC_SC->LCD_CFG = ((uint32_t)div - 1) }

/**
 * @brief	Get the LCD clock prescaler
 * @return	Current divider value
 */
#define Chip_Clock_GetLCDClockDiv() ((uint32_t) (LPC_SC->LCD_CFG & 0x1F) + 1) 

/**
 * Clock sources for the CLKOUT pin
 */
typedef enum {
	SYSCTL_CLKOUTSRC_CPU,			/*!< CPU clock as CLKOUT source */
	SYSCTL_CLKOUTSRC_MAINOSC,		/*!< Main oscillator clock as CLKOUT source */
	SYSCTL_CLKOUTSRC_IRC,			/*!< IRC oscillator clock as CLKOUT source */
	SYSCTL_CLKOUTSRC_USB,			/*!< USB clock as CLKOUT source */
	SYSCTL_CLKOUTSRC_RTC,			/*!< RTC clock as CLKOUT source */
	SYSCTL_CLKOUTSRC_SPIFI,		/*!< SPIFI clock as CLKOUT source */
	SYSCTL_CLKOUTSRC_WATCHDOGOSC,	/*!< Watchdog oscillator as CLKOUT source */
	SYSCTL_CLKOUTSRC_RESERVED3
} CHIP_SYSCTL_CLKOUTSRC_T;

/**
 * @brief	Selects a source clock and divider rate for the CLKOUT pin
 * @param	src:	source selected
 * @param	div:	Divider for the clock source on CLKOUT, 1 to 16
 * @return	Nothing
 * @note	This function will disable the CLKOUT signal if its enabled. Use
 * Chip_Clock_EnableCLKOUT to re-enable CLKOUT after a call to this
 * function.
 */
void Chip_Clock_SetCLKOUTSource(CHIP_SYSCTL_CLKOUTSRC_T src, uint32_t div);

/**
 * @brief	Enables the clock on the CLKOUT pin
 * @return	Nothing
 */
#define Chip_Clock_EnableCLKOUT() { LPC_SC->CLKOUTCFG |= (1 << 8); }

/**
 * @brief	Disables the clock on the CLKOUT pin
 * @return	Nothing
 */
#define Chip_Clock_DisableCLKOUT() { LPC_SC->CLKOUTCFG &= ~(1 << 8); }

/**
 * @brief	Returns the CLKOUT activity indication status
 * @return	true if CLKOUT is enabled, false if disabled and stopped
 * @note	CLKOUT activity indication. Reads as true when CLKOUT is
 * enabled. Read as false when CLKOUT has been disabled via
 * the CLKOUT_EN bit and the clock has completed being stopped.
 */
#define  Chip_Clock_IsCLKOUTEnabled() { (bool) ((LPC_SC->CLKOUTCFG & (1 << 9)) != 0); }

/**
 * @brief	Returns the main oscillator clock rate
 * @return	main oscillator clock rate
 */
#define Chip_Clock_GetMainOscRate() ( (uint32_t)OscRateIn )

/**
 * @brief	Returns the internal oscillator (IRC) clock rate
 * @return	internal oscillator (IRC) clock rate
 */
#define  Chip_Clock_GetIntOscRate() (SYSCTL_IRC_FREQ)

/**
 * @brief	Returns the RTC oscillator clock rate
 * @return	RTC oscillator clock rate
 */
#define  Chip_Clock_GetRTCOscRate() { (uint32_t)RTCOscRateIn; }

/**
 * @brief	Returns the current SYSCLK clock rate
 * @return	SYSCLK clock rate
 * @note	SYSCLK is used for sourcing PLL0, SPIFI FLASH, the USB clock
 * divider, and the CPU clock divider.
 */
uint32_t Chip_Clock_GetSYSCLKRate(void);

/**
 * @brief	Return Main PLL (PLL0) input clock rate
 * @return	PLL0 input clock rate
 */
#define Chip_Clock_GetMainPLLInClockRate()  ((uint32_t) Chip_Clock_GetSYSCLKRate())

/**
 * @brief	Return PLL0 (Main PLL) output clock rate
 * @return	PLL0 output clock rate
 */
uint32_t Chip_Clock_GetMainPLLOutClockRate(void);

/**
 * @brief	Return USB PLL input clock rate
 * @return	USB PLL input clock rate
 */
#define Chip_Clock_GetUSBPLLInClockRate() ( (uint32_t) Chip_Clock_GetMainOscRate() )

/**
 * @brief	Return USB PLL output clock rate
 * @return	USB PLL output clock rate
 */
uint32_t Chip_Clock_GetUSBPLLOutClockRate(void);

/**
 * @brief	Return main clock rate
 * @return	main clock rate
 */
uint32_t Chip_Clock_GetMainClockRate(void);

/**
 * @brief	Return system clock rate
 * @return	system clock rate
 */
uint32_t Chip_Clock_GetSystemClockRate(void);

/**
 * @brief	Gets the USB clock (USB_CLK) rate
 * @return	USB clock (USB_CLK) clock rate
 * @note	The clock source and divider are used to generate the USB clock rate.
 */
uint32_t Chip_Clock_GetUSBClockRate(void);


/**
 * @brief	Returns the SPIFI clock rate
 * @return	SPIFI clock clock rate
 */
uint32_t Chip_Clock_GetSPIFIClockRate(void);

/**
 * @brief	Returns clock rate for EMC
 * @return	Clock rate for the peripheral
 */
#define Chip_Clock_GetEMCClockRate() ( (uint32_t) Chip_Clock_GetSystemClockRate() / Chip_Clock_GetEMCClockDiv() );

/**
 * @brief	Returns clock rate for a peripheral (from peripheral clock)
 * @param	clk:	Clock to get rate of
 * @return	Clock rate for the peripheral
 * @note	This covers most common peripheral clocks, but not every clock
 * in the system. LPC177x/8x and LPC407x/8x devices use the same
 * clock for all periphreals, while the LPC175x/6x have unique
 * dividers (except to RTC ) that may alter the peripheral clock rate.
 */
uint32_t Chip_Clock_GetPeripheralClockRate(void);

/**
 * @brief	Returns clock rate for Ethernet
 * @return	Clock rate for the peripheral
 */
#define  Chip_Clock_GetENETClockRate()  {( uint32_t) Chip_Clock_GetSystemClockRate(); }

/**
 * @brief	Returns clock rate for GPDMA
 * @return	Clock rate for the peripheral
 */
#define  Chip_Clock_GetGPDMAClockRate()  { (uint32_t) Chip_Clock_GetSystemClockRate(); }

//=============================================================================
/* Returns clock for the peripheral block */

CHIP_SYSCTL_CLOCK_T Chip_SSP_GetClockIndex(LPC_SSP_TypeDef *pSSP);
//=============================================================================

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __CLOCK_17XX_40XX_H_ */

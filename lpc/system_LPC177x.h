#ifndef __SYSTEM_LPC177x_8x_H
#define __SYSTEM_LPC177x_8x_H

extern void UserStartInit(void);

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

extern unsigned long SystemCoreClock;      // System Clock Frequency (Core Clock)
extern unsigned long PeripheralClock;	   // Peripheral Clock Frequency (Pclk)
extern unsigned long EMCClock;		   // EMC Clock
extern unsigned long USBClock;		   // USB Frequency

extern void SystemInit (void);

extern void SystemCoreClockUpdate (void);

/*----------------------------------------------------------------------------
 * Define clocks
 *----------------------------------------------------------------------------*/
#define XTAL        (12000000UL)        // Oscillator frequency
#define OSC_CLK     (      XTAL)        // Main oscillator frequency
#define RTC_CLK     (   32768UL)        // RTC oscillator frequency
#define IRC_OSC     (12000000UL)        // Internal RC oscillator frequency
#define WDT_OSC	    (  500000UL)	// Internal WDT oscillator frequency

/********************************************************************
* Power Control for Peripherals Definitions
**********************************************************************/
/** LCD controller power/clock control bit */
#define CLKPWR_PCONP_PCLCD	((uint32_t)(1<<0))

/** Timer/Counter 0 power/clock control bit */
#define	CLKPWR_PCONP_PCTIM0	((uint32_t)(1<<1))

/* Timer/Counter 1 power/clock control bit */
#define	CLKPWR_PCONP_PCTIM1	((uint32_t)(1<<2))

/** UART0 power/clock control bit */
#define	CLKPWR_PCONP_PCUART0  	((uint32_t)(1<<3))

/** UART1 power/clock control bit */
#define	CLKPWR_PCONP_PCUART1  	((uint32_t)(1<<4))

/** PWM0 power/clock control bit */
#define CLKPWR_PCONP_PCPWM0	((uint32_t)(1<<5))

/** PWM1 power/clock control bit */
#define	CLKPWR_PCONP_PCPWM1	((uint32_t)(1<<6))

/** The I2C0 interface power/clock control bit */
#define	CLKPWR_PCONP_PCI2C0	((uint32_t)(1<<7))

/** UART4 power/clock control bit */
#define CLKPWR_PCONP_PCUART4	((uint32_t)(1<<8))

/** The RTC power/clock control bit */
#define	CLKPWR_PCONP_PCRTC  	((uint32_t)(1<<9))

/** The SSP1 interface power/clock control bit */
#define	CLKPWR_PCONP_PCSSP1	 ((uint32_t)(1<<10))

/** External Memory controller power/clock control bit */
#define CLKPWR_PCONP_PCEMC	((uint32_t)(1<<11))

/** A/D converter 0 (ADC0) power/clock control bit */
#define	CLKPWR_PCONP_PCADC  	((uint32_t)(1<<12))

/** CAN Controller 1 power/clock control bit */
#define	CLKPWR_PCONP_PCAN1  	((uint32_t)(1<<13))

/** CAN Controller 2 power/clock control bit */
#define	CLKPWR_PCONP_PCAN2 	((uint32_t)(1<<14))

/** GPIO power/clock control bit */
#define	CLKPWR_PCONP_PCGPIO 	((uint32_t)(1<<15))

/** Motor Control PWM */
#define CLKPWR_PCONP_PCMCPWM 	((uint32_t)(1<<17))

/** Quadrature Encoder Interface power/clock control bit */
#define CLKPWR_PCONP_PCQEI 	((uint32_t)(1<<18))

/** The I2C1 interface power/clock control bit */
#define	CLKPWR_PCONP_PCI2C1  	((uint32_t)(1<<19))

/** The SSP2 interface power/clock control bit */
#define CLKPWR_PCONP_PCSSP2	((uint32_t)(1<<20))

/** The SSP0 interface power/clock control bit */
#define	CLKPWR_PCONP_PCSSP0	((uint32_t)(1<<21))

/** Timer 2 power/clock control bit */
#define	CLKPWR_PCONP_PCTIM2	((uint32_t)(1<<22))

/** Timer 3 power/clock control bit */
#define	CLKPWR_PCONP_PCTIM3	((uint32_t)(1<<23))

/** UART 2 power/clock control bit */
#define	CLKPWR_PCONP_PCUART2  	((uint32_t)(1<<24))

/** UART 3 power/clock control bit */
#define	CLKPWR_PCONP_PCUART3  	((uint32_t)(1<<25))

/** I2C interface 2 power/clock control bit */
#define	CLKPWR_PCONP_PCI2C2	((uint32_t)(1<<26))

/** I2S interface power/clock control bit*/
#define	CLKPWR_PCONP_PCI2S  	((uint32_t)(1<<27))

/** SD card interface power/clock control bit */
#define CLKPWR_PCONP_PCSDC	((uint32_t)(1<<28))

/** GP DMA function power/clock control bit*/
#define	 CLKPWR_PCONP_PCGPDMA  	((uint32_t)(1<<29))

/** Ethernet block power/clock control bit*/
#define	 CLKPWR_PCONP_PCENET	((uint32_t)(1<<30))

/** USB interface power/clock control bit*/
#define	 CLKPWR_PCONP_PCUSB  	((uint32_t)(1<<31))

//Здает группы  приоритета прерываний gr - группа, sub - подгруппа

//#define  IntPriority(gr, sub)    ((uint32_t)(gr<<3)+(sub)) 

#ifdef __cplusplus
}
#endif

#endif // __SYSTEM_LPC177x_8x_H

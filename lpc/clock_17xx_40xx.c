/*
 * @brief	LPC17xx/40xx System and Control driver
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.*/

#include "core_cm4.h"
#include "sysctl_17xx_40xx.h"
#include "clock_17xx_40xx.h"

const uint32_t OscRateIn = 12000000;
const uint32_t RTCOscRateIn = 32768;
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Enables or connects a PLL */
void Chip_Clock_EnablePLL(CHIP_SYSCTL_PLL_T PLLNum, uint32_t flags) {
	uint32_t temp;

	temp = LPC_SC->PLL[PLLNum].PLLCON;
	temp |= flags;
	LPC_SC->PLL[PLLNum].PLLCON = temp;
	Chip_Clock_FeedPLL(PLLNum);
}

/* Disables or disconnects a PLL */
void Chip_Clock_DisablePLL(CHIP_SYSCTL_PLL_T PLLNum, uint32_t flags) {
	uint32_t temp;

	temp = LPC_SC->PLL[PLLNum].PLLCON;
	temp &= ~flags;
	LPC_SC->PLL[PLLNum].PLLCON = temp;
	Chip_Clock_FeedPLL(PLLNum);
}

/* Sets up a PLL */
void Chip_Clock_SetupPLL(CHIP_SYSCTL_PLL_T PLLNum, uint32_t msel, uint32_t psel) {
	uint32_t PLLcfg;

	PLLcfg = (msel) | (psel << 5);


	LPC_SC->PLL[PLLNum].PLLCFG = PLLcfg;
	LPC_SC->PLL[PLLNum].PLLCON = 0x1;
	Chip_Clock_FeedPLL(PLLNum);
}

/* Enables power and clocking for a peripheral */
void Chip_Clock_EnablePeriphClock(CHIP_SYSCTL_CLOCK_T clk) {
	uint32_t bs = (uint32_t) clk;

	if (bs >= 32) {
		LPC_SC->PCONP1 |= (1 << (bs - 32));
	}
	else {
		LPC_SC->PCONP |= (1 << bs);
	}
}

/* Disables power and clocking for a peripheral */
void Chip_Clock_DisablePeriphClock(CHIP_SYSCTL_CLOCK_T clk) {
	uint32_t bs = (uint32_t) clk;

	if (bs >= 32) {
		LPC_SC->PCONP1 &= ~(1 << (bs - 32));
	}
	else {
		LPC_SC->PCONP |= ~(1 << bs);
	}
}

/* Returns power enables state for a peripheral */
bool Chip_Clock_IsPeripheralClockEnabled(CHIP_SYSCTL_CLOCK_T clk)
{
	uint32_t bs = (uint32_t) clk;

	if (bs >= 32) {
		bs = LPC_SC->PCONP1 & (1 << (bs - 32));
	}
	else {
		bs = LPC_SC->PCONP & (1 << bs);
	}

	return (bool) (bs != 0);
}

/* Sets the current CPU clock source */
void Chip_Clock_SetCPUClockSource(CHIP_SYSCTL_CCLKSRC_T src)
{
	/* LPC177x/8x and 407x/8x CPU clock source is based on CCLKSEL */
	if (src == SYSCTL_CCLKSRC_MAINPLL) {
		/* Connect PLL0 */
		LPC_SC->CCLKSEL |= (1 << 8);
	}
	else {
		LPC_SC->CCLKSEL &= ~(1 << 8);
	}
}

/* Returns the current CPU clock source */
CHIP_SYSCTL_CCLKSRC_T Chip_Clock_GetCPUClockSource(void)
{
	CHIP_SYSCTL_CCLKSRC_T src;

	/* LPC177x/8x and 407x/8x CPU clock source is based on CCLKSEL */
	if (LPC_SC->CCLKSEL & (1 << 8)) {
		src = SYSCTL_CCLKSRC_MAINPLL;
	}
	else {
		src = SYSCTL_CCLKSRC_SYSCLK;
	}


	return src;
}

/* Selects the CPU clock divider */
void Chip_Clock_SetCPUClockDiv(uint32_t div)
{
	uint32_t temp;

	/* Save state of CPU clock source bit */
	temp = LPC_SC->CCLKSEL & (1 << 8);
	LPC_SC->CCLKSEL = temp | div;

}

/* Gets the CPU clock divider */
uint32_t Chip_Clock_GetCPUClockDiv(void)
{
	return LPC_SC->CCLKSEL & 0x1F;
}


/* Selects the USB clock divider source */
void Chip_Clock_SetUSBClockSource(CHIP_SYSCTL_USBCLKSRC_T src)
{
	uint32_t temp;

	/* Mask out current source, but keep divider */
	temp = LPC_SC->USBCLKSEL & ~(0x3 << 8);
	LPC_SC->USBCLKSEL = temp | (((uint32_t) src) << 8);
}



/* Sets the USB clock divider */
void Chip_Clock_SetUSBClockDiv(uint32_t div)
{
	uint32_t temp;

	/* Mask out current divider */

	temp = LPC_SC->USBCLKSEL & ~(0x1F);
	LPC_SC->USBCLKSEL = temp | div;
}

/* Gets the USB clock divider */
uint32_t Chip_Clock_GetUSBClockDiv(void)
{
	return (LPC_SC->USBCLKSEL & 0x1F) + 1;
}


/* Selects a source clock and divider rate for the CLKOUT pin */
void Chip_Clock_SetCLKOUTSource(CHIP_SYSCTL_CLKOUTSRC_T src,
								uint32_t div)
{
	uint32_t temp;

	temp = LPC_SC->CLKOUTCFG & ~0x1FF;
	temp |= ((uint32_t) src) | ((div - 1) << 4);
	LPC_SC->CLKOUTCFG = temp;
}

/* Returns the current SYSCLK clock rate */
uint32_t Chip_Clock_GetSYSCLKRate(void)
{
	/* Determine clock input rate to SYSCLK based on input selection */
	switch (Chip_Clock_GetMainPLLSource()) {
	case (uint32_t) SYSCTL_PLLCLKSRC_IRC:
		return Chip_Clock_GetIntOscRate();

	case (uint32_t) SYSCTL_PLLCLKSRC_MAINOSC:
		return Chip_Clock_GetMainOscRate();

	}
	return 0;
}

/* Returns the main PLL output clock rate */
uint32_t Chip_Clock_GetMainPLLOutClockRate(void)
{
	uint32_t clkhr = 0;

	if (Chip_Clock_IsMainPLLEnabled()) {
		uint32_t msel;

		/* PLL0 rate is (FIN * MSEL) */
		msel = 1 + (LPC_SC->PLL[SYSCTL_MAIN_PLL].PLLCFG & 0x1F);
		clkhr = (Chip_Clock_GetMainPLLInClockRate() * msel);
	}

	return (uint32_t) clkhr;
}

/* Get USB output clock rate */
uint32_t Chip_Clock_GetUSBPLLOutClockRate(void)
{
	uint32_t clkhr = 0;

	/* Only valid if enabled */
	if (Chip_Clock_IsUSBPLLEnabled()) {
		uint32_t msel;

		/* PLL1 input clock (FIN) is always main oscillator */
		/* PLL1 rate is (FIN * MSEL) */
		msel = 1 + (LPC_SC->PLL[SYSCTL_USB_PLL].PLLCFG & 0x1F);
		clkhr = (Chip_Clock_GetUSBPLLInClockRate() * msel);
	}

	return (uint32_t) clkhr;
}

/* Get the main clock rate */
/* On 175x/6x devices, this is the input clock to the CPU divider.
   Additionally, on 177x/8x and 407x/8x devices, this is also the
   input clock to the peripheral divider. */
uint32_t Chip_Clock_GetMainClockRate(void)
{
	switch (Chip_Clock_GetCPUClockSource()) {
	case SYSCTL_CCLKSRC_MAINPLL:
		return Chip_Clock_GetMainPLLOutClockRate();

	case SYSCTL_CCLKSRC_SYSCLK:
		return Chip_Clock_GetSYSCLKRate();

	default:
		return 0;
	}
}

/* Get CCLK rate */
uint32_t Chip_Clock_GetSystemClockRate(void)
{
	return Chip_Clock_GetMainClockRate() / Chip_Clock_GetCPUClockDiv();
}

/* Returns the USB clock (USB_CLK) rate */
uint32_t Chip_Clock_GetUSBClockRate(void)
{
	uint32_t div, clkrate;

	/* Get clock from source drving USB */
	switch (Chip_Clock_GetUSBClockSource()) {
	case SYSCTL_USBCLKSRC_SYSCLK:
	default:
		clkrate = Chip_Clock_GetSYSCLKRate();
		break;

	case SYSCTL_USBCLKSRC_MAINPLL:
		clkrate = Chip_Clock_GetMainPLLOutClockRate();
		break;

	case SYSCTL_USBCLKSRC_USBPLL:
		clkrate = Chip_Clock_GetUSBPLLOutClockRate();
		break;
	}

	div = Chip_Clock_GetUSBClockDiv();

	return clkrate / div;
}

/* Selects the SPIFI clock divider source */
void Chip_Clock_SetSPIFIClockSource(CHIP_SYSCTL_SPIFICLKSRC_T src)
{
	uint32_t temp;

	/* Mask out current source, but keep divider */
	temp = LPC_SC->SPIFICLKSEL & ~(0x3 << 8);
	LPC_SC->SPIFICLKSEL = temp | (((uint32_t) src) << 8);
}

/* Sets the SPIFI clock divider */
void Chip_Clock_SetSPIFIClockDiv(uint32_t div)
{
	uint32_t temp;

	/* Mask out current divider */
	temp = LPC_SC->SPIFICLKSEL & ~(0x1F);
	LPC_SC->SPIFICLKSEL = temp | div;
}

/* Returns the SPIFI clock rate */
uint32_t Chip_Clock_GetSPIFIClockRate(void)
{
	uint32_t div, clkrate;

	/* Get clock from source drving USB */
	switch (Chip_Clock_GetSPIFIClockSource()) {
	case SYSCTL_SPIFICLKSRC_SYSCLK:
	default:
		clkrate = Chip_Clock_GetSYSCLKRate();
		break;

	case SYSCTL_SPIFICLKSRC_MAINPLL:
		clkrate = Chip_Clock_GetMainPLLOutClockRate();
		break;

	case SYSCTL_SPIFICLKSRC_USBPLL:
		clkrate = Chip_Clock_GetUSBPLLOutClockRate();
		break;
	}

	div = Chip_Clock_GetSPIFIClockDiv();

	return clkrate / div;
}


/* Returns the clock rate for all peripherals */
uint32_t Chip_Clock_GetPeripheralClockRate(void)
{
	uint32_t clkrate = 0, div;

	/* Get divider, a divider of 0 means the clock is disabled */
	div = Chip_Clock_GetPCLKDiv();
	if (div != 0) {
		/* Derived from periperhal clock input and peripheral clock divider */
		clkrate = Chip_Clock_GetMainClockRate() / div;
	}

	return clkrate;
}


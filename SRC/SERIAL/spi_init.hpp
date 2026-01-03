#pragma once

#include "LPC407x_8x_177x_8x.h"

namespace SPI_Config {
  
  constexpr unsigned int CR0_SCR(unsigned int scr) { return ((scr & 0xFF) << 8); }  
  constexpr unsigned int CR0_DSS(unsigned int bits){ return bits - 1; }
  constexpr unsigned int CR0_BITMASK	 = 0xFFFFUL;
  constexpr unsigned int CPSR_BITMASK	 = 0xFFUL; 
  constexpr unsigned int CR0_CPOL_HI     = 1UL << 6;
  constexpr unsigned int CR0_CPHA_SECOND = 1UL << 7;
  constexpr unsigned int CR1_SSP_EN	 = 1UL << 1;
  constexpr unsigned int SR_BSY          = 1UL << 4;
  constexpr unsigned int SR_RNE          = 1UL << 2;
  constexpr unsigned int SR_TFE          = 1UL << 0;
  constexpr unsigned int SR_TNF          = 1UL << 1;
  
  void inline set_spi_clock(LPC_SSP_TypeDef* pLPC_SSP, unsigned int Hz, unsigned long P_Clock)
  {
    unsigned int prescale = 2;
    unsigned int cr0_div  = 0;
    unsigned int cmp_clk  = 0xFFFFFFFF;
    unsigned int ssp_clk  = P_Clock;
    
    while (cmp_clk > Hz)
    {
      cmp_clk = ssp_clk / ((cr0_div + 1) * prescale);
      if (cmp_clk > Hz)
      {
        cr0_div++;
        if (cr0_div > 0xFF)
        {
          cr0_div = 0;
          prescale += 2;
        }
      }
    }      
    pLPC_SSP->CR0 &= (~CR0_SCR(0xFF)) & CR0_BITMASK;
    pLPC_SSP->CR0 |= (CR0_SCR(cr0_div)) & CR0_BITMASK;
    pLPC_SSP->CPSR = prescale & CPSR_BITMASK;  
  }
  
}
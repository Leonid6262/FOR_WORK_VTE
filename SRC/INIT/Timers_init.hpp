#pragma once
#include "LPC407x_8x_177x_8x.h"
#include "system_LPC177x.h"

class CSET_TIMER {
public:
  static void initTimers() {
    // Включение clock
    LPC_SC->PCONP |= CLKPWR_PCONP_PCTIM0;
    LPC_SC->PCONP |= CLKPWR_PCONP_PCTIM1;
    LPC_SC->PCONP |= CLKPWR_PCONP_PCTIM2;
    LPC_SC->PCONP |= CLKPWR_PCONP_PCTIM3;
    
    // Частота
    LPC_TIM0->PR =  6 - 1;  // 10 МГц, 1 тик = 0.1 мкс
    LPC_TIM1->PR = 60 - 1;  // 1 МГц, 1 тик = 1.0 мкс
    LPC_TIM2->PR = 60 - 1;  // 1 МГц
    LPC_TIM3->PR = 60 - 1;  // 1 МГц
    
    // Включение TIM0
    LPC_TIM0->TCR |= 0x1;
  }
};

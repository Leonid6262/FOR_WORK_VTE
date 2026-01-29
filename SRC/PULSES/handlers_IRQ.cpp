#include "handlers_IRQ.hpp"

#include <cmath>

#include "LPC407x_8x_177x_8x.h"
#include "system_LPC177x.h"

CProxyHandlerTIMER::CProxyHandlerTIMER() {};  // Прокси для доступа к Timers IRQ Handlers

CProxyHandlerTIMER& CProxyHandlerTIMER::getInstance() {
  static CProxyHandlerTIMER instance;
  return instance;
}

void CProxyHandlerTIMER::set_pointers(CSIFU* pPuls) {
  this->pPuls = pPuls;
}

extern "C" {
  // Compare таймера 3 используются для выдачи классической последовательности СИФУ
  void TIMER3_IRQHandler(void) {
    CProxyHandlerTIMER& rProxy = CProxyHandlerTIMER::getInstance();
    
    unsigned int IRQ = LPC_TIM3->IR;
    
    // Rising puls
    if (IRQ & rProxy.IRQ_MR0)  // Прерывание по Compare с MR0 (P->1)
    {
      LPC_TIM3->IR |= rProxy.IRQ_MR0;
      rProxy.pPuls->rising_puls();   // Фронт ИУ     
    }
    // Faling puls
    if (IRQ & rProxy.IRQ_MR1)  // Прерывание по Compare с MR1 (P->0)
    {
      LPC_TIM3->IR |= rProxy.IRQ_MR1;
      rProxy.pPuls->faling_puls();  // Спад ИУ
    }
  }
}

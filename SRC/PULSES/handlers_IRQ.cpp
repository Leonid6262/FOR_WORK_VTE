#include "handlers_IRQ.hpp"
#include "system_LPC177x.h"

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
    
    unsigned int IRQ = LPC_TIM3->IR;
    
    // Rising puls
    if (IRQ & CProxyHandlerTIMER::IRQ_MR0)  // Прерывание по Compare с MR0 (Puls->1)
    {
      LPC_TIM3->IR |= CProxyHandlerTIMER::IRQ_MR0;
      CProxyHandlerTIMER::getInstance().pPuls->rising_puls();   // Фронт ИУ     
    }
    // Faling puls
    if (IRQ & CProxyHandlerTIMER::IRQ_MR1)  // Прерывание по Compare с MR1 (Puls->0)
    {
      LPC_TIM3->IR |= CProxyHandlerTIMER::IRQ_MR1;
      CProxyHandlerTIMER::getInstance().pPuls->faling_puls();  // Спад ИУ
    }
  }
}



#pragma once

#include "SIFU.hpp"

/* Синглтон-прокси для доступа к TIMER_IRQHandler() */

class CProxyHandlerTIMER {
  
public:  
  CSIFU* pPuls;
  static CProxyHandlerTIMER& getInstance(); 
  void set_pointers(CSIFU*);
  
  static constexpr unsigned int IRQ_MR0 = 0x01;
  static constexpr unsigned int IRQ_MR1 = 0x02;
  
private:
    CProxyHandlerTIMER() = default; 
    CProxyHandlerTIMER(const CProxyHandlerTIMER&) = delete;
    CProxyHandlerTIMER& operator=(const CProxyHandlerTIMER&) = delete;    
};



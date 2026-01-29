#pragma once

#include "SIFU.hpp"
#include "adc.hpp"

/*
  Синглтон-прокси для доступа к TIMER_IRQHandler()
*/

class CProxyHandlerTIMER 
{
public:
  
  static CProxyHandlerTIMER& getInstance(); 

  CSIFU* pPuls;
 
  void set_pointers(CSIFU*);
  
  const unsigned int IRQ_MR0  = 0x01;
  const unsigned int IRQ_MR1  = 0x02;
  
private:

    CProxyHandlerTIMER(); 
    CProxyHandlerTIMER(const CProxyHandlerTIMER&) = delete;
    CProxyHandlerTIMER& operator=(const CProxyHandlerTIMER&) = delete; 
    
};

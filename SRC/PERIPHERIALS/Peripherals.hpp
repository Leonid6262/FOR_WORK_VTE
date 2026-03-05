#pragma once
#include "LPC407x_8x_177x_8x.h"

class CGPIO
{ 
private:  
  LPC_GPIO_TypeDef* port;
public:
  
  CGPIO(LPC_GPIO_TypeDef* p) : port(p) {}
  
  void set(unsigned int mask)   { port->SET = mask; }
  void clr(unsigned int mask)   { port->CLR = mask; }
  void dirOut(unsigned int mask){ port->DIR |= mask; }
  void dirIn(unsigned int mask) { port->DIR &= ~mask; }
  
};


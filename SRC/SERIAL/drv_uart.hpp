#pragma once
#include "LPC407x_8x_177x_8x.h"

class CUART_DRIVER {
private:
  LPC_UART_TypeDef* UART;
  static constexpr unsigned int THRE_F = 1UL << 5;     // THRE flag. FIFO empty
  static constexpr unsigned int THRE_I = 1UL << 1;     // THRE interrupt. FIFO empty
  static constexpr unsigned int RDR_F  = 1UL << 0;     // RDR flag. Есть данные для чтения
public:
  CUART_DRIVER(LPC_UART_TypeDef* UART) : UART(UART) {}
  
__intrinsic void writeByte(unsigned char data)   { UART->THR = data; }
__intrinsic unsigned char readByte()             { return UART->RBR; }

__intrinsic bool statusTHRE_F() { return (UART->LSR & THRE_F); }
__intrinsic bool statusRDR_F()  { return (UART->LSR & RDR_F);  }

__intrinsic bool statusTHRE_I()  { 
  unsigned int IRQ = UART->IIR;
  return ((IRQ & THRE_I) == THRE_I); 
}

__intrinsic void clrIER()        { UART->IER = 0; }
__intrinsic void EnableTHRE_I()  { UART->IER |= THRE_I;    }
__intrinsic void DisableTHRE_I() { UART->IER &= ~(THRE_I); }

};


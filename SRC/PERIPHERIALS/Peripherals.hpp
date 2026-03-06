#pragma once
#include "LPC407x_8x_177x_8x.h"

namespace P {
  // Указатели на структуры регистров GPIO
  constexpr LPC_GPIO_TypeDef* G0 = LPC_GPIO0;
  constexpr LPC_GPIO_TypeDef* G1 = LPC_GPIO1;
  constexpr LPC_GPIO_TypeDef* G2 = LPC_GPIO2;
  constexpr LPC_GPIO_TypeDef* G3 = LPC_GPIO3;
  constexpr LPC_GPIO_TypeDef* G4 = LPC_GPIO4;
  constexpr LPC_GPIO_TypeDef* G5 = LPC_GPIO5;

  // Биты физических связей GPIO
  static constexpr unsigned short B_ULED      = 9;        // Бит U-LED
  static constexpr unsigned short B_Q1VF      = 13;       // Бит Q1VF
  static constexpr unsigned short B0_PORT_OUT = 24;       // 1-й бит DOUT порта GPIO2  
  static constexpr unsigned int B0_PORT_IN    = 15;       // 0-й бит порта. Начало Pi0 в PORT2 контроллера
  
  static constexpr unsigned int OUT_E = 1UL << 8;
  static constexpr unsigned int HOLD = 1UL << 7;
  
  static constexpr unsigned int SUM1 = 0x02;              // Port1:2 PWM0:1
  static constexpr unsigned int SUM2 = 0x03;              // Port1:3 PWM0:2
  static constexpr unsigned int FIRST_PULS_PORT = 16;               // 1-й импульс в порту
  static constexpr unsigned int OFF_PULSES = 0x3F<<FIRST_PULS_PORT; // Все импульсы 
    
  // Указатель на структур регистров IOCON
  constexpr LPC_IOCON_TypeDef* IOCON = LPC_IOCON;
  // System Control
  constexpr LPC_SC_TypeDef* SC = LPC_SC;
  
}

// Управление GPIO
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
  
  unsigned int get_pin() { return port->PIN; }  
};


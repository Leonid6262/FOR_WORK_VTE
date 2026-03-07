#pragma once
#include "LPC407x_8x_177x_8x.h"
#include "Peripherals.hpp"
#include "ControlBits.hpp"

namespace P {
  // Указатели на структуры регистров GPIO
  constexpr LPC_GPIO_TypeDef* G0 = LPC_GPIO0;
  constexpr LPC_GPIO_TypeDef* G1 = LPC_GPIO1;
  constexpr LPC_GPIO_TypeDef* G2 = LPC_GPIO2;
  constexpr LPC_GPIO_TypeDef* G3 = LPC_GPIO3;
  constexpr LPC_GPIO_TypeDef* G4 = LPC_GPIO4;
  constexpr LPC_GPIO_TypeDef* G5 = LPC_GPIO5;
    
  constexpr LPC_IOCON_TypeDef* IOCON = LPC_IOCON;  // IOCON 
  constexpr LPC_SC_TypeDef* SC = LPC_SC;           // System Control  
  
  constexpr LPC_DAC_TypeDef* DAC = LPC_DAC;        // DAC0
  constexpr LPC_PWM_TypeDef* PWM1 = LPC_PWM1;      // PWM1 DAC_PWM
  
  constexpr LPC_PWM_TypeDef* puls_pwm = LPC_PWM0;  // PWM ИУ
}

// Включение питания (PCONP)
class CPCONP {
public:
  LPC_SC_TypeDef* sc;
  CPCONP(LPC_SC_TypeDef* sc) : sc(sc) {}
};

// Инициализация функционала (IOCON)
class CIOCON {
public:
  LPC_IOCON_TypeDef* base;
  CIOCON(LPC_IOCON_TypeDef* base) : base(base) {}
};

// Управление GPIO
class CGPIO { 
private:  
  LPC_GPIO_TypeDef* port;
   
public:
  
  CGPIO(LPC_GPIO_TypeDef* port) : port(port) {}
  
  void set(unsigned int mask)   { port->SET = mask; }
  void clr(unsigned int mask)   { port->CLR = mask; }
  void dirOut(unsigned int mask){ port->DIR |= mask; }
  void dirIn(unsigned int mask) { port->DIR &= ~mask; }  
  unsigned int get_pin()        { return port->PIN; }
  
};


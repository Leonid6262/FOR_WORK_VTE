#pragma once
#include "LPC407x_8x_177x_8x.h"
#include "Peripherals.hpp"
#include "ControlBits.hpp"

namespace P { 
  constexpr LPC_GPIO_TypeDef* G0 = LPC_GPIO0;
  constexpr LPC_GPIO_TypeDef* G1 = LPC_GPIO1;
  constexpr LPC_GPIO_TypeDef* G2 = LPC_GPIO2;
  constexpr LPC_GPIO_TypeDef* G3 = LPC_GPIO3;
  constexpr LPC_GPIO_TypeDef* G4 = LPC_GPIO4;
  constexpr LPC_GPIO_TypeDef* G5 = LPC_GPIO5;
  
  constexpr LPC_TIM_TypeDef* SYST = LPC_TIM0;
  constexpr LPC_TIM_TypeDef* TIM1 = LPC_TIM1;
  constexpr LPC_TIM_TypeDef* TIM2 = LPC_TIM2;
  constexpr LPC_TIM_TypeDef* TIM3 = LPC_TIM3;
  
  constexpr LPC_UART_TypeDef* UART0 = LPC_UART0;
  constexpr LPC_UART_TypeDef* UART2 = LPC_UART2;
  constexpr LPC_UART_TypeDef* UART3 = LPC_UART3;
  
  constexpr LPC_SSP_TypeDef* SPI0 = LPC_SSP0;
  constexpr LPC_SSP_TypeDef* SPI1 = LPC_SSP1;
  constexpr LPC_SSP_TypeDef* SPI2 = LPC_SSP2;
  
  constexpr LPC_CAN_TypeDef* CAN1 = LPC_CAN1;
  constexpr LPC_CAN_TypeDef* CAN2 = LPC_CAN2;
    
  constexpr LPC_IOCON_TypeDef* IOCON = LPC_IOCON;  // IOCON 
  constexpr LPC_SC_TypeDef* SC = LPC_SC;           // System Control
  constexpr LPC_EMC_TypeDef* EMC = LPC_EMC;        // EMC
  
  constexpr LPC_ADC_TypeDef* IADC = LPC_ADC;       // Внутреннее АЦП
  
  constexpr LPC_DAC_TypeDef* DAC = LPC_DAC;        // DAC0
  constexpr LPC_PWM_TypeDef* PWM_DAC = LPC_PWM1;   // PWM1 DAC_PWM
  
  constexpr LPC_PWM_TypeDef* PULS_PWM = LPC_PWM0;  // PWM ИУ

}

// Включение питания (PCONP)
class CPCONP {
public:
  LPC_SC_TypeDef* sc;
  CPCONP(LPC_SC_TypeDef* sc) : sc(sc) {}
};

// Инициализация EXTINT
class CEXTINT {
public:
  LPC_SC_TypeDef* sc;
  CEXTINT(LPC_SC_TypeDef* sc) : sc(sc) {}
};

// Инициализация функционала (IOCON)
class CIOCON {
public:
  LPC_IOCON_TypeDef* base;
  CIOCON(LPC_IOCON_TypeDef* base) : base(base) {}
};

// Инициализация таймеров
class CTIMER {
public:
  LPC_TIM_TypeDef* syst;
  LPC_TIM_TypeDef* tim1;
  LPC_TIM_TypeDef* tim2;
  LPC_TIM_TypeDef* tim3;
  CTIMER(LPC_TIM_TypeDef* syst, LPC_TIM_TypeDef* tim1, 
         LPC_TIM_TypeDef* tim2, LPC_TIM_TypeDef* tim3) 
    : syst(syst), tim1(tim1), tim2(tim2), tim3(tim3) {}
};

// Системный таймер
class SysT { public: inline static unsigned int TC() { return P::SYST->TC; } };

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

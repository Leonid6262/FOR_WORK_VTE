#pragma once
#include "LPC407x_8x_177x_8x.h"
#include "lpc177x_8x_clkpwr.h"
#include "Peripherals.hpp"
#include "ControlBits.hpp"

class CPERIPHERIALS_INIT {
private:
  CGPIO gpio0;
  CGPIO gpio1;
  CGPIO gpio2;
  CGPIO gpio3;
  CGPIO gpio4;
  CGPIO gpio5;
  
  CPCONP pconp;
  CIOCON iocon;
  CTIMER timer;
  
  CEXTINT extint;
  
public:
  CPERIPHERIALS_INIT() : 
    gpio0(P::G0), gpio1(P::G1), gpio2(P::G2), gpio3(P::G3), 
    gpio4(P::G4), gpio5(P::G5), iocon(P::IOCON), pconp(P::SC),
    timer(P::SYST, P::TIM1, P::TIM2, P::TIM3), extint(P::SC) {}
    
    void initDOutputs() {
      
      // Дискретные выходы
      gpio0.set(0x00000380);   // U-LED Off, OUT-E=1, HOLD
      gpio0.clr(0xFFFFFC7F);
      
      gpio1.set(0x00202000);   // Q2VF, Q1VF
      gpio1.clr(0xFFDFDFFF);
      
      gpio2.set(0x00000000);
      gpio2.clr(0xFFFFFFFF);
      
      gpio3.clr(0xFFFFFFFF);
      gpio3.set(0x3F3F0000);   // V6...V1, U6...U1
      
      gpio4.clr(0xFFFFFFFF);
      gpio5.clr(0xFFFFFFFF);
      
      // Настройка направления
      gpio0.dirOut(0x00000380);
      gpio1.dirOut(0x1020200C);
      gpio2.dirOut(0xFF000020);
      gpio3.dirOut(0x3F3F0000);
      gpio4.dirOut(0x00000000);
      gpio5.dirOut(0x00000008);
    }
    
    void initEXTINT() {
      
      // Сброс настроек и флагов EXTINT2
      extint.sc->EXTINT = 0xF; 
      extint.sc->EXTMODE = 0x0;
      extint.sc->EXTPOLAR = 0x0;
      // Настрока EXTINT2
      extint.sc->EXTMODE |= (1 << bf::LineEINT2);      // EDGE
      extint.sc->EXTPOLAR &= ~(1 << bf::LineEINT2);    // FALLING 
      extint.sc->EXTINT |= 0x0F; 
      
      NVIC_EnableIRQ(EINT2_IRQn);
    }
    
    
    void powerON() {
      pconp.sc->PCONP |= CLKPWR_PCONP_PCPWM1;
      
      pconp.sc->PCONP |= CLKPWR_PCONP_PCTIM0;
      pconp.sc->PCONP |= CLKPWR_PCONP_PCTIM1;
      pconp.sc->PCONP |= CLKPWR_PCONP_PCTIM2;
      pconp.sc->PCONP |= CLKPWR_PCONP_PCTIM3;
      
      pconp.sc->PCONP |= CLKPWR_PCONP_PCAN1;
      pconp.sc->PCONP |= CLKPWR_PCONP_PCAN2;
      
      pconp.sc->PCONP |= CLKPWR_PCONP_PCADC;
      
      pconp.sc->PCONP |= CLKPWR_PCONP_PCUART0;  
      pconp.sc->PCONP |= CLKPWR_PCONP_PCUART2;  
      pconp.sc->PCONP |= CLKPWR_PCONP_PCUART3;  
      
      pconp.sc->PCONP |= CLKPWR_PCONP_PCSSP0; 
      pconp.sc->PCONP |= CLKPWR_PCONP_PCSSP1;    
      pconp.sc->PCONP |= CLKPWR_PCONP_PCSSP2; 
      
      pconp.sc->PCONP |= CLKPWR_PCONP_PCGPDMA;
      
    }
    
    void initIOCON() {
      iocon.base->P0_26 = bf::IOCON_DAC0_EN;      // DAC-0
      iocon.base->P2_4  = bf::IOCON_PORT_PWM;     // P2_4 -> PWM1:5 PWM_DAC1
      iocon.base->P2_3  = bf::IOCON_PORT_PWM;     // P2_3 -> PWM1:4 PWM_DAC2     
      iocon.base->P0_23 = bf::IOCON_CH_ADC_IOCON; // Внутренее ADC ch0
      iocon.base->P0_24 = bf::IOCON_CH_ADC_IOCON; // Внутренее ADC ch1
      
      iocon.base->P0_15 = bf::IOCON_SPI0;       // SCK0
      // SSEL0 - не проведен, не используется
      iocon.base->P0_17 = bf::IOCON_SPI0;       // MISO0
      iocon.base->P0_18 = bf::IOCON_SPI0;       // MOSI0
      
      iocon.base->P4_20 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;  // SCK1
      iocon.base->P4_21 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;  // SSEL1
      iocon.base->P4_22 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;  // MISO1
      iocon.base->P4_23 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;  // MOSI1
      
      iocon.base->P5_2 = bf::IOCON_SPI2;           // SCK2
      // SSEL2 - не проведен, не используется
      iocon.base->P5_1 = bf::IOCON_SPI2;           // MISO2
      iocon.base->P5_0 = bf::IOCON_SPI2;           // MOSI2
      
      iocon.base->P0_2 = bf::IOCON_U0_TXD;         // U0_TXD
      iocon.base->P0_3 = bf::IOCON_U0_RXD;         // U0_RXD
      
      iocon.base->P2_6 = bf::IOCON_U2_OE;          // U2_OE
      iocon.base->P2_8 = bf::IOCON_U2_TXD;         // U2_TXD
      iocon.base->P2_9 = bf::IOCON_U2_RXD;         // U2_RXD
      
      iocon.base->P1_30 = bf::IOCON_U3_OE;         // U3_OE
      iocon.base->P4_28 = bf::IOCON_U3_TXD;        // U3_TXD
      iocon.base->P4_29 = bf::IOCON_U3_RXD;        // U3_RXD
      
      iocon.base->P2_15 = bf::IOCON_T2_CAP1;       // T2 CAP1
      iocon.base->P2_23 = bf::IOCON_T3_CAP1;       // T3 CAP1
      
      iocon.base->P0_0 = bf::IOCON_CAN1;  // CAN0.RX
      iocon.base->P0_1 = bf::IOCON_CAN1;  // CAN0.TX
      
      iocon.base->P0_4 = bf::IOCON_CAN2;  // CAN1.RX
      iocon.base->P0_5 = bf::IOCON_CAN2;  // CAN1.TX 
      
      iocon.base->P2_12 = bf::IOCON_EINT2;   // Прерывание EINT2  
      
    }
    
    void initTimers() {     
      timer.syst->PR =  6 - 1;  // 10 МГц, 1 тик = 0.1 мкс
      timer.tim1->PR = 60 - 1;  // 1 МГц, 1 тик = 1.0 мкс
      timer.tim2->PR = 60 - 1;  // 1 МГц
      timer.tim3->PR = 60 - 1;  // 1 МГц   
      // Включение системного таймера (TIM0)
      timer.syst->TCR |= 0x1;
    }
    
};


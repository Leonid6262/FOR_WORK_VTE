#pragma once
#include "LPC407x_8x_177x_8x.h"
#include "Peripherals.hpp"

class CSET_PORTS {
private:
    CGPIO gpio0;
    CGPIO gpio1;
    CGPIO gpio2;
    CGPIO gpio3;
    CGPIO gpio4;
    CGPIO gpio5;
    
    CIOCON iocon;
    
public:
  CSET_PORTS() : gpio0(P::G0), gpio1(P::G1), gpio2(P::G2), gpio3(P::G3), 
                 gpio4(P::G4), gpio5(P::G5), iocon(P::IOCON) {}
          
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
      
    static constexpr unsigned int DAC0_EN = 0x00010002;
    static constexpr unsigned int PORT_PWM = 1;
    static constexpr unsigned int CH_ADC_IOCON = 1;
    
    static constexpr unsigned int IOCON_SPI0 = 0x02;
    static constexpr unsigned int IOCON_SPI1 = 0x03;
    static constexpr unsigned int IOCON_SPI2 = 0x02;
    static constexpr unsigned int D_MODE_PULLUP = 0x02 << 3;

    static constexpr unsigned int IOCON_U0_TXD  = 0x1;  
    static constexpr unsigned int IOCON_U0_RXD  = 0x1;
  
    static constexpr unsigned int IOCON_U2_TXD  = 0x2;  
    static constexpr unsigned int IOCON_U2_RXD  = 0x2;
    static constexpr unsigned int IOCON_U2_OE   = 0x4;
  
    static constexpr unsigned int IOCON_U3_TXD  = 0x2;  
    static constexpr unsigned int IOCON_U3_RXD  = 0x2;
    static constexpr unsigned int IOCON_U3_OE   = 0x5;   
    
    static constexpr unsigned int IOCON_T2_CAP1 = 0x23;
    
    void initIOCON() {
      iocon.base->P0_26 = DAC0_EN;      // DAC-0
      iocon.base->P2_4  = PORT_PWM;     // P2_4 -> PWM1:5 PWM_DAC1
      iocon.base->P2_3  = PORT_PWM;     // P2_3 -> PWM1:4 PWM_DAC2     
      iocon.base->P0_23 = CH_ADC_IOCON; // Внутренее ADC ch0
      iocon.base->P0_24 = CH_ADC_IOCON; // Внутренее ADC ch1
      
      iocon.base->P0_15 = IOCON_SPI0;           // SCK0
                                                // SSEL0 - не проведен, не используется
      iocon.base->P0_17 = IOCON_SPI0;           // MISO0
      iocon.base->P0_18 = IOCON_SPI0;           // MOSI0
      
      iocon.base->P4_20 = D_MODE_PULLUP | IOCON_SPI1;  // SCK1
      iocon.base->P4_21 = D_MODE_PULLUP | IOCON_SPI1;  // SSEL1
      iocon.base->P4_22 = D_MODE_PULLUP | IOCON_SPI1;  // MISO1
      iocon.base->P4_23 = D_MODE_PULLUP | IOCON_SPI1;  // MOSI1
      
      iocon.base->P5_2 = IOCON_SPI2;          // SCK2
                                              // SSEL2 - не проведен, не используется
      iocon.base->P5_1 = IOCON_SPI2;          // MISO2
      iocon.base->P5_0 = IOCON_SPI2;          // MOSI2
      
      iocon.base->P0_2 = IOCON_U0_TXD;         // U0_TXD
      iocon.base->P0_3 = IOCON_U0_RXD;         // U0_RXD
      
      iocon.base->P2_6 = IOCON_U2_OE;          // U2_OE
      iocon.base->P2_8 = IOCON_U2_TXD;         // U2_TXD
      iocon.base->P2_9 = IOCON_U2_RXD;         // U2_RXD
      
      iocon.base->P1_30 = IOCON_U3_OE;         // U3_OE
      iocon.base->P4_28 = IOCON_U3_TXD;        // U3_TXD
      iocon.base->P4_29 = IOCON_U3_RXD;        // U3_RXD
      
      iocon.base->P2_15 = IOCON_T2_CAP1;       // T2 CAP1
      
    }
    
};


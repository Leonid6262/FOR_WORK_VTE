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
    
    static constexpr unsigned short B_ULED = 9;        // Бит U-LED
    
public:
  CSET_PORTS()
    : gpio0(LPC_GPIO0), gpio1(LPC_GPIO1),
    gpio2(LPC_GPIO2), gpio3(LPC_GPIO3),
    gpio4(LPC_GPIO4), gpio5(LPC_GPIO5) {}
    
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
      
      
      
      
      
      // Сброс настроек и флагов EXTINT
      LPC_SC->EXTINT   = 0xF;
      LPC_SC->EXTMODE  = 0x0;
      LPC_SC->EXTPOLAR = 0x0;
      
      
    /*
    // Дискретные выходы    
    LPC_GPIO0->SET  = 0x00000380; // U-LED Off, OUT-E=1 отключает выходы, HOLD исходное состояние
    LPC_GPIO0->CLR  = 0xFFFFFC7F;

    LPC_GPIO1->SET  = 0x00202000; // Q2VF, Q1VF единица отключает оптроны
    LPC_GPIO1->CLR  = 0xFFDFDFFF;  
    LPC_GPIO2->SET  = 0x00000000; 
    LPC_GPIO2->CLR  = 0xFFFFFFFF;
    
    LPC_GPIO3->CLR  = 0xFFFFFFFF;
    LPC_GPIO3->SET  = 0x3F3F0000; // V6...V1, U6...U1 -> пассивное состояние
    LPC_GPIO4->CLR  = 0xFFFFFFFF;
    LPC_GPIO5->CLR  = 0xFFFFFFFF;
    
    LPC_GPIO0->DIR |= 0x00000380; // U-LED, OUT-E, HOLD  
    LPC_GPIO1->DIR |= 0x1020200C; // SUM3, SUM2, Q2VF, Q1VF, SUM1, SUM0
    LPC_GPIO2->DIR |= 0xFF000020; // D-OUT7...D-OUT0, DAVR
    LPC_GPIO3->DIR |= 0x3F3F0000; // V6...V1, U6...U1
    LPC_GPIO4->DIR |= 0x00000000; // нет выходов
    LPC_GPIO5->DIR |= 0x00000008; // PROG_CS  
    
    // Сброс настроек и флагов EXTINT
    LPC_SC->EXTINT = 0xF;
    LPC_SC->EXTMODE = 0x0;
    LPC_SC->EXTPOLAR = 0x0;*/
  }
};


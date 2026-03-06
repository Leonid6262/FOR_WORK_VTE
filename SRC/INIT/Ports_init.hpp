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
    
public:
  CSET_PORTS() : gpio0(P::G0), gpio1(P::G1), gpio2(P::G2), gpio3(P::G3), gpio4(P::G4), gpio5(P::G5) {}
    
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
};


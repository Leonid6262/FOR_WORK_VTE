#pragma once

//#define CM3 

#include "system_LPC177x.h"
#include "dIOStorage.hpp"

class CSET_EMC {
  
public:  
  static void initAndCheck() {
        
    constexpr unsigned short* RAM_BEGIN   = reinterpret_cast<unsigned short*>(0x80000000);
    constexpr unsigned short* RAM_END     = reinterpret_cast<unsigned short*>(0x800FFFFF);
    constexpr unsigned short* ADR_TEST_55 = reinterpret_cast<unsigned short*>(0x80055555);
    constexpr unsigned short* ADR_TEST_AA = reinterpret_cast<unsigned short*>(0x800AAAAA);  
    
    P::SC->PCONP |= CLKPWR_PCONP_PCEMC;
    P::SC->EMCDLYCTL = 0x00001010;
    P::EMC->Control = 0x00000001;
    P::EMC->Config = 0x00000000;
    
    // Настройка выводов данных EMC.D0..D15 
    for (int i = 0; i < 16; i++) { 
      *(&P::IOCON->P3_0 + i) = bf::D_MODE_PULLUP | 0x1; 
    }
    // Настройка выводов адреса EMC.A0..A19 
    for (int i = 0; i < 20; i++) { 
      *(&P::IOCON->P4_0 + i) = bf::D_MODE_PULLUP | 0x1; 
    }

    P::IOCON->P4_24 = bf::D_MODE_PULLUP | 0x1;  // EMC.OE
    P::IOCON->P4_25 = bf::D_MODE_PULLUP | 0x1;  // EMC.WR
    P::IOCON->P4_26 = bf::D_MODE_PULLUP | 0x1;  // EMC.BLS0
    P::IOCON->P4_27 = bf::D_MODE_PULLUP | 0x1;  // EMC.BLS1
    
    P::IOCON->P4_30 = bf::D_MODE_PULLUP | 0x1;  // EMC.CS0
    P::IOCON->P4_31 = bf::D_MODE_PULLUP | 0x1;  // EMC.CS1
    
    //---------------   Конфигурация адресного пространства RAM   ------------------------- 
#ifdef CM3
    P::EMC->StaticConfig0 = 0x01; // 8 bits
#else
    P::EMC->StaticConfig0 = 0x81; // 16 bits
#endif
    P::EMC->StaticWaitWen0  = 0x00;     // Задержка разрешения записи. 0 - задержка в один цикл EMCCLK
    P::EMC->StaticWaitOen0  = 0x01;     // Задержка включения выхода. 0 - нет задержки
    P::EMC->StaticWaitRd0   = 0x03;     // Задержка чтения. 0 - задержка в один цикл EMCCLK
    P::EMC->StaticWaitPage0 = 0x00;     // Ожидане чтения в асинхронном режиме страниц. 0 - задержка в один цикл EMCCLK
    P::EMC->StaticWaitWr0   = 0x00;     // Ожидание записи. 0 - задержка в два цикла EMCCLK
    P::EMC->StaticWaitTurn0 = 0x00;     // Циклов реверсирования шины. 0 - задержка в один цикл EMCCLK
    
    //---------------   Конфигурация адресного пространства Jockey   -------------------------
    P::EMC->StaticConfig1   = 0x81;   // 16-bit
    P::EMC->StaticWaitWen1  = 0;
    P::EMC->StaticWaitOen1  = 0;    
    P::EMC->StaticWaitRd1   = 0x7;    
    P::EMC->StaticWaitPage1 = 0;
    P::EMC->StaticWaitWr1   = 0x5;
    P::EMC->StaticWaitTurn1 = 0;
    //-------------------------------------------------------------------------------------
    
    unsigned int Counter_Err_RAM;
    unsigned short* adr_ram;
    unsigned short dir;
    unsigned short dat1;
    unsigned short dat2;
    
    // Тест RAM
    dir = 0;
    dat1 = 0;
    // Пишем змейку
    for (adr_ram = RAM_BEGIN; adr_ram < (RAM_END + 1); adr_ram++) {
      *adr_ram = dat1;
      if (dir == 0) {
        if (dat1 < 0xFFFF) {
          dat1++;
        } else {
          dir = 1;
        }
      } else {
        if (dat1 > 0) {
          dat1--;
        } else {
          dir = 0;
        }
      }
    }    
    dir = 0;
    dat1 = 0;
    dat2 = 0;
    // Читаем змейку
    for (adr_ram = RAM_BEGIN; adr_ram < (RAM_END + 1); adr_ram++) {
      dat2 = *adr_ram;
      if (dat1 != dat2) {
        // Циклический тест, если была ошибка
        adr_ram = 0;
        P::G0->DIR |= 1U << bg::B_ULED;
        while (true) {
          adr_ram = ADR_TEST_55;
          *adr_ram = 0x5555;
          dat1 = *adr_ram;
          *adr_ram = 0xAAAA;
          dat2 = *adr_ram;
          adr_ram = ADR_TEST_AA;
          *adr_ram = 0x5555;
          dat1 = *adr_ram;
          *adr_ram = 0xAAAA;
          dat2 = *adr_ram;
          
          Counter_Err_RAM = (Counter_Err_RAM + 1) & 0x1FFFF;
          if (Counter_Err_RAM < 0x10000) {
            P::G0->CLR = 1UL << bg::B_ULED;
          } else {
            P::G0->SET = 1UL << bg::B_ULED;
          }
        }
      }
      if (dir == 0) {
        if (dat1 < 0xFFFF) {
          dat1++;
        } else {
          dir = 1;
        }
      } else {
        if (dat1 > 0) {
          dat1--;
        } else {
          dir = 0;
        }
      }
    }
  }
};




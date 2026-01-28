#pragma once

//#define CM3 

#include "system_LPC177x.h"
#include "dIOStorage.hpp"

class CSET_EMC {
  
public:  
  static void initAndCheck() {
    
    constexpr unsigned int D_MODE_PULLUP = 0x02 << 3;
    
    constexpr unsigned char* RAM_BEGIN   = reinterpret_cast<unsigned char*>(0x80000000);
    constexpr unsigned char* RAM_END     = reinterpret_cast<unsigned char*>(0x800FFFFF);
    constexpr unsigned char* ADR_TEST_55 = reinterpret_cast<unsigned char*>(0x80055555);
    constexpr unsigned char* ADR_TEST_AA = reinterpret_cast<unsigned char*>(0x800AAAAA);  
    
    LPC_SC->PCONP |= CLKPWR_PCONP_PCEMC;
    LPC_SC->EMCDLYCTL = 0x00001010;
    LPC_EMC->Control = 0x00000001;
    LPC_EMC->Config = 0x00000000;
    
    // Настройка выводов данных EMC.D0..D15 
    for (int i = 0; i < 16; i++) { 
      *(&LPC_IOCON->P3_0 + i) = D_MODE_PULLUP | 0x1; 
    }
    // Настройка выводов адреса EMC.A0..A19 
    for (int i = 0; i < 20; i++) { 
      *(&LPC_IOCON->P4_0 + i) = D_MODE_PULLUP | 0x1; 
    }

    LPC_IOCON->P4_24 = D_MODE_PULLUP | 0x1;  // EMC.OE
    LPC_IOCON->P4_25 = D_MODE_PULLUP | 0x1;  // EMC.WR
    LPC_IOCON->P4_26 = D_MODE_PULLUP | 0x1;  // EMC.BLS0
    LPC_IOCON->P4_27 = D_MODE_PULLUP | 0x1;  // EMC.BLS1
    
    LPC_IOCON->P4_30 = D_MODE_PULLUP | 0x1;  // EMC.CS0
    LPC_IOCON->P4_31 = D_MODE_PULLUP | 0x1;  // EMC.CS1
    
    //---------------   Конфигурация адресного пространства RAM   ------------------------- 
#ifdef CM3
    LPC_EMC->StaticConfig0 = 0x01; // 8 bits
#else
    LPC_EMC->StaticConfig0 = 0x81; // 16 bits
#endif
    LPC_EMC->StaticWaitWen0 = 0x00;     // Задержка разрешения записи. 0 - задержка в один цикл EMCCLK
    LPC_EMC->StaticWaitOen0 = 0x01;     // Задержка включения выхода. 0 - нет задержки
    LPC_EMC->StaticWaitRd0 = 0x03;      // Задержка чтения. 0 - задержка в один цикл EMCCLK
    LPC_EMC->StaticWaitPage0 = 0x00;    // Ожидане чтения в асинхронном режиме страниц. 0 - задержка в один цикл EMCCLK
    LPC_EMC->StaticWaitWr0 = 0x00;      // Ожидание записи. 0 - задержка в два цикла EMCCLK
    LPC_EMC->StaticWaitTurn0 = 0x00;    // Циклов реверсирования шины. 0 - задержка в один цикл EMCCLK
    //-------------------------------------------------------------------------------------
    
    unsigned int Counter_Err_RAM;
    unsigned char* adr_ram;
    unsigned char dir;
    unsigned char dat1;
    unsigned char dat2;
    
    // Тест RAM
    dir = 0;
    dat1 = 0;
    // Пишем змейку
    for (adr_ram = RAM_BEGIN; adr_ram < (RAM_END + 1); adr_ram++) {
      *adr_ram = dat1;
      if (dir == 0) {
        if (dat1 < 255) {
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
        while (true) {
          adr_ram = ADR_TEST_55;
          *adr_ram = 0x55;
          dat1 = *adr_ram;
          *adr_ram = 0xAA;
          dat2 = *adr_ram;
          adr_ram = ADR_TEST_AA;
          *adr_ram = 0x55;
          dat1 = *adr_ram;
          *adr_ram = 0xAA;
          dat2 = *adr_ram;
          
          Counter_Err_RAM = (Counter_Err_RAM + 1) & 0x1FFFF;
          if (Counter_Err_RAM < 0x10000) {
            CDIN_STORAGE::UserLedOn();
          } else {
            CDIN_STORAGE::UserLedOff();
          }
        }
      }
      if (dir == 0) {
        if (dat1 < 255) {
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




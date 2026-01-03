#pragma once

#include "settings_eep.hpp"
#include "spi_init.hpp"
#include <array>
#include <initializer_list>

class CADC{ 
  
private:
  
  LPC_SSP_TypeDef* SSP;
  /* 
  cN_CH - данные для запуска конвертации (формат - см. док.) 
  000m.0ccc.c000.0000 m - Manual mode, cccc - N channel (0...15)
  */
  static constexpr unsigned short cN_CH[G_CONST::NUMBER_CHANNELS] = 
  {
    0x1000, 0x1080, 0x1100, 0x1180, 0x1200, 0x1280, 0x1300, 0x1380, 
    0x1400, 0x1480, 0x1500, 0x1580, 0x1600, 0x1680, 0x1700, 0x1780
  };

public:
  
  CADC(LPC_SSP_TypeDef*);
  
  // Метод конвертации
  void conv_tnf(std::initializer_list<char>);
  
};

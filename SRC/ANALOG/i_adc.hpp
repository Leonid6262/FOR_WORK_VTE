#pragma once

#include "bool_name.hpp"

class CIADC{ 
  
private:
  
  static constexpr unsigned short CH_P5 = 0;           // +5V
  static constexpr unsigned short CH_N5 = 1;           // -5V 
  
  static constexpr unsigned int CH_ADC_IOCON = 1;
  static constexpr unsigned int PDN = 1UL << 21;
  static constexpr unsigned int SB_CLKDIV = 8;
  static constexpr unsigned int CLEAR = ~(0x000000FF | (1UL << 24));
  static constexpr unsigned int START = 1UL << 24;
  static constexpr unsigned int DONE = 1UL << 31;
  
  static constexpr unsigned int DELAY = 120; // 120*0.1 =12 us
  
  //С учётом Uop=3V и делителя 10к/10к -> (3V * 2) / 4095 = 0.0014652015
  static constexpr float K_P5 = 0.0014652015f;
  
  //С учётом Uop=3V и делителя 10к/30к между +5V и -5V -> (3V * 4) / 4095 = 0.0029304029
  static constexpr float K_N5 = 0.0029304029f;
 
public:
  
  CIADC();
  
  float P5_prev;
  float N5_prev;

  StatusRet measure_5V();
  
};

#pragma once

class CDin_cpu
{
  
private:
  
  static constexpr unsigned int B0_PORT0_IN = 15;      // 0-й бит порта. Начало Pi0 в PORT2 контроллера
  static constexpr unsigned int B0_PORT1_IN = 11;      // 0-й бит порта. Начало 1-й группы Pi1 в PORT2 контроллера
  static constexpr unsigned int MASK_G1   = 0x07;      // Маска бит 1-й группы
  static constexpr unsigned int BITS_G1      = 3;      // Битов в 1-й группе
  static constexpr unsigned int B3_PORT1_IN = 10;      // 3-й бит порта. Начало 2-й группы Pi1 в PORT0 контроллера
  static constexpr unsigned int MASK_G2   = 0xF8;      // Маска бит 2-й группы
  
  unsigned int prev_TC0;                               //Значение таймера на предыдыущем цикле
  
public:  
  
  CDin_cpu();
  
  unsigned char data_din_Pi0;    //Входные данные порта Pi0 (изолированного)
  unsigned char data_din_Pi1;    //Входные данные порта Pi1 (не изолированного)
  
  void input_Pi0();
  void input_Pi1();
  
};


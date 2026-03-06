#pragma once

#include "Peripherals.hpp"

class CDin_cpu
{
 
private: 
  unsigned int prev_TC0;         //Значение таймера на предыдыущем цикле
  CGPIO& gpio;
  
public:    
  CDin_cpu(CGPIO&);  
  unsigned char data_din_Pi0;    //Входные данные порта Pi0 (изолированного) 
  void input_Pi0();
  
};


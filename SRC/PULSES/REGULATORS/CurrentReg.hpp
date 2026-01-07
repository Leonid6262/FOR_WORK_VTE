#pragma once 
#include "bool_name.hpp"
#include "AdcStorage.hpp"

class CCurrentReg {
  
public:
 CCurrentReg();
 void reg(Mode);
  
private:
 CADC_STORAGE& pAdc;
 signed short set_current;
 
};

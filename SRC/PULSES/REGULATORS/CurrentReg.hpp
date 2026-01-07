#pragma once 
#include "bool_name.hpp"
#include "AdcStorage.hpp"
#include "SIFU.hpp"
#include "settings_eep.hpp"

class CSIFU;

class CCurrentReg {
  
public:
 CCurrentReg(CEEPSettings&);
 void step(Mode, CSIFU*);
  
private:
 CADC_STORAGE& pAdc;
 CEEPSettings& rSet;
 
 float u_i;
 
 signed short set_current;
 
};

#pragma once 
#include "bool_name.hpp"
#include "AdcStorage.hpp"
#include "SIFU.hpp"
#include "settings_eep.hpp"

class CSIFU;

class CCosReg {
  
public:
 CCosReg(CEEPSettings&);
 void start(CSIFU*);
 void step(bool, CSIFU*);
  
private:
 CADC_STORAGE& pAdc;
 CEEPSettings& rSet;
 
 float u_i;
 signed short u_min;
 signed short u_max;
 
};

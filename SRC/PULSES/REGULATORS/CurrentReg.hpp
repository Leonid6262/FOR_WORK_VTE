#pragma once 
#include "bool_name.hpp"
#include "AdcStorage.hpp"
#include "SIFU.hpp"
#include "settings_eep.hpp"

class CSIFU;

class CCurrentReg {
  
public:
 CCurrentReg(CEEPSettings&);

 void step(bool, CSIFU*);
 void set_Iset(unsigned short);
 bool bStart_reg = false;
  
private:
 CADC_STORAGE& pAdc;
 CEEPSettings& rSet;
 
 void start_reg(CSIFU*);
 void stop_reg(CSIFU*);
 
 float u_i;
 signed short u_min;
 signed short u_max;
 unsigned short Iset;
 
};

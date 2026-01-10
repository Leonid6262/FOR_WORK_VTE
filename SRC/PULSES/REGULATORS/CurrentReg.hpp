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
 bool start_r = false;
  
private:
 CADC_STORAGE& pAdc;
 CEEPSettings& rSet;
 
 void start(CSIFU*);
 void stop(CSIFU*);
 
 float u_i;
 signed short u_min;
 signed short u_max;
 signed short Iset;
 
};

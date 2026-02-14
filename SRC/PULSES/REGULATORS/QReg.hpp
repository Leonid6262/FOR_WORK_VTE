#pragma once 
#include "bool_name.hpp"
#include "AdcStorage.hpp"
#include "SIFU.hpp"
#include "settings_eep.hpp"

class CSIFU;

class CQReg {
  
public:
 CQReg(CEEPSettings&);
 void start(CSIFU*);
 void step(bool, CSIFU*);
 
private:
 CADC_STORAGE& pAdc;
 CEEPSettings& rSet;
 
 signed short Qset;
 float Q_part;
 float u_i;
 signed short u_min;
 signed short u_max;
 
 bool bStart_reg = false;
 
};

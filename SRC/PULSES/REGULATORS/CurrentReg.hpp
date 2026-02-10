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
 
 signed short Imeas_0;
 bool Res_connected = false;
 
 inline unsigned short* getPointerIset() { return &Iset; }

private:
 CADC_STORAGE& pAdc;
 CEEPSettings& rSet;
 
 void init_reg(CSIFU*);
 
 float I_part;
 float I_part_min;
 float I_part_max;
 unsigned short Iset;
 
};

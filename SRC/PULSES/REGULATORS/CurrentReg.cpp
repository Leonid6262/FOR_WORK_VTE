#include "CurrentReg.hpp" 

CCurrentReg::CCurrentReg(CEEPSettings& rSet) : pAdc(CADC_STORAGE::getInstance()), rSet(rSet) {}

void CCurrentReg::start_reg(CSIFU* pSIFU) {
  auto set = rSet.getSettings();
  u_i = set.set_reg.A0 - pSIFU->get_alpha();
  u_min = set.set_reg.A0 - pSIFU->s_const.AMax;
  u_max = set.set_reg.A0 - pSIFU->s_const.AMin;
  bStart_reg = true;
}

void CCurrentReg::stop_reg(CSIFU* pSIFU) {
  pSIFU->set_alpha(pSIFU->s_const.AMax);
  bStart_reg = false;
}

void CCurrentReg::set_Iset(unsigned short Iset) {
  this->Iset = Iset;
}

unsigned short* CCurrentReg::getPointerIset() {
  return &Iset;
}

void CCurrentReg::step(bool Permission, CSIFU* pSIFU) { 
 
  if(!Permission) { 
    if(bStart_reg) { 
      stop_reg(pSIFU); 
    } 
    return;
  }
  if(!bStart_reg) start_reg(pSIFU);
  
  auto set = rSet.getSettings();
  
  signed short Imeas = *pAdc.getEPointer(CADC_STORAGE::ROTOR_CURRENT);
 
  float e = static_cast<float>(Iset - Imeas);
  
  float u_p = set.set_reg.KpCr * e; 
  u_i += set.set_reg.KiCr * e;

  if (u_i < static_cast<float>(u_min)) u_i = static_cast<float>(u_min); 
  if (u_i > static_cast<float>(u_max)) u_i = static_cast<float>(u_max);
  
  float u_total = u_p + u_i;
  float new_Alpha = set.set_reg.A0 - u_total;
  
  if (new_Alpha < pSIFU->s_const.AMin) new_Alpha = pSIFU->s_const.AMin; 
  if (new_Alpha > pSIFU->s_const.AMax) new_Alpha = pSIFU->s_const.AMax;
  
  signed short Alpha_out = static_cast<signed short>(new_Alpha);
  
  pSIFU->set_alpha(Alpha_out);
}



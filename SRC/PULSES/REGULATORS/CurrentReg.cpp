#include "CurrentReg.hpp" 

CCurrentReg::CCurrentReg(CEEPSettings& rSet) : pAdc(CADC_STORAGE::getInstance()), rSet(rSet) {}

void CCurrentReg::start(CSIFU* pSIFU) {
  auto set = rSet.getSettings();
  u_i = set.set_reg.A0 - pSIFU->get_alpha();
  u_min = set.set_reg.A0 - pSIFU->s_const.AMax;
  u_max = set.set_reg.A0 - pSIFU->s_const.AMin;
  start_r = true;
}

void CCurrentReg::stop(CSIFU* pSIFU) {
  pSIFU->set_alpha(pSIFU->s_const.AMax);
  start_r = false;
}

void CCurrentReg::step(bool mode, CSIFU* pSIFU) { 
 
  if(!mode) { if(start_r) { stop(pSIFU); } return;}
  if(!start_r) start(pSIFU);
  
  auto set = rSet.getSettings();
  
  signed short Imeas = *pAdc.getEPointer(CADC_STORAGE::ROTOR_CURRENT);
  signed short Iset = set.set_reg.Iset;
 
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



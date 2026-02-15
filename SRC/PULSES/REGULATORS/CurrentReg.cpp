#include "CurrentReg.hpp" 
#include "dIOStorage.hpp"

CCurrentReg::CCurrentReg(CEEPSettings& rSet) : pAdc(CADC_STORAGE::getInstance()), rSet(rSet) {}

void CCurrentReg::init_reg(CSIFU* pSIFU) {
  I_part = rSet.getSettings().set_reg.A0 - pSIFU->get_alpha();
  I_part_min = rSet.getSettings().set_reg.A0 - pSIFU->s_const.AMax;
  I_part_max = rSet.getSettings().set_reg.A0 - pSIFU->s_const.AMin;
}

void CCurrentReg::set_Iset(unsigned short Iset) { this->Iset = Iset; }

void CCurrentReg::step(bool Permission, CSIFU* pSIFU) { 
 
  if(!Permission) { bStart_reg = false; return; }
  
  if(!bStart_reg) { bStart_reg = true; init_reg(pSIFU); }
  
  signed short Imeas = *pAdc.getEPointer(CADC_STORAGE::ROTOR_CURRENT);
  if(ResPusk == RPusk::CONNECT) { Imeas = Imeas_0;}
  
  float delta = static_cast<float>(Iset - Imeas);
  
  float P_part;
  if(ResPusk == RPusk::CONNECT) { 
    P_part = rSet.getSettings().set_reg.KpCrR * delta; 
  } else {
    P_part = rSet.getSettings().set_reg.KpCr * delta;
  }
  
  if(ResPusk == RPusk::CONNECT) {
    I_part += rSet.getSettings().set_reg.KiCrR * delta;
  } else {
    I_part += rSet.getSettings().set_reg.KiCr * delta;
  }

  if (I_part < I_part_min) I_part = I_part_min; 
  if (I_part > I_part_max) I_part = I_part_max;
  
  signed short new_Alpha = static_cast<signed short>(rSet.getSettings().set_reg.A0 - (P_part + I_part) + 0.5f);
  
  if (new_Alpha < pSIFU->s_const.AMin) new_Alpha = pSIFU->s_const.AMin; 
  if (new_Alpha > pSIFU->s_const.AMax) new_Alpha = pSIFU->s_const.AMax;
  
  pSIFU->set_alpha(new_Alpha);
}


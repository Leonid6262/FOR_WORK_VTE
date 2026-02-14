#include "QReg.hpp" 

CQReg::CQReg(CEEPSettings& rSet) : pAdc(CADC_STORAGE::getInstance()), rSet(rSet) {}

void CQReg::start(CSIFU* pSIFU) {
  auto set = rSet.getSettings();

}

void CQReg::step(bool Permission, CSIFU* pSIFU) {
  
  if(!Permission) { bStart_reg = false; return;}
  if(pSIFU->N_Pulse != 1) { return;}
   
  signed short Qmeas = *pSIFU->rPulsCalc.getPointer_Q_Power();
  float delta = static_cast<float>(Qset - Qmeas);
  float P_part = rSet.getSettings().set_reg.KpQ * delta; 
  Q_part += rSet.getSettings().set_reg.KiQ * delta;
  
}



#include "QReg.hpp" 

CQReg::CQReg(CEEPSettings& rSet) : pAdc(CADC_STORAGE::getInstance()), rSet(rSet) {}

void CQReg::start(CSIFU* pSIFU) {
  auto set = rSet.getSettings();

}

void CQReg::step(bool mode, CSIFU* pSIFU) {
  
  if(!mode) { return;}
  
  auto set = rSet.getSettings();
  
}



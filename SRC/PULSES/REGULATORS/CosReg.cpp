#include "CosReg.hpp" 

CCosReg::CCosReg(CEEPSettings& rSet) : pAdc(CADC_STORAGE::getInstance()), rSet(rSet) {}

void CCosReg::start(CSIFU* pSIFU) {
  auto set = rSet.getSettings();

}

void CCosReg::step(bool mode, CSIFU* pSIFU) {
  
  if(!mode) { return;}
  
  auto set = rSet.getSettings();
  
}



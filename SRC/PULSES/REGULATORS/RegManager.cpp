#include "RegManager.hpp"

CRegManager::CRegManager(CCurrentReg& rCurrent_reg) : rCurrent_reg(rCurrent_reg) {}

void CRegManager::getSIFU(CSIFU* pSIFU){
  this->pSIFU=pSIFU;
}

void CRegManager::dispatch() { 
  
  rCurrent_reg.step(Bit_switch::OFF, pSIFU); 

}

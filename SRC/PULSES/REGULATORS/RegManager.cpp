#include "RegManager.hpp"

CRegManager::CRegManager(CCurrentReg& rCurrent_reg, CQReg& rQ_reg, CCosReg& rCos_reg)
: rCurrent_reg(rCurrent_reg), rQ_reg(rQ_reg), rCos_reg(rCos_reg) { 
  URegMode_request.all = 0; 
}

void CRegManager::getSIFU(CSIFU* pSIFU) {
  this->pSIFU=pSIFU;
}

void CRegManager::applyModeRules() {  
  
  URegMode.all = URegMode_request.all;  // копируем запрос
  
  for (auto& rule : rules) { 
    bool allowed = 
      ((URegMode.all & rule.requiredModes) == rule.requiredModes) && ((URegMode.all & rule.forbiddenModes) == 0); 
    if (!allowed) { 
      URegMode.all &= ~rule.req_bit;    // снимаем бит согласно правил
    }                                                                 
  }
  
  URegMode_request.all = URegMode.all;  // синхронизация с источником
  
}

void CRegManager::stepAll() { 
  rCos_reg.step(URegMode.CosPhi, pSIFU);
  rQ_reg.step(URegMode.QPower, pSIFU);
  rCurrent_reg.step(URegMode.Current, pSIFU); 
}

#include "RegManager.hpp"

CRegManager::CRegManager(CCurrentReg& rCurrent_reg, CQReg& rQ_reg, CCosReg& rCos_reg)
: rCurrent_reg(rCurrent_reg), 
rQ_reg(rQ_reg),
rCos_reg(rCos_reg) { 
  URegMode_request.all = 0; 
}

void CRegManager::getSIFU(CSIFU* pSIFU) {
  this->pSIFU=pSIFU;
}

void CRegManager::dispatch() {  
  
  URegMode.all = URegMode_request.all;  // копируем запрос в рабочую область
  for (auto& rule : rules) { 
    bool allowed = 
      ((URegMode.all & rule.requiredModes) == rule.requiredModes) && ((URegMode.all & rule.forbiddenModes) == 0); 
    if (!allowed) { 
      URegMode.all &= ~rule.req_bit;    // снимаем бит по правилам
    }                                                                 
  }
  URegMode_request.all = URegMode.all;  // синхронизация
  
  rCurrent_reg.step(URegMode.Current, pSIFU);
  rQ_reg.step(URegMode.QPower, pSIFU);
  rCos_reg.step(URegMode.CosPhi, pSIFU);
}

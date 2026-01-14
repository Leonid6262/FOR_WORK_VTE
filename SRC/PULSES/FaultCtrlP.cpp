#include "FaultCtrlP.hpp" 
#include "SystemManager.hpp"
#include <stdlib.h>

CFaultCtrlP::CFaultCtrlP(CADC_STORAGE& rAdcStr, CEEPSettings& rSet) : 
  rAdcStr(rAdcStr), rSet(rSet) {} 

void CFaultCtrlP::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}

void CFaultCtrlP::check() {  
  
  if(pSys_manager->USystemStatus.Fault) return;
    
  auto& set = rSet.getSettings();
  Fault = F::NOT_FAULT;
  
  check(Fault, abs(*rAdcStr.getEPointer(sadc::ROTOR_CURRENT)) > set.set_faults.IdMax,   EFaultId::ID_MAX);
  
  if(Fault == F::FAULT){
    pSys_manager->setFault(State::ON);
    pSys_manager->rReg_manager.setCurrent(State::OFF);
    pSys_manager->rReg_manager.setQPower(State::OFF);
    pSys_manager->rReg_manager.setCosPhi(State::OFF);
    CategoryUtils::clearMessages(ECategory::COUNT);
    pSys_manager->rSIFU.pulses_stop();
  }
  
  
}



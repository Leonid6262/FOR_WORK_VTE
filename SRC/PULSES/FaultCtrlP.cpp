#include "FaultCtrlP.hpp" 
#include "SystemManager.hpp"
#include <stdlib.h>

CFaultCtrlP::CFaultCtrlP(CADC_STORAGE& rAdcStr, CEEPSettings& rSet) : 
  rAdcStr(rAdcStr), rSet(rSet) {} 

void CFaultCtrlP::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}

void CFaultCtrlP::check() {  
  
  //if(pSys_manager->USystemStatus.pFault) return;
    
  auto& set = rSet.getSettings();
  Fault = F::NOT_FAULT;
  
  check(Fault, *rAdcStr.getEPointer(sadc::ROTOR_CURRENT) > set.set_faults.IdMax,               EFaultId::ID_MAX_SOFT);
  check(Fault, !pSys_manager->rSIFU.getSyncStat() && !pSys_manager->USystemStatus.sAdjustment, EFaultId::NOT_SYNC);
  /*  
     Остальные аварийные ситуации определяемые в ИУ 
  */

  if(Fault == F::FAULT) pSys_manager->rFault_ctrl.fault_stop();
 
}



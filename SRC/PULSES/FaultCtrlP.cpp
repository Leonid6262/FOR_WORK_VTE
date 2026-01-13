#include "FaultCtrlP.hpp" 
#include <stdlib.h>

CFaultCtrlP::CFaultCtrlP(CADC_STORAGE& rAdcStr) : rAdcStr(rAdcStr) {}

//void CFaultCtrlP::getManager(CSystemManager* pSys_manager) {
//  this->pSys_manager = pSys_manager;
//}

void CFaultCtrlP::check() {  
  
  Fault = F::NOT_FAULT;
  
  check(Fault, abs(*rAdcStr.getEPointer(sadc::ROTOR_CURRENT)) > 1,   EFaultId::ID_MAX);
  
  if(Fault == F::FAULT){
    //pSys_manager->setReady(State::ON);
  }
  
  
}



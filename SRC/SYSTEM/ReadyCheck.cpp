#include "ReadyCheck.hpp" 
#include "message_factory.hpp"

CReadyCheck::CReadyCheck(){}

void CReadyCheck::getManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}

void CReadyCheck::check(bool mode) { 
  
  if(pSys_manager->USystemMode.Adjustment){
    SNotReady::setMessage(ENotReadyId::ADJ_MODE);
  }
  
}




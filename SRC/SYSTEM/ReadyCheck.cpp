#include "ReadyCheck.hpp" 

CReadyCheck::CReadyCheck(CADC_STORAGE& rAdcStr, CDIN_STORAGE& rDinStr) : rAdcStr(rAdcStr), rDinStr(rDinStr) {}

void CReadyCheck::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}

void CReadyCheck::check(bool mode) { 
  
  if(!mode) {CategoryUtils::clearMessages(ECategory::NOT_READY); return;}
  
  Ready = R::READY;

  check(Ready, pSys_manager->USystemMode.Adjustment,                    ENotReadyId::ADJ_MODE );
  check(Ready, !rDinStr.Bl_Contact_Q1(),                                ENotReadyId::Q1_is_OFF);
  check(Ready, abs(*rAdcStr.getEPointer(sadc::ROTOR_CURRENT)) > dMax,   ENotReadyId::SENS_CR_FAULT);
  check(Ready, abs(*rAdcStr.getEPointer(sadc::STATOR_CURRENT)) > dMax,  ENotReadyId::SENS_CS_FAULT);
  /*  
     Остальные условия готовности 
  */
  
  if(Ready == R::READY){
    SReady::setMessage(EReadyId::PUSK);
    SReady::setMessage(EReadyId::DRYING);
    pSys_manager->setReady(State::ON);
    rDinStr.Lamp_REDY(State::ON);
  }else{
    SReady::clrMessage(EReadyId::PUSK);
    SReady::clrMessage(EReadyId::DRYING);
    pSys_manager->setReady(State::OFF);
    rDinStr.Lamp_REDY(State::OFF);
  }
  
  
  
}




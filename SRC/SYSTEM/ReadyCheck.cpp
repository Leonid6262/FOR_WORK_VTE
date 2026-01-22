#include "ReadyCheck.hpp" 

CReadyCheck::CReadyCheck(CADC_STORAGE& rAdcStr, CDIN_STORAGE& rDinStr) : rAdcStr(rAdcStr), rDinStr(rDinStr) {}

void CReadyCheck::check(bool mode) { 
  
  if(!mode) {CategoryUtils::clearMessages(ECategory::NOT_READY); return;}
  
  Ready = R::READY;

  check(Ready, pSys_manager->USystemStatus.sAdjustment,                 ENotReadyId::ADJ_MODE );
  check(Ready, !rDinStr.Bl_Contact_Q1(),                                ENotReadyId::Q1_is_OFF);
  check(Ready, abs(*rAdcStr.getEPointer(sadc::ROTOR_CURRENT)) > dMax,   ENotReadyId::SENS_CR_FAULT);
  check(Ready, abs(*rAdcStr.getEPointer(sadc::ROTOR_VOLTAGE)) > dMax,   ENotReadyId::SENS_VR_FAULT);
  check(Ready, abs(*rAdcStr.getEPointer(sadc::STATOR_CURRENT)) > dMax,  ENotReadyId::SENS_CS_FAULT);
  check(Ready, !(*pSys_manager->rSIFU.getSyncStat()),                   ENotReadyId::NOT_SYNC);

  /*  
     Остальные условия готовности 
  */
  
  // Разрешения возможных режимов
  if(Ready == R::READY){
    pSys_manager->set_bsReady(State::ON);
    rDinStr.Lamp_REDY(State::ON);    
    SReady::setMessage(EReadyId::DRYING);
    SReady::setMessage(EReadyId::PUSK);
    SReady::setMessage(EReadyId::TESTING);        
  }
  
  // Запрет возможных режимов
  if(Ready == R::NOT_READY){
    pSys_manager->set_bsReady(State::OFF);
    rDinStr.Lamp_REDY(State::OFF);    
    SReady::clrMessage(EReadyId::DRYING);
    SReady::clrMessage(EReadyId::PUSK);
    SReady::clrMessage(EReadyId::TESTING);   
  }  
  
}

void CReadyCheck::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}



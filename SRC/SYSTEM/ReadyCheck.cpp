#include "ReadyCheck.hpp" 

CReadyCheck::CReadyCheck(CADC_STORAGE& rAdcStr, CDIN_STORAGE& rDinStr) : rAdcStr(rAdcStr), rDinStr(rDinStr) {}

void CReadyCheck::getManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}
unsigned short IR;
unsigned short IS;
void CReadyCheck::check(bool mode) { 
  
  if(!mode) return;
  
  IR = abs(*rAdcStr.getEPointer(sadc::ROTOR_CURRENT));
  IS = abs(*rAdcStr.getEPointer(sadc::STATOR_CURRENT));
  
  Ready = R::READY;
  
  check(Ready, pSys_manager->USystemMode.Adjustment,                    ENotReadyId::ADJ_MODE );
  check(Ready, !rDinStr.Bl_Contact_Q1(),                                ENotReadyId::Q1_is_OFF);
  check(Ready, abs(*rAdcStr.getEPointer(sadc::ROTOR_CURRENT)) > dMax,   ENotReadyId::SENS_CR_FAULT);
  check(Ready, abs(*rAdcStr.getEPointer(sadc::STATOR_CURRENT)) > dMax,  ENotReadyId::SENS_CS_FAULT);
  
  if(Ready == R::READY){
    SReady::setMessage(EReadyId::PUSK);
    SReady::setMessage(EReadyId::DRY);
    pSys_manager->setReady(State::ON);
  }else{
    SReady::clrMessage(EReadyId::PUSK);
    SReady::clrMessage(EReadyId::DRY);
    pSys_manager->setReady(State::OFF);
  }
  
  
  
}




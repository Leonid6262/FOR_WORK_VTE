#include "ReadyCheck.hpp" 
#include "message_factory.hpp"


CReadyCheck::CReadyCheck(CADC_STORAGE& rAdcStr, CDIN_STORAGE& rDinStr) : rAdcStr(rAdcStr), rDinStr(rDinStr) {}

void CReadyCheck::getManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}

void CReadyCheck::check(bool mode) { 
  
  if(!mode) return;
  
  Ready = R::READY;
  
  if(pSys_manager->USystemMode.Adjustment){
    SNotReady::setMessage(ENotReadyId::ADJ_MODE);
    Ready = R::NOT_READY;
  } else {
    SNotReady::clrMessage(ENotReadyId::ADJ_MODE);
  }
  
  if(!rDinStr.Bl_Contact_Q1()){
    SNotReady::setMessage(ENotReadyId::Q1_is_OFF);
    Ready = R::NOT_READY;
  } else {
    SNotReady::clrMessage(ENotReadyId::Q1_is_OFF);
  }
  
  if(abs(*rAdcStr.getEPointer(CADC_STORAGE::ROTOR_CURRENT)) > dMax){
    SNotReady::setMessage(ENotReadyId::SENS_CURR_FAULT);
    Ready = R::NOT_READY;
  } else {
    SNotReady::clrMessage(ENotReadyId::SENS_CURR_FAULT);
  }

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




#include "ReadyCheck.hpp" 

CReadyCheck::CReadyCheck(CADC_STORAGE& rAdcStr, CDIN_STORAGE& rDinStr) : rAdcStr(rAdcStr), rDinStr(rDinStr) {}

void CReadyCheck::check(bool Permission) { 
  
  if(!Permission) {
    CategoryUtils::clearMessages(ECategory::NOT_READY);
    CategoryUtils::clearMessages(ECategory::READY);
    pSys_manager->set_bsReadyCheck(State::OFF);
    return;
  }
  
  pSys_manager->set_bsReadyCheck(State::ON);
  
  Ready = R::READY;

  check(Ready, pSys_manager->USystemStatus.sAdjustment,                 ENotReadyId::ADJ_MODE );
  check(Ready, !rDinStr.Bl_Contact_Q1(),                                ENotReadyId::Q1_OFF);
  check(Ready, abs(*rAdcStr.getEPointer(sadc::ROTOR_CURRENT))  > dMax,  ENotReadyId::SENS_CR_FAULT);
  check(Ready, abs(*rAdcStr.getEPointer(sadc::ROTOR_VOLTAGE))  > dMax,  ENotReadyId::SENS_VR_FAULT);
  check(Ready, abs(*rAdcStr.getEPointer(sadc::STATOR_CURRENT)) > dMax,  ENotReadyId::SENS_CS_FAULT);
  check(Ready, !(*pSys_manager->rSIFU.getSyncStat()),                   ENotReadyId::NOT_SYNC);
  check(Ready, rDinStr.HVS_Status() == StatusHVS::ERR_BC,               ENotReadyId::BC_HVS_ERR);
  /*  
     Остальные условия готовности 
  */
  
  if(Ready == R::NOT_READY || prevKeyDrying) { check(Ready, rDinStr.Reg_Drying(),                  ENotReadyId::DRYING_ON); }
  if(Ready == R::NOT_READY || prevKeyTesting){ check(Ready, rDinStr.Stator_Key(),                  ENotReadyId::TESTING_ON);}
  if(Ready == R::NOT_READY || prevStatusHVS) { check(Ready, rDinStr.HVS_Status() == StatusHVS::ON, ENotReadyId::HVS_ON);    }
    
  prevKeyDrying  = rDinStr.Reg_Drying();
  prevKeyTesting = rDinStr.Stator_Key();
  prevStatusHVS = static_cast<bool>(rDinStr.HVS_Status());
 
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



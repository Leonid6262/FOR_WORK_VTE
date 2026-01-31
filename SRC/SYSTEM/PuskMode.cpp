#include "PuskMode.hpp"
#include "_SystemManager.hpp"

CPuskMode::CPuskMode(CDIN_STORAGE& rDinStr, CSIFU& rSIFU, CEEPSettings& rSet)
: rDinStr(rDinStr), rSIFU(rSIFU), rSet(rSet), cur_status(State::OFF), pSys_manager(nullptr) {
}

void CPuskMode::pusk(bool Permission) {
  
  if(!Permission) {
    cur_status = State::OFF; 
    pSys_manager->set_bsPuskMotor(State::OFF);
    SWork::clrMessage(EWorkId::PUSK); 
    return; 
  }
  
  if((rDinStr.HVS_Status() == StatusHVS::ON) && cur_status == State::OFF) { 
    pSys_manager->set_bsPuskMotor(State::ON);    
    cur_status = State::ON;
    phases_pusk = EPhasesPusk::CheckCurrent; 
    SWork::setMessage(EWorkId::PUSK);
    return;
  } 
  
  if((rDinStr.HVS_Status() == StatusHVS::OFF) && cur_status == State::ON) { 
    pSys_manager->set_bsPuskMotor(State::OFF);
    StopPusk();
    return;
  }
  
  switch(phases_pusk) {
  case EPhasesPusk::CheckCurrent:    CheckCurrent(); break;
  case EPhasesPusk::WaitCurrentDrop: WaitCurrentDrop(); break;
  case EPhasesPusk::Delay:           Delay(); break;
  case EPhasesPusk::Forcing:         Forcing(); break;
  case EPhasesPusk::RelayExOn:       RelayExOn(); break;
  case EPhasesPusk::RelayPause:      RelayPause(); break;
  case EPhasesPusk::ClosingKey:      ClosingKey(); break;
  case EPhasesPusk::ControlKey:      ControlKey(); break;
  }
}

void CPuskMode::CheckCurrent() {

}

void CPuskMode::WaitCurrentDrop() {

}

void CPuskMode::Delay() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if(dTrsPhase >= DELAY_TIME) {
    phases_pusk = EPhasesPusk::Forcing;
    prev_TC0_Phase = LPC_TIM0->TC;
  }
}

void CPuskMode::Forcing() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if (dTrsPhase >= rSet.getSettings().set_pusk.TFors * TICK_SEC) { 
    rSIFU.rReg_manager.rCurrent_reg.set_Iset(rSet.getSettings().work_set.Iset_0);
    rSIFU.main_bridge_pulses_On();
    phases_pusk = EPhasesPusk::RelayExOn;
    if(!rDinStr.CU_from_testing()) {
      SWarning::setMessage(EWarningId::PK_NOT_OPEN);
      PK_STATUS = StatusRet::ERROR;
    }
    prev_TC0_Phase = LPC_TIM0->TC;
  }  
}

void CPuskMode::RelayExOn() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if (dTrsPhase >= BRIDGE_CHANGAE) {
    rDinStr.Relay_Ex_Applied(State::ON);
    phases_pusk = EPhasesPusk::RelayPause;
    prev_TC0_Phase = LPC_TIM0->TC;
  }  
}

void CPuskMode::RelayPause() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if (dTrsPhase >= RELAY_PAUSE_OFF) {
    rSIFU.execute_mode_Wone();
    phases_pusk = EPhasesPusk::ClosingKey;
    prev_TC0_Phase = LPC_TIM0->TC;
  }
}

void CPuskMode::ClosingKey() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if (dTrsPhase >= CLOSING_KEY) { 
    phases_pusk = EPhasesPusk::ControlKey;
    if(rDinStr.CU_from_testing()) {
      SWarning::setMessage(EWarningId::PK_NOT_CLOSE);
      PK_STATUS = StatusRet::ERROR;
    }
    prev_TC0_Phase = LPC_TIM0->TC;
  }  
}

void CPuskMode::ControlKey() {
  if(PK_STATUS == StatusRet::ERROR) {
    StopPusk();
  } else {
    // Сообщения и переход
  }
}



void CPuskMode::StopPusk() {
  
}

void CPuskMode::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}


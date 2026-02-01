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
  
  if((rDinStr.HVS_Status() == StatusHVS::ON) && cur_status == State::OFF && !rDinStr.ControlPusk()) { 
    pSys_manager->set_bsPuskMotor(State::ON);    
    cur_status = State::ON;
    prev_cu = false;
    cu_toggle_count = 0;
    phases_pusk = EPhasesPusk::CheckISctrlPK; 
    SWork::setMessage(EWorkId::PUSK);
    prev_TC0_Phase = LPC_TIM0->TC;
    return;
  } 
  
  if((rDinStr.HVS_Status() == StatusHVS::OFF) && cur_status == State::ON) { 
    pSys_manager->set_bsPuskMotor(State::OFF);
    StopPusk();
    return;
  }
  
  if(cur_status == State::OFF) return;
  
  switch(phases_pusk) {
  case EPhasesPusk::CheckISctrlPK:    CheckISctrlPK();    break;
  case EPhasesPusk::WaitISdropOrSlip: WaitISdropOrSlip(); break;
  case EPhasesPusk::Delay:            Delay();            break;
  case EPhasesPusk::Forcing:          Forcing();          break;
  case EPhasesPusk::RelayExOn:        RelayExOn();        break;
  case EPhasesPusk::RelayPause:       RelayPause();       break;
  case EPhasesPusk::ClosingKey:       ClosingKey();       break;
  }
}

void CPuskMode::CheckISctrlPK() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if(dTrsPhase < CHECK_IS) {
    bool cur_cu = rDinStr.CU_from_testing();
    if(cur_cu != prev_cu) { 
      cu_toggle_count++; 
      prev_cu = cur_cu;
      prev_cu_time = LPC_TIM0->TC;
    }    
    return;
  }
  if(*rSIFU.rPulsCalc.getPointer_istator_rms() < rSet.getSettings().set_pusk.ISPusk*0.5f) {
    SFault::setMessage(EFaultId::NOT_IS);
    pSys_manager->rFault_ctrl.fault_stop();
  }
  if(cu_toggle_count < N_CU_TOGGLE) {
    SFault::setMessage(EFaultId::PK_FAULT);
    pSys_manager->rFault_ctrl.fault_stop();
  }
  if(!pSys_manager->USystemStatus.sFault) {
    prev_cu = rDinStr.CU_from_testing();
    cur_slip = 1.0f;
    pusk_slip = 1.0f;
    phases_pusk = EPhasesPusk::WaitISdropOrSlip;
    prev_TC0_Phase = LPC_TIM0->TC;
  }
}

void CPuskMode::WaitISdropOrSlip() {
  
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if(dTrsPhase > rSet.getSettings().set_pusk.TPusk * TICK_SEC) {
    SFault::setMessage(EFaultId::LONG_PUSK);
    pSys_manager->rFault_ctrl.fault_stop(); 
    return;
  }  
  
  bool cur_cu = rDinStr.CU_from_testing();
  if(cur_cu != prev_cu) {
    prev_cu = cur_cu;
    
    unsigned int now = LPC_TIM0->TC;
    float TSlipPhase = static_cast<float>(now - prev_cu_time);
    prev_cu_time = now;
    
    cur_slip = 1.0f - (HALF_NET_PERIOD / TSlipPhase);
    
  }
  
  if(*rSIFU.rPulsCalc.getPointer_istator_rms() <= rSet.getSettings().set_pusk.ISPusk) {
    SWork::setMessage(EWorkId::PUSK_ON_IS);
    phases_pusk = EPhasesPusk::Delay;
    prev_TC0_Phase = LPC_TIM0->TC;
    pusk_slip = cur_slip;
    return;
  } 
  
  if(cur_slip <= rSet.getSettings().set_pusk.sPusk) {
    SWork::setMessage(EWorkId::PUSK_ON_SLIPE);    
    phases_pusk = EPhasesPusk::Delay;
    prev_TC0_Phase = LPC_TIM0->TC;
    pusk_slip = cur_slip;
  } 
  
}

void CPuskMode::Delay() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if(dTrsPhase >= DELAY_TIME) {
    rSIFU.set_alpha(rSIFU.s_const.AMax);
    rSIFU.forcing_bridge_pulses_On();
    rSIFU.rReg_manager.rCurrent_reg.set_Iset(rSet.getSettings().set_pusk.IFors);
    rSIFU.rReg_manager.setCurrent(State::ON);    
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
    if(rDinStr.CU_from_testing()) {
      SFault::setMessage(EFaultId::PK_NOT_CLOSED);
      pSys_manager->rFault_ctrl.fault_stop();
      return;
    }
    SWork::clrMessage(EWorkId::PUSK);
    pSys_manager->set_bsPuskMotor(State::OFF);
    pSys_manager->set_bsWorkNormal(State::ON);
  }  
}

void CPuskMode::StopPusk(){
  SWork::clrMessage(EWorkId::PUSK);
  rDinStr.Relay_Ex_Applied(State::OFF);  
  pSys_manager->set_bsPuskMotor(State::OFF);
  rSIFU.rReg_manager.rCurrent_reg.set_Iset(0);
  rSIFU.rReg_manager.setCurrent(State::OFF);
  rSIFU.all_bridge_pulses_Off();
}

void CPuskMode::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}


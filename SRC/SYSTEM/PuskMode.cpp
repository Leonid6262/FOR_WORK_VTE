#include "PuskMode.hpp"
#include "_SystemManager.hpp"

CPuskMode::CPuskMode(CDIN_STORAGE& rDinStr, CSIFU& rSIFU, CEEPSettings& rSet)
: rDinStr(rDinStr), rSIFU(rSIFU), rSet(rSet), cur_status(State::OFF), pSys_manager(nullptr) {
  
  LPC_IOCON->P2_15 = IOCON_T2_CAP1;  // T2 CAP1
  LPC_TIM2->MCR = 0x00000000;        // disabled
  LPC_TIM2->IR = 0xFFFFFFFF;         // Очистка флагов прерываний
  LPC_TIM2->TCR |= TIM2_TCR_START;   // Старт таймера TIM3 
  LPC_TIM2->TC = 0;
  LPC_TIM2->CCR = TIM2_CAPTURE_RI;    // Захват T2 по спаду CAP1 без прерываний

}

void CPuskMode::pusk(bool Permission) {
  
  if(!Permission) {
    cur_status = State::OFF; 
    pSys_manager->set_bsPuskMotor(State::OFF);
    SWork::clrMessage(EWorkId::PUSK); 
    return; 
  }
  
  if(rDinStr.HVS_Status() == StatusHVS::ON && cur_status == State::OFF) { 
    pSys_manager->set_bsPuskMotor(State::ON);    
    cur_status = State::ON;
    cur_slip = -1.0f;
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
  case EPhasesPusk::SelfSync:         SelfSync();         break;
  case EPhasesPusk::Forcing:          Forcing();          break;
  case EPhasesPusk::RelayExOn:        RelayExOn();        break;
  case EPhasesPusk::RelayPause:       RelayPause();       break;
  case EPhasesPusk::ClosingKey:       ClosingKey();       break;
  }
}

// ---Ожидание тока статора и проверка работы ПК (Пускового ключа)--- 
void CPuskMode::CheckISctrlPK() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if(dTrsPhase < CHECK_IS) {
    cur_slip = CPuskMode::calc_slip(cur_slip);
    return;
  }
  if(*rSIFU.rPulsCalc.getPointer_istator_rms() < rSet.getSettings().set_pusk.ISPusk*0.5f) {
    ////SFault::setMessage(EFaultId::NOT_IS);
    ////pSys_manager->rFault_ctrl.fault_stop();
  }
  if(cur_slip < 0) {
    SFault::setMessage(EFaultId::PK_FAULT);
    pSys_manager->rFault_ctrl.fault_stop();
  }
  if(!pSys_manager->USystemStatus.sFault) {
    pusk_slip = 1.0f;
    phases_pusk = EPhasesPusk::WaitISdropOrSlip;
    prev_TC0_Phase = LPC_TIM0->TC;
  }
}

// ---Ожидание снижения тока статора до уставки подачи возбуждения---
void CPuskMode::WaitISdropOrSlip() {
  
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if(dTrsPhase > rSet.getSettings().set_pusk.TPusk * TICK_SEC) {
    ////SFault::setMessage(EFaultId::LONG_PUSK);
    ////pSys_manager->rFault_ctrl.fault_stop(); 
    ////return;
  }  
  cur_slip = CPuskMode::calc_slip(cur_slip); 
  
  /*
  if(*rSIFU.rPulsCalc.getPointer_istator_rms() <= rSet.getSettings().set_pusk.ISPusk) {
    SWork::setMessage(EWorkId::PUSK_ON_IS);
    phases_pusk = EPhasesPusk::SelfSync;
    prev_TC0_Phase = LPC_TIM0->TC;
    pusk_slip = cur_slip;
    return;
  } 
  
  if(cur_slip <= rSet.getSettings().set_pusk.sPusk) {
    SWork::setMessage(EWorkId::PUSK_ON_SLIPE);    
    phases_pusk = EPhasesPusk::SelfSync;
    prev_TC0_Phase = LPC_TIM0->TC;
    pusk_slip = cur_slip;
  } */
  
}

// ---Фаза самосинхронизации---
void CPuskMode::SelfSync() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if(dTrsPhase >= rSet.getSettings().set_pusk.TSelfSync * TICK_SEC) {
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
      ////pSys_manager->rFault_ctrl.fault_stop();
      ////return;
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


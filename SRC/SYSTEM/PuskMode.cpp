#include "PuskMode.hpp"
#include "_SystemManager.hpp"

CPuskMode::CPuskMode(CDIN_STORAGE& rDinStr, CSIFU& rSIFU, CEEPSettings& rSet)
: rDinStr(rDinStr), rSIFU(rSIFU), rSet(rSet), pAdc(CADC_STORAGE::getInstance()) {
  INIT_CAPTURE1_TIM2();
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
    slip_status.slip_value = -1.0f;
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
  case EPhasesPusk::WaitISdrop:       WaitISdrop();       break;
  case EPhasesPusk::SelfSync:         SelfSync();         break;
  case EPhasesPusk::Forcing:          Forcing();          break;
  case EPhasesPusk::Pause:            Pause();            break;
  case EPhasesPusk::ClosingKey:       ClosingKey();       break;
  }
}

// ---Ожидание тока статора и проверка работы ПК (Пускового ключа)--- 
void CPuskMode::CheckISctrlPK() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if(dTrsPhase < CHECK_IS) {
    status_slip();
    return;
  }
  if(*rSIFU.rPulsCalc.getPointer_istator_rms() < rSet.getSettings().set_pusk.ISPusk*0.5f) {
    SFault::setMessage(EFaultId::NOT_IS);
    pSys_manager->rFault_ctrl.fault_stop();
  }
  if(slip_status.slip_value < 0) {
    SFault::setMessage(EFaultId::PK_FAULT);
    pSys_manager->rFault_ctrl.fault_stop();
  }
  if(!pSys_manager->USystemStatus.sFault) {
    pusk_slip = 1.0f;
    phases_pusk = EPhasesPusk::WaitISdrop;
    prev_TC0_Phase = LPC_TIM0->TC;
  }
}

// ---Ожидание снижения тока статора до уставки подачи возбуждения---
void CPuskMode::WaitISdrop() {
  
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if(dTrsPhase > rSet.getSettings().set_pusk.TPusk * TICK_SEC) {
    SFault::setMessage(EFaultId::LONG_PUSK);
    pSys_manager->rFault_ctrl.fault_stop(); 
    return;
  }  

  if(*rSIFU.rPulsCalc.getPointer_istator_rms() <= rSet.getSettings().set_pusk.ISPusk) {
    phases_pusk = EPhasesPusk::SelfSync;
    prev_TC0_Phase = LPC_TIM0->TC;
    return;
  } 
  status_slip(); 
}

// ---Фаза самосинхронизации---
void CPuskMode::SelfSync() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  status_slip();
  if((slip_status.slip_value >= rSet.getSettings().set_pusk.sPusk) && slip_status.slip_event) {
    SWork::setMessage(EWorkId::PUSK_ON_SLIPE);
    StartEx();
    return;
  }    
  slip_status.slip_event = false;
  if((dTrsPhase >= rSet.getSettings().set_pusk.TSelfSync * TICK_SEC) && slip_status.slip_event) {
    SWork::setMessage(EWorkId::PUSK_ON_IS);
    StartEx();
  }
}

// ---Подача возбуждения---
void CPuskMode::StartEx() { 
  rDinStr.Relay_Ex_Applied(State::ON);
  pusk_slip = slip_status.slip_value;
  rSIFU.set_alpha(rSIFU.s_const.AMax);
  rSIFU.forcing_bridge_pulses_On();
  rSIFU.rReg_manager.rCurrent_reg.set_Iset(rSet.getSettings().set_pusk.IFors);
  rSIFU.rReg_manager.setCurrent(State::ON);    
  phases_pusk = EPhasesPusk::Forcing;
  prev_TC0_Phase = LPC_TIM0->TC;
}

// ---Форсировка---
void CPuskMode::Forcing() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if (dTrsPhase >= rSet.getSettings().set_pusk.TFors * TICK_SEC) { 
    rSIFU.rReg_manager.rCurrent_reg.set_Iset(rSet.getSettings().work_set.Iset_0);
    rSIFU.main_bridge_pulses_On();
    phases_pusk = EPhasesPusk::Pause;
    prev_TC0_Phase = LPC_TIM0->TC;
  }  
}

// ---Пауза перед закрытием ПК---
void CPuskMode::Pause() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if (dTrsPhase >= PAUSE) { 
    rSIFU.execute_mode_Wone();
    phases_pusk = EPhasesPusk::ClosingKey;
    prev_TC0_Phase = LPC_TIM0->TC;
  }
}

// ---Закрытие ПК и переход в режим Работа---
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

// ---Штатная остановка Пуска---
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


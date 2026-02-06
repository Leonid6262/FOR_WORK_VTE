#include "WorkMode.hpp" 
#include "_SystemManager.hpp"

CWorkMode::CWorkMode(CDIN_STORAGE& rDinStr, CSIFU& rSIFU, CEEPSettings& rSet) : 
  rDinStr(rDinStr), rSIFU(rSIFU), rSet(rSet), cur_status(State::OFF), pSys_manager(nullptr) {
  // Приращение задания dIset = Iset'*dT
  dIset = static_cast<unsigned short>((rSet.getSettings().work_set.derivIset * ChangeInterval) + 0.5f); 
}

void CWorkMode::work(bool Permission) {
  
  if(!Permission) {
    cur_status = State::OFF; 
    pSys_manager->set_bsWorkNormal(State::OFF);
    SWork::clrMessage(EWorkId::CURRENT_REG);
    SWork::clrMessage(EWorkId::Q_POWER_REG);
    SWork::clrMessage(EWorkId::COS_REG);  
    Iset = rSet.getSettings().work_set.Iset_0;
    return; 
  }
  
  if(pSys_manager->USystemStatus.all & CSystemManager::SBit::bsWorkNormal) {
    cur_status = State::ON;    
    unsigned char reg_mode = pSys_manager->rReg_manager.URegMode_request.all;
    if (reg_mode == CRegManager::MBit::Current) {
      SWork::setMessage(EWorkId::CURRENT_REG);
      SWork::clrMessage(EWorkId::Q_POWER_REG);
      SWork::clrMessage(EWorkId::COS_REG);
      CurrentSetting();
    } else if (reg_mode == CRegManager::MBit::QPower) {
      SWork::setMessage(EWorkId::Q_POWER_REG);
      SWork::clrMessage(EWorkId::CURRENT_REG);
      SWork::clrMessage(EWorkId::COS_REG); 
    } else if (reg_mode == CRegManager::MBit::CosPhi) {
      SWork::setMessage(EWorkId::COS_REG);
      SWork::clrMessage(EWorkId::CURRENT_REG);
      SWork::clrMessage(EWorkId::Q_POWER_REG);
    }
  }
  
  if((rDinStr.HVS_Status() == StatusHVS::OFF) && cur_status == State::ON) { 
    pSys_manager->set_bsWorkNormal(State::OFF);
    cur_status = State::OFF; 
    StopWork();
    return;
  }
  
}

// Задание тока
void CWorkMode::CurrentSetting() {
  
  dTrsReg = LPC_TIM0->TC - prev_TC0_Reg;
  if (dTrsReg >= ChangeInterval * TICK_SEC ) {
    prev_TC0_Reg = LPC_TIM0->TC;

    if(rDinStr.Setting_More()) { Iset += dIset; }   
    if(rDinStr.Setting_Less()) { Iset -= dIset; }
    
    if(Iset > rSet.getSettings().work_set.IsetMax) { Iset = rSet.getSettings().work_set.IsetMax; }
    if(Iset < rSet.getSettings().work_set.IsetMin) { Iset = rSet.getSettings().work_set.IsetMin; }
    
    rSIFU.rReg_manager.rCurrent_reg.set_Iset(Iset);
  }
  
}

// ---Штатная остановка Работы---
void CWorkMode::StopWork(){
  rDinStr.Relay_Ex_Applied(State::OFF);  
  rSIFU.rReg_manager.rCurrent_reg.set_Iset(0);
  rSIFU.rReg_manager.setCurrent(State::OFF);
  rSIFU.all_bridge_pulses_Off();
}


void CWorkMode::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}

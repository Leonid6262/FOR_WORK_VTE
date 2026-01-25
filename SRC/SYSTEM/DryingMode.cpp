#include "DryingMode.hpp" 
#include "_SystemManager.hpp"

CDryingMode::CDryingMode(CDIN_STORAGE& rDinStr, CSIFU& rSIFU, CEEPSettings& rSet) : 
  rDinStr(rDinStr), cur_status(State::OFF), rSIFU(rSIFU), rSet(rSet) {}

void CDryingMode::dry(bool Permission) { 
  
  if(!Permission) {
    cur_status = State::OFF; 
    pSys_manager->set_bsWorkDry(State::OFF);
    SWork::clrMessage(EWorkId::DRYING); 
    return; 
  }
  
  if(rDinStr.Reg_Drying() && cur_status == State::OFF) { 
    pSys_manager->set_bsWorkDry(State::ON);
    StartDrain(); 
  } 
  
  if(!rDinStr.Reg_Drying() && cur_status == State::ON) { 
    pSys_manager->set_bsWorkDry(State::OFF);
    StopDrain();  
  }
  
  switch (cur_status) {
  case State::ON:
    SWork::setMessage(EWorkId::DRYING);
    break;
  case State::OFF:
    SWork::clrMessage(EWorkId::DRYING);    
    break;
  }
  
}   

void CDryingMode::StartDrain(){

  
  
  
  
  
  rDinStr.Relay_Ex_Applied(State::ON);
  
  OnEx();
  cur_status = State::ON;
}

void CDryingMode::OnEx() { 
  rSIFU.set_alpha(rSIFU.s_const.AMax);
  rSIFU.main_bridge_pulses_On();
  rSIFU.rReg_manager.rCurrent_reg.set_Iset(rSet.getSettings().work_set.Idry_0);
  rSIFU.rReg_manager.setCurrent(State::ON);
}

void CDryingMode::StopDrain(){
  rDinStr.Relay_Ex_Applied(State::OFF);
  rSIFU.rReg_manager.rCurrent_reg.set_Iset(0);
  rSIFU.rReg_manager.setCurrent(State::OFF);
  rSIFU.all_bridge_pulses_Off();
  cur_status = State::OFF;
}

void CDryingMode::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}


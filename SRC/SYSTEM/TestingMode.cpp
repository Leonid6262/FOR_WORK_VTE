#include "TestingMode.hpp"
#include "_SystemManager.hpp"

CTestingMode::CTestingMode(CDIN_STORAGE& rDinStr, CSIFU& rSIFU, CEEPSettings& rSet) : 
  rDinStr(rDinStr), cur_status(State::OFF), rSIFU(rSIFU), rSet(rSet) {}
    
void CTestingMode::test(bool Permission) {
  
    if(!Permission) {
    cur_status = State::OFF; 
    pSys_manager->set_bsWorkTest(State::OFF);
    SWork::clrMessage(EWorkId::TESTING); 
    return; 
  }
  
  if(rDinStr.Stator_Key() && cur_status == State::OFF) { StartTest(); } 
  if(!rDinStr.Stator_Key() && cur_status == State::ON) { StopTest();  }
  
  switch (cur_status) {
  case State::ON:
    SWork::setMessage(EWorkId::TESTING);
    break;
  case State::OFF:
    SWork::clrMessage(EWorkId::TESTING);    
    break;
  }
  
}

void CTestingMode::StartTest(){
  cur_status = State::ON;
  pSys_manager->set_bsWorkTest(State::ON);
  rSIFU.set_alpha(rSIFU.s_const.AMax);
  rSIFU.forcing_bridge_pulses_On();
  rSIFU.rReg_manager.rCurrent_reg.set_Iset(rSet.getSettings().set_reg.Ifors);
  rSIFU.rReg_manager.setCurrent(State::ON);
  
  rSIFU.main_bridge_pulses_On();
  rSIFU.rReg_manager.rCurrent_reg.set_Iset(rSet.getSettings().set_reg.Iset0);
  
  //rSIFU.set_mode_w_one();
  
  //rSIFU.set_normal_mode();
  
}

void CTestingMode::StopTest(){
  cur_status = State::OFF;
  pSys_manager->set_bsWorkTest(State::OFF);
  rSIFU.rReg_manager.rCurrent_reg.set_Iset(0);
  rSIFU.rReg_manager.setCurrent(State::OFF);
  rSIFU.all_bridge_pulses_Off();
}

void CTestingMode::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}


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
  
  if(rDinStr.Stator_Key() && cur_status == State::OFF) { 
    cur_status = State::ON;
    pSys_manager->set_bsWorkTest(State::ON);
    phases_test = EPhasesTest::StartMode; 
    SWork::setMessage(EWorkId::TESTING);
  } 
  
  if(!rDinStr.Stator_Key() && cur_status == State::ON) { 
    cur_status = State::OFF;
    StopTest();  
  }
  
  switch (phases_test) {
  case EPhasesTest::StartMode:
    rSIFU.set_alpha(rSIFU.s_const.AMax);
    rSIFU.forcing_bridge_pulses_On();
    rSIFU.rReg_manager.rCurrent_reg.set_Iset(rSet.getSettings().set_pusk.IFors);
    rSIFU.rReg_manager.setCurrent(State::ON);
    phases_test = EPhasesTest::Forcing;
    PK_STATUS = StatusRet::SUCCESS;
    prev_TC0 = LPC_TIM0->TC;
    break;  
  case EPhasesTest::Forcing:
    dTrs = LPC_TIM0->TC - prev_TC0;
    if (dTrs >= rSet.getSettings().set_pusk.TFors * 10000000) { 
      rSIFU.rReg_manager.rCurrent_reg.set_Iset(rSet.getSettings().work_set.Iset_0);
      rSIFU.main_bridge_pulses_On();
      phases_test = EPhasesTest::BridgeChange;
      if(!rDinStr.CU_from_testing()) {
        SWarning::setMessage(EWarningId::PK_NOT_OPEN);
        PK_STATUS = StatusRet::ERROR;
      }
      prev_TC0 = LPC_TIM0->TC;
    }
    break;
  case EPhasesTest::BridgeChange:
    dTrs = LPC_TIM0->TC - prev_TC0;
    if (dTrs >= BRIDGE_CHANGAE) {
      rDinStr.Relay_Ex_Applied(State::ON);
      phases_test = EPhasesTest::RelayPause;
      prev_TC0 = LPC_TIM0->TC;
    }
    break;
  case EPhasesTest::RelayPause:
    dTrs = LPC_TIM0->TC - prev_TC0;
    if (dTrs >= RELAY_PAUSE_OFF) {
      rSIFU.execute_mode_Wone();
      phases_test = EPhasesTest::ClosingKey;
      prev_TC0 = LPC_TIM0->TC;
    }
    break;
  case EPhasesTest::ClosingKey:
    dTrs = LPC_TIM0->TC - prev_TC0;
    if (dTrs >= CLOSING_KEY) { 
      phases_test = EPhasesTest::Regulation;
      if(rDinStr.CU_from_testing()) {
        SWarning::setMessage(EWarningId::PK_NOT_CLOSE);
        PK_STATUS = StatusRet::ERROR;
      }
      prev_TC0 = LPC_TIM0->TC;
    }
    break;
  case EPhasesTest::Regulation:
    if(PK_STATUS == StatusRet::ERROR) {
      rSIFU.rReg_manager.rCurrent_reg.set_Iset(0);
      rSIFU.rReg_manager.setCurrent(State::OFF);
      rSIFU.all_bridge_pulses_Off();
      break; 
    }
    
    
    break; 
  }
  
}

void CTestingMode::StopTest(){
  SWork::clrMessage(EWorkId::TESTING);
  pSys_manager->set_bsWorkTest(State::OFF);
  rSIFU.rReg_manager.rCurrent_reg.set_Iset(0);
  rSIFU.rReg_manager.setCurrent(State::OFF);
  rSIFU.all_bridge_pulses_Off();
}

void CTestingMode::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}


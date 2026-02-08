#include "TestingMode.hpp"
#include "_SystemManager.hpp"

CTestingMode::CTestingMode(CDIN_STORAGE& rDinStr, CSIFU& rSIFU, CEEPSettings& rSet) : 
  rDinStr(rDinStr), rSIFU(rSIFU), rSet(rSet), cur_status(State::OFF), pSys_manager(nullptr) {
  // Приращение задания dIset = Iset'*dT
  dIset = static_cast<unsigned short>((rSet.getSettings().work_set.derivIset * ChangeInterval) + 0.5f); 
} 

void CTestingMode::test(bool Permission) {
  
  if(!Permission) {
    cur_status = State::OFF; 
    pSys_manager->set_bsWorkTest(State::OFF);
    SWork::clrMessage(EWorkId::TESTING); 
    return; 
  }
  
  if(pSys_manager->USystemStatus.sWorkTest && cur_status == State::OFF) { 
    cur_status = State::ON;
    rDinStr.Relay_Premission(State::OFF);
    phases_test = EPhasesTest::StartMode; 
    SWork::setMessage(EWorkId::TESTING);
    return;
  } 
  
  if(!pSys_manager->USystemStatus.sWorkTest && cur_status == State::ON) { 
    cur_status = State::OFF;
    StopTest();
    return;
  }
  
  if(cur_status == State::OFF) return;
  
  switch (phases_test) {
  case EPhasesTest::StartMode:    StartPhase();   break;  
  case EPhasesTest::Forcing:      Forcing();      break;
  case EPhasesTest::RelayExOn:    RelayExOn();    break;
  case EPhasesTest::RelayPause:   RelayPause();   break;
  case EPhasesTest::ClosingKey:   ClosingKey();   break;
  case EPhasesTest::ControlKey:   ControlKey();   break;  
  case EPhasesTest::Regulation:   Regulation();   break; 
  }
  
}

// Включение ИУ ф.моста и РТ с заданием форсировочного значения
void CTestingMode::StartPhase() {
  rSIFU.set_alpha(rSet.getSettings().set_reg.A0);
  rSIFU.forcing_bridge_pulses_On();
  rSIFU.rReg_manager.rCurrent_reg.set_Iset(rSet.getSettings().set_pusk.IFors);
  rSIFU.rReg_manager.setCurrent(State::ON);
  phases_test = EPhasesTest::Forcing;
  PK_STATUS = StatusRet::SUCCESS;
  prev_TC0_Phase = LPC_TIM0->TC;  
}

// Форсировка, переключение на рабочий мост и проверк открытого состояния ПК
void CTestingMode::Forcing() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if (dTrsPhase >= rSet.getSettings().set_pusk.TFors * TICK_SEC) { 
    rSIFU.rReg_manager.rCurrent_reg.set_Iset(rSet.getSettings().work_set.Iset_0);
    rSIFU.main_bridge_pulses_On();
    phases_test = EPhasesTest::RelayExOn;
    if(!rDinStr.CU_from_testing()) {
      SWarning::setMessage(EWarningId::PK_NOT_OPEN);
      PK_STATUS = StatusRet::ERROR;
    }
    prev_TC0_Phase = LPC_TIM0->TC;
  }  
}

// Включение реле "Возбуждение подано" после паузы
void CTestingMode::RelayExOn() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if (dTrsPhase >= BRIDGE_CHANGAE) {
    rDinStr.Relay_Ex_Applied(State::ON);
    phases_test = EPhasesTest::RelayPause;
    prev_TC0_Phase = LPC_TIM0->TC;
  }  
}

// Пауза на включение реле и включение режима СИФУ "Через один"
void CTestingMode::RelayPause() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if (dTrsPhase >= RELAY_PAUSE_OFF) {
    rSIFU.execute_mode_Wone();
    phases_test = EPhasesTest::ClosingKey;
    prev_TC0_Phase = LPC_TIM0->TC;
  }
}

// Проверка закрытого состояния ПК
void CTestingMode::ClosingKey() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if (dTrsPhase >= CLOSING_KEY) { 
    phases_test = EPhasesTest::ControlKey;
    if(rDinStr.CU_from_testing()) {
      SWarning::setMessage(EWarningId::PK_NOT_CLOSE);
      PK_STATUS = StatusRet::ERROR;
    }
    prev_TC0_Phase = LPC_TIM0->TC;
  }  
}

// Переход к регулированию если ПК Ok.
void CTestingMode::ControlKey() {
  if(PK_STATUS == StatusRet::ERROR) {
    StopTest();
  } else {
    Iset = rSet.getSettings().work_set.Iset_0;
    phases_test = EPhasesTest::Regulation;
    SWork::clrMessage(EWorkId::TESTING);
    SWork::setMessage(EWorkId::TESTING_OK);
    prev_TC0_Phase = LPC_TIM0->TC;
    prev_TC0_Reg = LPC_TIM0->TC;
  }
}

// Регулирование до окончания режима
void CTestingMode::Regulation() {
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  if (dTrsPhase >= rSet.getSettings().set_pusk.TPusk * TICK_SEC) {
    StopTest();
    return;
  }
  
  dTrsReg = LPC_TIM0->TC - prev_TC0_Reg;
  if (dTrsReg >= ChangeInterval * TICK_SEC ) {
    prev_TC0_Reg = LPC_TIM0->TC;
       
    if(rDinStr.Setting_More()) { Iset += dIset; }   
    if(rDinStr.Setting_Less()) { Iset -= dIset; }
    
    if(Iset > rSet.getSettings().work_set.IsetMax) { Iset = rSet.getSettings().work_set.IsetMax; }
    if(Iset < rSet.getSettings().work_set.IsetMin) { Iset = rSet.getSettings().work_set.IsetMin; }
    
    rSet.getSettings().work_set.Iset_0 = Iset;
    rSIFU.rReg_manager.rCurrent_reg.set_Iset(Iset);
  }
  
}

// Остановка теста
void CTestingMode::StopTest(){
  SWork::clrMessage(EWorkId::TESTING);
  SWork::clrMessage(EWorkId::TESTING_OK);
  rDinStr.Relay_Ex_Applied(State::OFF);
  pSys_manager->set_bsWorkTest(State::OFF);
  rSIFU.rReg_manager.rCurrent_reg.set_Iset(0);
  rSIFU.rReg_manager.setCurrent(State::OFF);
  rSIFU.all_bridge_pulses_Off();
}


void CTestingMode::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}



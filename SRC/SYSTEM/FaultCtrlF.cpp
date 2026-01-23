#include "FaultCtrlF.hpp" 
#include "LPC407x_8x_177x_8x.h"
#include "lpc177x_8x_can.h"
#include "lpc177x_8x_exti.h"
#include "_SystemManager.hpp"

CFaultCtrlF::CFaultCtrlF(CDIN_STORAGE& rDinStr) : rDinStr(rDinStr), pSys_manager(nullptr) {}

void CFaultCtrlF::control(bool Permission) {  
  
  if(!Permission) {return;}
  
  Fault = F::NOT_FAULT;
  
  check(Fault, !rDinStr.Bl_Contact_Q1(),      EFaultId::Q1_TRIPPED);
  /*  
     Остальные аварийные ситуации определяемые в фоновом цикле 
  */
  
  
  if(Fault == F::FAULT) fault_stop();
}

void CFaultCtrlF::fault_stop() {
  pSys_manager->USystemStatus.all = 0;
  pSys_manager->set_bsFault(State::ON);
  pSys_manager->rReg_manager.setCurrent(State::OFF);
  pSys_manager->rReg_manager.setQPower(State::OFF);
  pSys_manager->rReg_manager.setCosPhi(State::OFF);
  pSys_manager->rSIFU.pulses_stop();
  rDinStr.Relay_FAULT(State::ON);
}

void CFaultCtrlF::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}


// Реализация Синглтон-прокси к EINT2_IRQHandler
CProxyHandlerEINT2::CProxyHandlerEINT2() : pFaultCtrl(nullptr) {}

CProxyHandlerEINT2& CProxyHandlerEINT2::getInstance() {
  static CProxyHandlerEINT2 instance;
  return instance;
}
void CProxyHandlerEINT2::set_pFaultCtrl(CFaultCtrlF* pFaultCtrl) { 
  this->pFaultCtrl = pFaultCtrl;
}
// Handler EINT2
extern "C" {  
  void  EINT2_IRQHandler( void ) {
    LPC_SC->EXTINT |= EXTI_EINT2_BIT_MARK;
    SFault::setMessage(EFaultId::ID_MAX_HARD);
    CProxyHandlerEINT2::getInstance().pFaultCtrl->fault_stop();
  }
}

// Инициализация прерывания EINT2
void CFaultCtrlF::initEINT2() {
  LPC_IOCON->P2_12  = 0x1;   
  
  EXTI_InitTypeDef EXTICfg;
  EXTICfg.EXTI_Line = EXTI_EINT2;
  EXTICfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
  EXTICfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
  EXTI_Config(&EXTICfg);
  
  NVIC_EnableIRQ(EINT2_IRQn);
  LPC_SC->EXTINT |= EXTI_EINT2_BIT_MARK;
}

#include "FaultCtrlF.hpp" 
#include "LPC407x_8x_177x_8x.h"
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
  
  pSys_manager->rSIFU.all_bridge_pulses_Off();
  pSys_manager->rSIFU.rPulsCalc.stopDetectRotorPhase();
  
  rDinStr.Relay_FAULT(State::ON);
  rDinStr.Relay_Premission(State::OFF);
  
  rDinStr.Relay_Ex_Applied(State::OFF);
  pSys_manager->rReg_manager.rCurrent_reg.ResPusk = RPusk::CONNECT;
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
    LPC_SC->EXTINT |= CFaultCtrlF::EINT2_BIT_MARK;
    SFault::setMessage(EFaultId::ID_MAX_HARD);
    CProxyHandlerEINT2::getInstance().pFaultCtrl->fault_stop();
  }
}

// Инициализация прерывания EINT2
void CFaultCtrlF::initEINT2() {
  
  LPC_IOCON->P2_12  = IOCON_EINT2;    
  
  LPC_SC->EXTMODE |= (1 << LineEINT2);     // EDGE
  LPC_SC->EXTPOLAR &= ~(1 << LineEINT2);   // FALLING 
  LPC_SC->EXTINT |= 0x0F;
  
  NVIC_EnableIRQ(EINT2_IRQn);

}

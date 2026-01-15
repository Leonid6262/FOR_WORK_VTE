#include "FaultCtrlF.hpp" 
#include "LPC407x_8x_177x_8x.h"
#include "lpc177x_8x_can.h"
#include "lpc177x_8x_exti.h"
#include "SystemManager.hpp"

CFaultCtrlF::CFaultCtrlF() : pSys_manager(nullptr) {}

void CFaultCtrlF::control(bool mode) {  
  
    Fault = F::NOT_FAULT;
  /*

    Аварийные ситуации определяемые в фоновом цикле

  */
}

void CFaultCtrlF::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}


// Реализация Синглтон-прокси к EINT2_IRQHandler
CProxyHandlerEINT2::CProxyHandlerEINT2() : pSys_manager(nullptr) {}

CProxyHandlerEINT2& CProxyHandlerEINT2::getInstance() {
  static CProxyHandlerEINT2 instance;
  return instance;
}
void CProxyHandlerEINT2::set_pointers(CSystemManager* pSys_manager, CFaultCtrlF* pFaultCtrl) { 
  this->pSys_manager = pSys_manager;
  this->pFaultCtrl = pFaultCtrl;
}

extern "C" {  
  void  EINT2_IRQHandler( void )
  {
    LPC_SC->EXTINT |= EXTI_EINT2_BIT_MARK;
    auto* pFault = CProxyHandlerEINT2::getInstance().pFaultCtrl;
    auto* pSys_manager = CProxyHandlerEINT2::getInstance().pSys_manager;
    pSys_manager->setFault(State::ON);
    pSys_manager->rReg_manager.setCurrent(State::OFF);
    pSys_manager->rReg_manager.setQPower(State::OFF);
    pSys_manager->rReg_manager.setCosPhi(State::OFF);
    SFault::setMessage(EFaultId::ID_MAX_HARD);
    CategoryUtils::clearMessages(ECategory::COUNT);
    pSys_manager->rSIFU.pulses_stop();
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

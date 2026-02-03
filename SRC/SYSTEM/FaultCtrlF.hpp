#pragma once 
#include "bool_name.hpp"
#include "message_factory.hpp"

class CSystemManager;
class CDIN_STORAGE;

class CFaultCtrlF {
  
public:
  CFaultCtrlF(CDIN_STORAGE&);
  CDIN_STORAGE& rDinStr;
    
  F Fault = F::NOT_FAULT;
  
  void setSysManager(CSystemManager*);
  void initEINT2();
  void control(bool);
  void fault_stop();
  
  static constexpr unsigned int IOCON_EINT2 = 0x01;
  static constexpr unsigned int LineEINT2 = 2;
  static constexpr unsigned int EINT2_BIT_MARK = 0x04;
  
private:
  CSystemManager* pSys_manager;
  
  // Хелпер проверки 
  inline void check(F& fault, bool condition, EFaultId id ) { 
    if (condition) { 
      SFault::setMessage(id); 
      fault = F::FAULT;
    }
  }
  
};

/*
  Синглтон-прокси для доступа к EINT2_IRQHandler()
*/

class CProxyHandlerEINT2 
{
public:
  
  static CProxyHandlerEINT2& getInstance();  
  void set_pFaultCtrl(CFaultCtrlF*);
  
  CFaultCtrlF* pFaultCtrl;
  
private:
    CProxyHandlerEINT2(); 
    CProxyHandlerEINT2(const CProxyHandlerEINT2&) = delete;
    CProxyHandlerEINT2& operator=(const CProxyHandlerEINT2&) = delete; 

};



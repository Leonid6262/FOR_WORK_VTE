#pragma once 
#include "bool_name.hpp"

class CSystemManager;

class CFaultCtrlF {
  
public:
  CFaultCtrlF();
  
  F Fault = F::NOT_FAULT;
  
  void setSysManager(CSystemManager*);
  void initEINT2();
  void control(bool);
  
private:
  CSystemManager* pSys_manager;
 
};

/*
  Синглтон-прокси для доступа к EINT2_IRQHandler()
*/

class CProxyHandlerEINT2 
{
public:
  
  static CProxyHandlerEINT2& getInstance();  
  void set_pointers(CSystemManager*, CFaultCtrlF*);
  
  CSystemManager* pSys_manager;
  CFaultCtrlF* pFaultCtrl;
  
private:
    CProxyHandlerEINT2(); 
    CProxyHandlerEINT2(const CProxyHandlerEINT2&) = delete;
    CProxyHandlerEINT2& operator=(const CProxyHandlerEINT2&) = delete;     
};



#pragma once 
#include "bool_name.hpp"
//#include "SystemManager.hpp"
#include "AdcStorage.hpp"
#include "message_factory.hpp"

//class CSystemManager;

class CFaultCtrlP {
  
public:
  CFaultCtrlP(CADC_STORAGE&);
  
  F Fault = F::NOT_FAULT;
  
  //void getManager(CSystemManager*);
  void check();
  
private:
  //CSystemManager* pSys_manager;
  CADC_STORAGE& rAdcStr;
 
  using sadc = CADC_STORAGE;
  // Хелпер проверки 
  inline void check(F& fault, bool condition, EFaultId id ) { 
    if (condition) { 
      SFault::setMessage(id); 
      fault = F::FAULT;
    } else { 
      SFault::clrMessage(id); 
    } 
  }
 
};

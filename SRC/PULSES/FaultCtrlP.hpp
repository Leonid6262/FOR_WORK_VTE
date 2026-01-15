#pragma once 
#include "bool_name.hpp"
#include "AdcStorage.hpp"
#include "message_factory.hpp"

class CSystemManager;

class CFaultCtrlP {
  
public:
  CFaultCtrlP(CADC_STORAGE&, CEEPSettings&);
  
  F Fault = F::NOT_FAULT;
  
  void setSysManager(CSystemManager*);
  void check();
  
private:
  CSystemManager* pSys_manager;
  CADC_STORAGE& rAdcStr;
  CEEPSettings& rSet;
 
  using sadc = CADC_STORAGE;
  // Хелпер проверки 
  inline void check(F& fault, bool condition, EFaultId id ) { 
    if (condition) { 
      SFault::setMessage(id); 
      fault = F::FAULT;
    }
  }
 
};

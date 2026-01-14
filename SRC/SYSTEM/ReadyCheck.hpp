#pragma once 
#include "bool_name.hpp"
#include "SystemManager.hpp"
#include "dIOStorage.hpp"
#include "AdcStorage.hpp"
#include "message_factory.hpp"

class CSystemManager;

class CReadyCheck {
  
public:
  CReadyCheck(CADC_STORAGE&, CDIN_STORAGE&);
  
  R Ready = R::NOT_READY;
  
  void setSysManager(CSystemManager*);
  void check(bool);
  
private:
  CSystemManager* pSys_manager;
  CDIN_STORAGE& rDinStr;
  CADC_STORAGE& rAdcStr; 
  
  static constexpr unsigned short dMax = 10;
  
  using sadc = CADC_STORAGE;
  // Хелпер проверки 
  inline void check(R& ready, bool condition, ENotReadyId id ) { 
    if (condition) { 
      SNotReady::setMessage(id); 
      ready = R::NOT_READY;
    } else { 
      SNotReady::clrMessage(id); 
    } 
  }
  
};
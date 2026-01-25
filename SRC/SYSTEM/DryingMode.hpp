#pragma once 
#include "bool_name.hpp"
#include "dIOStorage.hpp"
#include "SIFU.hpp"

class CSystemManager;

class CDryingMode {
  
public:
  CDryingMode(CDIN_STORAGE&, CSIFU&, CEEPSettings&);

  void setSysManager(CSystemManager*);
  void dry(bool);
  
private:
  CDIN_STORAGE& rDinStr;
  CSIFU& rSIFU;
  CEEPSettings& rSet;
  CSystemManager* pSys_manager;
  
  State cur_status;
  unsigned int prev_TC0;
  static constexpr unsigned int RELAY_PAUSE_OFF = 5000000; // 0,5 сек
    
  void StartDrain();
  void StopDrain();

};

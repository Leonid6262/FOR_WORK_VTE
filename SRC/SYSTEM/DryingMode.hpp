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
  
  void OnEx();  
  void StartDrain();
  void StopDrain();

};

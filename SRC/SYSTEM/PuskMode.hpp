#pragma once 
#include "bool_name.hpp"
#include "dIOStorage.hpp"
#include "SIFU.hpp"

class CSystemManager;

class CPuskMode {
  
public:
  CPuskMode(CDIN_STORAGE&, CSIFU&, CEEPSettings&);
  void pusk(bool);
  
private:
  CDIN_STORAGE& rDinStr;
  CSIFU& rSIFU;
  CEEPSettings& rSet;
  CSystemManager* pSys_manager;
  
  State cur_status;
 
};

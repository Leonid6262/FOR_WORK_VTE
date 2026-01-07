#pragma once 

#include "Adjustment.hpp"
#include "FaultControl.hpp"
#include "ReadyCheck.hpp"
#include "PuskMode.hpp"
#include "WorkMode.hpp" 
#include "SIFU.hpp"
#include "RegManager.hpp" 

class CSystemManager {
  
public:
 CSystemManager(CSIFU&, CAdjustmentMode&, CReadyCheck&, CFaultControl&, CPuskMode&, CWorkMode&, CRegManager&);
 
 CSIFU& rSIFU;
 CAdjustmentMode& rAdj_mode; 
 CReadyCheck& rReady_check;
 CFaultControl& rFault_ctrl;
 CPuskMode& rPusk_mode;
 CWorkMode& rWork_mode;
 CRegManager& rReg_manager;
 
 void dispatch();
 
private:
   
 
};

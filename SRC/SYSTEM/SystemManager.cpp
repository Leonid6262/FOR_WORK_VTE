#include "SystemManager.hpp" 

CSystemManager::CSystemManager(CAdjustmentMode& rAdj_mode, CSIFU& rSIFU) : rAdj_mode(rAdj_mode), rSIFU(rSIFU){}

void CSystemManager::dispatch() {
  
  rAdj_mode.parsing_request(Mode::ALLOWED);
  
}



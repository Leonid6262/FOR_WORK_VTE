#include "PuskMode.hpp" 
#include "_SystemManager.hpp"

CPuskMode::CPuskMode(CDIN_STORAGE& rDinStr, CSIFU& rSIFU, CEEPSettings& rSet) : 
  rDinStr(rDinStr), cur_status(State::OFF), rSIFU(rSIFU), rSet(rSet) {
}

void CPuskMode::pusk(bool Permission) { 
  
  
}



#include "WorkMode.hpp" 
#include "_SystemManager.hpp"

CWorkMode::CWorkMode(CDIN_STORAGE& rDinStr, CSIFU& rSIFU, CEEPSettings& rSet) : 
  rDinStr(rDinStr), rSIFU(rSIFU), rSet(rSet), cur_status(State::OFF), pSys_manager(nullptr) {
  // Приращение задания dIset = Iset'*dT
  dIset = static_cast<unsigned short>((rSet.getSettings().work_set.derivIset * ChangeInterval) + 0.5f); 
}

void CWorkMode::work(bool Permission) {
  
  if(!Permission) {
    cur_status = State::OFF; 
    pSys_manager->set_bsWorkNormal(State::OFF);
    SWork::clrMessage(EWorkId::CURRENT_REG);
    SWork::clrMessage(EWorkId::Q_POWER_REG);
    SWork::clrMessage(EWorkId::COS_REG);   
    return; 
  }
  
  
  
  
}

void CWorkMode::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}

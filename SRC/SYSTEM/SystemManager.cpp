#include "SystemManager.hpp"

CSystemManager::CSystemManager(CSIFU& rSIFU, 
                               CAdjustmentMode& rAdj_mode, 
                               CReadyCheck& rReady_check, 
                               CFaultControl& rFault_ctrl,
                               CPuskMode& rPusk_mode,
                               CWorkMode& rWork_mode,
                               CRegManager& rReg_manager)
: rSIFU(rSIFU), 
rAdj_mode(rAdj_mode), 
rReady_check(rReady_check), 
rFault_ctrl(rFault_ctrl),
rPusk_mode(rPusk_mode),
rWork_mode(rWork_mode),
rReg_manager(rReg_manager){}

void CSystemManager::dispatch() { 
  
  rAdj_mode.parsing_request(Mode::ALLOWED); 
  
  rReady_check.check(Mode::ALLOWED);
  
  rPusk_mode.pusk(Mode::FORBIDDEN);
  
  rWork_mode.work(Mode::FORBIDDEN);

  rFault_ctrl.control(Mode::FORBIDDEN);

}

#include "SystemManager.hpp"

CSystemManager::CSystemManager(CSIFU& rSIFU, 
                               CAdjustmentMode& rAdj_mode, 
                               CReadyCheck& rReady_check, 
                               CFaultControl& rFault_ctrl,
                               CPuskMode& rPusk_mode,
                               CWorkMode& rWork_mode)
: rSIFU(rSIFU), 
rAdj_mode(rAdj_mode), 
rReady_check(rReady_check), 
rFault_ctrl(rFault_ctrl),
rPusk_mode(rPusk_mode),
rWork_mode(rWork_mode){}

void CSystemManager::dispatch() { 
  rAdj_mode.parsing_request(Mode::ALLOWED); 
}

#include "SystemManager.hpp"

CSystemManager::CSystemManager(CSIFU& rSIFU, 
                               CAdjustmentMode& rAdj_mode, 
                               CReadyCheck& rReady_check, 
                               CFaultControl& rFault_ctrl,
                               CPuskMode& rPusk_mode,
                               CWorkMode& rWork_mode,
                               CWarningMode& rWarning_ctrl,
                               CRegManager& rReg_manager)
: rSIFU(rSIFU), 
rAdj_mode(rAdj_mode), 
rReady_check(rReady_check), 
rFault_ctrl(rFault_ctrl),
rPusk_mode(rPusk_mode),
rWork_mode(rWork_mode),
rWarning_ctrl(rWarning_ctrl),
rReg_manager(rReg_manager)
{
  USystemMode.all = 0;
  USystemStatus.all = 0;
//  setReadyCheck(Mode::ALLOWED);
//  setAdjustment(Mode::FORBIDDEN);
//  setFaultCtrl(Mode::FORBIDDEN);
//  setPuskMode(Mode::FORBIDDEN);
//  setWorkMode(Mode::FORBIDDEN);
  
}

void CSystemManager::dispatch() { 
  
  for (auto& rule : rules) {
    bool allowed = 
      ((USystemStatus.all & rule.requiredStatus)  == rule.requiredStatus) &&
      ((USystemStatus.all & rule.forbiddenStatus) == 0) && 
      ((USystemMode.all   & rule.forbiddenModes)  == 0);   
    
    switch (rule.req_bit) {
    case MBit::ReadyCheck: setReadyCheck(allowed ? Mode::ALLOWED : Mode::FORBIDDEN); break;
    case MBit::Adjustment: setAdjustment(allowed ? Mode::ALLOWED : Mode::FORBIDDEN); break;
    case MBit::FaultCtrl:  setFaultCtrl(allowed  ? Mode::ALLOWED : Mode::FORBIDDEN); break;
    case MBit::PuskMode:   setPuskMode(allowed   ? Mode::ALLOWED : Mode::FORBIDDEN); break;
    case MBit::WorkMode:   setWorkMode(allowed   ? Mode::ALLOWED : Mode::FORBIDDEN); break;
    }
  }
  
  rAdj_mode.parsing_request(USystemMode.Adjustment);  
  rReady_check.check(USystemMode.ReadyCheck);   
  rPusk_mode.pusk(USystemMode.PuskMode);  
  rWork_mode.work(USystemMode.WorkMode);
  rFault_ctrl.control(USystemMode.FaultCtrl);
  rWarning_ctrl.control();
  
}

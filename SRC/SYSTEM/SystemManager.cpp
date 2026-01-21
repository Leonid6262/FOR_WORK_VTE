#include "SystemManager.hpp"

CSystemManager::CSystemManager(CSIFU& rSIFU, CAdjustmentMode& rAdj_mode, CReadyCheck& rReady_check, 
                               CFaultCtrlF& rFault_ctrl, CPuskMode& rPusk_mode, CWorkMode& rWork_mode,
                               CWarningMode& rWarning_ctrl, CDryingMode& rDrying_mode, CRegManager& rReg_manager)

: rSIFU(rSIFU), rAdj_mode(rAdj_mode), rReady_check(rReady_check), 
rFault_ctrl(rFault_ctrl), rPusk_mode(rPusk_mode), rWork_mode(rWork_mode),
rWarning_ctrl(rWarning_ctrl), rDrying_mode(rDrying_mode), rReg_manager(rReg_manager){
  
  UPermissionsList.all = 0;
  USystemStatus.all = 0;
  set_bpReadyCheck(Mode::ALLOWED);
}

void CSystemManager::dispatch() { 
  
  UPermissionsList.all = UPermissionsList_r.all; // копируем запросы
  
/*  for (auto& rule : rules) {
    bool allowed =
      ((USystemStatus.all & rule.requiredStatus)  == rule.requiredStatus) &&
      ((USystemStatus.all & rule.forbiddenStatus) == 0) &&
      ((UPermissionsList.all   & rule.requiredModes)   == rule.requiredModes) &&
      ((UPermissionsList.all   & rule.forbiddenModes)  == 0);    
    if (!allowed) { 
      UPermissionsList.all &= ~rule.req_bit; // снимаем бит 
    }    
  }
*/  
  UPermissionsList_r.all = UPermissionsList.all; // синхронизация с источником
  
  rAdj_mode.parsing_request(UPermissionsList.pAdjustment);    // Обработка запросов вкл. наладочных режимов
  rReady_check.check(UPermissionsList.pReadyCheck);           // Сборка готовности
  rPusk_mode.pusk(UPermissionsList.pPuskMotor);                // Режим работы при пуске двигателя
  rWork_mode.work(UPermissionsList.pNormalWork);                // Режим работы после успешного пуска двигателя
  rDrying_mode.dry(UPermissionsList.pWorkDry);             // Режим работы при сушке ротора
  rFault_ctrl.control(UPermissionsList.pFaultCtrlF);           // Проверка аварийных состояний
  rWarning_ctrl.control();                               // Проверка предупреждающих состояний (выполняется всегда)
  
}

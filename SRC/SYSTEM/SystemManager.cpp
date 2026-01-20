#include "SystemManager.hpp"

CSystemManager::CSystemManager(CSIFU& rSIFU, CAdjustmentMode& rAdj_mode, CReadyCheck& rReady_check, 
                               CFaultCtrlF& rFault_ctrl, CPuskMode& rPusk_mode, CWorkMode& rWork_mode,
                               CWarningMode& rWarning_ctrl, CDryingMode& rDrying_mode, CRegManager& rReg_manager)

: rSIFU(rSIFU), rAdj_mode(rAdj_mode), rReady_check(rReady_check), 
rFault_ctrl(rFault_ctrl), rPusk_mode(rPusk_mode), rWork_mode(rWork_mode),
rWarning_ctrl(rWarning_ctrl), rDrying_mode(rDrying_mode), rReg_manager(rReg_manager){
  
  USystemMode.all = 0;
  USystemStatus.all = 0;
  setReadyCheck(Mode::ALLOWED);
}

void CSystemManager::dispatch() { 
  
  USystemMode.all = USMode_r.all; // копируем запросы
  
  for (auto& rule : rules) {
    bool allowed =
      ((USystemStatus.all & rule.requiredStatus)  == rule.requiredStatus) &&
      ((USystemStatus.all & rule.forbiddenStatus) == 0) &&
      ((USystemMode.all   & rule.requiredModes)   == rule.requiredModes) &&
      ((USystemMode.all   & rule.forbiddenModes)  == 0);    
    if (!allowed) { 
      USystemMode.all &= ~rule.req_bit; // снимаем бит 
    }    
  }
  
  USMode_r.all = USystemMode.all; // синхронизация с источником
  
  rAdj_mode.parsing_request(USystemMode.Adjustment);    // Обработка запросов вкл. наладочных режимов
  rReady_check.check(USystemMode.ReadyCheck);           // Сборка готовности
  rPusk_mode.pusk(USystemMode.PuskMode);                // Режим работы при пуске двигателя
  rWork_mode.work(USystemMode.WorkMode);                // Режим работы после успешного пуска двигателя
  rDrying_mode.dry(USystemMode.DryingMode);             // Режим работы при сушке ротора
  rFault_ctrl.control(USystemMode.FaultCtrlF);          // Проверка аварийных состояний
  rWarning_ctrl.control();                              // Проверка предупреждающих состояний (выполняется всегда)
  
}

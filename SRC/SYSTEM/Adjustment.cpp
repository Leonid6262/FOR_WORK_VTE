#include "Adjustment.hpp" 

CAdjustmentMode::CAdjustmentMode(){};

void CAdjustmentMode::read_request() {                          
  state.currBits = check_permission(set_adj_mode);              // Приводим комбинацию к разрешенной 
  unsigned short changed = state.currBits ^ state.prevBits;     // Определяем изменения      
  
  if (!(set_adj_mode & AdjMode)) {                              // Выключаем все если снят режим AdjMode 
    applyChanges(state.prevBits, 0); 
    state.prevBits = 0;
    set_adj_mode = 0;                                           // Обратная связьт по индикации
    return; 
  }
  
  applyChanges(changed, state.currBits);                        // Применяем изменения
  state.prevBits = state.currBits;                              // Фиксируем
  set_adj_mode = state.prevBits;                                // Обратная связьт по индикации
}

unsigned short CAdjustmentMode::check_permission(unsigned short req_functions) {
  
  if (!(req_functions & AdjMode)) return 0;  
  
  if (!(req_functions & SIFU)) return AdjMode;
  
  // проверка по таблице правил
  for (auto const& rule : rules) {
    if (req_functions & rule.req_reg) {
      bool okRequired = (req_functions & rule.requiredMask) == rule.requiredMask;
      bool okForbidden = (req_functions & rule.forbiddenMask) == 0;
      if (!okRequired || !okForbidden) {
        req_functions &= ~rule.req_reg;
      }
    }
  }
  
  // если включена фазировка → регулятор и циклы OFF
  if (req_functions & Phase) {
    req_functions &= ~(CurrReg | CurrCycle);
  }
  
  return req_functions;
}

void CAdjustmentMode::applyChanges(unsigned short changed, unsigned short normalized) {
  if (changed & SIFU) {
    if (normalized & SIFU) EnableSIFU();
    else DisableSIFU();
  }
  if (changed & CurrReg) {
    if (normalized & CurrReg) EnableCurrentReg();
    else DisableCurrentReg();
  }
  if (changed & CurrCycle) {
    if (normalized & CurrCycle) EnableCurrentCycles();
    else DisableCurrentCycles();
  }
  if (changed & Phase) {
    if (normalized & Phase) EnablePhasing();
    else DisablePhasing();
  }
}

// Заглушки — здесь будут реальные реализации
void CAdjustmentMode::EnableSIFU()            { /* ... */ }
void CAdjustmentMode::DisableSIFU()           { /* ... */ }
void CAdjustmentMode::EnableCurrentReg()      { /* ... */ }
void CAdjustmentMode::DisableCurrentReg()     { /* ... */ }
void CAdjustmentMode::EnableCurrentCycles()   { /* ... */ }
void CAdjustmentMode::DisableCurrentCycles()  { /* ... */ }
void CAdjustmentMode::EnablePhasing()         { /* ... */ }
void CAdjustmentMode::DisablePhasing()        { /* ... */ }

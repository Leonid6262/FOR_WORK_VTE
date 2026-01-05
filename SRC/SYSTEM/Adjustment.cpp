#include "Adjustment.hpp" 
  
CAdjustmentMode::CAdjustmentMode(CSIFU& rSIFU) : rSIFU(rSIFU) {}

void CAdjustmentMode::parsing_request(Mode mode) {
  
  // 0. Если наладка запрещена - всё обнуляем
  if(mode == Mode::FORBIDDEN) {
    state.prevBits = 0;
    state.currBits = 0;
    req_adj_mode = 0;           // обратная связь по индикации
    return;
  }
  
  // 1. Если снят AdjMode - всё выключаем 
  if (!(req_adj_mode & AdjMode)) {
    applyChanges(state.prevBits, 0);
    state.prevBits = 0;
    req_adj_mode   = 0;         // обратная связь по индикации
    return;
  }
  
  // 2. Проверка штатного отключения любого режима (tmp var для нпглядности)
  unsigned short prev_active   = state.prevBits & ~AdjMode;   // что было реально включено
  unsigned short req_active    = req_adj_mode  & ~AdjMode;    // что пользователь хочет сейчас
  unsigned short disabled_mask = prev_active & ~req_active;   // что пользователь штатно снял
  
  if (disabled_mask != 0) {  // Если снят любой из режимов - отключаем все остальные
    unsigned short normalized = AdjMode;
    applyChanges(state.prevBits, normalized);
    state.prevBits = normalized;
    req_adj_mode   = normalized; // обратная связь по индикации
    return;
  }
  
  // 3. Проверка соответствия с таблицей правил
  unsigned short req_functions = req_adj_mode;
  for (auto const& rule : rules) {
    if (req_functions & rule.req_reg) {
      bool okRequired  = (req_functions & rule.requiredMask) == rule.requiredMask;
      bool okForbidden = (req_functions & rule.forbiddenMask) == 0;
      if (!okRequired || !okForbidden) {
        req_functions &= ~rule.req_reg;
      }
    }
  }
  
  // 4. Применение изменений проверенных на соответствии таблицы правил
  unsigned short changed = req_functions ^ state.prevBits;
  applyChanges(changed, req_functions);
  state.prevBits = req_functions;
  req_adj_mode   = state.prevBits; // обратная связь по индикации
}

// Включение/Отключение наладочнвх режимов
void CAdjustmentMode::applyChanges(unsigned short changed, unsigned short normalized) {
  if (changed & PulsesF) {
    if (normalized & PulsesF) rSIFU.set_forcing_bridge();
    else rSIFU.pulses_stop();
  }
  if (changed & PulsesM) {
    if (normalized & PulsesM) rSIFU.set_main_bridge();
    else rSIFU.pulses_stop();
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
    if (normalized & Phase) rSIFU.start_phasing_mode();
    else rSIFU.stop_phasing_mode();
  }
}

// Функции включения/отключения
void CAdjustmentMode::EnableCurrentReg()      { /* ... */ }
void CAdjustmentMode::DisableCurrentReg()     { /* ... */ }
void CAdjustmentMode::EnableCurrentCycles()   { /* ... */ }
void CAdjustmentMode::DisableCurrentCycles()  { /* ... */ }

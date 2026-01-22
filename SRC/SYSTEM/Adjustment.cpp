#include "Adjustment.hpp"
#include "_SystemManager.hpp"
  
CAdjustmentMode::CAdjustmentMode(CSIFU& rSIFU, CEEPSettings& rSettings) : rSIFU(rSIFU), rSettings(rSettings) {} 

void CAdjustmentMode::parsing_request(bool mode) {
  
  // 1. Если наладка запрещена - всё обнуляем
  if(!mode) {
    pSys_manager->set_bsAdjustmen(State::OFF);
    applyChanges(prevBits, 0);
    prevBits = 0;
    reqADJmode = 0;
    return;
  } else {
    reqADJmode |= AdjMode;
  }
  
  // 2. Проверка штатного отключения любого режима 
  unsigned short prev_active   = prevBits & ~AdjMode;       // что было реально включено
  unsigned short req_active    = reqADJmode  & ~AdjMode;    // что пользователь хочет сейчас
  unsigned short disabled_mask = prev_active & ~req_active; // что пользователь штатно снял
  
  if (disabled_mask != 0) {  // Если снят любой из режимов - отключаем все остальные
    unsigned short normalized = AdjMode;
    applyChanges(prevBits, normalized);
    prevBits = normalized;
    reqADJmode   = normalized;
    cur_mode = EModeAdj::None;
    return;
  }
  
  // 3. Проверка соответствия с таблицей правил
  unsigned short req_functions = reqADJmode;
  for (unsigned short bit : check_bits) {
    if (req_functions & bit) {
      // Если бит уже был активен — сохраняем приоритет
      if (prevBits & bit) {
        continue; // не снимаем, даже если конфликт
      }     
      // Новый бит — проверяем правила
      bool valid = false;
      for (auto const& rule : rules) {
        if (rule.req_reg == bit) {
          bool okRequired  = (req_functions & rule.requiredMask) == rule.requiredMask;
          bool okForbidden = (req_functions & rule.forbiddenMask) == 0;
          if (okRequired && okForbidden) {
            valid = true;
            break; // достаточно одного совпадения
          }
        }
      }     
      if (!valid) {
        req_functions &= ~bit; // снимаем только если ни одно правило не прошло
      }
    }
  }
  
  // 4. Применение изменений проверенных на соответствии таблицы правил
  unsigned short changed = req_functions ^ prevBits;
  applyChanges(changed, req_functions);
  prevBits = req_functions;
  reqADJmode = prevBits;
  
  ex_mode(cur_mode);
}

// Включение/Отключение наладочнвх режимов
void CAdjustmentMode::applyChanges(unsigned short changed, unsigned short normalized) {  
  
  if (changed & PulsesF) {
    if (normalized & PulsesF) {
      AlphaAdj = rSIFU.s_const.AMax;
      rSIFU.set_forcing_bridge();
      cur_mode = EModeAdj::ForcingPulses;
    }
    else {
      AlphaAdj = rSIFU.s_const.AMax;
      rSIFU.pulses_stop();      
    }
  }
  
  if (changed & PulsesM) {
    if (normalized & PulsesM) {
      AlphaAdj = rSIFU.s_const.AMax;
      rSIFU.set_main_bridge();
      cur_mode = EModeAdj::MainPulses;
    }
    else {
      AlphaAdj = rSIFU.s_const.AMax;
      rSIFU.pulses_stop();
    }
  }
  
  if (changed & CurrReg) {
    if (normalized & CurrReg) {
      rSIFU.rReg_manager.rCurrent_reg.set_Iset(0);
      rSIFU.rReg_manager.setCurrent(State::ON);
      cur_mode = (normalized & PulsesF) ? EModeAdj::CurrentRegF : EModeAdj::CurrentRegM;
    } else {
      rSIFU.rReg_manager.rCurrent_reg.set_Iset(0);
      rSIFU.rReg_manager.setCurrent(State::OFF);
    }
  }
  
  if (changed & CurrCycle) {
    if (normalized & CurrCycle) {
      pulse_counter = 0; 
      phase = false; 
      IsetCycle = IsetCyc_1; 
      last_pulse = rSIFU.N_Pulse;
      cur_mode = (normalized & PulsesF) ? EModeAdj::CurrentCycleF : EModeAdj::CurrentCycleM;
    } 
  }
  
  if (changed & Phase) {
    if (normalized & Phase) {
      rSIFU.start_phasing_mode();
      cur_mode = (normalized & PulsesF) ? EModeAdj::PhasingF : EModeAdj::PhasingM;
    } else {
      rSIFU.stop_phasing_mode();
    }
  }
  
}

void CAdjustmentMode::ex_mode(EModeAdj ex_mode){
  switch (ex_mode){
  case EModeAdj::ForcingPulses:
  case EModeAdj::MainPulses:
    rSIFU.set_alpha(AlphaAdj);
    break;
  case EModeAdj::CurrentRegF:
  case EModeAdj::CurrentRegM:
    rSIFU.rReg_manager.rCurrent_reg.set_Iset(IsetAdj);
    break;
  case EModeAdj::CurrentCycleF:
  case EModeAdj::CurrentCycleM:
    stepCycle();
    rSIFU.rReg_manager.rCurrent_reg.set_Iset(IsetCycle);
    break;
  case EModeAdj::PhasingF:
  case EModeAdj::PhasingM:
    rSIFU.set_d_shift(rSettings.getSettings().set_sifu.d_power_shift);
    rSIFU.set_a_shift(rSettings.getSettings().set_sifu.power_shift);
    break;  
  case EModeAdj::None:
    break;  
  }

}

// Циклы задания тока
void CAdjustmentMode::stepCycle() { 
  if (rSIFU.N_Pulse != last_pulse) { 
    last_pulse = rSIFU.N_Pulse; 
    pulse_counter++;  
    if (pulse_counter >= NpulsCyc) { 
      pulse_counter = 0; 
      phase = !phase; 
      IsetCycle = phase ? IsetCyc_2 : IsetCyc_1; 
    } 
  }
}

void CAdjustmentMode::setSysManager(CSystemManager* pSys_manager) {
  this->pSys_manager = pSys_manager;
}

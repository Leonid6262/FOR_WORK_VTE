#include "SIFU.hpp"
#include <algorithm>
#include "system_LPC177x.h"

const unsigned char CSIFU::pulsesAllP[] = {0x00, 0x21, 0x03, 0x06, 0x0C, 0x18, 0x30}; // Индекс 0 не используется
const unsigned char CSIFU::pulsesWone[] = {0x00, 0x21, 0x00, 0x06, 0x00, 0x18, 0x00}; // Индекс 0 не используется

const signed short CSIFU::offsets[] = {
  0,
  SIFUConst::_60gr,    // Диапазон 0...60        (sync "видит" 1-й: ->2-3-4-5-6-sync-1->2-3-4...)
  SIFUConst::_120gr,   // Диапазон -60...0       (sync "видит" 2-й: ->3-4-5-6-1-sync-2->3-4-5...)
  SIFUConst::_180gr,   // Диапазон -120...-60    (sync "видит" 3-й: ->4-5-6-1-2-sync-3->4-5-6...) - чисто теоретически
  -SIFUConst::_120gr,  // Диапазон 180...240     (sync "видит" 4-й: ->5-6-1-2-3-sync-4->5-6-1...) - чисто теоретически
  -SIFUConst::_60gr,   // Диапазон 120...180     (sync "видит" 5-й: ->6-1-2-3-4-sync-5->6-1-2...)
  SIFUConst::_0gr      // Диапазон 60...120      (sync "видит" 6-й: ->1-2-3-4-5-sync-6->1-2-3...)
};  // Индекс 0 не используется

CSIFU::CSIFU(CPULSCALC& rPulsCalc, CRegManager& rReg_manager, CFaultCtrlP& rFault_p, CEEPSettings& rSettings) 
: rPulsCalc(rPulsCalc), rReg_manager(rReg_manager), rFault_p(rFault_p), rSettings(rSettings){}

void CSIFU::rising_puls() {
  N_Pulse = (N_Pulse % s_const.N_PULSES) + 1; // Текущий номер импульса (1...6)
  
  // Фронт ИУ рабочего моста
  if (main_bridge) {
    if(!wone_reg){
      LPC_GPIO3->CLR = pulsesAllP[(((N_Pulse - 1) + v_sync.d_power_shift) % s_const.N_PULSES) + 1] << FIRS_PULS_PORT;
    } else{
      LPC_GPIO3->CLR = pulsesWone[(((N_Pulse - 1) + v_sync.d_power_shift) % s_const.N_PULSES) + 1] << FIRS_PULS_PORT;
    }

    LPC_SC->PCONP |= CLKPWR_PCONP_PCPWM0; 
    
    LPC_PWM0->PR = PWM_div_0 - 1;
    LPC_PWM0->MCR = MR0R; // Reset TC on MR0
    LPC_PWM0->MR0 = PWM_WIDTH * 2;
    LPC_PWM0->MR1 = PWM_WIDTH;
    LPC_PWM0->LER = LER_012; 
    LPC_PWM0->PCR |= PCR_PWMENA1;
    // Важно
    LPC_PWM0->TCR = COUNTER_RESET; // Обнулили TC и PR
    LPC_PWM0->TC  = LPC_PWM0->MR0; // Вручную ставим счетчик в значение финиша
    // ----
    LPC_IOCON->P1_2 = IOCON_P_PWM; // P1_2 -> PWM
    LPC_PWM0->TCR = COUNTER_START; // Запускаем      
  }
  // Фронт ИУ  форсировочного моста
  if (forcing_bridge) {
    LPC_GPIO3->CLR = pulsesAllP[(((N_Pulse - 1) + v_sync.d_power_shift) % s_const.N_PULSES) + 1] << FIRS_PULS_PORT;
    
    LPC_SC->PCONP |= CLKPWR_PCONP_PCPWM0; 
    
    LPC_PWM0->PR = PWM_div_0 - 1;
    LPC_PWM0->MCR = MR0R; // Reset TC on MR0
    LPC_PWM0->MR0 = PWM_WIDTH * 2;
    LPC_PWM0->MR2 = PWM_WIDTH;
    LPC_PWM0->LER = LER_012; 
    LPC_PWM0->PCR |= PCR_PWMENA2;
    // Важно
    LPC_PWM0->TCR = COUNTER_RESET; // Обнулили TC и PR
    LPC_PWM0->TC  = LPC_PWM0->MR0; // Вручную ставим счетчик в значение финиша
    // ----
    LPC_IOCON->P1_3 = IOCON_P_PWM; // P1_3 -> PWM
    LPC_PWM0->TCR = COUNTER_START; // Запускаем
  }
  
  rPulsCalc.conv_and_calc();            // Измерения и вычисления.
  control_fault_and_reg();              // Контроль аварий и регулирование
 
  control_sync();  // Мониторинг события захвата CR1 синхроимпульсом
  
  signed int cur_MR0 = static_cast<signed int>(LPC_TIM3->MR0);
  
  signed int res; // служебная переменная
  
  switch (Operating_mode) {
  
  // СИФУ не синхронизировано, ИУ следуют через 60 градусов 
  case EOperating_mode::NO_SYNC:
    res = static_cast<signed int>(LPC_TIM3->MR0) + s_const._60gr;    // Вычисление следующего значения MR0
    LPC_TIM3->MR0 = static_cast<unsigned int>(res);                  // Установка следующего значения MR0
    curSyncStat = static_cast<unsigned char>(State::OFF);            // Статус синхронизации
    break;
    
  // СИФУ синхронизировано. ИУ следуют через 60 градусов + dAlpha
  case EOperating_mode::NORMAL:
    Alpha_setpoint = limits_val(&Alpha_setpoint, s_const.AMin, s_const.AMax);   // Ограничения величины альфа
    LPC_TIM3->MR0 = timing_calc();                                              // Вычисление и установка следующего значения MR0
    curSyncStat = static_cast<unsigned char>(State::ON);                        // Статус синхронизации
    break;
    
  // Вход в синхронизированный режим. Начало с 1-го ИУ. Начальный угол Alpha_current = Amax  
  case EOperating_mode::RESYNC:
    Operating_mode = EOperating_mode::NORMAL;  
    Alpha_setpoint = s_const.AMax;
    Alpha_current = s_const.AMax;
    // ...5-6-1-2-sync-3->6-1-2-3-4-sync-5->6-1-2... <-- пример последовательности
    
    res =                                               // Вычисление значения MR0 для 1-го ИУ в Amax
      static_cast<signed int>(v_sync.CURRENT_SYNC) +    // Значение CR1. Момент прихода синхроимпульса
      static_cast<signed int>(Alpha_current) +          // Alpha_current = Amax
      static_cast<signed int>(v_sync.cur_power_shift);  // Смещение относительно силового питания

    LPC_TIM3->MR0 = static_cast<unsigned int>(res);             // Установка значения MR0 для 1-го ИУ
    curSyncStat = static_cast<unsigned char>(State::OFF);       // Статус синхронизации
    N_Pulse = 6;                                                // 6-й устанавливаем текущим    
    break;
    
  // СИФУ синхронизировано. Режим фазировка с Alpha_current = 0
  case EOperating_mode::PHASING:
    // Ограничения величины сдвига синхронизации
    v_sync.task_power_shift = limits_val(&v_sync.task_power_shift, s_const.MinPshift, s_const.MaxPshift);
    // Ограничения приращения сдвига синхронизации
    limits_dval(&v_sync.task_power_shift, &v_sync.cur_power_shift, s_const.dAlpha);
    
    Alpha_setpoint = s_const._0gr;                              // Задание Alpha ноль градусов
    LPC_TIM3->MR0 = timing_calc();                              // Вычисление и установка следующего значения MR0
    curSyncStat = static_cast<unsigned char>(State::ON);        // Статус синхронизации
    break;

  }
  
  res = cur_MR0 + SIFUConst::PULSE_WIDTH;               // Вычисление момента выключения ИУ
  LPC_TIM3->MR1 = static_cast<unsigned int>(res);       // Задание значения MR1  
  
  void off_wone_reg();                                  // Контроль отключения режима "Через один"
  void off_pulses_control();                            // Контроль фазы выключения ИУ

}

// Вычисление следующего значения MR0 (фронт следующего ИУ)
unsigned int CSIFU::timing_calc() {
  // Ограничения приращения альфа (обновляется Alpha_current с учётом максимального приращения)
  signed short d_Alpha = limits_dval(&Alpha_setpoint, &Alpha_current, s_const.dAlpha);
  signed int ret;

  if (v_sync.SYNC_EVENT) {
     v_sync.SYNC_EVENT = false;
     
    // Событие синхронизации было, корректируем выдачу
    // следующего ИУ с учётом текущего Alpha
    ret = 
      static_cast<signed int>(v_sync.CURRENT_SYNC) +    // Значение CR1. Момент прихода синхроимпульса.
      static_cast<signed int>(Alpha_current) +          // Текущий угол управления.
      static_cast<signed int>(offsets[N_Pulse]) +       // Кррекция по номеру ИУ "увидевшего" событие захвата
      static_cast<signed int>(v_sync.cur_power_shift);  // Смещение относительно силового питания
 
  } else {
    
    // События синхронизации не было,
    // продолжаем последовательность ИУ. 60гр + dAlpha
    ret = 
      static_cast<signed int>(LPC_TIM3->MR0) + 
      static_cast<signed int>(s_const._60gr) +
      static_cast<signed int>(d_Alpha);
 
  }
  return static_cast<unsigned int>(ret);
}

// Выключения портов и PWM при окончании ИУ
void CSIFU::faling_puls() {
  
  LPC_IOCON->P1_2 = IOCON_P_PORT;  // P1_2 -> Port
  LPC_GPIO1->CLR = 1UL << P1_2;
  LPC_IOCON->P1_3 = IOCON_P_PORT;  // P1_3 -> Port
  LPC_GPIO1->CLR = 1UL << P1_3;
  
  LPC_GPIO3->SET = OFF_PULSES;           // Выкл. импульсы  
  LPC_SC->PCONP &= ~CLKPWR_PCONP_PCPWM0; // Выкл. PWM
  
}

// Определение события прихода синхроимпульса
void CSIFU::control_sync() {
  
  v_sync.cur_capture = LPC_TIM3->CR1;   // Копируем текущее значение защёлки
  
  // --- Режим определения гарантированной синхронизации ---
  if (Operating_mode == EOperating_mode::NO_SYNC) {
    // Если предыдущее значение CR1 отлично от текущего - был импульс
    if (v_sync.prev_capture != v_sync.cur_capture) { 
      // Вычисляем дельту и обновляем предыдущее значение
      unsigned int dt = v_sync.cur_capture - v_sync.prev_capture;
      v_sync.prev_capture = v_sync.cur_capture;
      // Проверяем, что дельта в пределах 20мс
      if (dt >= s_const.DT_MIN && dt <= s_const.DT_MAX) {
        // Увеличиваем счётчик корректных синхроимпульсов
        v_sync.sync_pulses++;
        if (v_sync.sync_pulses > 50) {
          // Если 50 импульсов подряд (1сек) корректные, ресинхронизируем СИФУ
          v_sync.CURRENT_SYNC = v_sync.cur_capture;
          Operating_mode = EOperating_mode::RESYNC;
          v_sync.no_sync_pulses = 0; 
        }
      } else {
        // дельта не равна периоду сети. 
        // Сбрасываем счётчик корректных синхроимпульсов.
        v_sync.sync_pulses = 0;
      }
      return;
    }
  }
  // ––------------–----------------------------------------
  
  // --- Штатный режим и режим фазировки (СИФУ синхронизировано)
  if (Operating_mode == EOperating_mode::NORMAL || Operating_mode == EOperating_mode::PHASING) {
    // Если предыдущее значение CR1 отлично от текущего - был импульс       
    if (v_sync.prev_capture != v_sync.cur_capture) {
      // Вычисляем дельту и обновляем предыдущее значение
      unsigned int dt = v_sync.cur_capture - v_sync.prev_capture;
      v_sync.prev_capture = v_sync.cur_capture;
      // Проверяем, что дельта в пределах 20мс
      if (dt >= s_const.DT_MIN && dt <= s_const.DT_MAX) {
        // Устанавливаем флаг события синхроимпульса, 
        // фиксируем текущее значение синхронизации,
        // вычисляем частоту сети
        v_sync.SYNC_EVENT = true;
        v_sync.CURRENT_SYNC = v_sync.cur_capture;
        v_sync.SYNC_FREQUENCY = s_const.TIC_SEC / static_cast<float>(dt);       
        v_sync.no_sync_pulses = 0; // сбрасываем счётчик ИУ не "увидевших" синхронизацию
      } else {
        // Сбой. Дельта не равна периоду сети.
        v_sync.SYNC_FREQUENCY = 0;
        v_sync.sync_pulses = 0;
        // Переводим СИФУ в режим "Без синхронизации"
        Operating_mode = EOperating_mode::NO_SYNC;
      }
    } else {
      // Этот импульс СИФУ синхронизацию не "увидел". Считаем ИУ
      v_sync.no_sync_pulses++;
      if (v_sync.no_sync_pulses > (s_const.N_PULSES * 4)) {
        // Если 4-ре периода сихроимпульса не было,
        // переводим СИФУ в режим "Без синхронизации"
        v_sync.SYNC_FREQUENCY = 0;
        v_sync.sync_pulses = 0;
        Operating_mode = EOperating_mode::NO_SYNC;
      }
    }
  }
}

// Ограничение значения
signed short CSIFU::limits_val(signed short* input, signed short min, signed short max) {
  if (*input > max) return max;
  if (*input < min) return min;
  return *input;
}
// Ограничение приращения
signed short CSIFU::limits_dval(signed short* input, signed short* output, signed short max) {
  signed short d = *input - *output;
  if (abs(d) < max)
    *output = *input;
  else {
    d = (d > 0 ? max : -max);
    *output += d;
  }
  return d;
}

void CSIFU::set_alpha(signed short alpha) { Alpha_setpoint = alpha; }
signed short CSIFU::get_alpha() { return Alpha_current; }

signed short* CSIFU::getPointerAlpha() { return &Alpha_current; }

void CSIFU::control_fault_and_reg() {
  if (forcing_bridge || main_bridge) {
    rFault_p.check();                   // Контроль аварийных ситуаций
    rReg_manager.applyModeRules();      // Контроль правил комбинации регуляторов
    rReg_manager.stepAll();             // Регулирование
  }
}

void CSIFU::forcing_bridge_pulses_On() {
  main_bridge = false;
  forcing_bridge = true;
}

void CSIFU::main_bridge_pulses_On() {
  forcing_bridge = false;
  main_bridge = true;
}

void CSIFU::execute_mode_Wone(){
  n_pulses_wone = s_const.N_PULSES_WONE;
  wone_reg = true;
}

void CSIFU::off_wone_reg() {  
  if(!wone_reg) return;  
  if(--n_pulses_wone <= 0) {
    wone_reg = false;
  }
}

void CSIFU::all_bridge_pulses_Off() {
  set_alpha(s_const.AMax);
  n_pulses_stop = s_const.N_PULSES_STOP;
  phase_stop = State::ON;
}

void CSIFU::off_pulses_control() { 
  
  if(phase_stop == State::OFF) return; 
  
  if(--n_pulses_stop <= 0){
    phase_stop = State::OFF;
    forcing_bridge = false;
    main_bridge = false;
  }   
}

void CSIFU::start_phasing_mode() {
  v_sync.task_power_shift = rSettings.getSettings().set_sifu.power_shift;
  v_sync.cur_power_shift = v_sync.task_power_shift;
  Operating_mode = EOperating_mode::PHASING;
}

void CSIFU::stop_phasing_mode() {
  Alpha_setpoint = s_const.AMax;
  Operating_mode = EOperating_mode::NORMAL;
}

void CSIFU::set_a_shift(signed short shift) {
  if (Operating_mode == EOperating_mode::PHASING) {
    v_sync.task_power_shift = shift;
  }
}

void CSIFU::set_d_shift(unsigned char d_shift) {
  if (Operating_mode == EOperating_mode::PHASING) {
    if (d_shift > (s_const.N_PULSES - 1)) d_shift = s_const.N_PULSES - 1;  // 0...5
    v_sync.d_power_shift = d_shift;
  }
}

float* CSIFU::get_Sync_Frequency() { return &v_sync.SYNC_FREQUENCY; }

unsigned char* CSIFU::getSyncStat() { return &curSyncStat; } 

void CSIFU::init_and_start(CProxyPointerVar& PPV) {
  
  v_sync.d_power_shift = rSettings.getSettings().set_sifu.d_power_shift;
  v_sync.task_power_shift = rSettings.getSettings().set_sifu.power_shift;
  v_sync.cur_power_shift = v_sync.task_power_shift;
 
  // Регистрация Alpha в реестре указателей
  PPV.registerVar (NProxyVar::ProxyVarID::AlphaCur, 
                   &Alpha_current, 
                   cd::Alpha, 
                   NProxyVar::Unit::Deg);
  
  LPC_IOCON->P2_23 = IOCON_T3_CAP1;  // T3 CAP1
  LPC_TIM3->MCR = 0x00000000;        // Compare TIM3 с MR0 и MR1, с прерываниями (disabled)
  LPC_TIM3->IR = 0xFFFFFFFF;         // Очистка флагов прерываний
  LPC_TIM3->TCR |= TIM3_TCR_START;   // Старт таймера TIM3
  
  LPC_TIM3->TC = 0;
  LPC_TIM3->MR0 = s_const._60gr;
  LPC_TIM3->MR1 = s_const._60gr + SIFUConst::PULSE_WIDTH;
  LPC_TIM3->CCR = TIM3_CAPTURE_RI;    // Захват T3 по фронту CAP1 без прерываний
  LPC_TIM3->MCR = TIM3_COMPARE_MR0;   // Compare TIM3 с MR0 - enabled
  LPC_TIM3->MCR |= TIM3_COMPARE_MR1;  // Compare TIM3 с MR1 - enabled
  
  NVIC_EnableIRQ(TIMER3_IRQn);
}

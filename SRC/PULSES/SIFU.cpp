#include "SIFU.hpp"
#include <algorithm>
#include "system_LPC177x.h"

const unsigned char CSIFU::pulses[] = {0x00, 0x21, 0x03, 0x06, 0x0C, 0x18, 0x30};       // Индекс 0 не используется
const unsigned char CSIFU::pulse_w_one[] = {0x00, 0x21, 0x00, 0x06, 0x00, 0x18, 0x00};  // Индекс 0 не используется
const signed short CSIFU::offsets[] = {
    0,
    SIFUConst::_60gr,    // Диапазон 0...60        (sync "видит" 1-й: ->2-3-4-5-6-sync-1->2-3-4...)
    SIFUConst::_120gr,   // Диапазон -60...0       (sync "видит" 2-й: ->3-4-5-6-1-sync-2->3-4-5...)
    SIFUConst::_180gr,   // Диапазон -120...-60    (sync "видит" 3-й: ->4-5-6-1-2-sync-3->4-5-6...) - чисто теоретически
    -SIFUConst::_120gr,  // Диапазон 180...240     (sync "видит" 4-й: ->5-6-1-2-3-sync-4->5-6-1...) - чисто теоретически
    -SIFUConst::_60gr,   // Диапазон 120...180     (sync "видит" 5-й: ->6-1-2-3-4-sync-5->6-1-2...)
    SIFUConst::_0gr      // Диапазон 60...120      (sync "видит" 6-й: ->1-2-3-4-5-sync-6->1-2-3...)
};  // Индекс 0 не используется

CSIFU::CSIFU(CPULSCALC& rPulsCalc, CRegManager& rReg_manager, CFaultCtrlP& rFault_p, CEEPSettings& rSettings) : 
  rPulsCalc(rPulsCalc), 
  rReg_manager(rReg_manager), 
  rFault_p(rFault_p),
  rSettings(rSettings){}

void CSIFU::rising_puls() {
  N_Pulse = (N_Pulse % s_const.N_PULSES) + 1;

  // Старт ИУ форсировочного моста
  if (main_bridge) {
    LPC_GPIO3->CLR = pulses[(((N_Pulse - 1) + v_sync.d_power_shift) % s_const.N_PULSES) + 1] << FIRS_PULS_PORT;
    LPC_IOCON->P1_2 = IOCON_P_PWM;  // P1_2->PWM0:1 (SUM-1)
    LPC_PWM0->PCR = PCR_PWMENA1;
    LPC_PWM0->TCR = COUNTER_START;  // Старт счётчик b1<-0
    LPC_PWM0->LER = LER_012;        // Обновление MR0,MR1 и MR2
  }
  // Старт ИУ рабочего моста
  if (forcing_bridge) {
    LPC_GPIO3->CLR = pulses[(((N_Pulse - 1) + v_sync.d_power_shift) % s_const.N_PULSES) + 1] << FIRS_PULS_PORT;
    LPC_IOCON->P1_3 = IOCON_P_PWM;  // P1_3->PWM0:2 (SUM-2)
    LPC_PWM0->PCR = PCR_PWMENA2;
    LPC_PWM0->TCR = COUNTER_START;  // Старт счётчик b1<-0
    LPC_PWM0->LER = LER_012;        // Обновление MR0,MR1 и MR2
  }

  control_sync();  // Мониторинг события захвата синхроимпульса

  signed int cur_MR0 = static_cast<signed int>(LPC_TIM3->MR0);

  switch (Operating_mode) {
    case EOperating_mode::NO_SYNC: {
      signed int res = static_cast<signed int>(LPC_TIM3->MR0) + s_const._60gr;
      LPC_TIM3->MR0 = static_cast<unsigned int>(res);
    }  // Старт следующего через 60 градусов
    break;
    case EOperating_mode::RESYNC: {
      Operating_mode = EOperating_mode::NORMAL;  // Синхронизация с 1-го в Alpha_max
      Alpha_setpoint = s_const.AMax;
      Alpha_current = s_const.AMax;
      // 1-2-3-4-sync-5->6-1-2-3-4-sync-5->6-1-2....
      // Здесь и далее, кастования слагаемых приведены для наглядности,
      // без магии циклической арифметики таймера по модулю 2^32
      signed int res = static_cast<signed int>(v_sync.CURRENT_SYNC) + static_cast<signed int>(Alpha_current) +
                       static_cast<signed int>(v_sync.cur_power_shift);
      LPC_TIM3->MR0 = static_cast<unsigned int>(res);
      N_Pulse = 6;
    } break;
    case EOperating_mode::PHASING:
      // Ограничения величины сдвига
      v_sync.task_power_shift = limits_val(&v_sync.task_power_shift, s_const.MinPshift, s_const.MaxPshift);
      // Ограничения приращения сдвига
      limits_dval(&v_sync.task_power_shift, &v_sync.cur_power_shift, s_const.dAlpha);
      Alpha_setpoint = s_const._0gr;
      LPC_TIM3->MR0 = timing_calc();  // Задание тайминга для следующего импульса
      break;
    case EOperating_mode::NORMAL:
      // Ограничения величины альфа
      Alpha_setpoint = limits_val(&Alpha_setpoint, s_const.AMin, s_const.AMax);
      LPC_TIM3->MR0 = timing_calc();  // Задание тайминга для следующего импульса
      break;
  }

  signed int res = cur_MR0 + SIFUConst::PULSE_WIDTH;
  //
  //
  // Ограничение длительности импульса при движении Alpha в сторону Alpha Min
  //
  //
  LPC_TIM3->MR1 = static_cast<unsigned int>(res);  // Окончание текущего

  rPulsCalc.conv_and_calc();     // Измерения, вычисления и т.п.
  rReg_manager.applyModeRules();
  rReg_manager.stepAll();       // Регулирование
  
}

signed int CSIFU::timing_calc() {
  // Ограничения приращения альфа
  signed short d_Alpha = limits_dval(&Alpha_setpoint, &Alpha_current, s_const.dAlpha);
  if (v_sync.SYNC_EVENT) {
    // Коррекция по синхронизации
    v_sync.SYNC_EVENT = false;
    signed int ret = static_cast<signed int>(v_sync.CURRENT_SYNC) + static_cast<signed int>(Alpha_current) +
                     static_cast<signed int>(offsets[N_Pulse]) + static_cast<signed int>(v_sync.cur_power_shift);
    return static_cast<unsigned int>(ret);
    // return (v_sync.CURRENT_SYNC + (A_Cur_tick + offsets[N_Pulse]) + v_sync.cur_power_shift);
  } else {
    // Продолжение последовательности
    signed int ret = static_cast<signed int>(LPC_TIM3->MR0) + static_cast<signed int>(s_const._60gr) +
                     static_cast<signed int>(d_Alpha);

    return static_cast<unsigned int>(ret);
    // return LPC_TIM3->MR0 + s_const._60gr + d_Alpha;
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

void CSIFU::faling_puls() {
  LPC_IOCON->P1_2 = IOCON_P_PORT;  // P1_2 - Port
  LPC_GPIO1->CLR = 1UL << P1_2;
  LPC_IOCON->P1_3 = IOCON_P_PORT;  // P1_3 - Port
  LPC_GPIO1->CLR = 1UL << P1_3;

  LPC_GPIO3->SET = OFF_PULSES;

  LPC_PWM0->TCR = COUNTER_STOP;  // Стоп счётчик b1<-1
  LPC_PWM0->TCR = COUNTER_RESET;
  
  rFault_p.check(); // Контроль аварийных ситуаций
}

void CSIFU::control_sync() {
  v_sync.current_cr = LPC_TIM3->CR1;

  if ((Operating_mode == EOperating_mode::NO_SYNC) && (v_sync.previous_cr != v_sync.current_cr)) {
    unsigned int dt = v_sync.current_cr - v_sync.previous_cr;
    v_sync.previous_cr = v_sync.current_cr;
    if (dt >= s_const.DT_MIN && dt <= s_const.DT_MAX) {
      v_sync.sync_pulses++;
      if (v_sync.sync_pulses > 100) {
        Operating_mode = EOperating_mode::RESYNC;
        v_sync.no_sync_pulses = 0;
        v_sync.CURRENT_SYNC = v_sync.current_cr;
      }
    } else {
      v_sync.sync_pulses = 0;
    }
    return;
  }

  if (Operating_mode == EOperating_mode::NORMAL || Operating_mode == EOperating_mode::PHASING) {
    if (v_sync.previous_cr != v_sync.current_cr) {
      unsigned int dt = v_sync.current_cr - v_sync.previous_cr;
      v_sync.previous_cr = v_sync.current_cr;
      if (dt >= s_const.DT_MIN && dt <= s_const.DT_MAX) {
        v_sync.SYNC_FREQUENCY = s_const.TIC_SEC / static_cast<float>(dt);
        v_sync.CURRENT_SYNC = v_sync.current_cr;
        v_sync.no_sync_pulses = 0;
        v_sync.SYNC_EVENT = true;
      } else {
        v_sync.SYNC_FREQUENCY = 0;
        v_sync.sync_pulses = 0;
        Operating_mode = EOperating_mode::NO_SYNC;
      }
    } else {
      v_sync.no_sync_pulses++;
      if (v_sync.no_sync_pulses > (s_const.N_PULSES * 4)) {
        v_sync.SYNC_FREQUENCY = 0;
        v_sync.sync_pulses = 0;
        Operating_mode = EOperating_mode::NO_SYNC;
      }
    }
  }
}

void CSIFU::set_alpha(signed short alpha) { Alpha_setpoint = alpha; }
signed short CSIFU::get_alpha() { return Alpha_current; }

signed short* CSIFU::getPointerAlpha() { return &Alpha_current; }

void CSIFU::set_forcing_bridge() {
  main_bridge = false;
  forcing_bridge = true;
}
void CSIFU::set_main_bridge() {
  forcing_bridge = false;
  main_bridge = true;
}
void CSIFU::pulses_stop() {
  forcing_bridge = false;
  main_bridge = false;
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
float CSIFU::get_Sync_Frequency() { return v_sync.SYNC_FREQUENCY; }

void CSIFU::init_and_start() {
  forcing_bridge = false;
  main_bridge = false;

  N_Pulse = 1;
  v_sync.d_power_shift = rSettings.getSettings().set_sifu.d_power_shift;
  v_sync.task_power_shift = rSettings.getSettings().set_sifu.power_shift;
  v_sync.cur_power_shift = v_sync.task_power_shift;
  v_sync.SYNC_EVENT = false;
  v_sync.no_sync_pulses = 0;
  v_sync.sync_pulses = 0;
  Alpha_setpoint = s_const.AMax;
  Alpha_current = s_const.AMax;
  Operating_mode = EOperating_mode::NO_SYNC;
  
  CProxyPointerVar::getInstance().registerVar   // Регистрация Alpha в реестре указателей
    (           
     NProxyVar::ProxyVarID::AlphaCur, 
     &Alpha_current, 
     cd::Alpha, 
     NProxyVar::Unit::Deg);

  LPC_SC->PCONP |= CLKPWR_PCONP_PCPWM0;  // PWM0 power/clock control bit.
  LPC_PWM0->PR = PWM_div_0 - 1;          // при PWM_div=60, F=60МГц/60=1МГц, 1тик=1мкс

  LPC_PWM0->TCR = COUNTER_CLR;    // Сброс регистра таймера
  LPC_PWM0->TCR = COUNTER_RESET;  // Сброс таймера

  LPC_PWM0->PCR = 0x00;           // Отключение PWM0
  LPC_PWM0->MR0 = PWM_WIDTH * 2;  // Период ШИМ. MR0 - включение
  LPC_PWM0->MR1 = PWM_WIDTH;      // Выключение PWM0:1 по MR1
  LPC_PWM0->MR2 = PWM_WIDTH;      // Выключение PWM0:2 по MR2

  LPC_PWM0->LER = LER_012;       // Обновление MR0,MR1 и MR2
  LPC_PWM0->TCR = COUNTER_STOP;  // Включение PWM. Счётчик - стоп
  LPC_PWM0->TCR = COUNTER_RESET;

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

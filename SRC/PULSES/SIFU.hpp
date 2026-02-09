#pragma once

#include "LPC407x_8x_177x_8x.h"
#include "proxy_pointer_var.hpp"
#include "puls_calc.hpp"
#include "rem_osc.hpp"
#include "RegManager.hpp"
#include "FaultCtrlP.hpp" 
#include "bool_name.hpp"

class CRegManager;

class CSIFU {
 public:
  CSIFU(CPULSCALC&, CRegManager&, CFaultCtrlP&, CEEPSettings&, CREM_OSC&);

  CPULSCALC& rPulsCalc;
  CRegManager& rReg_manager;
  CFaultCtrlP& rFault_p;
  CREM_OSC& rRemOsc;

  void forcing_bridge_pulses_On();  // Подать импульсы на форсировочный мост
  void main_bridge_pulses_On();     // Подать импульсы на основной мост
  void all_bridge_pulses_Off();     // Снять импульсы с обоих мостов
  void start_phasing_mode();        // Установить режим фазировки
  void stop_phasing_mode();         // Снять режим фазировки
  void execute_mode_Wone();         // Выполнить режим "Через один"  

  void set_a_shift(signed short);   // Установка точного сдвига синхронизации
  void set_d_shift(unsigned char);  // Установка дискретного сдвига синхронизации
  void set_alpha(signed short);     // Установка Alpha
  signed short get_alpha();         // Чтение Alpha
  bool* get_pSyncStat();            // Чтение текущего статуса синхронизации
  float* get_Sync_Frequency();      // Возвращает частоту синхронизации
  signed short* getPointerAlpha();
   

  void init_and_start(CProxyPointerVar&);  // Инициализация
  void rising_puls();     // Фронт импульса
  void faling_puls();     // Спад импульса 

  // Структура констант
  struct SIFUConst {
    
    static constexpr float DT_MIN = 19608; // 51.0 Hz
    static constexpr float DT_MAX = 20408; // 49.0 Hz 
    
    static constexpr signed short _0gr = 0;
    static constexpr signed short _5gr =   (5 * 10000 + 90) / 180;
    static constexpr signed short _10gr = (10 * 10000 + 90) / 180;
    static constexpr signed short _13gr = (13 * 10000 + 90) / 180;
    static constexpr signed short _15gr = (15 * 10000 + 90) / 180;
    static constexpr signed short _30gr = (30 * 10000 + 90) / 180;
    static constexpr signed short _60gr = (60 * 10000 + 90) / 180;
    static constexpr signed short _90gr = 5000;
    static constexpr signed short _120gr = (120 * 10000 + 90) / 180;
    static constexpr signed short _150gr = (150 * 10000 + 90) / 180;
    static constexpr signed short _180gr = 10000;
    
    static constexpr signed int PULSE_WIDTH = 2515;  // us
    
    static constexpr float TIC_SEC = 1000000.0;
    
    static constexpr signed short MaxPshift = _90gr;
    static constexpr signed short MinPshift = -_90gr;
    
    static constexpr signed short AMax = _150gr;
    static constexpr signed short AMin = _30gr;
    static constexpr signed short dAlpha = _15gr;
    
    static constexpr unsigned int N_PULSES = 6;
    static constexpr unsigned int N_PULSES_STOP = 50;
    
    static constexpr signed char N_PULSES_WONE = 12;
    
  } s_const;
  
   unsigned char N_Pulse = 1;

private:
  static constexpr unsigned int FIRST_PULS_PORT = 16;               // 1-й импульс в порту
  static constexpr unsigned int OFF_PULSES = 0x3F<<FIRST_PULS_PORT; // Все импульсы 
  
  static constexpr unsigned int pulsesAllP[] = { 
    0x00, 
    0x21 << FIRST_PULS_PORT, 
    0x03 << FIRST_PULS_PORT, 
    0x06 << FIRST_PULS_PORT, 
    0x0C << FIRST_PULS_PORT, 
    0x18 << FIRST_PULS_PORT,
    0x30 << FIRST_PULS_PORT 
  }; 
  
  static constexpr unsigned int pulsesWone[] = {
    0x00, 
    pulsesAllP[1], 0x00, 
    pulsesAllP[3], 0x00, 
    pulsesAllP[5], 0x00
  };    
  
  static constexpr signed short offsets[] = {
   0x00,
   SIFUConst::_60gr,    // Диапазон 0...60        (sync "видит" 1-й: ->2-3-4-5-6-sync-1->2-3-4...)
   SIFUConst::_120gr,   // Диапазон -60...0       (sync "видит" 2-й: ->3-4-5-6-1-sync-2->3-4-5...)
   SIFUConst::_180gr,   // Диапазон -120...-60    (sync "видит" 3-й: ->4-5-6-1-2-sync-3->4-5-6...) - чисто теоретически
  -SIFUConst::_120gr,   // Диапазон 180...240     (sync "видит" 4-й: ->5-6-1-2-3-sync-4->5-6-1...) - чисто теоретически
  -SIFUConst::_60gr,    // Диапазон 120...180     (sync "видит" 5-й: ->6-1-2-3-4-sync-5->6-1-2...)
   SIFUConst::_0gr      // Диапазон 60...120      (sync "видит" 6-й: ->1-2-3-4-5-sync-6->1-2-3...)
  };  // Индекс 0 не используется

  inline void StartMainBridgePWM0() {
    
    if(!wone_reg) {
      LPC_GPIO3->CLR = pulsesAllP[(((N_Pulse - 1) + v_sync.d_power_shift) % s_const.N_PULSES) + 1];
    } else{
      LPC_GPIO3->CLR = pulsesWone[(((N_Pulse - 1) + v_sync.d_power_shift) % s_const.N_PULSES) + 1];
    }    
    
    LPC_SC->PCONP |= CLKPWR_PCONP_PCPWM0;
    
    LPC_PWM0->PR  = PWM_div_0 - 1;
    LPC_PWM0->MCR = MR0R; // Reset TC on MR0
    LPC_PWM0->MR0 = PWM_WIDTH * 2;
    LPC_PWM0->MR1 = PWM_WIDTH;
    LPC_PWM0->LER = LER_012;
    LPC_PWM0->PCR |= PCR_PWMENA1;
    
    // Важно
    LPC_PWM0->TCR = COUNTER_RESET; // Обнулили TC и PR
    LPC_PWM0->TC  = LPC_PWM0->MR0; // Вручную ставим счетчик в значение финиша
    
    // Настройка вывода
    LPC_IOCON->P1_2 = IOCON_P_PWM; // P1_2 -> PWM
    
    LPC_PWM0->TCR = COUNTER_START; // Запуск
  }
  
  inline void StartForsingBridgePWM0() {
    
    LPC_GPIO3->CLR = pulsesAllP[(((N_Pulse - 1) + v_sync.d_power_shift) % s_const.N_PULSES) + 1];
    
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
  
  
  bool forcing_bridge = false;
  bool main_bridge = false;
  bool wone_reg = false;
  State phase_stop = State::OFF;
  PulsPhase puls_phase = PulsPhase::RISING;
  signed int RISING_MR0;
  signed short n_pulses_stop = 0;
  signed short n_pulses_wone = 0;

  signed short Alpha_setpoint = s_const.AMax;
  signed short Alpha_current = s_const.AMax;
  
  void off_pulses_control();        // Контроль фазы ртключения ИУ
  void control_fault_and_reg();     // Контроль аварий и регулирование
  void off_wone_reg();              // Контроль отключения режима "Через один"
  
  void control_sync();
  unsigned int timing_calc();
  signed short limits_val(signed short*, signed short, signed short);
  signed short limits_dval(signed short*, signed short*, signed short);

  enum class EOperating_mode { NO_SYNC, RESYNC, NORMAL, PHASING };

  EOperating_mode Operating_mode = EOperating_mode::NO_SYNC;  // Текущий режим работы СИФУ
  
  bool SyncStat;

  struct SyncState  // Структура переменных касающихся синхронизации
  {
    unsigned char d_power_shift;  // Дискретный сдвиг синхронизации по 60гр.

    signed short cur_power_shift;  // Точный сдвиг синхронизации.
    signed short task_power_shift;

    bool SYNC_EVENT = false;    // Флаг события захвата
    unsigned int CURRENT_SYNC;  // Актуальные данные захвата

    unsigned int cur_capture;           // Текущие данные захвата таймера
    unsigned int prev_capture;          // Предыдущие данные захвата таймера
    
    unsigned short no_sync_pulses = 0;  // Количество пульсов отсутствия события захвата
    unsigned short sync_pulses = 0;     // Количество пульсов с событиями захвата
    unsigned short err_sync_pulses = 0; // Количество событий захвата с недопустимым периодом


    float SYNC_FREQUENCY;  // Измеренная частота

  } v_sync;
  
  CEEPSettings& rSettings;

  // -- Аппаратные константы ----------------------------------------------------------------------

  static constexpr unsigned int IOCON_P1_PWM = 0x03;  // Тип портов - PWM
  static constexpr unsigned int PWM_div_0 = 60;       // Делитель частоты

  static constexpr unsigned int IOCON_P_PWM = 0x03;   // Тип портов - PWM
  static constexpr unsigned int IOCON_P_PORT = 0x00;  // Тип портов - Port
  static constexpr unsigned int P1_2 = 0x02;          // Port1:2
  static constexpr unsigned int P1_3 = 0x03;          // Port1:3

  static constexpr unsigned int PWM_WIDTH = 10;                    // us

  static constexpr unsigned int PCR_PWMENA1 = 0x200;
  static constexpr unsigned int PCR_PWMENA2 = 0x400;
  static constexpr unsigned int LER_012 = 0x07;
  static constexpr unsigned int MR0R = 1 << 1;
  static constexpr unsigned int COUNTER_CLR = 0x00;
  static constexpr unsigned int COUNTER_RESET = 0x02;
  static constexpr unsigned int COUNTER_STOP = 0x0B;
  static constexpr unsigned int COUNTER_START = 0x09;

  static constexpr unsigned int TIM3_TCR_START = 0x01;

  static constexpr unsigned int TIM3_COMPARE_MR0 = 0x01;
  static constexpr unsigned int TIM3_COMPARE_MR1 = 0x08;
  static constexpr unsigned int TIM3_CAPTURE_RI = 0x08;
  static constexpr unsigned int IOCON_T3_CAP1 = 0x23;
};

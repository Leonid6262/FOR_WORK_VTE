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
  CSIFU(CPULSCALC&, CRegManager&, CFaultCtrlP&, CEEPSettings&);

  CPULSCALC& rPulsCalc;
  CRegManager& rReg_manager;
  CFaultCtrlP& rFault_p;

  void forcing_bridge_pulses_On();  // Подать импульсы на форсировочный мост
  void main_bridge_pulses_On();     // Подать импульсы на основной мост
  void all_bridge_pulses_Off();     // Снять импульсы с обоих мостов
  void start_phasing_mode();  // Установить режим фазировки
  void stop_phasing_mode();   // Снять режим фазировки

  void set_a_shift(signed short);   // Установка точного сдвига синхронизации
  void set_d_shift(unsigned char);  // Установка дискретного сдвига синхронизации
  void set_alpha(signed short);     // Установка Alpha
  signed short get_alpha();         // Чтение Alpha
  unsigned char* getSyncStat();     // Чтение текущего статуса синхронизации
  float* get_Sync_Frequency();      // Возвращает частоту синхронизации
  signed short* getPointerAlpha();

  void init_and_start(CProxyPointerVar&);  // Инициализация
  void rising_puls();     // Фронт импульса
  void faling_puls();     // Спад импульса
  
  struct SIFUConst  // Структура констант
  {
    static constexpr float DT_MIN = 19608; // 51.0 Hz
    static constexpr float DT_MAX = 20408; // 49.0 Hz 
    static constexpr signed short _0gr = 0;
    static constexpr signed short _5gr = 278;
    static constexpr signed short _10gr = 555;
    static constexpr signed short _30gr = 1667;
    static constexpr signed short _60gr = 3333;
    static constexpr signed short _90gr = 5000;
    static constexpr signed short _120gr = 6667;
    static constexpr signed short _150gr = 8333;
    static constexpr signed short _180gr = 10000;
    
    static constexpr signed int PULSE_WIDTH = 1515;  // us
    
    static constexpr float TIC_SEC = 1000000.0;
    
    static constexpr signed short MaxPshift = _90gr;
    static constexpr signed short MinPshift = -_90gr;
    
    static constexpr signed short AMax = _150gr;
    static constexpr signed short AMin = _30gr;
    static constexpr signed short dAlpha = _10gr;
    
    static constexpr unsigned int N_PULSES = 6;
    static constexpr unsigned int N_PULSES_STOP = 50;
    
  } s_const;
  
   unsigned char N_Pulse;

 private:
  static const unsigned char pulses[];
  static const unsigned char pulse_w_one[];
  static const signed short offsets[];

  bool forcing_bridge;
  bool main_bridge;
  State phase_stop = State::OFF;
  signed short n_pulses_stop = 0;

  signed short Alpha_setpoint;
  signed short Alpha_current;
  
  void off_pulses_control();        // Контроль фазы ртключения ИУ
  void control_fault_and_reg();     // Контроль аварий и регулирование
  
  void control_sync();
  unsigned int timing_calc();
  signed short limits_val(signed short*, signed short, signed short);
  signed short limits_dval(signed short*, signed short*, signed short);

  enum class EOperating_mode { NO_SYNC, RESYNC, NORMAL, PHASING };

  EOperating_mode Operating_mode;  // Текущий режим работы СИФУ
  unsigned char curSyncStat;

  struct SyncState  // Структура переменных касающихся синхронизации
  {
    unsigned char d_power_shift;  // Дискретный сдвиг синхронизации по 60гр.

    signed short cur_power_shift;  // Точный сдвиг синхронизации.
    signed short task_power_shift;

    bool SYNC_EVENT;            // Флаг события захвата
    unsigned int CURRENT_SYNC;  // Актуальные данные захвата

    unsigned int cur_capture;       // Текущие данные захвата таймера
    unsigned int prev_capture;      // Предыдущие данные захвата таймера
    unsigned short no_sync_pulses;  // Количество пульсов отсутствия события захвата
    unsigned short sync_pulses;     // Количество пульсов с событиями захвата

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

  static constexpr unsigned int PWM_WIDTH = 10;           // us
  static constexpr unsigned int OFF_PULSES = 0x003F0000;  // Импульсы в порту
  static constexpr unsigned int FIRS_PULS_PORT = 16;      // 1-й импульс в порту

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

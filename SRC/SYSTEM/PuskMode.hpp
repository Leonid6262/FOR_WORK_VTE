#pragma once
#include "bool_name.hpp"
#include "dIOStorage.hpp"
#include "AdcStorage.hpp"
#include "SIFU.hpp"

class CSystemManager;

class CPuskMode {
public:
  CPuskMode(CDIN_STORAGE&, CSIFU&, CEEPSettings&);
  
  void setSysManager(CSystemManager*);
  void pusk(bool Permission);   // основной цикл автомата
  
  inline float* getPointerPslip() { return &StartingSlip;  }
  inline float* getPointerPis()   { return &StartingIS;    }
  inline bool*  getPointerWex()   { return &WithoutExMode; }
  inline bool*  getPointerSlE()   { return &slip_ev;       }
  inline bool*  getPointerSPK()   { return &PK_Status;     }
  
private:
  CDIN_STORAGE& rDinStr;
  CSIFU& rSIFU;
  CEEPSettings& rSet;
  CADC_STORAGE& pAdc;
  CSystemManager* pSys_manager;
  
  State cur_status = State::OFF;
  unsigned int prev_TC0_Phase;
  unsigned int dTrsPhase;

  float StartingIS   = 0.0f;    // Ток статора при котором осуществлён пуск (информационно)
  float StartingSlip = 1.0f;    // Скольжение при котором осуществлён пуск (информационно)  
  bool PK_Status = false;       // Статус ПК
  StartIS PuskIS = StartIS::NO; // Наличие IS при пуске
  char last_processed_ind = -1; // Храним индекс последнего обработанного замера IS
  unsigned short is_confirm_cnt = 0;
  
  bool WithoutExMode = false;   // Режим пуска без подачи возбуждения
  bool slip_ev = false;         // Индикатор события - скольжение измеренно, глубина достигнута
  unsigned char c_slip_ev = 0;  // Счётчик событий slip_ev (для снижения частоты индикации)
  
  enum class EPhasesPusk : unsigned short {
    CheckISctrlPK,
    WaitISdrop,
    SelfSync,
    Forcing,
    Pause,
    ClosingKey
  };
  
  EPhasesPusk phases_pusk;
  
  // Фазы пуска
  void CheckISctrlPK();
  void WaitISdrop();
  void SelfSync();
  void Forcing();
  void Pause();
  void ClosingKey();
  
  void StopPusk();
  void StartEx();
  
  inline void INIT_CAPTURE1_TIM2() { 
    LPC_IOCON->P2_15 = IOCON_T2_CAP1;  // T2 CAP1
    LPC_TIM2->MCR = 0x00000000;        // disabled
    LPC_TIM2->IR = 0xFFFFFFFF;         // Очистка флагов прерываний
    LPC_TIM2->TCR |= TIM2_TCR_START;   // Старт таймера TIM3 
    LPC_TIM2->TC = 0;
    LPC_TIM2->CCR = TIM2_CAPTURE_RI;    // Захват T2 по спаду CAP1 без прерываний
    
    NVIC_EnableIRQ(TIMER2_IRQn);
  }
    
  
  inline StartIS control_IS() { 
    // Достаем текущий индекс из расчета IS
    char current_ind = rSIFU.rPulsCalc.get_ind_d_avr();
    // Если это новый замер - контролируем
    if (current_ind != last_processed_ind) {
      last_processed_ind = current_ind; // Замер учтён
      if(*rSIFU.rPulsCalc.getPointer_istator_rms() > rSet.getSettings().set_pusk.ISPusk * 0.3f) {
        is_confirm_cnt++;
      } 
    }    
    // Порог принятия решения - 2 периода
    if(is_confirm_cnt >= 12) {
      return StartIS::YES;
    } else {
      return StartIS::NO;
    }
  }
  
  inline bool switching_check_pk(Check mode) { 
    static unsigned int prev_capture;
    static unsigned short n_switch = 0;
    static unsigned short u_rotor_p = 0;
    static unsigned short u_rotor_n = 0;
    static constexpr unsigned char MIN_TOGGLE = 15;
    
    if (mode == Check::RESET) {
      prev_capture = LPC_TIM2->CR1;
      n_switch = 0;
      u_rotor_p = 0;
      u_rotor_n = 0;
      return false;
    }
    
    unsigned int cur_capture = LPC_TIM2->CR1;
    if (prev_capture != cur_capture) {
      unsigned int dt = cur_capture - prev_capture;
      prev_capture = cur_capture;
      if (dt > (HALF_NET_PERIOD * 1.8f)) {
        n_switch++;
        if(pAdc.getExternal(CADC_STORAGE::ROTOR_VOLTAGE) > 0) u_rotor_p++;
        if(pAdc.getExternal(CADC_STORAGE::ROTOR_VOLTAGE) < 0) u_rotor_n++;
      }      
    }    
    if((n_switch > MIN_TOGGLE) && (u_rotor_p  > MIN_TOGGLE) && (u_rotor_n  > MIN_TOGGLE)) {
      return true;
    }
    return false;    
  }
  
  static constexpr unsigned int   TICK_SEC        = 10000000;  
  static constexpr unsigned int   CHECK_IS        = 10000000; // 1,0 сек  
  static constexpr unsigned int   BRIDGE_CHANGAE  = 1000000;  // 0,1 сек
  static constexpr unsigned int   PAUSE           = 5000000;  // 0,5 сек
  static constexpr unsigned int   CLOSING_KEY     = 5000000;  // 0,5 сек
  static constexpr unsigned short EVENT_INDICAT   = 3; // Пропусков события для индикации 
  static constexpr unsigned char  HARD_TIMEOUT    = 2; // сек
  
  static constexpr unsigned int DELAY_TIME       = 2000000;  // 2 сек
  
  static constexpr float HALF_NET_PERIOD = 10000.0f;
  
  static constexpr unsigned int TIM2_TCR_START = 0x01;
  static constexpr unsigned int TIM2_CAPTURE_RI = 0x10;
  static constexpr unsigned int IOCON_T2_CAP1 = 0x23;
};


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
  
  inline float* getPointerPslip() { return &pusk_slip; }
  
private:
  CDIN_STORAGE& rDinStr;
  CSIFU& rSIFU;
  CEEPSettings& rSet;
  CADC_STORAGE& pAdc;
  CSystemManager* pSys_manager;
  
  State cur_status = State::OFF;
  unsigned int prev_TC0_Phase;
  unsigned int dTrsPhase;
  float pusk_slip = 1.0f;
  
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
    
  inline bool switching_check_pk(Mode mode) { 
    static unsigned int prev_capture;
    static unsigned short n_switch = 0;
    static unsigned short u_rotor_p = 0;
    static unsigned short u_rotor_n = 0;
    static constexpr unsigned char MIN_TOGGLE = 15;
    
    if (mode == Mode::FORBIDDEN) {
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
  
  static constexpr unsigned int  TICK_SEC        = 10000000;  
  static constexpr unsigned int  CHECK_IS        = 10000000; // 1,0 сек  
  static constexpr unsigned int  BRIDGE_CHANGAE  = 1000000;  // 0,1 сек
  static constexpr unsigned int  PAUSE           = 5000000;  // 0,5 сек
  static constexpr unsigned int  CLOSING_KEY     = 5000000;  // 0,5 сек
  
  static constexpr unsigned int DELAY_TIME       = 2000000;  // 2 сек
  
  static constexpr float HALF_NET_PERIOD = 10000.0f;
  
  static constexpr unsigned int TIM2_TCR_START = 0x01;
  static constexpr unsigned int TIM2_CAPTURE_RI = 0x10;
  static constexpr unsigned int IOCON_T2_CAP1 = 0x23;
};


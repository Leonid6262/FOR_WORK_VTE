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
  
private:
  CDIN_STORAGE& rDinStr;
  CSIFU& rSIFU;
  CEEPSettings& rSet;
  CADC_STORAGE& pAdc;
  CSystemManager* pSys_manager;
  
  State cur_status = State::OFF;
  unsigned int prev_TC0_Phase;
  unsigned int dTrsPhase;
  
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
  }
  
  struct SlipeStatus {
    bool slipe_event;
    float slipe_value;
    signed char ud_polarity;
  } slipe_status;
   
  inline bool status_pk(bool reset) { 
    static unsigned int prev_capture;
    static unsigned int N_PK = 0;
    static bool status = false;
    
    if (reset) {
      N_PK = 0;
      status = false;
      return false;
    }
    
    unsigned int cur_capture = LPC_TIM2->CR1;
    if (prev_capture != cur_capture) {
      prev_capture = cur_capture;
      N_PK++;
      if(N_PK > 5) status = true;
    }
    
    return status;    
  }
  
  static constexpr unsigned char N_CU_TOGGLE     = 10;    
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


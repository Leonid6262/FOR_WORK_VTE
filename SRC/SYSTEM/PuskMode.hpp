#pragma once
#include "bool_name.hpp"
#include "dIOStorage.hpp"
#include "SIFU.hpp"

class CSystemManager;

class CPuskMode {
public:
  CPuskMode(CDIN_STORAGE&, CSIFU&, CEEPSettings&);
  
  void setSysManager(CSystemManager*);
  void pusk(bool Permission);   // основной цикл автомата
  
  float pusk_slip;
  
private:
  CDIN_STORAGE& rDinStr;
  CSIFU& rSIFU;
  CEEPSettings& rSet;
  CSystemManager* pSys_manager;
  
  State cur_status;
  unsigned int prev_TC0_Phase;
  unsigned int dTrsPhase;
  
  float cur_slip;
 
  enum class EPhasesPusk : unsigned short {
    CheckISctrlPK,
    WaitISdropOrSlip,
    SelfSync,
    Forcing,
    RelayExOn,
    RelayPause,
    ClosingKey
  };
  
  EPhasesPusk phases_pusk;
  
  // Фазы пуска
  void CheckISctrlPK();
  void WaitISdropOrSlip();
  void SelfSync();
  void Forcing();
  void RelayExOn();
  void RelayPause();
  void ClosingKey();
  
  void StopPusk();
  
  inline float calc_slip(float slip) { 
    static unsigned int prev_capture;
    unsigned int cur_capture = LPC_TIM2->CR1;
    if (prev_capture != cur_capture) { 
      unsigned int dt = cur_capture - prev_capture;
      prev_capture = cur_capture;
      float cur_slip = 1.0f - (HALF_NET_PERIOD / dt);
      return ((cur_slip > 0) ? cur_slip : 0);
    }
    return slip;
  }
  
  
  static constexpr unsigned char N_CU_TOGGLE     = 10;    
  static constexpr unsigned int  TICK_SEC        = 10000000;  
  static constexpr unsigned int  CHECK_IS        = 10000000; // 1,0 сек  
  static constexpr unsigned int  BRIDGE_CHANGAE  = 1000000;  // 0,1 сек
  static constexpr unsigned int  RELAY_PAUSE_OFF = 5000000;  // 0,5 сек
  static constexpr unsigned int  CLOSING_KEY     = 500000;   // 0,05 сек
  
  static constexpr unsigned int DELAY_TIME      = 2000000; // 2 сек
  
  static constexpr float HALF_NET_PERIOD = 10000.0f;
  
  static constexpr unsigned int TIM2_TCR_START = 0x01;
  static constexpr unsigned int TIM2_CAPTURE_RI = 0x10;
  static constexpr unsigned int IOCON_T2_CAP1 = 0x23;
};


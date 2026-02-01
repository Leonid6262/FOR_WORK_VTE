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
  
private:
  CDIN_STORAGE& rDinStr;
  CSIFU& rSIFU;
  CEEPSettings& rSet;
  CSystemManager* pSys_manager;
  
  State cur_status;
  unsigned int prev_TC0_Phase;
  unsigned int prev_cu_time;
  unsigned int dTrsPhase;
  StatusRet PK_STATUS;
  
  bool prev_cu;
  unsigned short cu_toggle_count;
  float slip;
 
  enum class EPhasesPusk : unsigned short {
    CheckISctrlPK,
    WaitISdrop,
    Delay,
    Forcing,
    RelayExOn,
    RelayPause,
    ClosingKey,
    ControlKey
  };
  
  EPhasesPusk phases_pusk;
  
  // Фазы пуска
  void CheckISctrlPK();
  void WaitISdrop();
  void Delay();
  void Forcing();
  void RelayExOn();
  void RelayPause();
  void ClosingKey();
  void ControlKey();
  
  static constexpr unsigned char N_CU_TOGGLE     = 10;    
  static constexpr unsigned int  TICK_SEC        = 10000000;  
  static constexpr unsigned int  CHECK_IS        = 5000000; // 0,5 сек  
  static constexpr unsigned int  BRIDGE_CHANGAE  = 1000000; // 0,1 сек
  static constexpr unsigned int  RELAY_PAUSE_OFF = 5000000; // 0,5 сек
  static constexpr unsigned int  CLOSING_KEY     = 500000;  // 0,05 сек
  static constexpr float         HALF_NET_PERIOD = 100000.0f;
  
  static constexpr unsigned int DELAY_TIME      = 2000000; // 2 сек
};


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
  unsigned int dTrsPhase;
  StatusRet PK_STATUS;
  
  enum class EPhasesPusk : unsigned short {
    CheckCurrent,
    WaitCurrentDrop,
    Delay,
    Forcing,
    RelayExOn,
    RelayPause,
    ClosingKey,
    ControlKey
  };
  
  EPhasesPusk phases_pusk;
  
  // Фазы пуска
  void CheckCurrent();
  void WaitCurrentDrop();
  void Delay();
  void Forcing();
  void RelayExOn();
  void RelayPause();
  void ClosingKey();
  void ControlKey();
  
  void StopPusk();
  
  // Константы времени
  static constexpr unsigned int TICK_SEC        = 10000000;
  
  static constexpr unsigned int CHECK_IS        = 5000000; // 0,5 сек
  
  static constexpr unsigned int BRIDGE_CHANGAE  = 1000000; // 0,1 сек
  static constexpr unsigned int RELAY_PAUSE_OFF = 5000000; // 0,5 сек
  static constexpr unsigned int CLOSING_KEY     = 500000;  // 0,05 сек
  
  
  static constexpr unsigned int DELAY_TIME      = 2000000; // 2 сек
};


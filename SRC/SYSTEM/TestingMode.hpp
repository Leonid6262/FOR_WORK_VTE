#pragma once 
#include "bool_name.hpp"
#include "dIOStorage.hpp"
#include "SIFU.hpp"

class CSystemManager;

class CTestingMode {
  
public:
  CTestingMode(CDIN_STORAGE&, CSIFU&, CEEPSettings&);
  
  void StartPhase();
  void Forcing();
  void RelayExOn();
  void RelayPause();    
  void ClosingKey();
  void ControlKey();
  void Regulation();
  
  void setSysManager(CSystemManager*);
  void test(bool);
  
private:
  CDIN_STORAGE& rDinStr;
  CSIFU& rSIFU;
  CEEPSettings& rSet;
  CSystemManager* pSys_manager;
  
  State cur_status;
  unsigned int prev_TC0_Phase;
  unsigned int dTrsPhase;
  unsigned int prev_TC0_Reg;
  unsigned int dTrsReg;
  StatusRet PK_STATUS; 
  
  unsigned short dIset; 
  static constexpr float dTset = 0.2f; // 0.2  сек
    
  static constexpr unsigned int _to_sec         = 10000000;
  static constexpr unsigned int BRIDGE_CHANGAE  = 1000000; // 0,1 сек
  static constexpr unsigned int RELAY_PAUSE_OFF = 5000000; // 0,5 сек
  static constexpr unsigned int CLOSING_KEY     = 500000;  // 0,05 сек

  enum class EPhasesTest : unsigned short {
    StartMode,
    Forcing,
    RelayExOn,
    RelayPause,    
    ClosingKey,
    ControlKey,
    Regulation
  };
  
  EPhasesTest phases_test;
  
  void StopTest();
 
};

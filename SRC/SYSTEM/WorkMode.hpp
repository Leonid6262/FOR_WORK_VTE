#pragma once 
#include "bool_name.hpp"
#include "dIOStorage.hpp"
#include "dac.hpp"
#include "SIFU.hpp"

class CSystemManager;

class CWorkMode {
  
public:
  CWorkMode(CDIN_STORAGE&, CSIFU&, CEEPSettings&, CDAC_PWM&);
  
  void setSysManager(CSystemManager*);
  void work(bool);
  
private:
  CDIN_STORAGE& rDinStr;
  CSIFU& rSIFU;
  CEEPSettings& rSet;
  CDAC_PWM& rDacCos;
  CSystemManager* pSys_manager; 
  
  State cur_status;
  
  void CurrentSetting();
  void StopWork();

  unsigned int prev_TC0_Reg;
  unsigned int dTrsReg;
  unsigned short Iset;
  
  unsigned short dIset; 
  static constexpr float ChangeInterval = 0.2f; // 0.2  сек
    
  static constexpr unsigned int TICK_SEC        = 10000000;
  
};

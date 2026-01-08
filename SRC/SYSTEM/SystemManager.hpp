#pragma once 

#include "Adjustment.hpp"
#include "FaultControl.hpp"
#include "ReadyCheck.hpp"
#include "PuskMode.hpp"
#include "WorkMode.hpp"
#include "WarningControl.hpp" 
#include "SIFU.hpp"
#include "RegManager.hpp" 

class CSystemManager {
  
public:
 CSystemManager(CSIFU&, 
                CAdjustmentMode&, 
                CReadyCheck&, 
                CFaultControl&, 
                CPuskMode&, 
                CWorkMode&,
                CWarningMode&,
                CRegManager&);
 
 CSIFU& rSIFU;
 CAdjustmentMode& rAdj_mode; 
 CReadyCheck& rReady_check;
 CFaultControl& rFault_ctrl;
 CPuskMode& rPusk_mode;
 CWorkMode& rWork_mode;
 CWarningMode& rWarning_ctrl;
 CRegManager& rReg_manager;
 
 // --- Статусы системы ---
 union {
   unsigned char all;
   struct {
     unsigned char Ready        : 1; // Готовность собрана
     unsigned char Pusk         : 1; // Двигатель запущен
     unsigned char Work         : 1; // Штатный режим работы
     unsigned char Warning      : 1; // Предупреждение
     unsigned char Fault        : 1; // Авария
   };
 } USystemStatus;
 
 void setReady(Bit_switch state)        { USystemStatus.Ready   = static_cast<unsigned char>(state); }
 void setPusk(Bit_switch state)         { USystemStatus.Pusk    = static_cast<unsigned char>(state); }
 void setWork(Bit_switch state)         { USystemStatus.Work    = static_cast<unsigned char>(state); }
 void setWarning(Bit_switch state)      { USystemStatus.Warning = static_cast<unsigned char>(state); }
 void setFault(Bit_switch state)        { USystemStatus.Fault   = static_cast<unsigned char>(state); }

 // --- Разрешения режимов ---
 union {
   unsigned char all;
   struct {
     unsigned char Adjustment   : 1;  // Разрешение Наладки
     unsigned char ReadyCheck   : 1;  // Разрешение Сборки готовности
     unsigned char FaultCtrl    : 1;  // Разрешение Контроля аварий
     unsigned char PuskMode     : 1;  // Разрешение Фазы пуска
     unsigned char WorkMode     : 1;  // Разрешение режима Работа
   };
 } USystemMode;
 
 void setAdjustment(Mode mode)  { USystemMode.Adjustment = static_cast<unsigned char>(mode); } 
 void setReadyCheck(Mode mode)  { USystemMode.ReadyCheck = static_cast<unsigned char>(mode); } 
 void setFaultCtrl(Mode mode)   { USystemMode.FaultCtrl  = static_cast<unsigned char>(mode); } 
 void setPuskMode(Mode mode)    { USystemMode.PuskMode   = static_cast<unsigned char>(mode); } 
 void setWorkMode(Mode mode)    { USystemMode.WorkMode   = static_cast<unsigned char>(mode); }
 
 void dispatch();
 
private:
  
  // --- Биты статуса системы --- 
  enum SBit : unsigned char { 
    Ready   = 1 << 0, // Готовность собрана 
    PuskOK  = 1 << 1, // Двигатель запущен 
    Work    = 1 << 2, // Штатный режим работы 
    Warning = 1 << 3, // Предупреждение 
    Fault   = 1 << 4  // Авария 
  };
  
  // --- Биты разрешений режимов --- 
  enum MBit : unsigned char { 
    Adjustment = 1 << 0, // Разрешение Наладки 
    ReadyCheck = 1 << 1, // Разрешение Сборки готовности 
    FaultCtrl  = 1 << 2, // Разрешение Контроля аварий 
    PuskMode   = 1 << 3, // Разрешение Фазы пуска 
    WorkMode   = 1 << 4  // Разрешение режима Работа 
  };
  
  // --- Правило зависимости --- 
  struct DependencyRule { 
    MBit req_bit;                     // какой режим проверяем 
    unsigned char requiredStatus;     // какие статусы должны быть включены 
    unsigned char forbiddenStatus;    // какие статусы должны быть выключены   
    unsigned char forbiddenModes;     // какие режимы должны быть выключены
  };
  
  // --- Таблица правил --- 
  static constexpr std::array<DependencyRule, 5> rules {{ 
    { MBit::ReadyCheck, 0,                                                   SBit::Fault, MBit::PuskMode | MBit::WorkMode },
    { MBit::Adjustment, MBit::ReadyCheck,                                    SBit::Fault, MBit::PuskMode | MBit::WorkMode },  
    { MBit::PuskMode,   SBit::Ready,                                         SBit::Fault, MBit::WorkMode }, 
    { MBit::WorkMode,   SBit::PuskOK,                                        SBit::Fault, MBit::PuskMode }, 
    { MBit::FaultCtrl,  MBit::PuskMode | MBit::WorkMode | MBit::Adjustment,  SBit::Fault,  } 
  }};
  
};

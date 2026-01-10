#pragma once 

#include "Adjustment.hpp"
#include "FaultControl.hpp"
#include "ReadyCheck.hpp"
#include "PuskMode.hpp"
#include "WorkMode.hpp"
#include "WarningControl.hpp" 
#include "SIFU.hpp"

class CReadyCheck;

class CSystemManager {
  
public:
 CSystemManager(CSIFU&, CAdjustmentMode&, CReadyCheck&, CFaultControl&, 
                CPuskMode&, CWorkMode&, CWarningMode&, CRegManager&);
 
 CSIFU& rSIFU;
 CAdjustmentMode& rAdj_mode; 
 CReadyCheck& rReady_check;
 CFaultControl& rFault_ctrl;
 CPuskMode& rPusk_mode;
 CWorkMode& rWork_mode;
 CWarningMode& rWarning_ctrl;
 CRegManager& rReg_manager;
 
 // --- Статус системы ---
 union {
   unsigned char all;
   struct {
     unsigned char Ready        : 1; // Готовность собрана
     unsigned char Pusk         : 1; // Двигатель запущен
     unsigned char Work         : 1; // Штатный режим работы
     unsigned char Warning      : 1; // Есть предупреждения
     unsigned char Fault        : 1; // Авария
   };
 } USystemStatus;
 
 void setReady(Bit_switch state)        { USystemStatus.Ready   = static_cast<unsigned char>(state); }
 void setPusk(Bit_switch state)         { USystemStatus.Pusk    = static_cast<unsigned char>(state); }
 void setWork(Bit_switch state)         { USystemStatus.Work    = static_cast<unsigned char>(state); }
 void setWarning(Bit_switch state)      { USystemStatus.Warning = static_cast<unsigned char>(state); }
 void setFault(Bit_switch state)        { USystemStatus.Fault   = static_cast<unsigned char>(state); }

 // --- Текущие режимы ---
 union USystemMode_t{
   unsigned char all;
   struct {
     unsigned char Adjustment   : 1;  // Наладки
     unsigned char ReadyCheck   : 1;  // Сборка готовности
     unsigned char FaultCtrl    : 1;  // Контроль аварий
     unsigned char PuskMode     : 1;  // Фаза пуска
     unsigned char WorkMode     : 1;  // Работа
   };
 } USystemMode;                 // Текущие режимы
 
 USystemMode_t USMode_r;        // Запрос режимов
 
 void setAdjustment(Mode mode)  { USMode_r.Adjustment = static_cast<unsigned char>(mode); } 
 void setReadyCheck(Mode mode)  { USMode_r.ReadyCheck = static_cast<unsigned char>(mode); } 
 void setFaultCtrl(Mode mode)   { USMode_r.FaultCtrl  = static_cast<unsigned char>(mode); } 
 void setPuskMode(Mode mode)    { USMode_r.PuskMode   = static_cast<unsigned char>(mode); } 
 void setWorkMode(Mode mode)    { USMode_r.WorkMode   = static_cast<unsigned char>(mode); }
 
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
  
  // --- Биты режимов --- 
  enum MBit : unsigned char { 
    Adjustment = 1 << 0, // Наладка 
    ReadyCheck = 1 << 1, // Сборка готовности 
    FaultCtrl  = 1 << 2, // Контроль аварий 
    PuskMode   = 1 << 3, // Фаза пуска 
    WorkMode   = 1 << 4  // Работа 
  };
  
  // --- Таблица правил --- 
  struct DependencyRule {
    MBit req_bit;
    unsigned char requiredStatus;   // Какие биты статуса должны быть установлены
    unsigned char forbiddenStatus;  // Какие биты статуса должны быть сброшены
    unsigned char requiredModes;    // Какие режимы должны быть активны
    unsigned char forbiddenModes;   // Какие режимы должны быть отключены
  };

static constexpr std::array<DependencyRule, 5> rules {{
  //    Check Mode      status on      status off       mode on              mode off
  { MBit::ReadyCheck, 0,              SBit::Fault,   0,                   MBit::PuskMode | 
                                                                          MBit::WorkMode   },
  { MBit::Adjustment, 0,              SBit::Fault,   MBit::ReadyCheck,    MBit::PuskMode | 
                                                                          MBit::WorkMode   },
  { MBit::PuskMode,   SBit::Ready,    SBit::Fault,   0,                   MBit::WorkMode   },
  { MBit::WorkMode,   SBit::PuskOK,   SBit::Fault,   0,                   MBit::PuskMode   },
  { MBit::FaultCtrl,  0,              SBit::Fault,   MBit::PuskMode | 
                                                     MBit::WorkMode | 
                                                     MBit::Adjustment,    0                }
  }};
};

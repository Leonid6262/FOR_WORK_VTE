#pragma once 

#include "Adjustment.hpp"
#include "FaultCtrlF.hpp"
#include "ReadyCheck.hpp"
#include "PuskMode.hpp"
#include "WorkMode.hpp"
#include "WarningControl.hpp" 
#include "DryingMode.hpp" 
#include "SIFU.hpp"

class CReadyCheck;

class CSystemManager {
  
public:
 CSystemManager(CSIFU&, CAdjustmentMode&, CReadyCheck&, CFaultCtrlF&, 
                CPuskMode&, CWorkMode&, CWarningMode&, CDryingMode&, CRegManager&);
 
 CSIFU& rSIFU;
 CAdjustmentMode& rAdj_mode; 
 CReadyCheck& rReady_check;
 CFaultCtrlF& rFault_ctrl;
 CPuskMode& rPusk_mode;
 CWorkMode& rWork_mode;
 CWarningMode& rWarning_ctrl;
 CDryingMode& rDrying_mode;
 CRegManager& rReg_manager;
 
 // --- Статус системы ---
 union {
   unsigned short all;
   struct {
     unsigned char sReady        : 1; // Готовность собрана
     unsigned char sPuskMotor    : 1; // Режим пуска двигателя
     unsigned char sAdjustment   : 1; // Работа в режиме "Наладка"
     unsigned char sWorkDry      : 1; // Работа в режиме "Сушка"
     unsigned char sWorkTest     : 1; // Работа в режиме "Опробование"
     unsigned char sWorkCur      : 1; // Штатный режим работы с РТ
     unsigned char sWorkQ        : 1; // Штатный режим работы с РQ
     unsigned char sWorkCos      : 1; // Штатный режим работы с РCos
     unsigned char sWarning      : 1; // Есть предупреждения
     unsigned char sFault        : 1; // Авария
   };
 } USystemStatus;
 
   // --- Биты статуса системы --- 
 enum SBit : unsigned short { 
    bsReady      = 1 << 0, // Готовность собрана 
    bsPuskMotor  = 1 << 1, // Режим пуска двигателя
    bsAdjustment = 1 << 2, // Работа в режиме "Наладка"    
    bsWorkDry    = 1 << 3, // Работа в режиме "Сушка"
    bsWorkTest   = 1 << 4, // Работа в режиме "Опробование"
    bsWorkCur    = 1 << 5, // Штатный режим работы с РТ
    bsWorkQ      = 1 << 6, // Штатный режим работы с РQ
    bsWorkCos    = 1 << 7, // Штатный режим работы с РCos
    bsWarning    = 1 << 8, // Есть предупреждения
    bsFault      = 1 << 9  // Авария 
  };
 
 // Установка/сброс битов статуса
 void set_bsReady(State state)        { USystemStatus.sReady      = static_cast<unsigned char>(state); }
 void set_bsPuskMotor(State state)    { USystemStatus.sPuskMotor  = static_cast<unsigned char>(state); }
 void set_bsAdjustmen(State state)    { USystemStatus.sAdjustment = static_cast<unsigned char>(state); } 
 void set_bsWorkDry(State state)      { USystemStatus.sWorkDry    = static_cast<unsigned char>(state); }
 void set_bsWorkTest(State state)     { USystemStatus.sWorkTest   = static_cast<unsigned char>(state); }
 void set_bsWorkCur(State state)      { USystemStatus.sWorkCur    = static_cast<unsigned char>(state); }
 void set_bsWorkQ(State state)        { USystemStatus.sWorkQ      = static_cast<unsigned char>(state); }
 void set_bsWorkCos(State state)      { USystemStatus.sWorkCos    = static_cast<unsigned char>(state); }
 void set_bsWarning(State state)      { USystemStatus.sWarning    = static_cast<unsigned char>(state); }
 void set_bsFault(State state)        { USystemStatus.sFault      = static_cast<unsigned char>(state); }

 // --- Список текущих режимов разрешённых для включения ---
 union UPermissionsList_c{
   unsigned short all;
   struct {
     unsigned char pReadyCheck   : 1;  // Сборка готовности
     unsigned char pAdjustment   : 1;  // Наладка
     unsigned char pPuskMotor    : 1;  // Работа в фазе пуска двигателя    
     unsigned char pWorkDry      : 1;  // Работа в режим "Сушка"
     unsigned char pWorkTest     : 1;  // Работа в режим "Опробование"     
     unsigned char pNormalWork   : 1;  // Работа в штатных режимах     
     unsigned char pFaultCtrlF   : 1;  // Контроль аварий в фоновом режиме
     unsigned char pFaultCtrlP   : 1;  // Контроль аварий в ИУ
   };
 } UPermissionsList;          // Разрешённые режимы
 
   // --- Биты разрешённых режимов --- 
  enum PBit : unsigned short { 

    bpReadyCheck = 1 << 0, // Сборка готовности 
    bpAdjustment = 1 << 1, // Наладка 
    bpPuskMotor  = 1 << 2, // Работа в фазе пуска двигателя
    bpWorkDry    = 1 << 3, // Работа в режим "Сушка"
    bpWorkTest   = 1 << 4, // Работа в режим "Опробование" 
    bpNormalWork = 1 << 5, // Работа в штатных режимах  
    bpFaultCtrlF = 1 << 6, // Контроль аварий в фоновом режиме
    bpFaultCtrlP = 1 << 7  // Контроль аварий в ИУ
      
  };
  
 UPermissionsList_c UPermissionsList_r;        // Запрос на разрешение режима
 
 void set_bpReadyCheck(Mode mode)  { UPermissionsList_r.pReadyCheck = static_cast<unsigned short>(mode); } 
 void set_bpAdjustment(Mode mode)  { UPermissionsList_r.pAdjustment = static_cast<unsigned short>(mode); } 
 void set_bpPuskMotor(Mode mode)   { UPermissionsList_r.pPuskMotor  = static_cast<unsigned short>(mode); }
 void set_bpWorkDry(Mode mode)     { UPermissionsList_r.pWorkDry = static_cast<unsigned short>(mode); }
 void set_bpWorkTest(Mode mode)    { UPermissionsList_r.pWorkTest = static_cast<unsigned short>(mode); }
 void set_bpNormalWork(Mode mode)  { UPermissionsList_r.pNormalWork = static_cast<unsigned short>(mode); }
 void set_bpFaultCtrlF(Mode mode)  { UPermissionsList_r.pFaultCtrlF = static_cast<unsigned short>(mode); }
 void set_bpFaultCtrlP(Mode mode)  { UPermissionsList_r.pFaultCtrlP = static_cast<unsigned short>(mode); }
 
 void dispatch();
 
private:
  

  

  // --- Таблица правил --- 
  struct DependencyRule {
    PBit req_bit;
    unsigned short requiredStatus;   // Какие биты статуса должны быть установлены
    unsigned short forbiddenStatus;  // Какие биты статуса должны быть сброшены
    unsigned short requiredModes;    // Какие режимы должны быть активны
    unsigned short forbiddenModes;   // Какие режимы должны быть отключены
  };

static constexpr std::array<DependencyRule, 6> rules {{
  //    Check Mode      status on      status off       mode on              mode off
  //{ UPermissionsList.pReadyCheck,  0,             SBit::bsFault,   0,                   SBit::bsPuskMotor },                                                                         
  //{ MBit::Adjustment, 0,              SBit::bFault,   MBit::ReadyCheck,    MBit::PuskMode | MBit::WorkMode | MBit::DryMode },                                                                                                                                                  
  //{ MBit::PuskMode,   SBit::bReady,   SBit::bFault,   0,                   MBit::WorkMode | MBit::DryMode  },
  //{ MBit::DryMode,    SBit::bReady,   SBit::bFault,   0,                   MBit::WorkMode | MBit::PuskMode },
  //{ MBit::WorkMode,   0,              SBit::bFault,   0,                   MBit::PuskMode   },
  //{ MBit::FaultCtrlF, 0,              SBit::bFault,   MBit::PuskMode | 
  //                                                    MBit::WorkMode | 
  //                                                    MBit::Adjustment,    0                }
  {PBit::bpReadyCheck,0,0,0,0},
  {PBit::bpReadyCheck,0,0,0,0},
  {PBit::bpReadyCheck,0,0,0,0},
  {PBit::bpReadyCheck,0,0,0,0},
  {PBit::bpReadyCheck,0,0,0,0},
  {PBit::bpReadyCheck,0,0,0,0},
  }};
};

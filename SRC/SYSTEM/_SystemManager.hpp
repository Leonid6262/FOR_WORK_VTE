#pragma once 

#include "Adjustment.hpp"
#include "FaultCtrlF.hpp"
#include "ReadyCheck.hpp"
#include "PuskMode.hpp"
#include "WorkMode.hpp"
#include "WarningControl.hpp" 
#include "DryingMode.hpp" 
#include "TestingMode.hpp"
#include "SIFU.hpp"

class CReadyCheck;

class CSystemManager {
  
public:
 CSystemManager(CSIFU&,        CAdjustmentMode&, CReadyCheck&,  
                CFaultCtrlF&,  CPuskMode&,       CWorkMode&,       
                CWarningMode&, CDryingMode&,     CTestingMode&, 
                CRegManager&);
 
 CSIFU& rSIFU;
 CAdjustmentMode& rAdj_mode; 
 CReadyCheck& rReady_check;
 CFaultCtrlF& rFault_ctrl;
 CPuskMode& rPusk_mode;
 CWorkMode& rWork_normal;
 CWarningMode& rWarning_ctrl;
 CDryingMode& rDrying_mode;
 CRegManager& rReg_manager;
 CTestingMode& rTest_mode;
   
 // --- Статус системы ---
 union {
   unsigned short all;
   struct {
     unsigned char sReady        : 1; // Готовность собрана
     unsigned char sReadyCheck   : 1; // Режим сборки Готовности
     unsigned char sPuskMotor    : 1; // Режим пуска двигателя
     unsigned char sAdjustment   : 1; // Работа в режиме "Наладка"
     unsigned char sWorkDry      : 1; // Работа в режиме "Сушка"
     unsigned char sWorkTest     : 1; // Работа в режиме "Опробование"
     unsigned char sWorkNormal   : 1; // Штатный режим работы
     unsigned char sWarning      : 1; // Есть предупреждения
     unsigned char sFault        : 1; // Авария
   };
 } USystemStatus;
 
   // --- Биты статуса системы --- 
 enum SBit : unsigned short { 
    bsReady      = 1 << 0, // Готовность собрана 
    bsReadyCheck = 1 << 1, // Режим сборки Готовности
    bsPuskMotor  = 1 << 2, // Режим пуска двигателя
    bsAdjustment = 1 << 3, // Работа в режиме "Наладка"    
    bsWorkDry    = 1 << 4, // Работа в режиме "Сушка"
    bsWorkTest   = 1 << 5, // Работа в режиме "Опробование"
    bsWorkNormal = 1 << 6, // Штатный режим работы
    bsWarning    = 1 << 7, // Есть предупреждения
    bsFault      = 1 << 8  // Авария 
  };
 
 // Установка/сброс битов статуса
 void set_bsReady(State state)        { USystemStatus.sReady      = static_cast<unsigned char>(state); }
 void set_bsReadyCheck(State state)   { USystemStatus.sReadyCheck = static_cast<unsigned char>(state); }
 void set_bsPuskMotor(State state)    { USystemStatus.sPuskMotor  = static_cast<unsigned char>(state); }
 void set_bsAdjustmen(State state)    { USystemStatus.sAdjustment = static_cast<unsigned char>(state); } 
 void set_bsWorkDry(State state)      { USystemStatus.sWorkDry    = static_cast<unsigned char>(state); }
 void set_bsWorkTest(State state)     { USystemStatus.sWorkTest   = static_cast<unsigned char>(state); }
 void set_bsWorkNormal(State state)   { USystemStatus.sWorkNormal = static_cast<unsigned char>(state); }
 void set_bsWarning(State state)      { USystemStatus.sWarning    = static_cast<unsigned char>(state); }
 void set_bsFault(State state)        { USystemStatus.sFault      = static_cast<unsigned char>(state); }

 // --- Список текущих режимов разрешённых для включения ---
 union UPermissionsList_c{
   unsigned short all;
   struct {
     unsigned char pReadyCheck   : 1;  // Сборка готовности
     unsigned char pAdjustment   : 1;  // Наладка
     unsigned char pWorkDry      : 1;  // Работа в режим "Сушка"
     unsigned char pWorkTest     : 1;  // Работа в режим "Опробование" 
     unsigned char pPuskMotor    : 1;  // Работа в фазе пуска двигателя        
     unsigned char pNormalWork   : 1;  // Работа в штатных режимах     
     unsigned char pFaultCtrlF   : 1;  // Контроль аварий в фоновом режиме
   };
 } UPermissionsList;          // Разрешённые режимы
 
   // --- Биты разрешённых режимов --- 
  enum class PBit : unsigned short { 

    bpReadyCheck = 1 << 0, // Сборка готовности 
    bpAdjustment = 1 << 1, // Наладка 
    bpWorkDry    = 1 << 2, // Работа в режим "Сушка"
    bpWorkTest   = 1 << 3, // Работа в режим "Опробование" 
    bpPuskMotor  = 1 << 4, // Работа в фазе пуска двигателя    
    bpNormalWork = 1 << 5, // Работа в штатных режимах  
    bpFaultCtrlF = 1 << 6, // Контроль аварий в фоновом режиме
      
  };
 
 void dispatch();
 
private:
  unsigned short AllPermissions;
  
  // --- Таблица правил --- 
  struct DependencyRule {
    PBit req_bit;               // Проверяемый режим
    unsigned short bStatusOn;   // Какие биты статуса должны быть установлены
    unsigned short bStatusOff;  // Какие биты статуса должны быть сброшены
  };

static constexpr DependencyRule rules[] {
/*   Check Permission    bits status on                                 bits status off                        */    
  {PBit::bpReadyCheck,   0,                 bsWorkDry    | bsWorkTest   | bsPuskMotor  | bsWorkNormal | bsFault },
  {PBit::bpAdjustment,   bsReadyCheck,      0                                                                   },
  {PBit::bpWorkDry,      bsReady,           bsWorkTest   | bsPuskMotor  | bsWorkNormal | bsAdjustment | bsFault },
  {PBit::bpWorkTest,     bsReady,           bsPuskMotor  | bsWorkNormal | bsWorkDry    | bsAdjustment | bsFault },
  {PBit::bpPuskMotor,    bsReady,           bsWorkNormal | bsWorkDry    | bsWorkTest   | bsAdjustment | bsFault },
  {PBit::bpNormalWork,   0,                 bsFault                                                             },
  {PBit::bpFaultCtrlF,   0,                 bsReadyCheck | bsFault                                              },
  };
};

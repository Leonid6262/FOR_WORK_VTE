#pragma once
#include <stddef.h>
#include "settings_eep.hpp"

// Контроль эквивалентности MSG и EnumId
template <typename EnumId, size_t N, size_t L>
constexpr void checkMsgSize(const char* const (&)[N][L]) {
    static_assert(N == static_cast<unsigned char>(EnumId::COUNT), "MSG size must equal EnumId::COUNT!");
}
// Шаблон категорий
template <typename EnumId>
struct CategoryActive
{
  inline static bool active[static_cast<unsigned char>(EnumId::COUNT)]{};
  static void setMessage(EnumId id) { active[static_cast<unsigned char>(id)] = true; }
  static void clrMessage(EnumId id) { active[static_cast<unsigned char>(id)] = false; }
};

enum class ECategory { NOT_READY, READY, WORK, FAULT, WARNING, COUNT }; // Список категорий

// ======================= NOT_READY =======================
enum class ENotReadyId { 
  ADJ_MODE, NOT_SYNC, Q1_OFF, SENS_CR_FAULT, SENS_CS_FAULT, 
  SENS_VR_FAULT, BC_HVS_ERR, DRYING_ON, HVS_ON, PK_FAULT, 
  COUNT 
};
struct SNotReady : CategoryActive<ENotReadyId>{
  
  static constexpr const char* NAME[G_CONST::Nlang] = { "НЕТ ГОТОВНОСТ:", "NOT READY:", "НЕМА ГОТОВНОСТI:" };
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"Режим Наладка",    "Adjustment mode", "Режим Наладка"},
    {"Нет синх-ции",     "Not Sync",        "Нема Sync"},
    {"Отключен Q1",      "Q1 is OFF",       "Вимкнено Q1"},
    {"Неисправен ДТР",   "SCR FAULT",       "Несправний ДСР"},
    {"Неисправен ДТС",   "SCS FAULT",       "Несправний ДСС"},
    {"Неисправен ДНР",   "SVR FAULT",       "Несправний ДНР"},
    {"Неиспр-ны БК ВВ",  "BC HVS FAULT",    "Несправнi БК ВВ"},
    {"Включена Сушка",   "Drying is On",    "Увiмк. Сушiння"},
    {"Включен ВВ",       "HVS is On",       "Увiмкнен ВВ"},
    {"Неисправен ПК",    "PK is faulty",    "Несправний ПК"},  
  };
   
  static constexpr auto _checkMsg = (checkMsgSize<ENotReadyId>(MSG), 0);
};

// ======================= READY =======================
enum class EReadyId { PUSK, TESTING, DRYING, COUNT };
struct SReady : CategoryActive<EReadyId>{
  
  static constexpr const char* NAME[G_CONST::Nlang] = { "ГОТОВ", "READY", "ГОТОВИЙ" };
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"К Пуску",         "To Start",             "До Пуску"},
    {"К Опробованию",   "To Testing",           "До Опробування"},
    {"К режиму СУШКА",  "To Drying mode",       "До СУШIННЯ"},
  };      
  static constexpr auto _checkMsg = (checkMsgSize<EReadyId>(MSG), 0);
};
// ======================= WORK =======================
enum class EWorkId { PUSK, CURRENT_REG, COS_REG, Q_POWER_REG, DRYING, TESTING, TESTING_OK, 
                     PUSK_ON_IS, PUSK_ON_SLIP, PUSK_ON_TIMEOUT, CONTROL_PUSK, PUSK_WEX, COUNT };
struct SWork : CategoryActive<EWorkId> {
  
  static constexpr const char* NAME[G_CONST::Nlang] = { "РАБОТА:", "WORK:", "РОБОТА:" };
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"ПУСК",                "PUSK",                 "ПУСК"},    
    {"Регулятор тока",      "Current Reg",          "Регулятор струму"},
    {"Регулятор Cos",       "Cos Reg",              "Регулятор Cos"},
    {"Регулятор Q",         "Q Power Reg",          "Регулятор Q"},
    {"Сушка",               "Drying",               "Сушiння"},
    {"Опробование",         "Testing",              "Опробування"},
    {"Опробование Ok!",     "Testing Ok!",          "Опробування Ok!"},
    {"Пуск по IS",          "Pusk on IS",           "Пуск по IS"},
    {"Пуск по скольж.",     "Pusk on Slip",         "Пуск по ковзанню"},
    {"Пуск по таймауту",    "Pusk on timeout",      "Пуск по таймауту"},    
    {"Контрольный Пуск",    "Control Pusk",         "Контрольний Пуск"},
    {"Пуск без возб-я",     "Pusk without Ex",      "Пуск без збудж-я"},
  };
  static constexpr auto _checkMsg = (checkMsgSize<EWorkId>(MSG), 0);  
};

// ======================= FAULT =======================
enum class EFaultId { ID_MAX_SOFT, ID_MAX_HARD, ID_MIN, NOT_SYNC, Q1_TRIPPED, PK_FAULT, NOT_IS, 
                      LONG_PUSK, PK_NOT_CLOSED, COUNT };
struct SFault : CategoryActive<EFaultId> {
   
  static constexpr const char* NAME[G_CONST::Nlang] = { "АВАРИЯ:", "FAULT:", "АВАРIЯ:" };
  static constexpr const char* MSG[][G_CONST::Nlang] = { 
    {"Id max soft",     "Id max soft",       "Id max soft"},
    {"Id max hard",     "Id max hard",       "Id max hard"},
    {"Id min",          "Id min",            "Id min"},
    {"Нет синх-ции",    "Not Sync",          "Нема Sync"},
    {"Отключился Q1",   "Q1 has tripped",    "Вимкнувся Q1"},
    {"Неисправен ПК",   "PK is faulty",      "Несправний ПК"},
    {"Нет IS",          "Not IS",            "Нема IS"}, 
    {"Затянутый Пуск",  "Prolonged Pusk",    "Тривалий Пуск"},
    {"Не закрыт ПК",    "PK is not closed",  "Не закрився ПК"},    
  };

  static constexpr auto _checkMsg = (checkMsgSize<EFaultId>(MSG), 0);  
};

// ======================= WARNING =======================
enum class EWarningId { DEFAULT_SET, W_RINS1, W_RINS2, PK_NOT_OPEN, PK_NOT_CLOSE, COUNT};
struct SWarning : CategoryActive<EWarningId> {

  static constexpr const char* NAME[G_CONST::Nlang] = { "ПРЕДУПРЕЖДЕНИЕ:", "WARNING:", "ПОПЕРЕДЖЕННЯ:" };  
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"Дефолтные уст-ки", "Default settings", "Дефолтнi уставки"},
    {"Rизол. < Rmin1",   "Rinsul. < Rmin1",  "Riзол. < Rmin1"},
    {"Rизол. < Rmin2",   "Rinsul. < Rmin2",  "Riзол. < Rmin2"},
    {"Не открылся ПК",   "PK Not open",      "Не вiдкрився ПК"},
    {"Не закрылся ПК",   "PK Not close",     "Не закрився ПК"},    
  };
  static constexpr auto _checkMsg = (checkMsgSize<EWarningId>(MSG), 0);
};

// ===== Утилита очистки =====
struct CategoryUtils {
  static void clearMessages(ECategory category) {
    switch (category) {
    case ECategory::NOT_READY:
      for (unsigned char i = 0; i < static_cast<unsigned char>(ENotReadyId::COUNT); ++i)
        SNotReady::active[i] = false;
      break;
    case ECategory::READY:
      for (unsigned char i = 0; i < static_cast<unsigned char>(EReadyId::COUNT); ++i)
        SReady::active[i] = false;
      break;
    case ECategory::WORK:
      for (unsigned char i = 0; i < static_cast<unsigned char>(EWorkId::COUNT); ++i)
        SWork::active[i] = false;
      break;
    case ECategory::FAULT:
      for (unsigned char i = 0; i < static_cast<unsigned char>(EFaultId::COUNT); ++i)
        SFault::active[i] = false;
      break;
    case ECategory::WARNING:
      for (unsigned char i = 0; i < static_cast<unsigned char>(EWarningId::COUNT); ++i)
        SWarning::active[i] = false;
      break;
    case ECategory::COUNT: // глобальная очистка
      clearMessages(ECategory::NOT_READY);
      clearMessages(ECategory::READY);
      clearMessages(ECategory::WORK);
      clearMessages(ECategory::FAULT);
      clearMessages(ECategory::WARNING);
      break;
    }
  }
};



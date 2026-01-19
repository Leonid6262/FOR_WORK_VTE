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
  ADJ_MODE, NOT_SYNC, Q1_is_OFF, SENS_CR_FAULT, SENS_CS_FAULT, SENS_VR_FAULT, BC_HVS_FAUL, COUNT 
};
struct SNotReady : CategoryActive<ENotReadyId>{
  
  static constexpr const char* NAME[G_CONST::Nlang] = { "НЕТ ГОТОВНОСТ:", "NOT READY:", "НЕМА ГОТОВНОСТI:" };
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"Режим Наладки",   "Adjustment mode", "Режим Наладки"},
    {"Нет синх-ции",    "Not Sync",        "Нема Sync"},
    {"Отключен Q1",     "Q1 is OFF",       "Вимкнено Q1"},
    {"Неисправен ДТР",  "SCR FAULT",       "Несправний ДСР"},
    {"Неисправен ДТС",  "SCS FAULT",       "Несправний ДСС"},
    {"Неисправен ДНР",  "SVR FAULT",       "Несправний ДНР"},
    {"Неиспр-ны БК ВВ", "BC HVS FAULT",    "Несправнi БК ВВ"}
  };
  
  
  
  static constexpr auto _checkMsg = (checkMsgSize<ENotReadyId>(MSG), 0);
};

// ======================= READY =======================
enum class EReadyId { PUSK, DRYING, COUNT };
struct SReady : CategoryActive<EReadyId>{
  
  static constexpr const char* NAME[G_CONST::Nlang] = { "ГОТОВ", "READY", "ГОТОВИЙ" };
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"К Пуску",         "To Start",             "До Пуску"},
    {"К режиму СУШКА",  "To Drying mode",       "До СУШIННЯ"},
  };      
  static constexpr auto _checkMsg = (checkMsgSize<EReadyId>(MSG), 0);
};
// ======================= WORK =======================
enum class EWorkId { CURRENT_REG, COS_REG, Q_POWER_REG, DRYING, COUNT };
struct SWork : CategoryActive<EWorkId> {
  
  static constexpr const char* NAME[G_CONST::Nlang] = { "РАБОТА:", "WORK:", "РОБОТА:" };
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"Регулятор тока",      "Current Reg",          "Регулятор струму"},
    {"Регулятор Cos",       "Cos Reg",              "Регулятор Cos"},
    {"Регулятор Q",         "Q Power Reg",          "Регулятор Q"},
    {"Сушка",               "Drying",               "Сушiння"}
  };
  static constexpr auto _checkMsg = (checkMsgSize<EWorkId>(MSG), 0);  
};

// ======================= FAULT =======================
enum class EFaultId { ID_MAX_SOFT, ID_MAX_HARD, ID_MIN, NOT_SYNC, Q1_TRIPPED, COUNT };
struct SFault : CategoryActive<EFaultId> {
   
  static constexpr const char* NAME[G_CONST::Nlang] = { "АВАРИЯ:", "FAULT:", "АВАРIЯ:" };
  static constexpr const char* MSG[][G_CONST::Nlang] = { 
    {"Id max soft",   "Id max soft",       "Id max soft"},
    {"Id max hard",   "Id max hard",       "Id max hard"},
    {"Id min",        "Id min",            "Id min"},
    {"Нет синх-ции",  "Not Sync",          "Нема Sync"},
    {"Отключился Q1", "Q1 has tripped",    "Вимкнувся Q1"},
  };
  


  static constexpr auto _checkMsg = (checkMsgSize<EFaultId>(MSG), 0);  
};

// ======================= WARNING =======================
enum class EWarningId { DEFAULT_SET, W_RINS1, W_RINS2, COUNT};
struct SWarning : CategoryActive<EWarningId> {

  static constexpr const char* NAME[G_CONST::Nlang] = { "ПРЕДУПРЕЖДЕНИЕ:", "WARNING:", "ПОПЕРЕДЖЕННЯ:" };  
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"Дефолтные уст-ки", "Default settings", "Дефолтнi уставки"},
    {"Rизол. < Rmin1",   "Rinsul. < Rmin1",  "Riзол. < Rmin1"},
    {"Rизол. < Rmin2",   "Rinsul. < Rmin2",  "Riзол. < Rmin2"},
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



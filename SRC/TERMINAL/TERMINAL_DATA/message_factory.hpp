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

enum class ECategory { NOT_READY, READY, WORK, FAULT, WARNING, COUNT }; // Спмсок категорий

// ======================= NOT_READY =======================
enum class ENotReadyId { ADJ_MODE, Q1_is_OFF, SENS_CR_FAULT, SENS_CS_FAULT, COUNT };
struct SNotReady : CategoryActive<ENotReadyId>{
  
  static constexpr const char* NAME[G_CONST::Nlang] = { "НЕТ ГОТОВНОСТ:", "NOT READY:", "НЕМА ГОТОВНОСТI:" };
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"Режим Наладки",  "Adjustment mode", "Режим Наладки"},
    {"Отключен Q1",    "Q1 is OFF",       "Вимкнено Q1"},
    {"Неисправен ДТР", "Sens Curr FAULT", "Несправний ДСР"},
    {"Неисправен ДТС", "Stat Curr FAULT", "Несправний ДСС"}
  };
  
  
  
  static constexpr auto _checkMsg = (checkMsgSize<ENotReadyId>(MSG), 0);
};

// ======================= READY =======================
enum class EReadyId { PUSK, DRY, COUNT };
struct SReady : CategoryActive<EReadyId>{
  
  static constexpr const char* NAME[G_CONST::Nlang] = { "ГОТОВ", "READY", "ГОТОВИЙ" };
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"К Пуску",         "To Start",             "До Пуску"},
    {"К режиму СУШКА",  "To Drying mode",       "До режимум СУШКА"},
  };      
  static constexpr auto _checkMsg = (checkMsgSize<EReadyId>(MSG), 0);
};
// ======================= WORK =======================
enum class EWorkId { ADJ_MODE, CURRENT_REG, COS_REG, Q_POWER_REG, COUNT };
struct SWork : CategoryActive<EWorkId> {
  
  static constexpr const char* NAME[G_CONST::Nlang] = { "РАБОТА:", "WORK:", "РОБОТА:" };
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"Режим Наладки",       "Adjustmen mode",       "Режим Наладки"},
    {"Регулятор тока",      "Current Reg",          "Регулятор струму"},
    {"Регулятор Cos",       "Cos Reg",              "Регулятор Cos"},
    {"Регулятор Q",         "Q Power Reg",          "Регулятор Q"}
  };
    


  static constexpr auto _checkMsg = (checkMsgSize<EWorkId>(MSG), 0);  
};

// ======================= FAULT =======================
enum class EFaultId { ID_MAX, COUNT };
struct SFault : CategoryActive<EFaultId> {
   
  static constexpr const char* NAME[G_CONST::Nlang] = { "АВАРИЯ:", "FAULT:", "АВАРIЯ:" };
  static constexpr const char* MSG[][G_CONST::Nlang] = { 
    {"Id max",   "Id max",       "Id max"} 
  };
  


  static constexpr auto _checkMsg = (checkMsgSize<EFaultId>(MSG), 0);  
};

// ======================= WARNING =======================
enum class EWarningId { DEFAULT_SET, COUNT};
struct SWarning : CategoryActive<EWarningId> {

  static constexpr const char* NAME[G_CONST::Nlang] = { "ПРЕДУПРЕЖДЕНИЕ:", "WARNING:", "ПОПЕРЕДЖЕННЯ:" };  
  static constexpr const char* MSG[][G_CONST::Nlang] = {
    {"Дефолтные уст-ки", "Default settings", "Дефолтнi уставки"}
  };
  


  static constexpr auto _checkMsg = (checkMsgSize<EWarningId>(MSG), 0);
};

// ===== Утилита очистки =====
struct CategoryUtils {
  static void clearAllMessages() {
    for (unsigned char i = 0; i < static_cast<unsigned char>(ENotReadyId::COUNT); ++i)
      SNotReady::active[i] = false;    
    for (unsigned char i = 0; i < static_cast<unsigned char>(EWorkId::COUNT); ++i)
      SWork::active[i] = false;    
    for (unsigned char i = 0; i < static_cast<unsigned char>(EFaultId::COUNT); ++i)
      SFault::active[i] = false;    
    for (unsigned char i = 0; i < static_cast<unsigned char>(EWarningId::COUNT); ++i)
      SWarning::active[i] = false;
  }
};



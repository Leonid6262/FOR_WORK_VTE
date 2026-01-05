#pragma once 

#include <array>
#include "bool_name.hpp"
#include "SIFU.hpp"

// Класс управления режимом Наладка
class CAdjustmentMode {
public:
  CAdjustmentMode(CSIFU&);
  void parsing_request(Mode);           // Анализ иипринятие решения по запросу от внешнего источника
  unsigned short req_adj_mode = 0;      // Битовая маска запросов режимов от внешнего источника
  
  // Текущий статус AdjustmentMode
  struct {
    unsigned short prevBits = 0;
    unsigned short currBits = 0;
  } state;
  
private:
  // Битовые флаги режимов AdjustmentMode
  enum EAdjStatus : unsigned short {
    AdjMode    = 1 << 0,
    PulsesF    = 1 << 1,
    PulsesM    = 1 << 2,
    CurrReg    = 1 << 3,
    CurrCycle  = 1 << 4,
    Phase      = 1 << 5
  };
  
  // Правило зависимости: 1-что проверяем, 2-что должно быть включено, 3-что должно быть выключено
  struct DependencyRule {
    unsigned short req_reg;
    unsigned short requiredMask;
    unsigned short forbiddenMask;
  };
  
  static constexpr std::array<DependencyRule, 5> rules {{
    { PulsesF,          0,                              PulsesM                  },
    { PulsesM,          0,                              PulsesF                  },
    { CurrReg,          PulsesF | PulsesM,              Phase                    },
    { CurrCycle,        PulsesF | PulsesM | CurrReg,    Phase                    },
    { Phase,            PulsesF | PulsesM,              CurrReg | CurrCycle      }
  }};
  
  void applyChanges(unsigned short, unsigned short);
  
  CSIFU& rSIFU;
  
  // Функции включения/отключения
  void EnableCurrentReg();
  void DisableCurrentReg();
  void EnableCurrentCycles();
  void DisableCurrentCycles();
};

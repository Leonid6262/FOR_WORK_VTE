#pragma once 

#include <array>
#include "bool_name.hpp"
#include "SIFU.hpp"

// Класс управления режимом Наладка
class CAdjustmentMode {
public:
  CAdjustmentMode(CSIFU&);
  
  void parsing_request(Mode);         // Анализ иипринятие решения по запросу от внешнего источника
  unsigned short reqADJmode = 0;      // Битовая маска запросов режимов от внешнего источника
  
  signed short AlphaAdj = rSIFU.s_const.AMax;

private:
  // Битовые флаги сборки режимов AdjustmentMode
  enum EAdjBits : unsigned short {
    AdjMode    = 1 << 0,
    PulsesF    = 1 << 1,
    PulsesM    = 1 << 2,
    CurrReg    = 1 << 3,
    CurrCycle  = 1 << 4,
    Phase      = 1 << 5,
    NONE       = 0
  };
  
  static constexpr std::array<unsigned short, 5> check_bits = { 
    PulsesF, PulsesM, CurrReg, CurrCycle, Phase 
  };
  
  // Список режимов (разрешённые комбинации EAdjBits)
  enum class EModeAdj : unsigned short {
    None          = EAdjBits::NONE,
    
    ForcingPulses = AdjMode | PulsesF,
    MainPulses    = AdjMode | PulsesM,
    
    CurrentRegF   = AdjMode | PulsesF | CurrReg,
    CurrentRegM   = AdjMode | PulsesM | CurrReg,
    
    CurrentCycleF = AdjMode | PulsesF | CurrReg | CurrCycle,
    CurrentCycleM = AdjMode | PulsesM | CurrReg | CurrCycle,
    
    PhasingF      = AdjMode | PulsesF | Phase,
    PhasingM      = AdjMode | PulsesM | Phase
  };
  
  EModeAdj cur_mode = EModeAdj::None;
  
  // Правило зависимости: 1-что проверяем, 2-что должно быть включено, 3-что должно быть выключено
  struct DependencyRule {
    unsigned short req_reg;
    unsigned short requiredMask;
    unsigned short forbiddenMask;
  };
  
  static constexpr std::array<DependencyRule, 8> rules {{
    { PulsesF,          0,                     PulsesM                 },
    { PulsesM,          0,                     PulsesF                 },   
    { CurrReg,          PulsesF,               Phase                   },
    { CurrReg,          PulsesM,               Phase                   },
    { CurrCycle,        CurrReg | PulsesF,     Phase                   },
    { CurrCycle,        CurrReg | PulsesM,     Phase                   },    
    { Phase,            PulsesF,               CurrReg | CurrCycle     },
    { Phase,            PulsesM,               CurrReg | CurrCycle     }   
  }};
  
  void applyChanges(unsigned short, unsigned short);
  void ex_mode(EModeAdj);
 
  CSIFU& rSIFU;
  
  unsigned short prevBits = 0;
  
};

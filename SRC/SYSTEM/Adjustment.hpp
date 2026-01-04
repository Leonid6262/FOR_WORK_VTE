#pragma once 

#include <cstdint>
#include <array>

// Класс управления режимом Наладка
class CAdjustmentMode {
public:
  CAdjustmentMode();
  void read_request();
  
  unsigned short set_adj_mode = 0;
  
  // Текущий статус AdjustmentMode
  struct CurAdjState {
    unsigned short prevBits = 0;
    unsigned short currBits = 0;
  };
  CurAdjState state;
  
private:
  // Битовые флаги режимов AdjustmentMode
  enum EAdjStatus : unsigned short {
    AdjMode    = 1 << 0,
    SIFU       = 1 << 1,
    CurrReg    = 1 << 2,
    CurrCycle  = 1 << 3,
    Phase      = 1 << 4
  };
  
  // Правило зависимости: что проверяем, что должно быть включено, что выключено
  struct DependencyRule {
    unsigned short req_reg;
    unsigned short requiredMask;
    unsigned short forbiddenMask;
  };
  
  static constexpr std::array<DependencyRule, 4> rules {{
    { SIFU,             AdjMode,                       0                        },
    { CurrReg,          AdjMode | SIFU,                Phase                    },
    { CurrCycle,        AdjMode | SIFU | CurrReg,      Phase                    },
    { Phase,            AdjMode | SIFU,                CurrReg | CurrCycle      }
  }};
  
  unsigned short check_permission(unsigned short);
  void applyChanges(unsigned short, unsigned short);
  
  // Заглушки — реальные функции включения/выключения
  void EnableSIFU();
  void DisableSIFU();
  void EnableCurrentReg();
  void DisableCurrentReg();
  void EnableCurrentCycles();
  void DisableCurrentCycles();
  void EnablePhasing();
  void DisablePhasing();
};

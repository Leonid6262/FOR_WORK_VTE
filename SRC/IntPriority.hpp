#pragma once
#include "LPC407x_8x_177x_8x.h" 

/*
    Такое распределение обусловленно проекцией типовых
    8-ми бит Cortex на 5-ти битный диапазон NXP LPC4078

  NVIC_SetPriorityGrouping(2) -	5 бит  на группу и 0 бит  на подгруппу
  NVIC_SetPriorityGrouping(3) -	4 бита на группу и 1 бит  на подгруппу
  NVIC_SetPriorityGrouping(4) -	3 бита на группу и 2 бита на подгруппу
  NVIC_SetPriorityGrouping(5) -	2 бита на группу и 3 бита на подгруппу <-----
  NVIC_SetPriorityGrouping(6) -	1 бит  на группу и 4 бита на подгруппу
  NVIC_SetPriorityGrouping(7) -	0 бит  на группу и 5 бит  на подгруппу

*/

namespace Priorities {
  constexpr unsigned int G4S8 = 5;          // 4 группы (2 бита) по 8 подгрупп (3 бита)
  constexpr unsigned int SubgroupBits = 3;  // Количество бит подгруппы
  
  enum class EG { group0, group1, group2, group3, group4 };
  enum class ES { sgroup0, sgroup1, sgroup2, sgroup3, sgroup4, sgroup5, sgroup6, sgroup7 };
 
  constexpr unsigned int make_priority(EG group, ES subgroup) {
    return (static_cast<unsigned int>(group) << SubgroupBits) | static_cast<unsigned int>(subgroup);
  }
  
  // --- Группа 0 --- 8 подгрупп (критические и системные прерывания) ---
  constexpr unsigned int INT2 = make_priority(EG::group0, ES::sgroup0);
  
  // ---  Группа 1  --- 8 подгрупп
  constexpr unsigned int Timer3 = make_priority(EG::group1, ES::sgroup0);
  
  // ---  Группа 2  --- 8 подгрупп
  constexpr unsigned int UART = make_priority(EG::group2, ES::sgroup0);
  
  // ---  Группа 3  --- 8 подгрупп
  
  // ---  Группа 4  --- 8 подгрупп
  
  // --- инициализация NVIC --- 
  inline void initPriorities() { 
    
    NVIC_SetPriorityGrouping(G4S8); // распределение по группам 
    
    NVIC_SetPriority(EINT2_IRQn, INT2);     // IdMax hard
    NVIC_SetPriority(TIMER3_IRQn, Timer3);  // СИФУ
    NVIC_SetPriority(UART0_IRQn, UART);     // Терминал
  }
}


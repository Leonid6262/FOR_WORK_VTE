#pragma once

#include "LPC407x_8x_177x_8x.h"

//Пауза испульзует LPC_TIM0 (настройка таймера: 1-тик - 0.1мкс)

#pragma inline = forced
void Pause_us(unsigned int us) 
{
  unsigned int t = SysT::TC();
  while ((SysT::TC() - t) < (us * 10) );
}
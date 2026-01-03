#pragma once

#include "LPC407x_8x_177x_8x.h"

//Пауза испульзует LPC_TIM0 (настройка таймера: 1-тик - 0.1мкс)

#pragma inline = forced
void Pause_us(unsigned int us) 
{
  unsigned int t = LPC_TIM0->TC;
  while ((LPC_TIM0->TC - t) < (us * 10) );
}
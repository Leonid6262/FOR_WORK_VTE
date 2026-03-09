#pragma once
#include "LPC407x_8x_177x_8x.h"

class CCAN_DRIVER {
private:
  LPC_CAN_TypeDef* CAN;
public:
  CCAN_DRIVER(LPC_CAN_TypeDef* CAN) : CAN(CAN) {}
  
};


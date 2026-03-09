#pragma once
#include "LPC407x_8x_177x_8x.h"
#include "spi_init.hpp"

class CSPI_DRIVER {
private:
  LPC_SSP_TypeDef* SSP;
public:
  CSPI_DRIVER(LPC_SSP_TypeDef* SSP) : SSP(SSP) {}
  
__intrinsic void writeByte(unsigned char data)   { SSP->DR = data; }
__intrinsic void writeShort(unsigned short data) { SSP->DR = data; }

__intrinsic unsigned char readByte()   { return SSP->DR; }
__intrinsic unsigned short readShort() { return SSP->DR; }

__intrinsic bool statusBSY() { return (SSP->SR & SPI_Config::SR_BSY); } 
__intrinsic bool statusTNF() { return (SSP->SR & SPI_Config::SR_TNF); }
__intrinsic bool statusRNE() { return (SSP->SR & SPI_Config::SR_RNE); }
  
};


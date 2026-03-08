#pragma once

//#include "LPC407x_8x_177x_8x.h"
//#include "spi_init.hpp"
//#include "system_LPC177x.h"

class CSPI_driver {
private:
    LPC_SSP_TypeDef* SSP;
public:
    CSPI_driver(LPC_SSP_TypeDef* SSP) : SSP(SSP) {}

    void writeByte(unsigned char data) { SSP->DR = data; }
    unsigned char readByte() { return SSP->DR; }
    bool statusBSY() { return (SSP->SR & SPI_Config::SR_BSY); } 
    bool statusTNF() { return (SSP->SR & SPI_Config::SR_TNF); }
    bool statusRNE() { return (SSP->SR & SPI_Config::SR_RNE); }

};


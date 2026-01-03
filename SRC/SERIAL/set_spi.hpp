#pragma once

#include "LPC407x_8x_177x_8x.h"
#include "spi_init.hpp"
#include "system_LPC177x.h"

class CSET_SPI {
 public:
  enum class ESPIInstance {
    SPI_0,
    SPI_1,
    SPI_2
  };

  static LPC_SSP_TypeDef* configure(ESPIInstance);

 private:
  static constexpr unsigned int Hz_SPI0 = 800000;
  static constexpr unsigned int Hz_SPI1 = 10000000;
  static constexpr unsigned int Hz_SPI2 = 1000000;

  static constexpr unsigned int IOCON_SPI0 = 0x02;
  static constexpr unsigned int IOCON_SPI1 = 0x03;
  static constexpr unsigned int IOCON_SPI2 = 0x02;
  static constexpr unsigned int D_MODE_PULLUP = 0x02 << 3;

  static constexpr unsigned int RXDMAE = 1UL << 0;
  static constexpr unsigned int TXDMAE = 1UL << 1;

  LPC_SSP_TypeDef* SSP;
};

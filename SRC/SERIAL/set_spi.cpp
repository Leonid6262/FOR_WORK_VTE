#include "set_spi.hpp"
#include "Peripherals.hpp"

LPC_SSP_TypeDef* CSET_SPI::config(ESPIInstance SN) {
  LPC_SSP_TypeDef* SSP;

  switch (SN) {
    case ESPIInstance::SPI_0:
      SSP = P::SPI0;
      SSP->CR0 = 0;
      SSP->CR0 = 8 - 1;  // (8 - 1) -> 8 bits
      SSP->CR1 = 0;
      SPI_Config::set_spi_clock(SSP, Hz_SPI0, PeripheralClock);
      SSP->CR1 |= SPI_Config::CR1_SSP_EN;
      break;
    case ESPIInstance::SPI_1:
      SSP = P::SPI1;
      SSP->CR0 = 0;
      SSP->CR0 = 16 - 1;  // (16 - 1) -> 16 bits
      SSP->CR1 = 0;
      SPI_Config::set_spi_clock(SSP, Hz_SPI1, PeripheralClock);
      SSP->CR1 |= SPI_Config::CR1_SSP_EN;
      break;
    case ESPIInstance::SPI_2:
      SSP = P::SPI2;
      SSP->CR0 = 0;
      SSP->CR0 = 16 - 1;  // (16 - 1) -> 16 bits
      SSP->CR1 = 0;
      SPI_Config::set_spi_clock(SSP, Hz_SPI2, PeripheralClock);
      SSP->CR1 |= SPI_Config::CR1_SSP_EN;

      SSP->DMACR = TXDMAE | RXDMAE;  // Приём и передача по DMA
      break;
  }
  return SSP;
}

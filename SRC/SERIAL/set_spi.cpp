#include "set_spi.hpp"

LPC_SSP_TypeDef* CSET_SPI::config(ESPIInstance SN) {
  LPC_SSP_TypeDef* SSP = nullptr;

  switch (SN) {
    case ESPIInstance::SPI_0:
      SSP = LPC_SSP0;
      LPC_IOCON->P0_15 = IOCON_SPI0;         // SCK0
                                             // SSEL0 - не проведен, не используется
      LPC_IOCON->P0_17 = IOCON_SPI0;         // MISO0
      LPC_IOCON->P0_18 = IOCON_SPI0;         // MOSI0
      LPC_SC->PCONP |= CLKPWR_PCONP_PCSSP0;  // Включение питания SPI0
      SSP->CR0 = 0;
      SSP->CR0 = 8 - 1;  // (8 - 1) -> 8 bits
      SSP->CR1 = 0;
      SPI_Config::set_spi_clock(SSP, Hz_SPI0, PeripheralClock);
      SSP->CR1 |= SPI_Config::CR1_SSP_EN;

      break;
    case ESPIInstance::SPI_1:
      SSP = LPC_SSP1;
      LPC_IOCON->P4_20 = D_MODE_PULLUP | IOCON_SPI1;  // SCK1
      LPC_IOCON->P4_21 = D_MODE_PULLUP | IOCON_SPI1;  // SSEL1
      LPC_IOCON->P4_22 = D_MODE_PULLUP | IOCON_SPI1;  // MISO1
      LPC_IOCON->P4_23 = D_MODE_PULLUP | IOCON_SPI1;  // MOSI1
      LPC_SC->PCONP |= CLKPWR_PCONP_PCSSP1;           // Включение питания SPI1
      SSP->CR0 = 0;
      SSP->CR0 = 16 - 1;  // (16 - 1) -> 16 bits
      SSP->CR1 = 0;
      SPI_Config::set_spi_clock(SSP, Hz_SPI1, PeripheralClock);
      SSP->CR1 |= SPI_Config::CR1_SSP_EN;
      break;
    case ESPIInstance::SPI_2:
      SSP = LPC_SSP2;
      LPC_IOCON->P5_2 = IOCON_SPI2;          // SCK2
                                             // SSEL2 - не проведен, не используется
      LPC_IOCON->P5_1 = IOCON_SPI2;          // MISO2
      LPC_IOCON->P5_0 = IOCON_SPI2;          // MOSI2
      LPC_SC->PCONP |= CLKPWR_PCONP_PCSSP2;  // Включение питания SPI2
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

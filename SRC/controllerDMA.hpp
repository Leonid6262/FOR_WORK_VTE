#pragma once

#include "lpc177x_8x_clkpwr.h"
#include "lpc407x_8x_177x_8x.h"

class CDMAcontroller {
 public:
  CDMAcontroller();

  // Карта привязки каналов
  enum class ChannelMap : unsigned char { 
    Ch0 = 0, 
    Ch1 = 1, 
    Ch2 = 2, 
    Ch3 = 3, 
    Ch4 = 4, 
    Ch5 = 5, 
    SPI2_Tx_Channel = 6, 
    SPI2_Rx_Channel = 7 
  };

  // Типы каналов
  enum class ETransferType : unsigned char {
    TYPE_M2P = 1,  // Memory to Peripheral
    TYPE_P2M = 2,  // Peripheral to Memory
  };

  // Размер элемента
  enum class EWidth : unsigned char { BYTE = 0, SHORT = 1, LONG = 2 };

  // Размер единичной передачи
  enum class DmaBurst : unsigned char {
    SIZE_1 = 0,
    SIZE_4 = 1,  // Оптимум для SPI
    SIZE_8 = 2,
    SIZE_16 = 3,
    SIZE_32 = 4,
    SIZE_64 = 5,
    SIZE_128 = 6,
    SIZE_256 = 7
  };

  // Номер подключения
  enum class EConnNumber : unsigned int {
    MCI = 1,
    SSP0_Tx = 2,
    SSP0_Rx = 3,
    SSP1_Tx = 4,
    SSP1_Rx = 5,
    SSP2_Tx = 6,
    SSP2_Rx = 7,
    ADC = 8,
    DAC = 9,
    UART0_Tx = 10,
    UART0_Rx = 11,
    UART1_Tx = 12,
    UART1_Rx = 13,
    UART2_Tx = 14,
    UART2_Rx = 15,
    /*~~~~~~~~~~~~~~~~~~~~~~~
    MAT0_0     = 16,
    MAT0_1     = 17,
    MAT1_0     = 18,
    MAT1_1     = 19,
    MAT2_0     = 20,
    MAT2_1     = 21,
    I2S_Channel_0 = 22,
    I2S_Channel_1 = 23,
    UART3_Tx   = 26,
    UART3_Rx   = 27,
    UART4_Tx   = 28,
    UART4_Rx   = 29,
    MAT3_0     = 30,
    MAT3_1     = 31
      ~~~~~~~~~~~~~~~~~~~~*/
  };

  // Конфигурация канала
  struct SChannelConfig {
    ChannelMap channelNum;       // Номер канала
    ETransferType transferType;  // Тип канала
    EConnNumber Conn;            // Номер периферийного подключения
    DmaBurst BurstSize;          // Количество единичных элементов транзакции
    EWidth Width;                // Размер единичного элемента
    bool enableInterrupt;        // Разрешение/запрет события окончания передачи (ON/OFF)
  };

  void init_M2P2M_Channel(const SChannelConfig*);

  void StartTxTransfer(ChannelMap, unsigned int, unsigned int);
  void StartRxTransfer(ChannelMap, unsigned int, unsigned int);

  void enableIrq();
  void disableIrq();

 private:
  static constexpr unsigned char MaxChannels = 8;
  static constexpr unsigned int TransferSize_MASK = 0x00000FFF;

  static constexpr LPC_GPDMACH_TypeDef* const DmaChannels[MaxChannels] = {LPC_GPDMACH0, LPC_GPDMACH1, LPC_GPDMACH2, LPC_GPDMACH3,
                                                                          LPC_GPDMACH4, LPC_GPDMACH5, LPC_GPDMACH6, LPC_GPDMACH7};

  static constexpr const volatile void* LUTPerAddr[] = {
      nullptr,          // Reserved
      &LPC_MCI->FIFO,   // SD Card
      &LPC_SSP0->DR,    // SSP0 Tx
      &LPC_SSP0->DR,    // SSP0 Rx
      &LPC_SSP1->DR,    // SSP1 Tx
      &LPC_SSP1->DR,    // SSP1 Rx
      &LPC_SSP2->DR,    // SSP2 Tx
      &LPC_SSP2->DR,    // SSP2 Rx
      &LPC_ADC->GDR,    // ADC
      &LPC_DAC->CNR,    // DAC
      &LPC_UART0->THR,  // UART0 Tx
      &LPC_UART0->RBR,  // UART0 Rx
      &LPC_UART1->THR,  // UART1 Tx
      &LPC_UART1->RBR,  // UART1 Rx
      &LPC_UART2->THR,  // UART2 Tx
      &LPC_UART2->RBR,  // UART2 Rx
      /*~~~~~~~~~~~~~~~~~~~~~~~
      &LPC_TIM0->MR0,                     // MAT0.0
      &LPC_TIM0->MR1,                     // MAT0.1
      &LPC_TIM1->MR0,                     // MAT1.0
      &LPC_TIM1->MR1,                     // MAT1.1
      &LPC_TIM2->MR0,                     // MAT2.0
      &LPC_TIM2->MR1,                     // MAT2.1
      &LPC_I2S->TXFIFO,                   // I2S Tx
      &LPC_I2S->RXFIFO,                   // I2S Rx
      nullptr,                            // Reserved
      nullptr,                            // Reserved
      &LPC_UART3->THR,                    // UART3 Tx
      &LPC_UART3->RBR,                    // UART3 Rx
      &LPC_UART4->THR,                    // UART4 Tx
      &LPC_UART4->RBR,                    // UART4 Rx
      &LPC_TIM3->MR0,                     // MAT3.0
      &LPC_TIM3->MR1                      // MAT3.1
                              ~~~~~~~~~~~~~~~~~~~~*/
  };
  static constexpr unsigned char bSBSize = 12;
  static constexpr unsigned char bDBSize = 15;
  static constexpr unsigned char bSWidth = 18;
  static constexpr unsigned char bDWidth = 21;
  static constexpr unsigned int bSI = 1UL << 26;
  static constexpr unsigned int bDI = 1UL << 27;

  static constexpr unsigned int DMACConfig_E = 0x01;
  static constexpr unsigned int DMACCxConfig_E = 1UL << 0;
  static constexpr unsigned int DMACCxConfig_IE = 1UL << 14;
  static constexpr unsigned int DMACCxControl_I = 1UL << 31;
  static constexpr unsigned int DMACCxConfig_ITC = 1UL << 15;

  static constexpr unsigned int bTransferType = 11;
  static constexpr unsigned int bSrcPeripheral = 1;
  static constexpr unsigned int bDestPeripheral = 6;
};

#include "controllerDMA.hpp"

#include "IntPriority.hpp"

CDMAcontroller::CDMAcontroller() {
  LPC_SC->PCONP |= CLKPWR_PCONP_PCGPDMA;
  for (unsigned char i = 0; i < MaxChannels; ++i) {
    DmaChannels[i]->CConfig = 0;
  }
  LPC_GPDMA->IntTCClear = 0xFF;
  LPC_GPDMA->IntErrClr = 0xFF;
}

/*
  Для каждого запуска транзакции, необходимо повторное задание указателя и длины.
*/
void CDMAcontroller::StartTxTransfer(ChannelMap ch, unsigned int SrcAddr, unsigned int length) {
  DmaChannels[static_cast<unsigned char>(ch)]->CSrcAddr = static_cast<unsigned int>((long long)SrcAddr);
  DmaChannels[static_cast<unsigned char>(ch)]->CControl &= ~TransferSize_MASK;
  DmaChannels[static_cast<unsigned char>(ch)]->CControl |= (length & TransferSize_MASK);
  DmaChannels[static_cast<unsigned char>(ch)]->CConfig |= DMACCxConfig_E;
}

void CDMAcontroller::StartRxTransfer(ChannelMap ch, unsigned int DestAddr, unsigned int length) {
  DmaChannels[static_cast<unsigned char>(ch)]->CDestAddr = static_cast<unsigned int>((long long)DestAddr);
  DmaChannels[static_cast<unsigned char>(ch)]->CControl &= ~TransferSize_MASK;
  DmaChannels[static_cast<unsigned char>(ch)]->CControl |= (length & TransferSize_MASK);
  DmaChannels[static_cast<unsigned char>(ch)]->CConfig |= DMACCxConfig_E;
}

// Реализована инициализация каналов M2P(M2P_DEST_CTRL) и P2M(P2M_SRC_CTRL) типов
void CDMAcontroller::init_M2P2M_Channel(const SChannelConfig* cfg) {
  // Указатель на структуру канала указанного в cfg
  LPC_GPDMACH_TypeDef* ch = DmaChannels[static_cast<unsigned char>(cfg->channelNum)];

  // В любом случае сбрасывается Interrupt status
  LPC_GPDMA->IntTCClear = (1UL << static_cast<unsigned char>(cfg->channelNum));
  LPC_GPDMA->IntErrClr = (1UL << static_cast<unsigned char>(cfg->channelNum));

  ch->CControl = 0;
  ch->CConfig = 0;
  ch->CLLI = 0;  // Механизм Linked List Item (LLI) не используется

  ch->CControl = ((static_cast<unsigned int>(cfg->BurstSize) & 0x07) << bSBSize) |
                 ((static_cast<unsigned int>(cfg->BurstSize) & 0x07) << bDBSize) |
                 ((static_cast<unsigned int>(cfg->Width) & 0x07) << bSWidth) |
                 ((static_cast<unsigned int>(cfg->Width) & 0x07) << bDWidth);

  switch (cfg->transferType) {
    case ETransferType::TYPE_M2P:
      // Адрес перифери (адрес регистра назначения)
      ch->CDestAddr = static_cast<unsigned int>((long long)(LUTPerAddr[static_cast<unsigned int>(cfg->Conn)]));
      ch->CControl |= bSI;  // Control_SI

      /* Enable DMA channels, little endian */
      LPC_GPDMA->Config = DMACConfig_E;
      while (!(LPC_GPDMA->Config & DMACConfig_E)) {
      };

      ch->CConfig = ((static_cast<unsigned int>(cfg->transferType) & 0x07) << bTransferType) |
                    ((static_cast<unsigned int>(cfg->Conn) & 0x1F) << bDestPeripheral);
      break;
    case ETransferType::TYPE_P2M:
      // Адрес перифери (адрес регистра источника)
      ch->CSrcAddr = static_cast<unsigned int>((long long)(LUTPerAddr[static_cast<unsigned int>(cfg->Conn)]));
      ch->CControl |= bDI;  // Control_DI

      /* Enable DMA channels, little endian */
      LPC_GPDMA->Config = DMACConfig_E;
      while (!(LPC_GPDMA->Config & DMACConfig_E)) {
      };

      ch->CConfig = ((static_cast<unsigned int>(cfg->transferType) & 0x07) << bTransferType) |
                    ((static_cast<unsigned int>(cfg->Conn) & 0x1F) << bSrcPeripheral);
      break;
  }
  // Прерывания по ошибкам (IE) не используется. Если события окончания передачи разрешены,
  // для обработки прерываний от DMA, разумеется, нужно вызвать  enableIrq() {NVIC_EnableIRQ(DMA_IRQn);}
  ch->CConfig &= ~DMACCxConfig_IE;
  if (cfg->enableInterrupt) {
    ch->CControl |= DMACCxControl_I;
    ch->CConfig |= DMACCxConfig_ITC;
  }
}

void CDMAcontroller::enableIrq() { NVIC_EnableIRQ(DMA_IRQn); }

void CDMAcontroller::disableIrq() { NVIC_DisableIRQ(DMA_IRQn); }

#include "emac.hpp"

#include <stdio.h>

CEMAC_DRV::CEMAC_DRV() {
  // Настройка RMII пинов
  LPC_IOCON->P1_0 = FUNC_ENET;                  // ENET_TXD0
  LPC_IOCON->P1_1 = FUNC_ENET;                  // ENET_TXD1
  LPC_IOCON->P1_4 |= FUNC_ENET | MODE_PULLUP;   // ENET_TX_EN
  LPC_IOCON->P1_8 |= FUNC_ENET | MODE_PULLUP;   // ENET_CRS_DV
  LPC_IOCON->P1_9 = FUNC_ENET;                  // ENET_RXD0
  LPC_IOCON->P1_10 = FUNC_ENET;                 // ENET_RXD1
  LPC_IOCON->P1_14 |= FUNC_ENET | MODE_PULLUP;  // RXER
  LPC_IOCON->P1_15 |= FUNC_ENET | MODE_PULLUP;  // RXCK
  LPC_IOCON->P1_16 |= FUNC_ENET | MODE_PULLUP;  // MDC
  LPC_IOCON->P1_17 |= FUNC_ENET | MODE_PULLUP;  // MDIO
}

// MAC адрес контроллера
const unsigned char CEMAC_DRV::MAC_Controller[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

// ---Передача кадра---
void CEMAC_DRV::sendFrame(const unsigned char* data) {
  short dl = ETH_FRAG_SIZE - 4;                           // Не обязательно все 60 байт. Можно только кол. данных.
                                                          // При вкл. PAD, EMAC сам добавит нули до конца кадра.
  unsigned int idx = LPC_EMAC->TxProduceIndex;            // Получаем текущий индекс для передачи
  memcpy(reinterpret_cast<void*>(txBuf(idx)), data, dl);  // Копируем данные в соответствующий передающий буфер

  txDescPacket(idx) = txBuf(idx);  // Устанавливаем адрес буфера и длину кадра в дескриптор
  txDescCtrl(idx) &= ~DESC_RX_SIZE;
  txDescCtrl(idx) |= (dl - 1);
  idx = (idx + 1) % NUM_TX_FRAG;
  LPC_EMAC->TxProduceIndex = idx;  // Запуск передачи
}

// ---Приём кадра---
CEMAC_DRV::ReceiveStatus CEMAC_DRV::receiveFrame(unsigned char* buffer) {
  idx_Cons = LPC_EMAC->RxConsumeIndex;
  idx_Prod = LPC_EMAC->RxProduceIndex;

  if (idx_Cons == idx_Prod) {
    return ReceiveStatus::EMPTY;  // Нет непринятого кадра
  }

  LPC_EMAC->RxConsumeIndex = (idx_Cons + 1) % NUM_RX_FRAG;  // Продвинуть ConsumeIndex
  status_rx = rxStatInfo(idx_Cons);

  if (status_rx & ~RX_CTRL_ERR_BITS) {
    return ReceiveStatus::ERROR;  // Кадр принят с ошибками
  }

  memcpy(buffer, reinterpret_cast<void*>(rxBuf(idx_Cons)), ETH_FRAG_SIZE);  // Скопировать кадр (весь)

  return ReceiveStatus::FRAME_RECIVED;  // Кадр принят
}

StatusRet CEMAC_DRV::writePHY(unsigned char reg, unsigned short value) {
  // Запись регистра PHY через MDIO
  LPC_EMAC->MADR = (PHY_ADDR << PHYADDR) | reg;
  LPC_EMAC->MWTD = value;
  for (int i = 0; i < 1000; i++) {
    if (!(LPC_EMAC->MIND & BUSY)) {
      return StatusRet::SUCCESS;
    }
  }
  return StatusRet::ERROR;
}

StatusRet CEMAC_DRV::readPHY(unsigned char reg, unsigned short& value) {
  // Чтение регистра PHY через MDIO
  LPC_EMAC->MADR = (PHY_ADDR << PHYADDR) | reg;
  LPC_EMAC->MCMD = READ;  // Команда чтения

  for (int i = 0; i < 1000; ++i) {
    if (!(LPC_EMAC->MIND & BUSY)) {
      LPC_EMAC->MCMD = 0;
      value = LPC_EMAC->MRDD;
      return StatusRet::SUCCESS;
    }
  }
  LPC_EMAC->MCMD = 0;
  return StatusRet::ERROR;
}

// Инициализация PHY
StatusRet CEMAC_DRV::initPHY() {
  LPC_EMAC->SUPP = 0;                                      // 10 Мбит/с.
  if (writePHY(0x00, PHY_BMCR_RESET) == StatusRet::ERROR)  // Сброс PHY через BMCR (регистр 0, бит 15)
  {
    return StatusRet::ERROR;
  }
  Pause_us(10000);

  LPC_EMAC->SUPP = SUPP_SPEED;  // 100 Мбит/с.
  // Проверка "link up". Мак. время ожидания 40*100мс = 4сек.
  link_up = false;
  unsigned short reg1 = 0;
  unsigned short reg2 = 0;
  for (int i = 0; i < 40; ++i) {
    Pause_us(100000);
    if (static_cast<bool>(readPHY(PHY_REG_BMSR, reg1)) && static_cast<bool>(readPHY(PHY_REG_BMSR, reg2))) {
      if (reg2 & PHY_BMSR_LINK) {
        link_up = true;
        break;
      }
    }
  }
  // Проверка что установлена LAN8720
  ID_LAN8720 = false;
  if ((readPHY(PHY_REG_IDR1, reg1) == StatusRet::SUCCESS) && readPHY(PHY_REG_IDR2, reg2) == StatusRet::SUCCESS) {
    if (((reg1 << PHY_IDR1_SHIFT) | (reg2 & PHY_IDR2_MASK)) == LAN8720_ID) {
      ID_LAN8720 = true;
      return StatusRet::SUCCESS;
    }
  }
  return StatusRet::ERROR;
}

// Инициализация EMAC
StatusRet CEMAC_DRV::initEMAC() {
  LPC_SC->PCONP |= CLKPWR_PCONP_PCENET;  // Включить питание, Настроить MDC частоту для MDIO (максимум 2.5 МГц)
  LPC_EMAC->MCFG = MCFG_RES_MII | MCFG_CLK_DIV48;  // сброс MII-логики
  LPC_EMAC->MCFG &= ~MCFG_RES_MII;

  LPC_EMAC->MAC1 = MAC1_RES_TX | MAC1_RES_RX | MAC1_RES_MCS_TX | MAC1_RES_MCS_RX | MAC1_SIM_RES | MAC1_SOFT_RES;
  LPC_EMAC->MAC1 = MAC1_PASS_ALL | MAC1_RXENABLE;

  LPC_EMAC->Command = CR_REGRESET | CR_TXRESET | CR_RXRESET | CR_PASSRUNTFRAME;
  Pause_us(250);

  if (initPHY() == StatusRet::ERROR)  // Инициализация PHY
  {
    // Если PHY проинициализировать не удалось, всё остальное бессмысленно
    EMAC_Ready = false;
    return StatusRet::ERROR;
  }
  LPC_EMAC->MAC2 |= MAC2_FULL_DUP | MAC2_CRC_EN | MAC2_PAD_EN;
  LPC_EMAC->Command |= CR_FULLDUPLEX;

  LPC_EMAC->IPGT = IPGT_FULL_DUP;

  LPC_EMAC->MAXF = ETH_MAX_FLEN;
  LPC_EMAC->CLRT = CLRT_DEF;
  LPC_EMAC->IPGR = IPGR_DEF;

  LPC_EMAC->MCFG = MCFG_CLK_DIV48 | MCFG_RES_MII;
  Pause_us(250);

  LPC_EMAC->Command = CR_RMII | CR_PASSRUNTFRAME;

  // Установка MAC адресов устройства (в обратном порядке!)
  LPC_EMAC->SA2 = (MAC_Controller[1] << 8) | MAC_Controller[0];  // байт 0 и 1
  LPC_EMAC->SA1 = (MAC_Controller[3] << 8) | MAC_Controller[2];  // байт 2 и 3
  LPC_EMAC->SA0 = (MAC_Controller[5] << 8) | MAC_Controller[4];  // байт 4 и 5

  initDescriptors();  // Настройка дескрипторов

  // Включить фильтрацию пакетов по MAC
  LPC_EMAC->RxFilterCtrl = 0;
  LPC_EMAC->RxFilterCtrl = RFC_PERFECT_EN;
  LPC_EMAC->Command &= ~CR_PASSRXFILTER;

  // Разрешить приём/передачу
  LPC_EMAC->Command |= CR_RXENABLE | CR_TXENABLE | CR_FULLDUPLEX;

  EMAC_Ready = true;
  return StatusRet::SUCCESS;
}

void CEMAC_DRV::initDescriptors() {
  for (int i = 0; i < NUM_RX_FRAG; ++i) {
    rxDescPacket(i) = rxBuf(i);         // Адрес приемного буфера
    rxDescCtrl(i) = ETH_FRAG_SIZE - 1;  // Длина кадра

    rxStatInfo(i) = 0;
    rxStatHashCrc(i) = 0;
  }

  for (int i = 0; i < NUM_TX_FRAG; ++i) {
    txDescPacket(i) = txBuf(i);  // Адрес буфера передачи
    txDescCtrl(i) = 0;
    txDescCtrl(i) = TX_CTRL_DEFAULT;  // Управляющие биты
    txStatInfo(i) = 0;
  }

  // Установка указателей EMAC на области дескрипторов
  LPC_EMAC->RxDescriptor = RX_DESC_BASE;
  LPC_EMAC->RxStatus = RX_STAT_BASE;
  LPC_EMAC->RxConsumeIndex = 0;
  LPC_EMAC->RxDescriptorNumber = NUM_RX_FRAG - 1;

  LPC_EMAC->TxDescriptor = TX_DESC_BASE;
  LPC_EMAC->TxStatus = TX_STAT_BASE;
  LPC_EMAC->TxProduceIndex = 0;
  LPC_EMAC->TxDescriptorNumber = NUM_TX_FRAG - 1;
}

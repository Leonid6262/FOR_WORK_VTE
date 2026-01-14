#include "rem_osc.hpp"

#include <stdio.h>

#include "AdcStorage.hpp"
#include "pause_us.hpp"
#include "proxy_pointer_var.hpp"
#include "settings_eep.hpp"
#include "system_LPC177x.h"

/* Буферы передачи/приёма по DMA класса CREM_OSC */
#pragma location = ".dma_buffers"
__root signed short CREM_OSC::tx_dma_buffer[CREM_OSC::TRANSACTION_LENGTH];
#pragma location = ".dma_buffers"
__root signed short CREM_OSC::rx_dma_buffer[CREM_OSC::TRANSACTION_LENGTH];

CREM_OSC::CREM_OSC(CDMAcontroller& rContDMA, CPULSCALC& rPulsCalc, CADC_STORAGE& adc_instans) : rContDMA(rContDMA) {
  
  // Структура инициализирующих значений CREM_OSC. Дистанционный осцилограф (ESP32 c WiFi модулем)
  set_init = {
    // Указатели на отображаемые переменные (до 10-ти переменных).
    {
      adc_instans.getEPointer(CADC_STORAGE::ROTOR_CURRENT),    // Ток ротора
      rPulsCalc.getPointer_ISTATORA(),                         // Полный ток статора
      rPulsCalc.getPointer_USTATORA(),                         // Напряжение статора
      adc_instans.getEPointer(CADC_STORAGE::ROTOR_VOLTAGE)     // Напряжение ротора
    },
    {
      "IROT", "ISTAT", "USTAT", "U_ROT" // Имена треков (как будут подписаны треки в ПО ПК)
    }, 
    {
      // Уставки коэффициентов отображения (дискрет на 100%)
      cd::ADC_DISCR_ID,
      cd::ADC_DISCR_IS,
      cd::ADC_DISCR_US,
      cd::ADC_DISCR_UD,               
      // Количество треков определяется по размеру этого массива (d_100p)
    },
    CREM_OSC::Operating_mode::Access_point,                   // Режим работы Access_point или Station
    CEEPSettings::getInstance().getSettings().SNboard_number, // Серийный номер платы контроллера (задаётся для режима Access_point)
    // Имя сети и пароль (задаются для режима Station)
    /* Как задавать ssid и password пока не ясно. При отсутствии панели оператора
    и сетевых интерфейсов, возможно с ноутбука. Задавать с ПТ удовольствие так себе.*/
    CEEPSettings::getInstance().getSettings().ssid,
    CEEPSettings::getInstance().getSettings().password};
  init_dma();
  number_actual_tracks = get_actual_number();
  transfer_disp_c();
  transfer_name();
  transfer_mode();
}

// Старт передачи по DMA
void CREM_OSC::start_dma_transfer() {
  // Старт чтения
  rContDMA.StartRxTransfer(CDMAcontroller::ChannelMap::SPI2_Rx_Channel,
                           static_cast<unsigned int>((long long)rx_dma_buffer), TRANSACTION_LENGTH);
  // Старт записи
  rContDMA.StartTxTransfer(CDMAcontroller::ChannelMap::SPI2_Tx_Channel,
                           static_cast<unsigned int>((long long)tx_dma_buffer), TRANSACTION_LENGTH);
}

/* Конфигурирование используемых каналов DMA (карту каналов с.м. в controllerDMA.hpp) */
/* BurstSize = 4. При EWidth::Short → 16 бит → 4 × 2 байта = 8 байт, при FIFO = 16 байт, с запасом */
void CREM_OSC::init_dma() {
  // Конфигурация канала записи
  CDMAcontroller::SChannelConfig cfg_ch_tx{
    CDMAcontroller::ChannelMap::SPI2_Tx_Channel,  // Номер канала
    CDMAcontroller::ETransferType::TYPE_M2P,      // Тип канала
    CDMAcontroller::EConnNumber::SSP2_Tx,         // Номер периферийного подключения
    CDMAcontroller::DmaBurst::SIZE_4,             // Количество единичных элементов транзакции
    CDMAcontroller::EWidth::SHORT,                // Размер единичного элемента
    static_cast<bool>(State::OFF)            // Разрешение/запрет события окончания передачи (ON/OFF)
  };

  rContDMA.init_M2P2M_Channel(&cfg_ch_tx);

  // Конфигурация канала чтения
  CDMAcontroller::SChannelConfig cfg_ch_rx{
      CDMAcontroller::ChannelMap::SPI2_Rx_Channel,  // Номер канала
      CDMAcontroller::ETransferType::TYPE_P2M,      // Тип канала
      CDMAcontroller::EConnNumber::SSP2_Rx,         // Номер периферийного подключения
      CDMAcontroller::DmaBurst::SIZE_4,             // Количество единичных элементов транзакции
      CDMAcontroller::EWidth::SHORT,                // Размер единичного элемента
      static_cast<bool>(State::OFF)            // Разрешение/запрет события окончания приёма (ON/OFF)
  };

  rContDMA.init_M2P2M_Channel(&cfg_ch_rx);
}

// Передача данных треков
void CREM_OSC::send_data() {
  if (!StatusESP32) return;

  /*  ip_ap_sta[0] = rx_dma_buffer[1];
    ip_ap_sta[1] = rx_dma_buffer[2];
    ip_ap_sta[2] = rx_dma_buffer[3];
    ip_ap_sta[3] = rx_dma_buffer[4];*/

  memset(rx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
  tx_dma_buffer[0] = send_TRACKS;
  for (unsigned char n_data = 1; n_data < number_actual_tracks + 1; n_data++) {
    tx_dma_buffer[n_data] = *set_init.pData[n_data - 1];
  }
  start_dma_transfer();
}

// Получение актуального количества треков
unsigned char CREM_OSC::get_actual_number() {
  unsigned char count = 0;
  while ((count < NUMBER_TRACKS) && set_init.d_100p[count]) {
    ++count;
  }
  return count;
}

// Передача в ESP32 коэффициентов отображения
void CREM_OSC::transfer_disp_c() {
  memset(tx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
  memset(rx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
  tx_dma_buffer[0] = send_CIND;
  for (unsigned short i = 1; i < NUMBER_TRACKS + 1; ++i) {
    tx_dma_buffer[i] = set_init.d_100p[i - 1];
  }
  start_dma_transfer();
  Pause_us(6000);
  start_dma_transfer();
  Pause_us(6000);
  start_dma_transfer();
  Pause_us(6000);
}

// Передача в ESP32 имён треков. Два имени за одну транзакцию
void CREM_OSC::transfer_name() {
  unsigned char position;
  unsigned char track = 0;
  unsigned char HALF_LENGTH = ((TRANSACTION_LENGTH - 1) / 2) + 1;  // 6 при длине TRANSACTION_LENGHT = 11
  for (unsigned short nt = 0; nt < (NUMBER_TRACKS / 2); nt++) {
    memset(tx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
    memset(rx_dma_buffer, 0, TRANSACTION_LENGTH * 2);

    position = 0;
    tx_dma_buffer[position++] = NAME_CODES[nt];

    for (unsigned short nch = 0; nch < (NAME_LENGTH - 1) && set_init.Names[track][nch]; nch++) {
      tx_dma_buffer[position++] = set_init.Names[track][nch];
    }
    position = HALF_LENGTH;
    track++;
    for (short nch = 0; nch < (NAME_LENGTH - 1) && set_init.Names[track][nch]; nch++) {
      tx_dma_buffer[position++] = set_init.Names[track][nch];
    }
    track++;
    start_dma_transfer();
    Pause_us(6000);
    start_dma_transfer();
    Pause_us(6000);
    start_dma_transfer();
    Pause_us(6000);
  }
}

// Определение режима работы и готовности ESP32
void CREM_OSC::transfer_mode() {
  StatusRet Ret;
  switch (set_init.mode) {
    case Operating_mode::Access_point:
      Ret = transfer_SN_ID();
      break;
    case Operating_mode::Station:
      Ret = transfer_SSID();
      if (Ret == StatusRet::SUCCESS) {
        Ret = transfer_Password();
      }
      break;
  }
  memset(tx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
  memset(rx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
  StatusESP32 = static_cast<bool>(Ret);
}

// Передача в ESP32 двух последних цифр SN_ID
StatusRet CREM_OSC::transfer_SN_ID() {
  memset(tx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
  memset(rx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
  tx_dma_buffer[0] = send_SNID;
  char formVar[4];
  sprintf(formVar, "%02u", set_init.SNboard_number % 100);
  tx_dma_buffer[1] = formVar[0];
  tx_dma_buffer[2] = formVar[1];
  start_dma_transfer();
  Pause_us(6000);
  for (short n = 1; n < n_repeat; n++) {
    memset(rx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
    start_dma_transfer();
    Pause_us(6000);
  }
  if (echo_check() == StatusRet::SUCCESS) {
    return StatusRet::SUCCESS;
  }
  return StatusRet::ERROR;
}

// Передача в ESP32 SSID
StatusRet CREM_OSC::transfer_SSID() {
  memset(tx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
  memset(rx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
  tx_dma_buffer[0] = send_SSID;
  pack_chars(set_init.pSSID);
  start_dma_transfer();
  Pause_us(6000);
  for (short n = 1; n < n_repeat; n++) {
    memset(rx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
    start_dma_transfer();
    Pause_us(6000);
    if (echo_check() == StatusRet::SUCCESS) {
      return StatusRet::SUCCESS;
    }
  }
  return StatusRet::ERROR;
}

// Передача в ESP32 Пароля
StatusRet CREM_OSC::transfer_Password() {
  memset(tx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
  memset(rx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
  tx_dma_buffer[0] = send_PASS;
  pack_chars(set_init.pPassword);
  start_dma_transfer();
  Pause_us(6000);
  for (short n = 1; n < n_repeat; n++) {
    memset(rx_dma_buffer, 0, TRANSACTION_LENGTH * 2);
    start_dma_transfer();
    Pause_us(6000);
    if (echo_check() == StatusRet::SUCCESS) {
      return StatusRet::SUCCESS;
    }
  }
  return StatusRet::ERROR;
}

// Контроль эха
StatusRet CREM_OSC::echo_check() {
  for (short n = 0; n < TRANSACTION_LENGTH; n++) {
    if (rx_dma_buffer[n] != tx_dma_buffer[n]) {
      return StatusRet::ERROR;
    }
  }
  return StatusRet::SUCCESS;
}

// Упаковка строки
void CREM_OSC::pack_chars(unsigned char* str) {
  unsigned char position = 1;
  for (short ch = 0; (ch < G_CONST::SSID_PS_L) && str[ch]; ch += 2) {
    tx_dma_buffer[position++] = (str[ch] << 8) | str[ch + 1];
  }
}

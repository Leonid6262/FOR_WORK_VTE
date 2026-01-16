#include "spi_ports.hpp"

#include "dIOStorage.hpp"
#include "spi_init.hpp"
#include "system_LPC177x.h"

void CSPI_ports::rw() {
  unsigned int dT = LPC_TIM0->TC - prev_TC0;  // Текущая дельта [0.1*mks]
  prev_TC0 = LPC_TIM0->TC;

  auto& settings = CEEPSettings::getInstance().getSettings();
  CDIN_STORAGE& s_instans = CDIN_STORAGE::getInstance();

  unsigned char n_for_storage = static_cast<unsigned char>(CDIN_STORAGE::EIBNumber::CPU_SPI);

  for (char byte = 0; byte < G_CONST::BYTES_RW_REAL; byte++) {
    // Запись в dout с учётом инверсии
    
    SSP->DR = s_instans.UData_dout[byte + (G_CONST::BYTES_RW_MAX - G_CONST::BYTES_RW_REAL)].all ^
              settings.dout_spi_invert[byte + (G_CONST::BYTES_RW_MAX - G_CONST::BYTES_RW_REAL)];

    // Фильтр (интегратор входного сигнала) и фиксация в CDIN_STORAGE
    s_instans.filter(data_din[byte], dT, n_for_storage + byte);

    // После окончание операции r/w считываем байт din порта.
    // учитывая, что процесс фильтрации происходит на фоне транзакции spi,ожидания
    // при частотах spi до 900 кГц в while (SSP->SR & SR_BSY){} - не происходит
    while (SSP->SR & SPI_Config::SR_BSY) {
    }
    data_din[byte] = ~SSP->DR;
  }

  // Захват din и обновление dout (1->0->1 HOLD bit).
  LPC_GPIO0->CLR = HOLD;
  for (short Counter = 0x10; Counter > 0; Counter--) {
  }
  LPC_GPIO0->SET = HOLD;
}

CSPI_ports::CSPI_ports(LPC_SSP_TypeDef* SSP) : SSP(SSP) {
  // Обнуление случайных значений в выходных регистрах.
  for (char byte = 0; byte < G_CONST::BYTES_RW_MAX; byte++) {
    CDIN_STORAGE::getInstance().UData_dout[byte].all = 0;
  }
  rw();
  // Активизация выходных регистров (перевод из 3-го состояния в активное)
  LPC_GPIO0->CLR = OUT_E;
  prev_TC0 = LPC_TIM0->TC;
}

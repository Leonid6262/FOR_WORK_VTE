#include "spi_ports.hpp"
#include "dIOStorage.hpp"
#include "spi_init.hpp"
#include "drv_spi.hpp"
#include "system_LPC177x.h"

void CSPI_ports::rw() {
  unsigned int dT = SysT::TC() - prev_TC0;  // Текущая дельта [0.1*mks]
  prev_TC0 = SysT::TC();

  auto& settings = CEEPSettings::getInstance().getSettings();
  CDIN_STORAGE& s_instans = CDIN_STORAGE::getInstance();

  unsigned char n_for_storage = static_cast<unsigned char>(CDIN_STORAGE::EIBNumber::CPU_SPI);
  
  for (char byte = 0; byte < G_CONST::BYTES_RW_REAL; byte++) {
    // Запись в dout с учётом инверсии
    spi_drv.writeByte(s_instans.UData_dout[byte + (G_CONST::BYTES_RW_MAX - G_CONST::BYTES_RW_REAL)].all ^
              settings.dout_spi_invert[byte + (G_CONST::BYTES_RW_MAX - G_CONST::BYTES_RW_REAL)]);

    // Фильтр (интегратор входного сигнала) и фиксация в CDIN_STORAGE
    s_instans.filter(data_din[byte], dT, n_for_storage + byte, CEEPSettings::getInstance());

    // После окончание операции r/w считываем байт din порта.
    // учитывая, что процесс фильтрации происходит на фоне транзакции spi,ожидания
    // при частотах spi до 900 кГц в while (SSP->SR & SR_BSY){} - не происходит
    
    while (spi_drv.statusBSY()) {}
    data_din[byte] = ~spi_drv.readByte();
  }
  
  // Захват din и обновление dout (1->0->1 HOLD bit).
  gpio.clr(bg::HOLD);
  for (short Counter = 0x10; Counter > 0; Counter--) {}
  gpio.set(bg::HOLD);
}

CSPI_ports::CSPI_ports(CSPI_DRIVER& spi_drv, CGPIO& gpio) : spi_drv(spi_drv), gpio(gpio) {  
  // Обнуление случайных значений в выходных регистрах.
  for (char byte = 0; byte < G_CONST::BYTES_RW_MAX; byte++) {
    CDIN_STORAGE::getInstance().UData_dout[byte].all = 0;
  }
  rw();
  // Активизация выходных регистров (перевод из 3-го состояния в активное)
  gpio.set(bg::OUT_E);
  prev_TC0 = SysT::TC();
}

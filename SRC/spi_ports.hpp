#pragma once

#include "settings_eep.hpp"
#include "Peripherals.hpp"
#include "drv_spi.hpp"

class CSPI_ports {
 public:
  CSPI_ports(CSPI_DRIVER&, CGPIO&);

  unsigned char data_din[G_CONST::BYTES_RW_MAX];  // Входные данные din портов

  void rw();

 private:
  CSPI_DRIVER& spi_drv;
  CGPIO& gpio;

  unsigned int prev_TC0;  // Значение таймера на предыдыущем цикле

  static constexpr unsigned int TIC_ms = 10000;

};

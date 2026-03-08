#pragma once

#include "settings_eep.hpp"
#include "Peripherals.hpp"

class CSPI_ports {
 public:
  CSPI_ports(LPC_SSP_TypeDef*, CGPIO&);

  unsigned char data_din[G_CONST::BYTES_RW_MAX];  // Входные данные din портов

  void rw();

 private:
  LPC_SSP_TypeDef* SSP;
  CGPIO& gpio;

  unsigned int prev_TC0;  // Значение таймера на предыдыущем цикле

  static constexpr unsigned int TIC_ms = 10000;

};

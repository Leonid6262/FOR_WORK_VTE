#pragma once

#include "settings_eep.hpp"

class CSPI_ports {
 public:
  CSPI_ports(LPC_SSP_TypeDef*);

  unsigned char data_din[G_CONST::BYTES_RW_MAX];  // Входные данные din портов

  void rw();

 private:
  LPC_SSP_TypeDef* SSP;

  unsigned int prev_TC0;  // Значение таймера на предыдыущем цикле

  static constexpr unsigned int TIC_ms = 10000;
  static constexpr unsigned int OUT_E = 1UL << 8;
  static constexpr unsigned int HOLD = 1UL << 7;
};

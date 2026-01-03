#pragma once

#include "LPC407x_8x_177x_8x.h"

class CCAN {
 public:
  enum class ECAN_Id_Instance { CAN1_Id = 0, CAN2_Id = 1 };

  CCAN(ECAN_Id_Instance);
  void transfer_short(unsigned short);
  unsigned short receive_short();

 private:
  static constexpr unsigned int baudrate_can1 = 125000;  // [bps]
  static constexpr unsigned int baudrate_can2 = 125000;  // [bps]

  void resetController();
  void clearAFTable();
  void setBaudrate(unsigned int, unsigned int);

  LPC_CAN_TypeDef* CAN_N;

  static constexpr unsigned int IOCON_CAN1 = 0x01;
  static constexpr unsigned int IOCON_CAN2 = 0x02;
};

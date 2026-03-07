#include "can.hpp"

#include "lpc177x_8x_can.h"
#include "system_LPC177x.h"
#include "Peripherals.hpp"

CCAN::CCAN(ECAN_Id_Instance CAN_Id) {
  switch (CAN_Id) {
    case ECAN_Id_Instance::CAN1_Id:
      CAN_N = LPC_CAN1;
     
      resetController();
      clearAFTable();
      setBaudrate(baudrate_can1, PeripheralClock);

      break;
    case ECAN_Id_Instance::CAN2_Id:
      CAN_N = LPC_CAN2;

      resetController();
      clearAFTable();
      setBaudrate(baudrate_can2, PeripheralClock);

      break;
  }
  CAN_N->TFI1 = 2UL << 16;  // 2 bytes
  CAN_N->TID1 = 0x00;       // TID1 = 0;
  CAN_N->MOD = CAN_MOD_RM;  // Enter Reset Mode
  // CAN_N->MOD |= CAN_MOD_STM;                        // Set Self Test Mode
  CAN_N->MOD &= ~CAN_MOD_RM;     // Clear Reset Mode
  LPC_CANAF->AFMR = CAN_ACC_BP;  // Acceptance Fileter Bypass Mode
}

void CCAN::resetController() {
  CAN_N->MOD = 1;
  CAN_N->IER = 0;
  CAN_N->GSR = 0;
  CAN_N->CMR = (1 << 1) | (1 << 2) | (1 << 3);
  volatile uint16_t dummy = CAN_N->ICR;
  CAN_N->MOD = 0;
}

void CCAN::clearAFTable() {
  LPC_CANAF->AFMR = 0x01;
  for (uint16_t i = 0; i < 512; ++i) {
    LPC_CANAF_RAM->mask[i] = 0x00;
  }
  LPC_CANAF->SFF_sa = 0;
  LPC_CANAF->SFF_GRP_sa = 0;
  LPC_CANAF->EFF_sa = 0;
  LPC_CANAF->EFF_GRP_sa = 0;
  LPC_CANAF->ENDofTable = 0;
  LPC_CANAF->AFMR = 0x00;
}

void CCAN::setBaudrate(unsigned int baudrate, unsigned int PClock) {
  unsigned int CANPclk = PClock;
  unsigned int result = CANPclk / baudrate;
  unsigned short NT, TSEG1 = 0, TSEG2 = 0;
  unsigned int BRP = 0;

  for (NT = 24; NT > 0; NT -= 2) {
    if (result % NT == 0) {
      BRP = result / NT - 1;
      NT--;
      TSEG2 = (NT / 3) - 1;
      TSEG1 = NT - (NT / 3) - 1;
      break;
    }
  }

  CAN_N->MOD = 1;
  CAN_N->BTR = (TSEG2 << 20) | (TSEG1 << 16) | (3 << 14) | BRP;
  CAN_N->MOD = 0;
}

#include "SDCard_init.hpp"

#include <string.h>

#include "LPC407x_8x_177x_8x.h"
#include "pause_us.hpp"

StatusRet CSDCard::init() {
  LPC_IOCON->P0_19 = IOCON_SD_CLK;
  LPC_IOCON->P0_20 = IOCON_SD_CMD;
  LPC_IOCON->P1_5 = IOCON_SD_PWR;
  LPC_IOCON->P1_6 = IOCON_SD_DAT0;
  LPC_IOCON->P1_7 = IOCON_SD_DAT1;
  LPC_IOCON->P1_11 = IOCON_SD_DAT2;
  LPC_IOCON->P1_12 = IOCON_SD_DAT3;

  LPC_SC->PCONP |= PCONP_PCSDC;  // Включаем питание модуля

  LPC_MCI->POWER = 0x00;  // Отключаем питание карты
  Pause_us(2000);
  LPC_MCI->POWER = POWER_ON;  // Включаем питание карты
  Pause_us(2000);
  LPC_MCI->CLOCK = 0x00;     // Остановить SDCLK
  LPC_MCI->COMMAND = 0x00;   // Сбросить командный регистр
  LPC_MCI->DATACTRL = 0x00;  // Сбросить передачу данных

  LPC_MCI->CLOCK = CLOCKDIV | ENABLE_CLK;  // CLOCKDIV = 74, (60000000/(2*(74+1)) = 400000)
  Pause_us(2000);

  sendCommand(CMD0, 0x00000000, 0x00000000);   // CMD0: GO_IDLE_STATE
  sendCommand(CMD8, RANGECHECK, RESP_EXPECT);  // CMD8: SEND_IF_COND

  unsigned short i;
  for (i = 0; i < 1000; ++i) {
    sendCommand(CMD55, 0x00000000, RESP_EXPECT);  // CMD55
    sendCommand(ACMD41, HCSVW, RESP_EXPECT);      // ACMD41
    if (LPC_MCI->RESP0 & (1 << 31)) {
      break;  // Ready
    }
    Pause_us(1000);
  }
  if (i == 1000) {
    return StatusRet::ERROR;
  }

  sendCommand(CMD2, 0x00000000, LONG_RESP);    // CMD2 Get CID
  sendCommand(CMD3, 0x00000000, RESP_EXPECT);  // CMD3 Get RCA
  RCA = (LPC_MCI->RESP0 >> 16) & 0xFFFF;       // Читаем RCA карты
  LPC_MCI->POWER = 0x0;                        // Отключаем питание карты
  if ((RCA == 0xFFFF) || (RCA == 0x0000)) {
    return StatusRet::ERROR;
  }
  return StatusRet::SUCCESS;
}

void CSDCard::sendCommand(unsigned int cmd, unsigned int arg, unsigned int flags) {
  LPC_MCI->ARGUMENT = arg;
  LPC_MCI->COMMAND = (cmd & 0x3F) | ENABLE | flags;
  Pause_us(5000);
  LPC_MCI->CLEAR = 0x7FF;
}

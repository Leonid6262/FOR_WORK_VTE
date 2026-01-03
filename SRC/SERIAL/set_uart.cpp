#include "set_uart.hpp"

LPC_UART_TypeDef* CSET_UART::configure(EUartInstance UN) {
  LPC_UART_TypeDef* UART = nullptr;

  switch (UN) {
    case EUartInstance::UART_0:
      UART = LPC_UART0;
      LPC_IOCON->P0_2 = IOCON_U0_TXD;         // U0_TXD
      LPC_IOCON->P0_3 = IOCON_U0_RXD;         // U0_RXD
      LPC_SC->PCONP |= CLKPWR_PCONP_PCUART0;  // Включение питания UART0
      /* Настройка частоты */
      UART->TER = 0x00;         // Запрет передачи на момент настройки
      UART->LCR = LCR_DLAB_ON;  // b7 - DLAB вкл.
      UART->DLM = baud_19200.DLM;
      UART->DLL = baud_19200.DLL;
      UART->FDR = baud_19200.FDR;
      break;
    case EUartInstance::UART_1:
      UART = LPC_UART2;
      LPC_IOCON->P2_6 = IOCON_U2_OE;          // U2_OE
      LPC_IOCON->P2_8 = IOCON_U2_TXD;         // U2_TXD
      LPC_IOCON->P2_9 = IOCON_U2_RXD;         // U2_RXD
      LPC_SC->PCONP |= CLKPWR_PCONP_PCUART2;  // Включение питания UART2
      UART->RS485CTRL = DCTRL;                // Автоматическое переключение OE
      UART->RS485CTRL |= OINV;                // Инверсия OE
      /* Настройка частоты */
      UART->TER = 0x00;         // Запрет передачи на момент настройки
      UART->LCR = LCR_DLAB_ON;  // b7 - DLAB вкл.
      UART->DLM = baud_115200.DLM;
      UART->DLL = baud_115200.DLL;
      UART->FDR = baud_115200.FDR;
      break;
    case EUartInstance::UART_2:
      UART = LPC_UART3;
      LPC_IOCON->P1_30 = IOCON_U3_OE;         // U3_OE
      LPC_IOCON->P4_28 = IOCON_U3_TXD;        // U3_TXD
      LPC_IOCON->P4_29 = IOCON_U3_RXD;        // U3_RXD
      LPC_SC->PCONP |= CLKPWR_PCONP_PCUART3;  // Включение питания UART3
      UART->RS485CTRL = DCTRL;                // Автоматическое переключение OE
      UART->RS485CTRL |= OINV;                // Инверсия OE
      /* Настройка частоты */
      UART->TER = 0x00;         // Запрет передачи на момент настройки
      UART->LCR = LCR_DLAB_ON;  // b7 - DLAB вкл.
      UART->DLM = baud_115200.DLM;
      UART->DLL = baud_115200.DLL;
      UART->FDR = baud_115200.FDR;
      break;
  }
  UART->LCR = LCR_DLAB_OFF;  // b7 - DLAB откл., чётность откл., 1-стоп бит, символ 8бит
  UART->FCR = FIFOEN;        // FIFO. b2-очистка TXFIFO, b1-очистка RXFIFO, b0-вкл FIFO
  UART->TER = TXEN;          // Разрешение передачи
  while (UART->LSR & RDR) {
    unsigned int tmp = UART->RBR;  // Очистка приёмника
  }
  while (!(UART->LSR & THRE)) {
  };  // Очистка передатчика
  
  return UART;
}

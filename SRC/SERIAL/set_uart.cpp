#include "set_uart.hpp"
#include "Peripherals.hpp"

LPC_UART_TypeDef* CSET_UART::configure(EUartInstance UN) {
  
  LPC_UART_TypeDef* UART;

  switch (UN) {
    case EUartInstance::UART_0:
      UART = P::UART0;
      /* Настройка частоты */
      UART->TER = 0x00;         // Запрет передачи на момент настройки
      UART->LCR = LCR_DLAB_ON;  // b7 - DLAB вкл.
      UART->DLM = baud_19200.DLM;
      UART->DLL = baud_19200.DLL;
      UART->FDR = baud_19200.FDR;
      break;
    case EUartInstance::UART_2:
      UART = P::UART2;
      UART->RS485CTRL = DCTRL;                // Автоматическое переключение OE
      UART->RS485CTRL |= OINV;                // Инверсия OE
      /* Настройка частоты */
      UART->TER = 0x00;         // Запрет передачи на момент настройки
      UART->LCR = LCR_DLAB_ON;  // b7 - DLAB вкл.
      UART->DLM = baud_115200.DLM;
      UART->DLL = baud_115200.DLL;
      UART->FDR = baud_115200.FDR;
      break;
    case EUartInstance::UART_3:
      UART = P::UART3;
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

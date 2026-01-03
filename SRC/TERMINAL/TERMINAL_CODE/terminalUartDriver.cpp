#include "terminalUartDriver.hpp"
#include "IntPriority.hpp"
#include "LPC407x_8x_177x_8x.h"
#include "system_LPC177x.h"

CTerminalUartDriver::CTerminalUartDriver() {};

void CTerminalUartDriver::init(LPC_UART_TypeDef* UART, IRQn_Type UART_IRQ) {
  this->UART = UART;
  txbuf.head = 0;
  txbuf.tail = 0;
  // Настройка прерываний
  UART->IER = 0;
  UART->IER |= THRE_I;  // b1-THRE

  NVIC_EnableIRQ(UART_IRQ);
}

CTerminalUartDriver& CTerminalUartDriver::getInstance() {
  static CTerminalUartDriver instance;
  return instance;
}

// -------------------------------------------------------------
//              Реализации RingBuffer
// -------------------------------------------------------------
bool CTerminalUartDriver::RingBuffer::push(unsigned char byte) {
  unsigned short next = (head + 1) % SIZE;
  if (next == tail) return false;  // переполнение
  buf[head] = byte;
  head = next;
  return true;
}

bool CTerminalUartDriver::RingBuffer::pop(unsigned char& byte) {
  if (head == tail) return false;  // пусто
  byte = buf[tail];
  tail = (tail + 1) % SIZE;
  return true;
}

bool CTerminalUartDriver::RingBuffer::empty() const { return head == tail; }
//-----------------------------------------------------------------

bool CTerminalUartDriver::sendBuffer(const unsigned char* data, unsigned char len) {
  // Проверка свободного места в кольцевом буфере
  unsigned short free = (txbuf.tail + RingBuffer::SIZE - txbuf.head - 1) % RingBuffer::SIZE;
  if (free < len) return false;

  // Загрузка буфера передачи
  for (unsigned char i = 0; i < len; i++) {
    txbuf.push(data[i]);
  }

  // Если FIFO пустое — сразу пишем
  if (UART->LSR & THRE_F) {
    unsigned char FIFO_SPACE = UART_FIFO_SIZE;
    while (FIFO_SPACE--) {
      unsigned char byte;
      if (txbuf.pop(byte)) {
        UART->THR = byte;
      } else {
        break;
      }
    }
  }

  // Включаем прерывание, если остались данные
  if (!txbuf.empty()) {
    UART->IER |= THRE_I;
  }
  return true;
}

// Опрос RX
bool CTerminalUartDriver::poll_rx(unsigned char& byte) {
  if (UART->LSR & RDR_F)  // RDR flag. Есть данные для чтения
  {
    byte = UART->RBR;
    return true;
  }
  return false;
}

// THRE Handler
void CTerminalUartDriver::irq_handler() {
  unsigned int IRQ = UART->IIR;
  if ((IRQ & THRE_I) == THRE_I)  // THRE interrupt
  {
    unsigned char FIFO_SPACE = UART_FIFO_SIZE;
    while (FIFO_SPACE--) {
      unsigned char byte;
      if (txbuf.pop(byte)) {
        UART->THR = byte;
      } else {
        UART->IER &= ~(THRE_I);  // очередь пуста → отключаем прерывание
        break;
      }
    }
  }
}

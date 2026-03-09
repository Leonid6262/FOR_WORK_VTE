#include "terminalUartDriver.hpp"
#include "IntPriority.hpp"
#include "LPC407x_8x_177x_8x.h"
#include "system_LPC177x.h"

CTerminalUartDriver::CTerminalUartDriver() {};

void CTerminalUartDriver::init(CUART_DRIVER* uart_drv) { 
  this->uart_drv = uart_drv;
  txbuf.head = 0;
  txbuf.tail = 0;
  uart_drv->clrIER();
}

CTerminalUartDriver& CTerminalUartDriver::getInstance() {
  static CTerminalUartDriver instance;
  return instance;
}
             
// -------------- Реализации RingBuffer-------------------------
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
  for (unsigned char i = 0; i < len; i++) { txbuf.push(data[i]); }

  // Если FIFO пустое — сразу пишем
  if (uart_drv->statusTHRE_F()) {   
    unsigned char FIFO_SPACE = UART_FIFO_SIZE;
    while (FIFO_SPACE--) {
      unsigned char byte;
      if (txbuf.pop(byte)) {
        uart_drv->writeByte(byte);
      } else {
        break;
      }
    }
  }

  // Включаем прерывание, если остались данные
  if (!txbuf.empty()) { uart_drv->EnableTHRE_I(); }
  return true;
}

// Опрос RX
bool CTerminalUartDriver::poll_rx(unsigned char& byte) {
  if (uart_drv->statusRDR_F()) {  // RDR flag. Есть данные для чтения
    byte = uart_drv->readByte();
    return true;
  }
  return false;
}

// THRE Handler
void CTerminalUartDriver::irq_handler() {
  // Control THRE interrupt
  if(uart_drv->statusTHRE_I()) {
    unsigned char FIFO_SPACE = UART_FIFO_SIZE;
    while (FIFO_SPACE--) {
      unsigned char byte;
      if (txbuf.pop(byte)) { uart_drv->writeByte(byte); } 
      else { uart_drv->DisableTHRE_I(); break; } // очередь пуста - отключаем прерывание
    }
  }
}

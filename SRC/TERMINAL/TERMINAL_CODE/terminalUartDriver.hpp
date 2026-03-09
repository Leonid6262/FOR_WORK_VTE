#pragma once
#include <cstdint>
#include "drv_uart.hpp"
#include "LPC407x_8x_177x_8x.h"

// Singleton класс драйвера UART для терминала
// Использует прерывание по FIFO empty и всю глубину (16 byte) FIFO
// Для 16 байтных строк терминала: "0123456789012345\r" - одно прерывание дописывающее \r
class CTerminalUartDriver {
 public:
  void init(CUART_DRIVER*);
  static CTerminalUartDriver& getInstance();

  bool sendBuffer(const unsigned char* data, unsigned char len);
  bool poll_rx(unsigned char& byte);
  void irq_handler();

 private:
  static constexpr unsigned char UART_FIFO_SIZE = 16;  // глубина аппаратного FIFO

  // Внутренний кольцевой буфер
  struct RingBuffer {
    static constexpr int SIZE = 256;
    unsigned char buf[SIZE];
    unsigned short head = 0;
    unsigned short tail = 0;

    bool push(unsigned char b);
    bool pop(unsigned char& b);
    bool empty() const;
  };

  RingBuffer txbuf;
  CUART_DRIVER* uart_drv;

  CTerminalUartDriver();
  CTerminalUartDriver(const CTerminalUartDriver&) = delete;
  CTerminalUartDriver& operator=(const CTerminalUartDriver&) = delete;
};

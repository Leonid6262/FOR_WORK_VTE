#pragma once

#include <cstdint>

#include "LPC407x_8x_177x_8x.h"

// Singleton класс драйвера UART для терминала
// Использует прерывание по FIFO empty и всю глубину (16 byte) FIFO
// Для 16 байтных строк терминала: "0123456789012345\r" - одно прерывание дописывающее \r
class CTerminalUartDriver {
 public:
  void init(LPC_UART_TypeDef*, IRQn_Type);
  static CTerminalUartDriver& getInstance();

  bool sendBuffer(const unsigned char* data, unsigned char len);
  bool poll_rx(unsigned char& byte);
  void irq_handler();

 private:
  static constexpr unsigned char UART_FIFO_SIZE = 16;  // глубина аппаратного FIFO
  static constexpr unsigned int THRE_F = 1UL << 5;     // THRE flag. FIFO empty
  static constexpr unsigned int THRE_I = 1UL << 1;     // THRE interrupt. FIFO empty
  static constexpr unsigned int RDR_F = 1UL << 0;      // RDR flag. Есть данные для чтения

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

  LPC_UART_TypeDef* UART;

  CTerminalUartDriver();
  CTerminalUartDriver(const CTerminalUartDriver&) = delete;
  CTerminalUartDriver& operator=(const CTerminalUartDriver&) = delete;
};

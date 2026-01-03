#pragma once

#include "LPC407x_8x_177x_8x.h"
#include "system_LPC177x.h"

class CSET_UART{
public:    
  enum class EUartInstance {
    UART_0,              
    UART_1,             
    UART_2              
  };
  
  static LPC_UART_TypeDef* configure(EUartInstance);
  
private: 
    // Настройки скорости
    struct SBaudRateSettings {
        unsigned int DLM;
        unsigned int DLL;
        unsigned int FDR;
    };
    /* Настройка на 19200. UART_baudrate = 60000000 / ( (16*101)  *  (1+(14/15)) ) = 19205 (+0.026%) */ 
    static constexpr SBaudRateSettings baud_19200 = {
        .DLM = 0,
        .DLL = 101,
        .FDR = 0xFE     // MulVal = 15, DivAddVal = 14 
    };
    /* Настройка на 115200. UART_baudrate = 60000000 / ( (16*21)  *  (1+(5/9)) ) =  114796 (-0.35%) */
    static constexpr SBaudRateSettings baud_115200 = {
        .DLM = 0,
        .DLL = 21,
        .FDR = 0x95     // MulVal = 9, DivAddVal = 5
    };
  
  // Пины  
  static constexpr unsigned int IOCON_U0_TXD  = 0x1;  
  static constexpr unsigned int IOCON_U0_RXD  = 0x1;
  
  static constexpr unsigned int IOCON_U2_TXD  = 0x2;  
  static constexpr unsigned int IOCON_U2_RXD  = 0x2;
  static constexpr unsigned int IOCON_U2_OE   = 0x4;
  
  static constexpr unsigned int IOCON_U3_TXD  = 0x2;  
  static constexpr unsigned int IOCON_U3_RXD  = 0x2;
  static constexpr unsigned int IOCON_U3_OE   = 0x5;

  // Битовые масоки
  enum RegisterFlags {
    THRE         = 1UL << 5,
    RDR          = 1UL << 0,
    DCTRL        = 1UL << 4,
    OINV         = 1UL << 5,
    LCR_DLAB_ON  = 0x80,        // b7-DLAB 
    LCR_DLAB_OFF = 0x03,        // Чётность откл., 1-стоп бит, символ 8 бит
    FIFOEN       = 0x07,        // b2-очистка TXFIFO, b1-очистка RXFIFO, b0-вкл FIFO
    TXEN         = 1UL << 7     // Разрешение передачи
  };  
  
  LPC_UART_TypeDef* UART;
};


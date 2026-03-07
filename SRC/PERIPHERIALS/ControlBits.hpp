#pragma once

namespace bg {
  // Биты физических связей GPIO
  static constexpr unsigned short B_ULED      = 9;        // Бит U-LED
  static constexpr unsigned short B_Q1VF      = 13;       // Бит Q1VF
  static constexpr unsigned short B0_PORT_OUT = 24;       // 1-й бит DOUT порта GPIO2  
  static constexpr unsigned int B0_PORT_IN    = 15;       // 0-й бит порта. Начало Pi0 в PORT2 контроллера
  
  static constexpr unsigned int OUT_E = 1UL << 8;
  static constexpr unsigned int HOLD = 1UL << 7;
  
  static constexpr unsigned int SUM1 = 0x02;              // Port1:2 PWM0:1
  static constexpr unsigned int SUM2 = 0x03;              // Port1:3 PWM0:2
  static constexpr unsigned int FIRST_PULS_PORT = 16;               // 1-й импульс в порту
  static constexpr unsigned int OFF_PULSES = 0x3F<<FIRST_PULS_PORT; // Все импульсы управления
    
}

namespace bf {
  // Биты управления функциями GPIO
  static constexpr unsigned int D_MODE_PULLUP = 0x02 << 3;
  static constexpr unsigned int IOCON_DAC0_EN = 0x00010002;
  static constexpr unsigned int IOCON_PORT_PWM = 1;
  static constexpr unsigned int IOCON_CH_ADC_IOCON = 1;
  
  static constexpr unsigned int IOCON_SPI0 = 0x02;
  static constexpr unsigned int IOCON_SPI1 = 0x03;
  static constexpr unsigned int IOCON_SPI2 = 0x02;
  
  static constexpr unsigned int IOCON_U0_TXD  = 0x1;  
  static constexpr unsigned int IOCON_U0_RXD  = 0x1;
  
  static constexpr unsigned int IOCON_U2_TXD  = 0x2;  
  static constexpr unsigned int IOCON_U2_RXD  = 0x2;
  static constexpr unsigned int IOCON_U2_OE   = 0x4;
  
  static constexpr unsigned int IOCON_U3_TXD  = 0x2;  
  static constexpr unsigned int IOCON_U3_RXD  = 0x2;
  static constexpr unsigned int IOCON_U3_OE   = 0x5;   
  
  static constexpr unsigned int IOCON_T2_CAP1 = 0x23;
  static constexpr unsigned int IOCON_T3_CAP1 = 0x23;
  
  static constexpr unsigned int IOCON_CAN1 = 0x01;
  static constexpr unsigned int IOCON_CAN2 = 0x02;    
  
  static constexpr unsigned int IOCON_EINT2 = 0x01;
  

  
}



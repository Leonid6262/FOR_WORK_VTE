#pragma once

#include "bool_name.hpp"
#include "settings_eep.hpp"

class CDIN_STORAGE {
 public:
   
  //--- Входы контроллера, порт Pi0 ---
  inline bool Bl_Contact_Q1()   {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b0;} // Блок контакт Q1
  inline bool Reg_Auto()        {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b1;} // Режим "Автоматический"
  inline bool Reg_Drying()      {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b2;} // Режим "Сушка"
  inline bool Setting_More()    {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b3;} // Задание "Больше"
  inline bool Setting_Less()    {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b4;} // Задание "Меньше"
  inline bool CU_from_testing() {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b5;} // Сигнал "CU" от платы опробования
  inline bool Bl_HVS_NO()       {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b6;} // NO блок контакт ВВ
  inline bool Bl_HVS_NC()       {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b7;} // NC блок контакт ВВ
  
  //--- Входы контроллера byte-0 SPI ---
  inline bool Heating()         {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b0;} // Подогрев
  inline bool Stator_Key()      {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b1;} // Статорный выкл.
  inline bool DinControllerB2() {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b2;}
  inline bool DinControllerB3() {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b3;}
  inline bool IN1()             {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b4;} // Опция IN1
  inline bool IN2()             {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b5;} // Опция IN2
  inline bool IN3()             {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b6;} // Опция IN3
  inline bool Fuses_BC()        {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b7;} // БК предохранителей
  
  //--- Входы S600 byte-0 ---
  //
  //--- Входы S600 byte-1 ---
  //
  
  //--- Выходы контроллера Po0 ---
  static inline void bNamePo0B0(State state)    {edit_bit(0, state);}
  static inline void REL_LEAKAGE_P(State state) {edit_bit(1, state);} // Реле контроля изоляции К1
  static inline void REL_LEAKAGE_N(State state) {edit_bit(2, state);} // Реле контроля изоляции К2
  static inline void bNamePo0B3(State state)    {edit_bit(3, state);}
  static inline void bNamePo0B4(State state)    {edit_bit(4, state);}
  static inline void bNamePo0B5(State state)    {edit_bit(5, state);}
  static inline void bNamePo0B6(State state)    {edit_bit(6, state);}
  static inline void bNamePo0B7(State state)    {edit_bit(7, state);} 

  //--- Выходы контроллера SPI ---
  inline void Lamp_REDY         (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b0 = static_cast<bool>(state);}
  inline void DoutControllerB1  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b1 = static_cast<bool>(state);}
  inline void DoutControllerB2  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b2 = static_cast<bool>(state);}
  inline void DoutControllerB3  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b3 = static_cast<bool>(state);}
  inline void Relay_FAULT       (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b4 = static_cast<bool>(state);}
  inline void Relay_Ex_Applied  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b5 = static_cast<bool>(state);}
  inline void Relay_Premission  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b6 = static_cast<bool>(state);}
  inline void Relay_Shunt_HVS   (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b7 = static_cast<bool>(state);}
  
  //--- Выходы S600 ---
  //
  
  // Номера считываемых байтов
  enum class EIBNumber {
    CPU_PORT = 0,    // Контроллер Pi0
    CPU_SPI = 1,     // Контроллер SPI
    BYTE_0_EX1 = 2,  // S600 byte-0 SPI
    BYTE_1_EX1 = 3   // S600 byte-1 SPI
  };

  // Номера записываемых байтов последовательности SPI
  enum class EOBNumber {
    BYTE_1_EX1 = 0,  // S600 byte-0 (в S600 2/1 - отсутствует)
    BYTE_0_EX1 = 1,  // S600 byte-1
    CPU_SPI = 2      // Контроллер (всегда (BYTES_RW_MAX-1)), так как, замыкает последовательность)
  };

  /*                    Последовательность цикла:

       MISO: <- byte_din_cpu  <- byte_0_din_ex1  <- byte_1_din_ex1
                     0                 1                  2
       MOSI: -> byte_dout_cpu -> byte_0_dout_ex1 -> byte_1_dout_ex1(*)
                     2                 1                  0

       (*) При использовании S600 2/1, byte_1_dout_ex1 - выходит в эфир
 */

  union {
    unsigned char all;
    struct {
      unsigned char b0 : 1;
      unsigned char b1 : 1;
      unsigned char b2 : 1;
      unsigned char b3 : 1;
      unsigned char b4 : 1;
      unsigned char b5 : 1;
      unsigned char b6 : 1;
      unsigned char b7 : 1;
    };
  } UData_din_f[G_CONST::BYTES_RW_MAX + 1];  // Данные din портов после фильтра

  void filter(unsigned char, unsigned int, unsigned char, CEEPSettings&);

  union {
    unsigned char all;
    struct {
      unsigned char b0 : 1;
      unsigned char b1 : 1;
      unsigned char b2 : 1;
      unsigned char b3 : 1;
      unsigned char b4 : 1;
      unsigned char b5 : 1;
      unsigned char b6 : 1;
      unsigned char b7 : 1;
    };
  } UData_dout[G_CONST::BYTES_RW_MAX];  // Данные для записи в д.выходы

  static CDIN_STORAGE& getInstance() {
    static CDIN_STORAGE instance;
    return instance;
  }

  static inline void UserLedOn() { LPC_GPIO0->CLR = (1UL << B_ULED); }   // Вкл. ULED
  static inline void UserLedOff() { LPC_GPIO0->SET = (1UL << B_ULED); }  // Выкл.ULED
  static inline void Q1VF_On() { LPC_GPIO1->CLR = (1UL << B_Q1VF); }     // Вкл. Q1VF
  static inline void Q1VF_Off() { LPC_GPIO1->SET = (1UL << B_Q1VF); }    // Выкл.Q1VF

  static inline void edit_bit(char bit_number, State state) {
    switch (state) {
      case State::ON: {
        LPC_GPIO2->SET = static_cast<unsigned int>(1UL << (B0_PORT_OUT + (bit_number & 0x07)));
      }  // dout-on
      break;
      case State::OFF: {
        LPC_GPIO2->CLR = static_cast<unsigned int>(1UL << (B0_PORT_OUT + (bit_number & 0x07)));
      }  // dout-off
      break;
    }
  }

 private:
  static constexpr unsigned short B_ULED = 9;        // Бит U-LED
  static constexpr unsigned short B_Q1VF = 13;       // Бит Q1VF
  static constexpr unsigned short B0_PORT_OUT = 24;  // 1-й бит порта

  static constexpr unsigned short N_BITS = 8;  // Количество бит в портах
  static constexpr unsigned int TIC_ms = 10000;

  signed int integrator[G_CONST::BYTES_RW_MAX + 1][N_BITS];  // Интеграторы

  // Постоянные времени интегрирования фильтра (ms первый множитель) din портов.
  // TIC_ms = 10000 дискрет таймера на 1ms.
  // То есть, 50*TIC_ms = 50ms, 0*TIC_ms - нет фильтрации, и т.п.
  static constexpr unsigned int cConst_integr_Pi0[G_CONST::BYTES_RW_MAX + 1][N_BITS] = 
  {
    {50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms}, // Контроллер Pi0
    {50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms}, // Контроллер SPI
    {50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms}, // S600 byte-0 SPI
    {50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms}, // S600 byte-1 SPI
  }; 

  // --- Механизмы Singleton ---
  CDIN_STORAGE();
  CDIN_STORAGE(const CDIN_STORAGE&) = delete;
  CDIN_STORAGE& operator=(const CDIN_STORAGE&) = delete;
};
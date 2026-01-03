#pragma once

#include "bool_name.hpp"

class CSDCard {
public:
  
    unsigned int RCA;    
    StatusRet init();
    
private:
     
  static constexpr unsigned int IOCON_SD_CLK  = 0x2; 
  static constexpr unsigned int IOCON_SD_CMD  = 0x2;        
  static constexpr unsigned int IOCON_SD_PWR  = 0x82;    
  static constexpr unsigned int IOCON_SD_DAT0 = 0x82;    
  static constexpr unsigned int IOCON_SD_DAT1 = 0x82;    
  static constexpr unsigned int IOCON_SD_DAT2 = 0x2;    
  static constexpr unsigned int IOCON_SD_DAT3 = 0x2;    
  
  static constexpr unsigned int CMD_START          = 1UL << 6;   // Start command
  static constexpr unsigned int RESP_EXPECT        = 1UL << 6;   // Ожидание ответа (бит совпадает с CMD_START)
  static constexpr unsigned int LONG_RESP          = 1UL << 7;   // Длинный ответ (CID, CSD)
  static constexpr unsigned int CHECK_RESPONSE_CRC = 1UL << 8;   // Проверка CRC ответа
  static constexpr unsigned int ENABLE             = 1UL << 10;
  static constexpr unsigned int RANGECHECK         = 0x000001AA;
  static constexpr unsigned int HCSVW              = 0x40300000;
  static constexpr unsigned int CMD55              = 55;
  static constexpr unsigned int ACMD41             = 41;
  static constexpr unsigned int CMD0               = 0;
  static constexpr unsigned int CMD8               = 8;
  static constexpr unsigned int CMD2               = 2;
  static constexpr unsigned int CMD3               = 3;
  
  static constexpr unsigned int PCONP_PCSDC     = 1UL << 28;
  static constexpr unsigned int ENABLE_CLK      = 1UL << 8;
  static constexpr unsigned int POWER_ON        = 0x03;
  static constexpr unsigned int CLOCKDIV        = 74;
    
  void sendCommand(unsigned int, unsigned int, unsigned int);

};

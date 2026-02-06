#pragma once 

#include "SIFU.hpp"
#include "CurrentReg.hpp"
#include "QReg.hpp"
#include "CosReg.hpp"

class CSIFU;
class CCurrentReg;
class CQReg;
class CCosReg;

class CRegManager {
  
public:
 CRegManager(CCurrentReg&, CQReg&, CCosReg&);
 
 CSIFU* pSIFU;
 CCurrentReg& rCurrent_reg; 
 CQReg& rQ_reg; 
 CCosReg& rCos_reg;

 void applyModeRules();
 void stepAll();
 void getSIFU(CSIFU*);
 
 // --- Режимы регулирования---
 union URegMode_t {
   unsigned char all;
   struct {
     unsigned char Current  : 1;  // Разрешение РТ
     unsigned char QPower   : 1;  // Разрешение РQ
     unsigned char CosPhi   : 1;  // Разрешение РCos
   };
 } URegMode_request;    // Текущие режимы
 
  URegMode_t URegMode;  // Запрос режимов
 
  void setCurrent(State mode) { URegMode_request.Current = static_cast<unsigned char>(mode); } 
  void setQPower(State mode)  { URegMode_request.QPower  = static_cast<unsigned char>(mode); } 
  void setCosPhi(State mode)  { URegMode_request.CosPhi  = static_cast<unsigned char>(mode); } 
 
    // --- Биты режимов регулирования --- 
  enum MBit : unsigned char { 
    Current = 1 << 0, // Разрешение РТ 
    QPower  = 1 << 1, // Разрешение РQ 
    CosPhi  = 1 << 2, // Разрешение РCos 
  };
  
private:

  // --- Правила --- 
  struct DependencyRule { 
    MBit req_bit;                    // какой режим проверяется
    unsigned char requiredModes;     // какие режимы должны быть включены 
    unsigned char forbiddenModes;    // какие режимы должны быть выключены
  };
  
  // --- Таблица правил --- 
  static constexpr std::array<DependencyRule, 3> rules {{ 
    { MBit::Current, 0,             0            }, 
    { MBit::QPower,  MBit::Current, MBit::CosPhi }, 
    { MBit::CosPhi,  MBit::Current, MBit::QPower }, 
  }};
  
};


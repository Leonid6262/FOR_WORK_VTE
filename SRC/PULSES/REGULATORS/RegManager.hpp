#pragma once 

#include "SIFU.hpp"
#include "CurrentReg.hpp"

class CSIFU;
class CCurrentReg;

class CRegManager {
  
public:
 CRegManager(CCurrentReg&);
 
 CSIFU* pSIFU;
 CCurrentReg& rCurrent_reg; 
 
 void dispatch();
 void getSIFU(CSIFU*);
 
private:
   
 
};

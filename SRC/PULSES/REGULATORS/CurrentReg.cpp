#include "CurrentReg.hpp" 

CCurrentReg::CCurrentReg() : pAdc(CADC_STORAGE::getInstance()) {}

void CCurrentReg::reg(Mode mode) {
  signed short cur_current = *pAdc.getEPointer(CADC_STORAGE::ROTOR_CURRENT);
  
}



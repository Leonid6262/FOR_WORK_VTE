#pragma once 

#include "Adjustment.hpp" 
#include "SIFU.hpp"

class CSystemManager {
  
public:
 CSystemManager(CAdjustmentMode&, CSIFU&);
 void dispatch();
 
 CAdjustmentMode& rAdj_mode; 

private:
  CSIFU& rSIFU; 
 
};

#pragma once 
#include "bool_name.hpp"
#include "AdcStorage.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"

class CMBSLAVE {
  
public:
 CMBSLAVE(CDMAcontroller&);
 CDMAcontroller& rDMAc;
 void monitor();
  
private:
 

};

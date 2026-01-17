#pragma once 
#include "bool_name.hpp"
#include "AdcStorage.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"

class CMBSLAVE {
  
public:
 CMBSLAVE(CDMAcontroller&, LPC_UART_TypeDef*);
 CDMAcontroller& rDMAc;
 void monitor();
  
private:
 LPC_UART_TypeDef* UART;

};

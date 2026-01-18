#pragma once 
#include "bool_name.hpp"
#include "AdcStorage.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"

class CMBSLAVE {
  
private:
 LPC_UART_TypeDef* UART;
 
 static constexpr unsigned short TRANSACTION_LENGTH = 0x100;  

public:
  
 CMBSLAVE(CDMAcontroller&, LPC_UART_TypeDef*);
 CDMAcontroller& rDMAc;
 
 static signed short tx_dma_buffer[TRANSACTION_LENGTH];
 static signed short rx_dma_buffer[TRANSACTION_LENGTH];
 
 void monitor();
  

};

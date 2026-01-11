#pragma once

#include "crc16.hpp"
#include "settings_eep.hpp"
#include "set_uart.hpp"
#include "set_spi.hpp"
#include "spi_ports.hpp"
#include "can.hpp"
#include "dac.hpp"
#include "adc.hpp"
#include "i_adc.hpp"
#include "din_cpu.hpp"
#include "emac.hpp"
#include "rtc.hpp"
#include "controllerDMA.hpp"
#include "FaultCtrlP.hpp" 
#include "SIFU.hpp"
#include "RegManager.hpp" 
#include "puls_calc.hpp"
#include "handlers_IRQ.hpp"
#include "terminalUartDriver.hpp"
#include "menu_navigation.hpp"
#include "message_display.hpp"
#include "terminal_manager.hpp"
#include "AdcStorage.hpp"
#include "bool_name.hpp"
#include "Adjustment.hpp" 
#include "SystemManager.hpp" 

class CFactory {
public:
    
    static StatusRet load_settings();
  
    LPC_UART_TypeDef* initRS485_01();
    LPC_UART_TypeDef* initRS485_02();
    
    static CCAN initCAN1();
    static CCAN initCAN2();

    static CDAC0 createDAC0();
    static CDAC_PWM createPWMDac1();
    static CDAC_PWM createPWMDac2();
    
    static CIADC createIADC();
   
    static CSPI_ports createSPIports();    
    static CDin_cpu createDINcpu();
    
    static CEMAC_DRV createEMACdrv();
    
    static CDMAcontroller createDMAc();
   
    static CRTC createRTC();
    
    static CSystemManager& createSysManager(CSIFU&, CRegManager&);
    
    static CTerminalManager& createTM(CSystemManager&);
    
    static CSIFU& start_puls_system(CDMAcontroller&, CRegManager&);
    
    static CRegManager createRegManager();
};


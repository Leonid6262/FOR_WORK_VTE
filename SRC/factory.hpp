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
#include "FaultCtrlF.hpp" 
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
#include "TestingMode.hpp"
#include "_SystemManager.hpp"
#include "mb_slave.hpp" 
#include "iso_meas.hpp" 

class CFactory {
public:    
  static StatusRet load_settings();
  static CDAC0 createDAC0();    
  static CIADC createIADC();  
  static CSPI_ports createSPIports();    
  static CDin_cpu createDINcpu();    
  static CEMAC_DRV createEMACdrv();        
  static CTerminalManager& createTM(CSystemManager&);    
  static CMBSLAVE create_MBslave();    
  static CRegManager createRegManager();       
  static CSystemManager& start_system(CMBSLAVE&);     
  static CIsoMeas createIsoMeas();
};


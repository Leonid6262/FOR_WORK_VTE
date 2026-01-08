#include "crc16.hpp"
#include "factory.hpp"

LPC_UART_TypeDef* CFactory::initRS485_01() { return CSET_UART::configure(CSET_UART::EUartInstance::UART_1); } // For the control class
LPC_UART_TypeDef* CFactory::initRS485_02() { return CSET_UART::configure(CSET_UART::EUartInstance::UART_2); } // For the control class
CCAN CFactory::initCAN1()               { return CCAN(CCAN::ECAN_Id_Instance::CAN1_Id); }           // For the control class
CCAN CFactory::initCAN2()               { return CCAN(CCAN::ECAN_Id_Instance::CAN2_Id); }           // For the control class
CDAC0 CFactory::createDAC0()            { return CDAC0(); }                                         // For the control class
CDAC_PWM CFactory::createPWMDac1()      { return CDAC_PWM(CDAC_PWM::EPWM_DACInstance::PWM_DAC1); }  // For the control class
CDAC_PWM CFactory::createPWMDac2()      { return CDAC_PWM(CDAC_PWM::EPWM_DACInstance::PWM_DAC2); }  // For the control class
CEMAC_DRV CFactory::createEMACdrv()     { return CEMAC_DRV(); }                                     // For the control class

CIADC CFactory::createIADC()            { return CIADC(); }
CDin_cpu CFactory::createDINcpu()       { return CDin_cpu(); }
CRTC CFactory::createRTC()              { return CRTC(); }
CDMAcontroller CFactory::createDMAc()   { return CDMAcontroller(); }

// Загрузка уставок
StatusRet CFactory::load_settings()     { return CEEPSettings::getInstance().loadSettings(); }

// Создание объекта доступа к dIO доступных по SPI
CSPI_ports CFactory::createSPIports() { 
  return CSPI_ports(CSET_SPI::configure(CSET_SPI::ESPIInstance::SPI_0));
}

// Создание системного менеджера 
CSystemManager CFactory::createSysManager(CSIFU& rSIFU, CRegManager& rReg_manager) { 
  static CAdjustmentMode adjustment(rSIFU);
  static CReadyCheck ready_check;
  static CFaultControl fault_ctrl;
  static CPuskMode pusk_mode;
  static CWorkMode work_mode;
  static CWarningMode warning_ctrl;
  static CSystemManager sys_manager(rSIFU, 
                                    adjustment, 
                                    ready_check, 
                                    fault_ctrl, 
                                    pusk_mode, 
                                    work_mode,
                                    warning_ctrl,
                                    rReg_manager); 
  return sys_manager; 
}

// Инициализация драйвера и создание объектов Пультового терминала
CTerminalManager& CFactory::createTM(CSystemManager& rSysMgr) {   
  LPC_UART_TypeDef* U0 = CSET_UART::configure(CSET_UART::EUartInstance::UART_0);        // Конфигурация UART-0 - пультовый терминал
  CTerminalUartDriver::getInstance().init(U0, UART0_IRQn);                              // Инициализация драйвера UART-0             
  static CMenuNavigation menu_navigation(CTerminalUartDriver::getInstance(), rSysMgr);  // Пультовый терминал (менеджер меню).
  static CMessageDisplay mes_display(CTerminalUartDriver::getInstance());               // Пультовый терминал (менеджер сообщений).
  static CTerminalManager terminal_manager(menu_navigation, mes_display);               // Управление режимами пультового терминал
  menu_navigation.set_pTerminal(&terminal_manager);                                     // Создание циклической зависимости menu  
  mes_display.set_pTerminal(&terminal_manager);                                         // Создание циклической зависимости mes
  return terminal_manager;                                                              // Возврат ссылки на менеджер терминпла
}
extern "C" void UART0_IRQHandler(void) { CTerminalUartDriver::getInstance().irq_handler(); }  // Вызов обработчика UART-0

// Инициализация и создание объектов связанных с ИУ. Запуск СИФУ
CSIFU& CFactory::start_puls_system(CDMAcontroller& rCont_dma, CRegManager& rReg_manager) {
  static CADC adc(CSET_SPI::configure(CSET_SPI::ESPIInstance::SPI_1));  // Внешнее ADC. Подключено к SPI-1
  static CPULSCALC puls_calc(adc);                                      // Измерение и обработка всех аналоговых сигналов.
  static CSIFU sifu(puls_calc, rReg_manager);                           // СИФУ.  
  rReg_manager.getSIFU(&sifu);
  CSET_SPI::configure(CSET_SPI::ESPIInstance::SPI_2);   // Конфигурация SPI-2 для WiFi на ESP32
  static CREM_OSC rem_osc(                               // Дистанционный осциллограф (WiFi модуль на ESP32).                              
                          rCont_dma,                     // Контроллер DMA
                          puls_calc,                     // Измерение и обработка
                          CADC_STORAGE::getInstance());  // Данные АЦП
  
  CProxyPointerVar& Ppv = CProxyPointerVar::getInstance();
  Ppv.registerVar(                                       // Регистрация Alpha в реестре указателей
                  NProxyVar::ProxyVarID::AlphaCur, 
                  sifu.getPointerAlpha(), 
                  cd::Alpha, 
                  NProxyVar::Unit::Deg);
  
  CProxyHandlerTIMER::getInstance().set_pointers(&sifu, &rem_osc);  // Proxy Singleton доступа к Handler TIMER.
  sifu.init_and_start(); // Старт SIFU
  return sifu;
}

// Создание регуляторов и менеджера регуляторов
CRegManager CFactory::createRegManager() { 
  static CCurrentReg curr_reg(CEEPSettings::getInstance());
  static CQReg q_reg(CEEPSettings::getInstance());
  static CCosReg cos_reg(CEEPSettings::getInstance());
  static CRegManager reg_manager(curr_reg, q_reg, cos_reg); 
  return reg_manager; 
}


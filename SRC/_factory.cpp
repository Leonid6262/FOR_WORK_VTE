#include "crc16.hpp"
#include "factory.hpp"

LPC_UART_TypeDef* CFactory::initRS485_01() { return CSET_UART::configure(CSET_UART::EUartInstance::UART_1); } // For the control class
LPC_UART_TypeDef* CFactory::initRS485_02() { return CSET_UART::configure(CSET_UART::EUartInstance::UART_2); } // For the control class
CCAN CFactory::initCAN1()               { return CCAN(CCAN::ECAN_Id_Instance::CAN1_Id); }           // For the control class
CCAN CFactory::initCAN2()               { return CCAN(CCAN::ECAN_Id_Instance::CAN2_Id); }           // For the control class
CEMAC_DRV CFactory::createEMACdrv()     { return CEMAC_DRV(); }                                     // For the control class

CDAC0 CFactory::createDAC0()            { return CDAC0(); }                                        // DAC0 
CDAC_PWM CFactory::createPWMDac1()      { return CDAC_PWM(CDAC_PWM::EPWM_DACInstance::PWM_DAC1); } // DAC1 
CDAC_PWM CFactory::createPWMDac2()      { return CDAC_PWM(CDAC_PWM::EPWM_DACInstance::PWM_DAC2); } // DAC2 
 
CIADC CFactory::createIADC()            { return CIADC(); }                                     // Внутренее ADC.
CRTC CFactory::createRTC()              { return CRTC(); }                                      // Системные часы  

StatusRet CFactory::load_settings()     { return CEEPSettings::getInstance().loadSettings(); }  // Загрузка уставок

CDin_cpu CFactory::createDINcpu()       { return CDin_cpu(); }                                  // Дискретные входы контроллера                                      

// Создание объекта доступа к dIO доступных по SPI
CSPI_ports CFactory::createSPIports() { 
  return CSPI_ports(CSET_SPI::configure(CSET_SPI::ESPIInstance::SPI_0));
}

// Создание регуляторов и их менеджера
CRegManager CFactory::createRegManager() { 
  static CCurrentReg curr_reg(CEEPSettings::getInstance());
  static CQReg q_reg(CEEPSettings::getInstance());
  static CCosReg cos_reg(CEEPSettings::getInstance());
  return CRegManager(curr_reg, q_reg, cos_reg); 
}

// ModBus slave
CMBSLAVE CFactory::create_MBslave() {
  static CDMAcontroller cont_dma;       // Управление каналами DMA
  return CMBSLAVE(cont_dma);
}

// Запуск всей системы: System Manager + СИФУ 
CSystemManager& CFactory::start_system(CMBSLAVE& rModBusSlave) {
  // --- Регуляторы ---
  static auto reg_manager = CFactory::createRegManager();
  
  // --- СИФУ и его окружение ---
  static CADC adc(CSET_SPI::configure(CSET_SPI::ESPIInstance::SPI_1));
  static CPULSCALC puls_calc(adc);                                              
  static CFaultCtrlP fault_ctrl_p(CADC_STORAGE::getInstance(), CEEPSettings::getInstance());                      
  static CSIFU sifu(puls_calc, reg_manager, fault_ctrl_p, CEEPSettings::getInstance());
  reg_manager.getSIFU(&sifu);
  
  CSET_SPI::configure(CSET_SPI::ESPIInstance::SPI_2);
  static CREM_OSC rem_osc(rModBusSlave.rDMAc, puls_calc, CADC_STORAGE::getInstance());
  CProxyHandlerTIMER::getInstance().set_pointers(&sifu, &rem_osc);
  sifu.init_and_start();
  
  // --- System Manager ---
  static CAdjustmentMode adjustment(sifu, CEEPSettings::getInstance());
  static CReadyCheck ready_check(CADC_STORAGE::getInstance(), CDIN_STORAGE::getInstance());
  static CFaultCtrlF fault_ctrl_f(CDIN_STORAGE::getInstance());
  static CPuskMode pusk_mode;
  static CWorkMode work_mode;
  static CWarningMode warning_ctrl;
  
  static CSystemManager sys_manager(sifu, adjustment, ready_check, fault_ctrl_f, 
                                    pusk_mode, work_mode, warning_ctrl, reg_manager);
  
  ready_check.setSysManager(&sys_manager);
  fault_ctrl_f.setSysManager(&sys_manager);
  fault_ctrl_f.initEINT2();                       // Настройка прерывания аппаратной защиты IdMax hard

  CProxyHandlerEINT2::getInstance().set_pFaultCtrl(&fault_ctrl_f);
  fault_ctrl_p.setSysManager(&sys_manager); 
  
  return sys_manager;
}

// Инициализация драйвера ПТ и создание объектов Пультового терминала
CTerminalManager& CFactory::createTM(CSystemManager& rSysMgr) {   
  // Конфигурация и инициализация UART-0 - пультовый терминал
  LPC_UART_TypeDef* U0 = CSET_UART::configure(CSET_UART::EUartInstance::UART_0);  
  auto& udrv = CTerminalUartDriver::getInstance();
  udrv.init(U0, UART0_IRQn);                       
  
  // Вычисление коэффициентов отображения в системе СИ
  auto& set = CEEPSettings::getInstance().getSettings();
  cd::cdr.Id = cd::cd_r(set.set_params.IdNom, cd::ADC_DISCR_ID);
  cd::cdr.Ud = cd::cd_r(set.set_params.UdNom, cd::ADC_DISCR_UD);
  cd::cdr.IS = cd::cd_r(set.set_params.ISNom, cd::ADC_DISCR_IS);
  cd::cdr.US = cd::cd_r(set.set_params.USNom, cd::ADC_DISCR_US);
  
  static CRTC rt_clock;                                                         // Системные часы
  static CMenuNavigation menu_navigation(udrv, rSysMgr, rt_clock);              // Пультовый терминал (менеджер меню).
  static CMessageDisplay mes_display(udrv, rSysMgr, rt_clock);                  // Пультовый терминал (менеджер сообщений).
  static CTerminalManager terminal_manager(menu_navigation, mes_display);       // Управление режимами пультового терминал
  menu_navigation.set_pTerminal(&terminal_manager);                             // Создание циклической зависимости menu  
  mes_display.set_pTerminal(&terminal_manager);                                 // Создание циклической зависимости mes
  
  return terminal_manager;                                                      // Возврат ссылки на менеджер терминпла
}
extern "C" void UART0_IRQHandler(void) { CTerminalUartDriver::getInstance().irq_handler(); }  // Вызов обработчика UART-0


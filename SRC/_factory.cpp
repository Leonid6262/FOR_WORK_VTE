#include "factory.hpp"

#pragma data_section=".dma_buffers"
__root signed short CREM_OSC::tx_dma_buffer[CREM_OSC::TRANSACTION_LENGTH];
__root signed short CREM_OSC::rx_dma_buffer[CREM_OSC::TRANSACTION_LENGTH];
__root signed short CMBSLAVE::tx_dma_buffer[CMBSLAVE::TRANSACTION_LENGTH];
__root signed short CMBSLAVE::rx_dma_buffer[CMBSLAVE::TRANSACTION_LENGTH];
#pragma data_section

using ESET = CEEPSettings;
using EUART = CSET_UART::EUartInstance;
using ESPI = CSET_SPI::ESPIInstance;

CEMAC_DRV CFactory::createEMACdrv()   { return CEMAC_DRV(); }                 // For the control class
CDAC0 CFactory::createDAC0()          { return CDAC0(ESET::getInstance()); }  // DAC0. For system test
 
CIADC CFactory::createIADC()          { return CIADC(); }                                   // Внутренее ADC.
StatusRet CFactory::load_settings()   { return ESET::getInstance().loadSettings(); }        // Загрузка уставок
CDin_cpu CFactory::createDINcpu()     { return CDin_cpu(); }                                // Дискретные входы контроллера
CSPI_ports CFactory::createSPIports() { return CSPI_ports(CSET_SPI::config(ESPI::SPI_0)); } // R/W  dIO доступные по SPI
CIsoMeas CFactory::createIsoMeas()    { return CIsoMeas(); }                                // Измерение сопротивления изоляции 

// Создание регуляторов и их менеджера
CRegManager CFactory::createRegManager() { 
  static CCurrentReg curr_reg(ESET::getInstance());
  static CQReg q_reg(ESET::getInstance());
  static CCosReg cos_reg(ESET::getInstance());
  return CRegManager(curr_reg, q_reg, cos_reg); 
}

// ModBus slave
CMBSLAVE CFactory::create_MBslave() {
  static CDMAcontroller cont_dma;     // Управление каналами DMA
  return CMBSLAVE(cont_dma, CSET_UART::configure(EUART::UART_1));
}

// Запуск всей системы: System Manager + СИФУ 
CSystemManager& CFactory::start_system(CMBSLAVE& rModBusSlave) {
  // --- Регуляторы ---
  static auto reg_manager = CFactory::createRegManager();
  
  // --- СИФУ и его окружение ---
  static CADC adc(CSET_SPI::config(ESPI::SPI_1));
  static CDAC_PWM dac_cos(CDAC_PWM::EPWM_DAC::PWM_DAC1, ESET::getInstance());
  static CPULSCALC puls_calc(adc, CProxyPointerVar::getInstance(), dac_cos); 
  static CFaultCtrlP fault_ctrl_p(CADC_STORAGE::getInstance(), ESET::getInstance());                      
  static CSIFU sifu(puls_calc, reg_manager, fault_ctrl_p, ESET::getInstance());
  reg_manager.getSIFU(&sifu);
  CSET_SPI::config(ESPI::SPI_2);
  static CREM_OSC rem_osc(rModBusSlave.rDMAc, puls_calc, CADC_STORAGE::getInstance());
  CProxyHandlerTIMER::getInstance().set_pointers(&sifu, &rem_osc);
  sifu.init_and_start(CProxyPointerVar::getInstance());
  
  // --- System Manager ---
  static CAdjustmentMode adjustment(sifu, ESET::getInstance());
  static CReadyCheck ready_check(CADC_STORAGE::getInstance(), CDIN_STORAGE::getInstance());
  static CFaultCtrlF fault_ctrl_f(CDIN_STORAGE::getInstance());
  static CPuskMode pusk_mode;
  static CWorkMode work_mode;
  static CWarningMode warning_ctrl;
  
  static CSystemManager sys_manager(sifu, adjustment, ready_check, fault_ctrl_f, 
                                    pusk_mode, work_mode, warning_ctrl, reg_manager);
  
  ready_check.setSysManager(&sys_manager);
  fault_ctrl_f.setSysManager(&sys_manager);
  fault_ctrl_f.initEINT2();                       

  CProxyHandlerEINT2::getInstance().set_pFaultCtrl(&fault_ctrl_f);
  fault_ctrl_p.setSysManager(&sys_manager); 
  
  return sys_manager;
}

// Инициализация драйвера ПТ, создание объектов ПТ и его окружения
CTerminalManager& CFactory::createTM(CSystemManager& rSysMgr) {   
  // Конфигурация и инициализация UART-0 - пультовый терминал 
  auto& udrv = CTerminalUartDriver::getInstance();
  udrv.init(CSET_UART::configure(EUART::UART_0), UART0_IRQn);                       
  
  // Вычисление коэффициентов отображения в системе СИ
  auto& set = ESET::getInstance().getSettings();
  cd::cdr.Id = cd::cd_r(set.set_params.IdNom, cd::ADC_DISCR_ID);
  cd::cdr.Ud = cd::cd_r(set.set_params.UdNom, cd::ADC_DISCR_UD);
  cd::cdr.IS = cd::cd_r(set.set_params.ISNom, cd::ADC_DISCR_IS);
  cd::cdr.US = cd::cd_r(set.set_params.USNom, cd::ADC_DISCR_US);
  cd::cdr.P  = (cd::cdr.US * cd::cdr.IS) / 1000.0f;
  cd::cdr.Q  = cd::cdr.P;
  
  static CRTC rt_clock;                                                                 // Системные часы
  static CMenuNavigation menu_navigation(udrv, rSysMgr, rt_clock, ESET::getInstance()); // Пультовый терминал (навигация по меню).
  static CMessageDisplay mes_display(udrv, rSysMgr, rt_clock);                          // Пультовый терминал (индикация сообщений).
  static CTerminalManager terminal_manager(menu_navigation, mes_display);               // Управление режимами пультового терминал
  menu_navigation.set_pTerminal(&terminal_manager);                                     // Создание циклической зависимости menu  
  mes_display.set_pTerminal(&terminal_manager);                                         // Создание циклической зависимости mes
  
  return terminal_manager;                                                              // Возврат ссылки на менеджер терминпла
}
extern "C" void UART0_IRQHandler(void) { CTerminalUartDriver::getInstance().irq_handler(); }  // Вызов обработчика UART-0


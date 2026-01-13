#include "main.hpp"

void UserStartInit() {
  CSET_PORTS::initDOutputs(); // Инициализация дискретных выходов микроконтроллера (pins)
  CDIN_STORAGE::UserLedOn();  // Визуальный контроль начала инициализации
  CSET_EMC::initAndCheck();   // Инициализации ext RAM и шины расширения. Контроль ext RAM
}

void main(void) {
  
  Priorities::initPriorities();                         // Распределение векторов по группам. см. в файле IntPriority.h
  CSET_TIMER::initTimers();                             // Инициализация таймеров.
  
  if (CFactory::load_settings() == StatusRet::ERROR) {  // Загрузка уставок (RAM <- EEPROM)   
    SWarning::setMessage(EWarningId::DEFAULT_SET);      // При ошибке - собщение: "Загружены дефолтные уставки" 
  }
SWarning::setMessage(EWarningId::DEFAULT_SET); 
  static auto int_adc = CFactory::createIADC();         // Внутренее ADC.
  static auto spi_ports = CFactory::createSPIports();   // Входы и выходы доступные по SPI.
  static auto din_cpu = CFactory::createDINcpu();       // Дискретные входы контроллера (порты Pi0 и Pi1 по аналогии с СМ3) 
                                                        // Выходы контроллера (порт Po0 по аналогии с СМ3 в dIOStorage.hpp)
  static auto cont_dma = CFactory::createDMAc();        // Управление каналами DMA.
               
  static auto reg_manager = CFactory::createRegManager();                   // Менеджер регуляторов
  static auto& rSIFU = CFactory::start_puls_system(cont_dma, reg_manager);  // Запуск СИФУ и всех её зависимостей.
  static auto& sys_manager = CFactory::createSysManager(rSIFU, reg_manager);// Системный менеджер
  static auto& term_manager = CFactory::createTM(sys_manager);              // Управление объектами ПТ
  
  CDIN_STORAGE::UserLedOff();                           // Визуальный контроль окончания инициализации
  
  while (true) {  
    
    int_adc.measure_5V();       // Измерение напряжения питания +/- 5V (внутреннее ADC)
    din_cpu.input_Pi0();        // Чтение состояния дискретных входов контроллера Pi0
    spi_ports.rw();             // Запись в дискретные выходы и чтение дискретных входов доступных по SPI
    term_manager.dispatch();    // Управление объектами (режимами) пультового терминал   
    sys_manager.dispatch();     // Управление объектами (режимами) устройства 
    
    Pause_us(3);
  }
}

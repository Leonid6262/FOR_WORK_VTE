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

  static auto int_adc = CFactory::createIADC();         // Внутренее ADC.
  static auto spi_ports = CFactory::createSPIports();   // Входы и выходы доступные по SPI.
  static auto din_cpu = CFactory::createDINcpu();       // Дискретные входы контроллера (порты Pi0 и Pi1 по аналогии с СМ3)                                                          
  static auto mb_slave = CFactory::create_MBslave();    // ModBus slave
  
  static auto& sys_manager = CFactory::start_system(mb_slave);  // Создание всех менеджеров и запуск СИФУ 
  static auto& term_manager = CFactory::createTM(sys_manager);  // Управление объектами ПТ
  
  CDIN_STORAGE::UserLedOff();                           // Визуальный контроль окончания инициализации
  
  while (true) {  
    
    int_adc.measure_5V();       // Измерение напряжения питания +/- 5V (внутреннее ADC)
    din_cpu.input_Pi0();        // Чтение состояния дискретных входов контроллера Pi0
    spi_ports.rw();             // Запись в дискретные выходы и чтение дискретных входов доступных по SPI
    term_manager.dispatch();    // Управление объектами (режимами) пультового терминал   
    sys_manager.dispatch();     // Управление объектами (режимами) устройства 
    mb_slave.monitor();         // Мониторинг запросов по ModBus
    
  }
}

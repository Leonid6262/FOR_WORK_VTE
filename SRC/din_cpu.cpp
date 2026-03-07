#include "din_cpu.hpp"
#include "dIOStorage.hpp"

CDin_cpu::CDin_cpu(CGPIO& gpio) : gpio(gpio) {
  prev_TC0 = LPC_TIM0->TC;
}

void CDin_cpu::input_Pi0() {
  // Входные данные порта Pi0
  data_din_Pi0 = ~static_cast<unsigned char>(gpio.get_pin() >> bg::B0_PORT_IN);
  // Фильтр (интегратор входного сигнала) и фиксация в CDIN_STORAGE
  unsigned int dT = LPC_TIM0->TC - prev_TC0;
  prev_TC0 = LPC_TIM0->TC;
  CDIN_STORAGE::getInstance().filter(data_din_Pi0, dT, 
                                     static_cast<unsigned char>(CDIN_STORAGE::EIBNumber::CPU_PORT),
                                     CEEPSettings::getInstance());
}


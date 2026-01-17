#include "din_cpu.hpp"

#include "dIOStorage.hpp"

CDin_cpu::CDin_cpu() { 
  prev_TC0 = LPC_TIM0->TC;
}

void CDin_cpu::input_Pi0() {
  // Входные данные порта Pi0
  data_din_Pi0 = ~static_cast<unsigned char>(LPC_GPIO2->PIN >> B0_PORT0_IN);
  // Фильтр (интегратор входного сигнала) и фиксация в CDIN_STORAGE
  unsigned int dT = LPC_TIM0->TC - prev_TC0;
  prev_TC0 = LPC_TIM0->TC;
  CDIN_STORAGE::getInstance().filter(data_din_Pi0, dT, 
                                     static_cast<unsigned char>(CDIN_STORAGE::EIBNumber::CPU_PORT),
                                     CEEPSettings::getInstance());
}

// Порт Pi1, по сути, набор входов прерываний. Не фильтруется как Pi0
// data_inp_Pi1 - нужен, только для визуального контроля на ПТ
void CDin_cpu::input_Pi1() {
  data_din_Pi1 = static_cast<unsigned char>(((LPC_GPIO2->PIN >> B0_PORT1_IN) & MASK_G1) |
                                            ((LPC_GPIO0->PIN >> (B3_PORT1_IN - BITS_G1)) & MASK_G2));
}

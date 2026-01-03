#include "dIOStorage.hpp"
#include "proxy_pointer_var.hpp"

CDIN_STORAGE::CDIN_STORAGE() {
  // Инициализация фильтра
  for (char n = 0; n < (G_CONST::BYTES_RW_MAX + 1); n++) {
    for (char b = 0; b < N_BITS; b++) {
      integrator[n][b] = 1;
    }
  }
  // Регистрация в реестре указателей
  CProxyPointerVar& Ppv = CProxyPointerVar::getInstance();
  Ppv.registerVar
    (
     NProxyVar::ProxyVarID::pi0_cpu,
     &CDIN_STORAGE::UData_din_f[static_cast<unsigned char>(EIBNumber::CPU_PORT)].all, 
     cd::one,                                             
     NProxyVar::Unit::bits);
  Ppv.registerVar
    (
     NProxyVar::ProxyVarID::pi0_spi,
     &CDIN_STORAGE::UData_din_f[static_cast<unsigned char>(EIBNumber::CPU_SPI)].all, 
     cd::one,                                             
     NProxyVar::Unit::bits);
};

void CDIN_STORAGE::filter(unsigned char data_din_Pi, unsigned int dT, unsigned char n_port) {
  // Входные данные порта  с учётом инверсии
  unsigned char data_din_invert = data_din_Pi ^ 
    CEEPSettings::getInstance().getSettings().din_Pi_invert[n_port];

  // Фильтр 8-ми бит
  for (char b = 0; b < N_BITS; b++) {
    if (data_din_invert & (1UL << b)) {
      // Если на входе 1
      integrator[n_port][b] += dT;
      if (integrator[n_port][b] >= cConst_integr_Pi0[n_port][b]) {
        // Если интегратор в насыщении, на выходе 1
        integrator[n_port][b] = cConst_integr_Pi0[n_port][b];
        UData_din_f[n_port].all |= (1UL << b);  // Обновляем бит
      }
    } else {
      // Если на входе 0
      integrator[n_port][b] -= dT;
      if (integrator[n_port][b] <= 0) {
        // Если интегратор в нуле, на выходе 0
        integrator[n_port][b] = 0;
        UData_din_f[n_port].all &= ~(1UL << b);  // Обновляем бит
      }
    }
  }
}
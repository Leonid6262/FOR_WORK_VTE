#include "AdcStorage.hpp"

#include "proxy_pointer_var.hpp"

CADC_STORAGE::CADC_STORAGE() : settings(CEEPSettings::getInstance()) {
  // Регистрация в реестре указателей
  CProxyPointerVar& Ppv = CProxyPointerVar::getInstance();
  Ppv.registerVar(NProxyVar::ProxyVarID::Irotor,
                  getEPointer(CADC_STORAGE::ROTOR_CURRENT), 
                  cd::IRotor,
                  NProxyVar::Unit::Amp);
  
  Ppv.registerVar(NProxyVar::ProxyVarID::Urotor,
                  getEPointer(CADC_STORAGE::ROTOR_VOLTAGE), 
                  cd::URotor,
                  NProxyVar::Unit::Volt);
};

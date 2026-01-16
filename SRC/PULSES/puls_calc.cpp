#include "puls_calc.hpp"
#include <string>
#include "AdcStorage.hpp"

CPULSCALC::CPULSCALC(CADC& rAdc, CProxyPointerVar& PPV) : rAdc(rAdc) {
  v_restoration.ind_d_avr = 0;

  // Регистрация в реестре указателейconst
  PPV.registerVar(NProxyVar::ProxyVarID::UstatRms, &U_STATORA, 
                  cd::cdr.US, NProxyVar::Unit::Volt);
  PPV.registerVar(NProxyVar::ProxyVarID::IstatRms, &I_STATORA, 
                  cd::cdr.IS, NProxyVar::Unit::Amp);
}

void CPULSCALC::conv_and_calc() {
  // Измерение всех используемых (в ВТЕ) аналоговых сигналов (внешнее ADC)
  rAdc.conv_tnf({
    CADC_STORAGE::ROTOR_CURRENT, 
    CADC_STORAGE::STATOR_VOLTAGE, 
    CADC_STORAGE::STATOR_CURRENT,
    CADC_STORAGE::ROTOR_VOLTAGE, 
    CADC_STORAGE::EXT_SETTINGS, 
    CADC_STORAGE::LE_CURRENT,
    CADC_STORAGE::NODE_CURRENT});
  /*
  Для сокращения записи аргументов здесь использована си нотация enum, вмесо типобезопасной enum class c++.
  CADC_STORAGE::ROTOR_CURRENT вместо static_cast<char>(CADC_STORAGE::EADC_NameCh::ROTOR_CURRENT) - считаю, разумный
  компромисс. Пример доступа к измеренным значениям - pStorage.getExternal[CADC_STORAGE::ROTOR_CURRENT]
  */

  sin_restoration();
}
void CPULSCALC::sin_restoration() {
  /*
  Восстановление сигналов произвадится по двум мгновенным значениям и углу (Theta) между ними:
  A = sqrt( (u1*u1 + u2*u2 - 2 * u1*u2 * cos(Theta)) / (sin(Theta) * sin(Theta)) );
  */

  auto& pStorage = CADC_STORAGE::getInstance();

  // Напряжение статора
  v_restoration.u_stator_2 = pStorage.getExternal(CADC_STORAGE::STATOR_VOLTAGE);
  v_restoration.timing_ustator_2 = pStorage.getTimings(CADC_STORAGE::STATOR_VOLTAGE + 1);

  unsigned int us1us1 = v_restoration.u_stator_1 * v_restoration.u_stator_1;
  unsigned int us2us2 = v_restoration.u_stator_2 * v_restoration.u_stator_2;
  signed int us1us2 = v_restoration.u_stator_1 * v_restoration.u_stator_2;

  v_restoration.dT_ustator = v_restoration.timing_ustator_2 - v_restoration.timing_ustator_1;

  v_restoration.u_stator_1 = v_restoration.u_stator_2;
  v_restoration.timing_ustator_1 = v_restoration.timing_ustator_2;

  float u_theta = (2.0f * v_restoration.pi * v_restoration.freq * v_restoration.dT_ustator) / 1000000.0f;

  float ucos = std::cos(u_theta);
  float usin = std::sin(u_theta);

  // Ток статора
  v_restoration.i_stator_2 = pStorage.getExternal(CADC_STORAGE::STATOR_CURRENT);
  v_restoration.timing_istator_2 = pStorage.getTimings(CADC_STORAGE::STATOR_CURRENT + 1);

  unsigned int is1is1 = v_restoration.i_stator_1 * v_restoration.i_stator_1;
  unsigned int is2is2 = v_restoration.i_stator_2 * v_restoration.i_stator_2;
  signed int is1is2 = v_restoration.i_stator_1 * v_restoration.i_stator_2;

  v_restoration.dT_istator = v_restoration.timing_istator_2 - v_restoration.timing_istator_1;

  v_restoration.i_stator_1 = v_restoration.i_stator_2;
  v_restoration.timing_istator_1 = v_restoration.timing_istator_2;

  float i_theta = (2.0f * v_restoration.pi * v_restoration.freq * v_restoration.dT_istator) / 1000000.0f;

  float icos = std::cos(i_theta);
  float isin = std::sin(i_theta);

  // Скользящее среднее по 6-ти пульсам
  v_restoration.ind_d_avr = (v_restoration.ind_d_avr + 1) % v_restoration.PULS_AVR;

  v_restoration.u_stat[v_restoration.ind_d_avr] = sqrt(((us1us1 + us2us2) - (us1us2 * 2 * ucos)) / (usin * usin));
  float uavr = (v_restoration.u_stat[0] + 
                v_restoration.u_stat[1] + 
                v_restoration.u_stat[2] + 
                v_restoration.u_stat[3] +
                v_restoration.u_stat[4] + 
                v_restoration.u_stat[5]) / v_restoration.PULS_AVR;
  U_STATORA = static_cast<int>(uavr + 0.5f);

  v_restoration.i_stat[v_restoration.ind_d_avr] = sqrt(((is1is1 + is2is2) - (is1is2 * 2 * icos)) / (isin * isin));
  float iavr = (v_restoration.i_stat[0] + 
                v_restoration.i_stat[1] + 
                v_restoration.i_stat[2] + 
                v_restoration.i_stat[3] +
                v_restoration.i_stat[4] + 
                v_restoration.i_stat[5]) / v_restoration.PULS_AVR;
  I_STATORA = static_cast<int>(iavr + 0.5f);
}

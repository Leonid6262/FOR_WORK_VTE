#pragma once

#include "adc.hpp"
#include "proxy_pointer_var.hpp"
#include "dac.hpp"

class CPULSCALC {
 public:
  CPULSCALC(CADC&, CProxyPointerVar&, CDAC_PWM&);

  CADC& rAdc;
  CDAC_PWM& dac_cos;
  void conv_and_calc();

  inline signed short* getPointer_USTATORA() { return &U_STATORA; }
  inline signed short* getPointer_ISTATORA() { return &I_STATORA; }

 private:
  signed short U_STATORA;
  signed short I_STATORA;

  struct RestorationState {
    static constexpr char PULS_AVR = 6;   // Пульсов усреднения
    static constexpr float freq = 50.0f;  // Частота сети
    static constexpr float pi = 3.141592653589793;

    float u_stat[PULS_AVR];
    float i_stat[PULS_AVR];
    char ind_d_avr;
    signed short u_stator_1;
    signed short u_stator_2;
    unsigned int timing_ustator_1;
    unsigned int timing_ustator_2;
    unsigned short dT_ustator;

    signed short i_stator_1;
    signed short i_stator_2;
    unsigned int timing_istator_1;
    unsigned int timing_istator_2;
    unsigned short dT_istator;

  } v_restoration;

  void sin_restoration();
};

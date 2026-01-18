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

  inline unsigned short* getPointer_AUSTATORA() { return &AU_STATORA; }
  inline unsigned short* getPointer_AISTATORA() { return &AI_STATORA; }

 private:
  float au_stator; 
  unsigned short AU_STATORA;
  float ai_stator;
  unsigned short AI_STATORA;
  float PHI;
  float COS_PHI;
  float P;
  float Q;

  struct RestorationState {
    static constexpr char PULS_AVR = 6;   // Пульсов усреднения
    static constexpr float freq = 50.0f;  // Частота сети
    static constexpr float pi = 3.141592653589793;

    float u_stat[PULS_AVR];
    float i_stat[PULS_AVR];
    char ind_d_avr;
    
    float u_stator_1;
    float u_stator_2;
    unsigned int timing_ustator_1;
    unsigned int timing_ustator_2;
    unsigned int dT_ustator;

    float i_stator_1;
    float i_stator_2;
    unsigned int timing_istator_1;
    unsigned int timing_istator_2;
    unsigned int dT_istator;

  } v_restoration;

  void sin_restoration();
};

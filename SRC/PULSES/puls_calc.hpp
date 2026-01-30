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

  inline unsigned short* getPointer_USTATOR_RMS() { return &U_STATOR_RMS; }
  inline unsigned short* getPointer_ISTATOR_RMS() { return &I_STATOR_RMS; }
  inline float* getPointer_ustator_rms() { return &u_stator_rms; }
  inline float* getPointer_istator_rms() { return &i_stator_rms; }

 private:
  float u_stator_rms; 
  unsigned short U_STATOR_RMS;
  float i_stator_rms;
  unsigned short I_STATOR_RMS;
  float phi;
  float cos_phi;
  unsigned short COS_PHI;
  float p;
  unsigned short P;
  float q;
  signed short Q;

  struct RestorationState {
    static constexpr char PULS_AVR = 12;  // Пульсов усреднения
    static constexpr float freq = 50.0f;  // Частота сети
    static constexpr float pi = 3.141592653589793;
    static constexpr float sqrt_2 = 1.414213562373095;

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

  } v_rest;

  void sin_restoration();
};

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

  inline bool getSlipEvent()      { return v_slip.slip_event; } 
  inline bool getU0Event()        { return v_slip.u0_event; }
  inline float getSlipValue()     { return v_slip.slip_value; }
  inline float* getPointer_slip() { return &v_slip.slip_value; }
  inline void resSlipEvent()      { v_slip.slip_event = false; }
  inline void resU0Event()        { v_slip.u0_event = false; }
  inline void setSlipPermission() { v_slip.Permission = true; }
  inline void clrSlipPermission() { v_slip.Permission = false; }
  
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
    char ind_d_avr = 0;
    
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
  
  struct SlipState {
    
    bool Permission = false;
    
    static constexpr char N_FRAME = 10; // Длина бегущего кадра
    signed short ud_frame[N_FRAME];     // Бегущий кадр
    unsigned char ind_ud_fram = 0;      // Индекс бегущего кадра           
    signed int sum_ud_frame = 0;        // Бегущая сумма
    
    signed int neg_accumulator = 0;     // Сумма отрицательных значений
    unsigned short  neg_samples = 0;    // Счетчик отсчетов в минусе
    bool collecting_neg_wave = false;   // Данные минуса накапливаются 
    unsigned short delta_adaptive;      // Адаптивный порог
        
    unsigned short nT_slip = 0;         // Счётчик перида скольжения
    bool detected_latch = false;        // Защёлка минуса
    
    bool slip_event = false;
    bool u0_event = false;
    float slip_value = 1.0f;
    
    static constexpr char min_neg_samples = 5;                // Минимальная длина полуволны
    static constexpr unsigned short min_delta_adaptive = 20;  // Минимальная дельта
    static constexpr unsigned short max_delta_adaptive = 500; // Максимальная дельта
    
  } v_slip; 
  
  void sin_restoration();
  void detectRotorPhaseAdaptive();
  
};

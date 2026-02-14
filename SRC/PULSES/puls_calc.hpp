#pragma once

#include "adc.hpp"
#include "proxy_pointer_var.hpp"
#include "dac.hpp"

class CRegManager;

class CPULSCALC {
 public:
  CPULSCALC(CADC&, CProxyPointerVar&, CDAC_PWM&, CRegManager&, CADC_STORAGE&);

  void conv_and_calc();
  void conv_Id();

  inline unsigned short* getPointer_USTATOR_RMS() { return &U_STATOR_RMS; }
  inline unsigned short* getPointer_ISTATOR_RMS() { return &I_STATOR_RMS; }
  inline float* getPointer_ustator_rms() { return &u_stator_rms; }
  inline float* getPointer_istator_rms() { return &i_stator_rms; }
  inline float* getPointer_S_Power()     { return &S_Power;      }
  inline float* getPointer_P_Power()     { return &P_Power;      }  
  inline float* getPointer_Q_Power()     { return &Q_Power;      }
  inline float* getPointer_CosPhi()      { return &cos_phi;      }  
  
  inline bool getSlipEvent()      { return v_slip.slip_event; } 
  inline bool getU0Event()        { return v_slip.u0_event; }
  inline float getSlipValue()     { return v_slip.slip_value; }
  inline float* getPointer_slip() { return &v_slip.slip_value; }
  inline void resSlipEvent()      { v_slip.slip_event = false; }
  inline void resU0Event()        { v_slip.u0_event = false; }
  
  inline void clearDetectRotorPhase() { 
    v_slip.Permission = true;    
    for(char i = 0; i < v_slip.N_FRAME; i++) v_slip.ud_frame[i] = 0;
    v_slip.slip_value = 1.0f;
    v_slip.nT_slip = 0;             
    v_slip.sum_ud_frame = 0;
    v_slip.ind_ud_fram = 0;    
    v_slip.tick_wait = 0;
    v_slip.target_tick = 0;
    
    v_slip.sum_pos_wave = 0;
    v_slip.sum_neg_wave = 0;
    
    v_slip.neg_wave = false; 
    v_slip.u0_event = false;      
    v_slip.wait_for_event = false;
    v_slip.slip_event = false;
    v_slip.u0_event = false; 
    
  }
  
  inline void stopDetectRotorPhase() { v_slip.Permission = false; }
  
 private:
  float u_stator_rms; 
  float i_stator_rms;
  float cos_phi;
  float sin_phi;

  float P_Power;
  float Q_Power; 
  float S_Power; 
  
    signed short COS_PHI;
  unsigned short U_STATOR_RMS;
  unsigned short I_STATOR_RMS;
  unsigned short S_POWER;
  unsigned short P_POWER;
    signed short Q_POWER;

  struct RestorationState {
    static constexpr char PULS_AVR = 12;  // Пульсов усреднения
    static constexpr float freq = 50.0f;  // Частота сети
    static constexpr float pi = 3.141592653589793;
    static constexpr float sqrt_2 = 1.414213562373095;

    float u_stat[PULS_AVR];
    float i_stat[PULS_AVR];
    float phi_buf[PULS_AVR];
    char ind_d_avr = 0;
    
    float u_stator_1;
    float u_stator_2;
    unsigned int timing_ustator_1;
    unsigned int timing_ustator_2;
    unsigned int dT_ustator;
    float Um;

    float i_stator_1;
    float i_stator_2;
    unsigned int timing_istator_1;
    unsigned int timing_istator_2;
    unsigned int dT_istator;
    float Im;
    
    float phi_deg;

  } v_rest;
  
  struct SlipState {
    
    bool Permission = false;
    
    static constexpr char N_FRAME = 8;  // Длина бегущего кадра
    signed short ud_frame[N_FRAME];     // Бегущий кадр
    unsigned char ind_ud_fram = 0;      // Индекс бегущего кадра           
    signed int sum_ud_frame = 0;        // Бегущая сумма
    
    bool neg_wave = false;              // Отрицательная полуволна 
       
    unsigned short nT_slip = 0;         // Счётчик перида скольжения
    
    bool slip_event = false;
    float slip_value = 1.0f;
    bool u0_event = false;
 
    bool wait_for_event = false;
    unsigned short tick_wait = 0;
    unsigned short target_tick = 0;
    
    unsigned short sum_pos_wave = 0;
    unsigned short sum_neg_wave = 0;
        
    static constexpr unsigned short min_nT_slip = 30;  // Минимальная длина периода (10Hz)
    static constexpr unsigned short max_nT_slip = 300; // Максимольная длина периода (1Hz)   
    static constexpr unsigned char delay_rc = 4;       // Задержка RC фильтра
    static constexpr unsigned char Depth_30deg = 30;   // Глубина 30 градусов

  } v_slip; 
  
  void sin_restoration();
  void detectRotorPhaseFixed();
  
  CADC& rAdc;
  CDAC_PWM& dac_cos;
  CADC_STORAGE& rStrADC;  
  CRegManager& rReg_manager; 
  
};

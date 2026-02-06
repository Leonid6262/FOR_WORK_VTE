#include "puls_calc.hpp"
#include "AdcStorage.hpp"
#include <string>

CPULSCALC::CPULSCALC(CADC& rAdc, CProxyPointerVar& PPV, CDAC_PWM& dac_cos) : rAdc(rAdc), dac_cos(dac_cos) {

  // Регистрация в реестре указателей
  PPV.registerVar(NProxyVar::ProxyVarID::Ustat,  &U_STATOR_RMS,  cd::cdr.US, NProxyVar::Unit::Volt);
  PPV.registerVar(NProxyVar::ProxyVarID::Istat,  &I_STATOR_RMS,  cd::cdr.IS, NProxyVar::Unit::Amp);
  PPV.registerVar(NProxyVar::ProxyVarID::P,      &P,             cd::cdr.P,  NProxyVar::Unit::kW);
  PPV.registerVar(NProxyVar::ProxyVarID::Q,      &Q,             cd::cdr.Q,  NProxyVar::Unit::kVAR);
  PPV.registerVar(NProxyVar::ProxyVarID::CosPhi, &COS_PHI,       0.01f,      "");
}

void CPULSCALC::conv_and_calc() {
  // Измерение всех используемых (в ВТЕ) аналоговых сигналов (внешнее ADC)
  rAdc.conv_tnf({
    CADC_STORAGE::ROTOR_CURRENT, 
    CADC_STORAGE::STATOR_VOLTAGE, 
    CADC_STORAGE::STATOR_CURRENT,
    CADC_STORAGE::ROTOR_VOLTAGE, 
    CADC_STORAGE::LE_CURRENT,
    CADC_STORAGE::NODE_CURRENT,
    CADC_STORAGE::EXT_SETTINGS, });
  /*
  Для сокращения записи аргументов здесь использована си нотация enum, вмесо типобезопасной enum class c++.
  CADC_STORAGE::ROTOR_CURRENT вместо static_cast<char>(CADC_STORAGE::EADC_NameCh::ROTOR_CURRENT) - считаю, разумный
  компромисс. Пример доступа к измеренным значениям - pStorage.getExternal[CADC_STORAGE::ROTOR_CURRENT]
  */

  sin_restoration();
  detectRotorPhaseAdaptive();
  
}

// ---Адаптивный алгоритм определения перехода напряжения ротора через ноль (с минус в плюс)---
// ---Используется в режиме пуска двигателя---
void CPULSCALC::detectRotorPhaseAdaptive() {
  
  if(!v_slip.Permission) {        
    v_slip.slip_event = false;
    v_slip.u0_event = false;
    v_slip.slip_value = 1.0f;
    v_slip.nT_slip = 0;       
    v_slip.neg_accumulator = 0;
    v_slip.neg_samples = 0;
    v_slip.collecting_neg_wave = false; 
    v_slip.u0_event = false;
    v_slip.collecting_neg_wave = false;        
    v_slip.detected_latch = false;        
    return;
  }
  
  v_slip.nT_slip++; // Считаем перид скольжения
  
  // --- 1. Работа с кадром (бегущая сумма) ---
  short new_val = CADC_STORAGE::getInstance().getExternal(CADC_STORAGE::ROTOR_VOLTAGE);  
  v_slip.sum_ud_frame -= v_slip.ud_frame[v_slip.ind_ud_fram];    
  v_slip.ud_frame[v_slip.ind_ud_fram] = new_val;
  v_slip.sum_ud_frame += v_slip.ud_frame[v_slip.ind_ud_fram];   
  v_slip.ind_ud_fram = (v_slip.ind_ud_fram + 1) % v_slip.N_FRAME; 
  
  // --- 2. ТОЧКА СОБЫТИЯ (Положительная область + Порог) ---
  if (v_slip.sum_ud_frame > v_slip.delta_adaptive) {
    if (!v_slip.detected_latch) {
      v_slip.detected_latch = true;                                   // Защёлка на одну полуволну 
      v_slip.slip_value = 6.0f / static_cast<float>(v_slip.nT_slip);  // Расчёт скольжения 6 = 20 / 3.333
      v_slip.slip_event = true;                                       // Заданная глубина достигнута
      v_slip.nT_slip = 0;                                             // Сброс счётчика
    }
  }
  
  // --- 3. Отрицательная область ---
  if (v_slip.sum_ud_frame < 0) {
    v_slip.neg_accumulator += abs(new_val);    // Накапливаем "вес" минуса
    v_slip.neg_samples++;                      // Считаем отсчёты
    v_slip.collecting_neg_wave = true;         // Данные минуса накапливаются 
    v_slip.detected_latch = false;             // Сброс защёлки (готовимся к новому плюсу)
  } 
  
  // --- 4. Выход из минуса в ноль ---
  else if (v_slip.collecting_neg_wave) {
    // Если полуволна была достаточно длинной (защита от шума)
    if (v_slip.neg_samples > v_slip.min_neg_samples) { 
      
      // Вычисляем порог для следующего плюса
      float avg_u = static_cast<float>(v_slip.neg_accumulator) / static_cast<float>(v_slip.neg_samples);
      v_slip.delta_adaptive = static_cast<int>(avg_u * v_slip.k_depth + 0.5f);
      
      // Санитарные границы
      if (v_slip.delta_adaptive < v_slip.min_delta_adaptive)  v_slip.delta_adaptive = v_slip.min_delta_adaptive;  
      if (v_slip.delta_adaptive > v_slip.max_delta_adaptive)  v_slip.delta_adaptive = v_slip.max_delta_adaptive;
      
      v_slip.u0_event = true; // Служебное событие
      
    }
    
    // Очистка накопителей статистики
    v_slip.neg_accumulator = 0;
    v_slip.neg_samples = 0;
    v_slip.collecting_neg_wave = false; 
  }
  
}

void CPULSCALC::sin_restoration() {
  /*
  Восстановление сигналов произвадится по двум мгновенным значениям и углу (Theta) между ними:
  A = sqrt( (u1*u1 + u2*u2 - 2 * u1*u2 * cos(Theta)) / (sin(Theta) * sin(Theta)) );
  */

  auto& pStorage = CADC_STORAGE::getInstance();

  // Напряжение статора
  v_rest.u_stator_2 = pStorage.getExternal(CADC_STORAGE::STATOR_VOLTAGE);
  v_rest.timing_ustator_2 = pStorage.getTimings(CADC_STORAGE::STATOR_VOLTAGE + 1);

  float us1us1 = v_rest.u_stator_1 * v_rest.u_stator_1;
  float us2us2 = v_rest.u_stator_2 * v_rest.u_stator_2;
  float us1us2 = v_rest.u_stator_1 * v_rest.u_stator_2;

  v_rest.dT_ustator = v_rest.timing_ustator_2 - v_rest.timing_ustator_1;

  v_rest.u_stator_1 = v_rest.u_stator_2;
  v_rest.timing_ustator_1 = v_rest.timing_ustator_2;

  float u_theta = (2.0f * v_rest.pi * v_rest.freq * v_rest.dT_ustator) / 1000000.0f;

  float ucos = std::cos(u_theta);
  float usin = std::sin(u_theta);

  // Ток статора
  v_rest.i_stator_2 = pStorage.getExternal(CADC_STORAGE::STATOR_CURRENT);
  v_rest.timing_istator_2 = pStorage.getTimings(CADC_STORAGE::STATOR_CURRENT + 1);

  float is1is1 = v_rest.i_stator_1 * v_rest.i_stator_1;
  float is2is2 = v_rest.i_stator_2 * v_rest.i_stator_2;
  float is1is2 = v_rest.i_stator_1 * v_rest.i_stator_2;

  v_rest.dT_istator = v_rest.timing_istator_2 - v_rest.timing_istator_1;

  v_rest.i_stator_1 = v_rest.i_stator_2;
  v_rest.timing_istator_1 = v_rest.timing_istator_2;

  float i_theta = (2.0f * v_rest.pi * v_rest.freq * v_rest.dT_istator) / 1000000.0f;

  float icos = std::cos(i_theta);
  float isin = std::sin(i_theta);

  // Скользящее среднее
  v_rest.ind_d_avr = (v_rest.ind_d_avr + 1) % v_rest.PULS_AVR;
  float cur_u_stat = sqrt(((us1us1 + us2us2) - (us1us2 * 2 * ucos)) / (usin * usin));
  v_rest.u_stat[v_rest.ind_d_avr] = cur_u_stat;
  
  float avr = 0;
  for(char u = 0; u < v_rest.PULS_AVR; u++) {
    avr += v_rest.u_stat[u];
  }
  avr = avr / v_rest.PULS_AVR;
  
  u_stator_rms = avr/v_rest.sqrt_2;
  U_STATOR_RMS = static_cast<unsigned short>((avr/v_rest.sqrt_2) + 0.5f);
  
  float cur_i_stat = sqrt(((is1is1 + is2is2) - (is1is2 * 2 * icos)) / (isin * isin));
  v_rest.i_stat[v_rest.ind_d_avr] = cur_i_stat;
  
  avr = 0;
  for(char i = 0; i < v_rest.PULS_AVR; i++) {
    avr += v_rest.i_stat[i];
  }
  avr = avr / v_rest.PULS_AVR;
  i_stator_rms = avr/v_rest.sqrt_2;
  I_STATOR_RMS = static_cast<unsigned short>((avr/v_rest.sqrt_2) + 0.5f);
  
  float u_phi = std::atan2((v_rest.u_stator_2*ucos - v_rest.u_stator_1),  v_rest.u_stator_2*usin);
  float i_phi = std::atan2((v_rest.i_stator_2*icos - v_rest.i_stator_1),  v_rest.i_stator_2*isin);
  phi = u_phi - i_phi;
  
  cos_phi = std::cos(phi);  
  p = cur_u_stat * cur_i_stat * cos_phi / 2.0f;
  q = cur_u_stat * cur_i_stat * std::sin(phi) / 2.0f;
  
  COS_PHI = static_cast<int>((cos_phi*100) + 0.5f);
  P = static_cast<unsigned short>(p + 0.5f);
  Q = static_cast<signed short>(q + 0.5f);
}

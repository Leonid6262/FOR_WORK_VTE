#include "puls_calc.hpp"
#include "AdcStorage.hpp"
#include "RegManager.hpp"

#include <string>

CPULSCALC::CPULSCALC(CADC& rAdc, CProxyPointerVar& PPV, CDAC_PWM& dac_cos, CRegManager& rReg_manager, CADC_STORAGE& rStrADC) : 
  rAdc(rAdc), dac_cos(dac_cos), rReg_manager(rReg_manager), rStrADC(rStrADC) {

  // Регистрация в реестре указателей
  PPV.registerVar(NProxyVar::ProxyVarID::Ustat,  &U_STATOR_RMS,  cd::cdr.US, NProxyVar::Unit::Volt);
  PPV.registerVar(NProxyVar::ProxyVarID::Istat,  &I_STATOR_RMS,  cd::cdr.IS, NProxyVar::Unit::Amp);
  PPV.registerVar(NProxyVar::ProxyVarID::P,      &P,             cd::cdr.P,  NProxyVar::Unit::kW);
  PPV.registerVar(NProxyVar::ProxyVarID::Q,      &Q,             cd::cdr.Q,  NProxyVar::Unit::kVAR);
  PPV.registerVar(NProxyVar::ProxyVarID::CosPhi, &COS_PHI,       0.01f,      "");
}

void CPULSCALC::conv_Id() {
  rAdc.conv_tnf({ CADC_STORAGE::ROTOR_CURRENT });
  rReg_manager.rCurrent_reg.Imeas_0 = *rStrADC.getEPointer(CADC_STORAGE::ROTOR_CURRENT);
}

void CPULSCALC::conv_and_calc() {
  // Измерение всех используемых (в ВТЕ) аналоговых сигналов (внешнее ADC)
  rAdc.conv_tnf({
    //CADC_STORAGE::ROTOR_CURRENT, 
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
  detectRotorPhaseFixed();
  
}

// --- Алгоритм определения перехода напряжения ротора через ноль 
// (с минус в плюс), вычисление скольжения и угла подачи возбуждения ---
void CPULSCALC::detectRotorPhaseFixed() {
  
  if(!v_slip.Permission) { return; }
  
  if (v_slip.nT_slip > v_slip.max_nT_slip) clearDetectRotorPhase();
  v_slip.nT_slip++;   // Счетчик периода
  
  // --- Работа с кадром бегущей суммы ---
  signed short new_val = CADC_STORAGE::getInstance().getExternal(CADC_STORAGE::ROTOR_VOLTAGE);  
  v_slip.sum_ud_frame -= v_slip.ud_frame[v_slip.ind_ud_fram];    
  v_slip.ud_frame[v_slip.ind_ud_fram] = new_val;
  v_slip.sum_ud_frame += v_slip.ud_frame[v_slip.ind_ud_fram];   
  v_slip.ind_ud_fram = (v_slip.ind_ud_fram + 1) % v_slip.N_FRAME; 
  
  // --- Проверяем пришло ли время события ---
  if (v_slip.wait_for_event) {
    v_slip.tick_wait++;    // Счетчик тиков угла события    
    if (v_slip.tick_wait >= v_slip.target_tick) { 
      // Глубина достигнута, выставляем событие
      v_slip.slip_event = true;
      v_slip.wait_for_event = false;
      v_slip.tick_wait = 0;
    }
  }
  
  // --- Положительная область ---
  if (v_slip.sum_ud_frame >= 0) {                     
    v_slip.sum_pos_wave++;        
  }  
  
  // --- Отрицательная область ---
  if (v_slip.sum_ud_frame < 0) {                     
    v_slip.neg_wave = true; 
    v_slip.sum_neg_wave++; 
  } 
  
  // --- Переход через ноль (из - в +) ---
  if ((v_slip.sum_ud_frame > 0) && v_slip.neg_wave) {
    v_slip.neg_wave = false; 
    v_slip.u0_event = true;   // Ноль пройден
    
    // Если период и полупериоды были достаточнй величины
    if ((v_slip.nT_slip > v_slip.min_nT_slip) && 
        (v_slip.sum_pos_wave > v_slip.min_nT_slip / 2) && 
        (v_slip.sum_neg_wave > v_slip.min_nT_slip / 2)) { 
      
      // Считаем скольжение
      v_slip.slip_value = 6.0f / static_cast<float>(v_slip.nT_slip);
      
      // Рассчитываем желаемую глубину захода в + (например, заход на 30 градусов)
      // Полный период = nT_slip. 30 градусов от начала полуволны — 1/12,  60гр - 1/6 и т.д.
      signed short  pure_target = v_slip.nT_slip / (360 / v_slip.DepthDeg);
      // Вычитаем задержку кадра (половина N_FRAME). Например для N = 8 будет 4 тика (13ms)
      // и задержку RC фильтра (на частотах 2...10Гц примерно 12ms/3.333ms = 4 тика)
      signed short  depth_target = pure_target - (v_slip.N_FRAME / 2) - v_slip.delay_rc;      
      
      if (depth_target <= 0) {
        // Задержка фильтров больше нужной глубины, выставляем событие 
        v_slip.target_tick = 0;
        v_slip.slip_event = true;
        v_slip.wait_for_event = false; 
      } else {
        // Глубина не достигнута. Ждём.
        v_slip.target_tick = static_cast<unsigned short>(depth_target);
        v_slip.tick_wait = 0;   // Сбрасываем счетчик паузы
        v_slip.wait_for_event = true;       
      }
    } else {
      clearDetectRotorPhase();
    }    
    v_slip.nT_slip = 0;       // Сброс для следующего периода
    v_slip.sum_pos_wave = 0; 
    v_slip.sum_neg_wave = 0; 
  }
}



// ---Восстановление синусоидальных сигналов по двум измерениям и углу.
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

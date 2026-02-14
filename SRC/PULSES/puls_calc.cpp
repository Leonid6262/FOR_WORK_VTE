#include "puls_calc.hpp"
#include "AdcStorage.hpp"
#include "RegManager.hpp"

#include <string>

CPULSCALC::CPULSCALC(CADC& rAdc, CProxyPointerVar& PPV, CDAC_PWM& dac_cos, CRegManager& rReg_manager, CADC_STORAGE& rStrADC) : 
  rAdc(rAdc), dac_cos(dac_cos), rReg_manager(rReg_manager), rStrADC(rStrADC) {

  // Регистрация в реестре указателей
  PPV.registerVar(NProxyVar::ProxyVarID::Ustat,  &U_STATOR_RMS,  cd::cdr.US, NProxyVar::Unit::Volt);
  PPV.registerVar(NProxyVar::ProxyVarID::Istat,  &I_STATOR_RMS,  cd::cdr.IS, NProxyVar::Unit::Amp);
  PPV.registerVar(NProxyVar::ProxyVarID::P,      &P_POWER,       cd::cdr.P,  NProxyVar::Unit::kW);
  PPV.registerVar(NProxyVar::ProxyVarID::Q,      &Q_POWER,       cd::cdr.Q,  NProxyVar::Unit::kVAR);
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
      signed short  pure_target = v_slip.nT_slip / (360 / v_slip.Depth_30deg);
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



// --- Восстановление синусоидальных сигналов по двум измерениям и углу, 
//     расчёт мощностей и косинуса. Алгоритмом Ширли ---
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

  float i_theta = (2.0f * v_rest.pi * v_rest.freq * v_rest.dT_istator) / 1000000.0f;

  float icos = std::cos(i_theta);
  float isin = std::sin(i_theta);

  // --- Скользящее среднее ---
  v_rest.ind_d_avr = (v_rest.ind_d_avr + 1) % v_rest.PULS_AVR;
  
  float cur_u_stat = sqrt(((us1us1 + us2us2) - (us1us2 * 2 * ucos)) / (usin * usin));
  v_rest.u_stat[v_rest.ind_d_avr] = cur_u_stat;
  
  v_rest.Um = 0;
  for(char u = 0; u < v_rest.PULS_AVR; u++) {
    v_rest.Um += v_rest.u_stat[u];
  }
  
  v_rest.Um = v_rest.Um / v_rest.PULS_AVR;  
  u_stator_rms = v_rest.Um / v_rest.sqrt_2;
  U_STATOR_RMS = static_cast<unsigned short>((v_rest.Um/v_rest.sqrt_2) + 0.5f);
  
  float cur_i_stat = sqrt(((is1is1 + is2is2) - (is1is2 * 2 * icos)) / (isin * isin));
  v_rest.i_stat[v_rest.ind_d_avr] = cur_i_stat;
  
  v_rest.Im = 0;
  for(char i = 0; i < v_rest.PULS_AVR; i++) {
    v_rest.Im += v_rest.i_stat[i];
  }
  
  v_rest.Im = v_rest.Im / v_rest.PULS_AVR;
  i_stator_rms = v_rest.Im / v_rest.sqrt_2;
  I_STATOR_RMS = static_cast<unsigned short>((v_rest.Im/v_rest.sqrt_2) + 0.5f);
  
  /*
      Расчёт S_POWER, P_POWER, Q_POWER и COS_PHI 
  */
  
  /* Рабочий за 13.02.26
  float u_norm = v_rest.u_stator_2 / v_rest.Um;
  float i_norm = v_rest.i_stator_2 / v_rest.Im;
  
  // ограничение [-1, +1]
  u_norm = fmaxf(-1.0f, fminf(1.0f, u_norm));
  i_norm = fmaxf(-1.0f, fminf(1.0f, i_norm));
  
  // синусная компонента
  float phi_u = asinf(u_norm);
  float phi_i = asinf(i_norm);
  
  // косинусная компонента (берём соседнюю выборку или вычисляем через фазовый сдвиг)
  float u_cos = v_rest.u_stator_1 / v_rest.Um; // например, предыдущая точка
  float i_cos = v_rest.i_stator_1 / v_rest.Im;
  
  // корректировка знака
  if (u_cos < 0) phi_u = v_rest.pi - phi_u;
  if (i_cos < 0) phi_i = v_rest.pi - phi_i;
  */
  
// Уточнённый
// --- Расчет фаз через арксинус с исправленной косинусной компонентой ---
  
  // 1. Нормализация синуса (текущее мгновенное значение)
  float u_norm = fmaxf(-1.0f, fminf(1.0f, v_rest.u_stator_2 / v_rest.Um));
  float i_norm = fmaxf(-1.0f, fminf(1.0f, v_rest.i_stator_2 / v_rest.Im));
  
  float phi_u = asinf(u_norm);
  float phi_i = asinf(i_norm);
  
  // 2. Восстановление косинусной компоненты (ортогональный вектор)
  float u_cos_val = (v_rest.u_stator_1 - v_rest.u_stator_2 * ucos) / usin;
  float i_cos_val = (v_rest.i_stator_1 - v_rest.i_stator_2 * icos) / isin;
  
  // 3. Корректировка угла по знаку восстановленного косинуса
  // Если косинус отрицательный — мы во 2-й или 3-й четверти (от 90 до 270 град)
  if (u_cos_val < 0.0f) {
      phi_u = v_rest.pi - phi_u;
  } 
  // Если косинус положительный, а синус отрицательный — мы в 4-й четверти
  else if (u_norm < 0.0f) {
      phi_u = 2.0f * v_rest.pi + phi_u; 
  }

  if (i_cos_val < 0.0f) {
      phi_i = v_rest.pi - phi_i;
  } 
  else if (i_norm < 0.0f) {
      phi_i = 2.0f * v_rest.pi + phi_i;
  }  
  
  //Вариант через проекции
  /*
  // Косинусная компонента для напряжения (ортогональный вектор)
  float u_ortho = (v_rest.u_stator_1 - v_rest.u_stator_2 * ucos) / usin;
  float i_ortho = (v_rest.i_stator_1 - v_rest.i_stator_2 * icos) / isin;

  // Получаем углы для каждого канала одним махом
  float phi_u = atan2f(v_rest.u_stator_2, u_ortho);
  float phi_i = atan2f(v_rest.i_stator_2, i_ortho);
  */

  float phi = phi_u - phi_i;
  
  while (phi > v_rest.pi) phi -= 2.0f * v_rest.pi; 
  while (phi < -v_rest.pi) phi += 2.0f * v_rest.pi;
  
  // --- Скользящее среднее для phi ---  
  v_rest.phi_buf[v_rest.ind_d_avr] = phi; 
  float phi_sum = 0.0f; 
  for (char k = 0; k < v_rest.PULS_AVR; k++) { 
    phi_sum += v_rest.phi_buf[k];  
  } 
  float phi_avg = phi_sum / v_rest.PULS_AVR;
  v_rest.phi_deg = phi_avg * 180.0f / v_rest.pi;
  
  cos_phi = cosf(phi_avg);
  
  // --- Полная мощность ---
  S_Power = (v_rest.Um * v_rest.Im) / 2.0f;
  
  // --- Активная мощность --- 
  P_Power = S_Power * cos_phi;
  
  // --- Реактивная мощность --- 
  float g_power = sqrtf(fmaxf(0.0f, S_Power*S_Power - P_Power*P_Power));
  if (phi_avg < 0) {
    Q_Power = -g_power;
    COS_PHI = -static_cast<signed short>((cos_phi * 100.0f) + 0.5f);
  }
  else {
    Q_Power = g_power;
    COS_PHI = static_cast<signed short>((cos_phi * 100.0f) + 0.5f);
  }
  
  S_POWER = static_cast<unsigned short>(S_Power + 0.5f);
  P_POWER = static_cast<unsigned short>(P_Power + 0.5f); 
  Q_POWER = static_cast<signed short>(Q_Power + 0.5f);  
    
  // обновляем предыдущие выборки
  v_rest.u_stator_1 = v_rest.u_stator_2;
  v_rest.timing_ustator_1 = v_rest.timing_ustator_2;
  v_rest.i_stator_1 = v_rest.i_stator_2;
  v_rest.timing_istator_1 = v_rest.timing_istator_2;

}

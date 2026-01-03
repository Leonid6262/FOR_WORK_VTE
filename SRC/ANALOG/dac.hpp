#pragma once

#include "LPC407x_8x_177x_8x.h"

// DAC контроллера
class CDAC0 {
 private:
  static constexpr unsigned int DAC0_EN = 0x00010002;
  static constexpr unsigned int START_BITS_VALUE = 6;

 public:
  CDAC0();

  static constexpr signed short DAC0_MAX_VAL = 511;   // Максимальное значение DAC0
  static constexpr signed short DAC0_MIN_VAL = -512;  // Минимальное значение DAC0

  void conv(signed short);
};

// DAC на бпзе PWM
class CDAC_PWM {
 private:
  static constexpr unsigned int PWM_div_1 = 6;  // Делитель частоты

  static constexpr unsigned int _MAT0LATCHEN = 1UL << 0;
  static constexpr unsigned int _MAT4LATCHEN = 1UL << 4;
  static constexpr unsigned int _MAT5LATCHEN = 1UL << 5;

  static constexpr unsigned int _PORT_PWM = 1;
  static constexpr unsigned int _CE_PWMEN = 0x09;

  static constexpr unsigned int _PWMENA4 = 1UL << 12;
  static constexpr unsigned int _PWMENA5 = 1UL << 13;

  static constexpr unsigned int _DAC_PWM_T = 500;  // Период PWM. dac_pwm_T * 0.1 uS

  volatile unsigned int* pMR;
  unsigned short LER;

 public:
  static constexpr signed short _DAC_PWM_MAX_VAL = _DAC_PWM_T - 1;  // Максимальное значение DAC_PWM
  static constexpr signed short _DAC_PWM_MIN_VAL = 0;               // Минимальное значение DAC_PWM

  enum class EPWM_DACInstance { PWM_DAC1, PWM_DAC2 };

  CDAC_PWM(EPWM_DACInstance);
  void conv(unsigned short);
};

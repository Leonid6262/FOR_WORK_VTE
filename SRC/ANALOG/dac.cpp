#include "dac.hpp"
#include "settings_eep.hpp"
#include "system_LPC177x.h"

CDAC0::CDAC0(CEEPSettings& rSet)  : rSet(rSet) {
  LPC_IOCON->P0_26 = DAC0_EN;
  LPC_DAC->CNR = ((DAC0_MAX_VAL - DAC0_MIN_VAL) >> 1) << START_BITS_VALUE;
}

CDAC_PWM::CDAC_PWM(EPWM_DAC DN, CEEPSettings& rSet) : rSet(rSet){
  LPC_SC->PCONP |= CLKPWR_PCONP_PCPWM1;  // PWM1 power/clock control bit.
  LPC_PWM1->PR = PWM_div_1 - 1;          // при PWM_div=6, F=60МГц/6=10МГц, 1тик=0.1мкс

  LPC_PWM1->TCR = 0x000;  // Сброс регистра таймера
  LPC_PWM1->TCR = 0x002;  // Сброс таймера

  switch (DN) {
    case EPWM_DAC::PWM_DAC1:
      pMR = &LPC_PWM1->MR5;
      LER = _MAT5LATCHEN;  // Обновление MR5
      *pMR = _DAC_PWM_MIN_VAL;
      LPC_IOCON->P2_4 = _PORT_PWM;          // P2_4 -> PWM1:5
      LPC_PWM1->PCR |= _PWMENA5;            // Включение PWM1:5 в стационарном режиме
      LPC_PWM1->MR0 = _DAC_PWM_T;           // Период ШИМ. Канал  PWM5, стационарный. MR0 - включение
      LPC_PWM1->LER |= LER | _MAT0LATCHEN;  // Обновление MR5 и MR0
      LPC_PWM1->TCR = _CE_PWMEN;            // Включение таймера и PWM
      break;
    case EPWM_DAC::PWM_DAC2:
      pMR = &LPC_PWM1->MR4;
      LER = _MAT4LATCHEN;  // Обновление MR4
      *pMR = _DAC_PWM_MIN_VAL;
      LPC_IOCON->P2_3 = _PORT_PWM;          // P2_3 -> PWM1:4
      LPC_PWM1->PCR |= _PWMENA4;            // Включение PWM1:4 в стационарном режиме
      LPC_PWM1->MR0 = _DAC_PWM_T;           // Период ШИМ. Канал  PWM4, стационарный. MR0 - включение
      LPC_PWM1->LER |= LER | _MAT0LATCHEN;  // Обновление MR4 и MR0
      LPC_PWM1->TCR = _CE_PWMEN;            // Включение таймера и PWM
      break;
  }
}

void CDAC0::conv(signed short data) { // dac0_min_val...dac0_max_val -> -5V...+5V
  if ((data + CEEPSettings::getInstance().getSettings().shift_dac0) > DAC0_MAX_VAL) {
    data = DAC0_MAX_VAL - rSet.getSettings().shift_dac0;
  }
  if ((data + CEEPSettings::getInstance().getSettings().shift_dac0) < DAC0_MIN_VAL) {
    data = DAC0_MIN_VAL + CEEPSettings::getInstance().getSettings().shift_dac0;
  }
  LPC_DAC->CNR = (DAC0_MAX_VAL - (data + CEEPSettings::getInstance().getSettings().shift_dac0)) << START_BITS_VALUE;
}

void CDAC_PWM::conv(unsigned short data) { // dac1_min_val...dac1_max_val -> 0V...+3V
  if (data > _DAC_PWM_MAX_VAL) {
    data = _DAC_PWM_MAX_VAL;
  }
  *pMR = data;
  LPC_PWM1->LER |= LER;
}
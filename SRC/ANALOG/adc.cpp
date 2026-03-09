#include "adc.hpp"
#include <math.h>
#include "system_LPC177x.h"

void CADC::conv_tnf(std::initializer_list<char> list) {
  char N_ch = list.size();

  char index_wr = 0;
  unsigned char timing_index = 0;
  char index_rd = 0;
  char ending_index = 0;

  signed short raw_adc_data;
  unsigned short Nch;

  while (true) {
    // Запись
    if (index_wr < N_ch) {
      if (spi_drv.statusTNF()) {
        spi_drv.writeShort(cN_CH[*(list.begin() + index_wr)]);       
        adstr.setTimings(timing_index, LPC_TIM3->TC);
        index_wr++;
        timing_index++;
      }
    } else {
      // Выталкиваем два последних байта из FIFO
      if (ending_index < 2) {
        ending_index++;
        spi_drv.writeShort(cN_CH[CADC_STORAGE::ch_HRf]);        
        adstr.setTimings(timing_index, LPC_TIM3->TC);
        timing_index++;
      }
    }

    // Чтение
    if (index_rd < (N_ch + 2)) {
      if (spi_drv.statusRNE()) {             
        raw_adc_data = spi_drv.readShort();
        Nch = (raw_adc_data & 0xF000) >> 12;
        if (Nch < G_CONST::NUMBER_CHANNELS) {
          adstr.setExternal(Nch, (raw_adc_data & 0x0FFF));
        }
        index_rd++;
      }
    }

    // Все байты записаны и прочитаны
    if ((index_wr == N_ch) && (index_rd == (N_ch + 2)) && (ending_index == 2)) break;
  }

  // Контрольная очистка FIFO
  while (spi_drv.statusRNE()) { raw_adc_data = spi_drv.readShort(); }
}

CADC::CADC(CADC_STORAGE& adstr, CSPI_DRIVER& spi_drv) : adstr(adstr), spi_drv(spi_drv) {
  unsigned short tmp_dat;
  spi_drv.writeShort((1UL << 12) | (1UL << 11)); // 0x1800 - manual mode and prog b0...b6
  while (spi_drv.statusRNE()) { tmp_dat = spi_drv.readShort(); }
  spi_drv.writeShort(cN_CH[0]);
  while (spi_drv.statusBSY()) { tmp_dat = spi_drv.readShort(); }
  while (spi_drv.statusRNE()) { tmp_dat = spi_drv.readShort(); }
};

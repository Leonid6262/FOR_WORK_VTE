#pragma once

#include "settings_eep.hpp"

class CADC_STORAGE {
 public:
  // Список имён каналов внешнего АЦП
  enum EADC_NameCh {
    ROTOR_CURRENT = 0,
    STATOR_VOLTAGE = 1,
    ROTOR_VOLTAGE = 2,
    LE_CURRENT = 3,
    STATOR_CURRENT = 4,
    NODE_CURRENT = 5,
    EXT_SETTINGS = 6,
    Name_ch07,
    Name_ch08,
    Name_ch09,
    Name_ch10,
    Name_ch11,
    Name_ch12,
    Name_ch13,
    Name_ch14,
    ch_HRf = 15  // Промежуточный канал (0.5 Ref)
  };

  // Список имён каналов (используемых) внутреннего АЦП
  enum EIADC_NameCh {
    SUPPLY_P5 = 0,
    SUPPLY_N5 = 1,
  };

  // --- Обработка и запись данных внешнего АЦП ---
  inline void setExternal(unsigned char channel, signed short raw_adc_data) {
    external_data[channel] = (raw_adc_data - rSettings.getSettings().shift_adc[channel]) *
                             (1.0f + rSettings.getSettings().incline_adc[channel]);
  }
  // --- Запись таймингов внешнего АЦП ---
  inline void setTimings(unsigned char channel, unsigned int timings) { this->timings[channel] = timings; }
  // --- Запись данных внутреннего АЦП ---
  inline void setInternal(unsigned char channel, float value) { internal_data[channel] = value; }
  // --- Чтение данных внешнего АЦП ---
  inline signed short getExternal(unsigned char channel) { return external_data[channel]; }
  // --- Чтение таймингов внешнего АЦП ---
  inline unsigned int getTimings(unsigned char channel) { return timings[channel]; }
  // --- Чтени данных внутреннего АЦП ---
  inline float getInternal(unsigned char channel) { return internal_data[channel]; }
  // --- Чтение internal_data[] указателей ---
  inline float* getIPointer(unsigned char channel) { return &internal_data[channel]; }
  // --- Чтение external_data[] указателей ---
  inline signed short* getEPointer(unsigned char channel) { return &external_data[channel]; }


  // Для отладочного указателя на external_data[]
  inline signed short (&getExternal()) [G_CONST::NUMBER_CHANNELS] { return external_data; }

  static CADC_STORAGE& getInstance() {
    static CADC_STORAGE instance;
    return instance;
  }

 private:
  signed short external_data[G_CONST::NUMBER_CHANNELS] = {};  // Обработанные данные полученные от внешнего АЦП
  unsigned int timings[G_CONST::NUMBER_CHANNELS + 2] = {};    // Тайминги внешнего АЦП
  float internal_data[G_CONST::NUMBER_I_CHANNELS] = {};       // Обработанные данные полученные от внутреннего АЦП

  CEEPSettings& rSettings;

  // --- Механизмы Singleton ---
  CADC_STORAGE();
  CADC_STORAGE(const CADC_STORAGE&) = delete;
  CADC_STORAGE& operator=(const CADC_STORAGE&) = delete;
};
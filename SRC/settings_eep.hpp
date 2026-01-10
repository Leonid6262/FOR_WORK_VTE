#pragma once

#include "bool_name.hpp"
#include "crc16.hpp"
#include "lpc_eeprom.h"

// Пространство имён глобальных констант
namespace G_CONST {
  constexpr unsigned short NUMBER_CHANNELS = 16;   // Количество какналов внешнего АЦП
  constexpr unsigned short NUMBER_I_CHANNELS = 2;  // Количество какналов внутреннего АЦП
  constexpr unsigned short SSID_PS_L = 20;         // Максимальная длина имени и пароля WiFi сети 20 символов
  constexpr unsigned short BYTES_RW_MAX = 3;       // Макс. количество байт чтения/записи по SPI (кол. структур данных)
  constexpr unsigned short BYTES_RW_REAL = 1;      // Фактическое количество байт чтения/записи по SPI (определяется схемой)
  constexpr unsigned short Nlang = 3;              // Количество языков
}
// Пространство имён коэффициентов отображения в единицах СИ
namespace cd {
  constexpr float one     = 1.0f;
  constexpr float Alpha   = 180.0f / 10000;     // 180deg/10000tick
  constexpr float IRotor  = 200.0f / 1000;      // 200A/1000d 
  constexpr float URotor  = 100.0f / 1500;      // 100V/1500d 
  constexpr float UStator = 400.0f / 1500;      // 400V/1500d 
  constexpr float IStator = 150.0f / 1500;      // 150A/1500d 
}
// Пространство имён коэффициентов отображения в %
namespace pd {
  constexpr float IRotor  = 100.0f / 1000;      // 100%/1000d 
  constexpr float URotor  = 100.0f / 1500;      // 100%/1500d 
  constexpr float UStator = 100.0f / 1500;      // 100%/1500d 
  constexpr float IStator = 100.0f / 1500;      // 100%/1500d 
}
class CEEPSettings {
 private:
  // --- Структура уставок ---
  struct WorkSettings {
    unsigned short checkSum;                            // 0 Контрольная сумма
    unsigned short SNboard_month;                       // 1 Серийный номер платы - дата (ст.б - месяц, мл.б - год)
    unsigned short SNboard_year;                        // 2 Серийный номер платы - дата (ст.б - месяц, мл.б - год)
    unsigned short SNboard_number;                      // 3 Серийный номер платы - порядковый номер (99.99.0999)
    unsigned short Language;                            // 4 Номер языка
    signed short shift_adc[G_CONST::NUMBER_CHANNELS];   // 5 Смещения АЦП
    float incline_adc[G_CONST::NUMBER_CHANNELS];        // 6 Наклон
    signed short shift_dac0;                            // 7 Смещение DAC0
    signed short shift_dac1_pwm;                        // 8 Смещение DAC1_pwm
    signed short shift_dac2_pwm;                                // 9 Смещение DAC2_pwm
    unsigned char din_Pi_invert[G_CONST::BYTES_RW_MAX + 1];     // 10 Признак инвертирования дискретных входов (+1 - порт Pi0)        
    unsigned char dout_spi_invert[G_CONST::BYTES_RW_MAX];       // 11 Признак инвертирования SPI выходов    
    struct SetReg                                               // 12 Коэффициенты регуляторов
    {
      float KpCr;
      float KiCr;
      signed short Iset0;
      signed short Ifors;
      signed short Idry;
      signed short A0;
      float KpCos;
      float KiCos;
      float Cos_set;
      float KpQ;
      float KiQ;
      float Qset;
    } set_reg;
    struct SetSIFU                                         // 13 Уставки СИФУ
    {
      signed short power_shift;                            /* Точный сдвиг силового напряжения */
      unsigned char d_power_shift;                         /* Дискретный сдвиг силового напряжения 60гр */
    } set_sifu;
    unsigned char ssid[G_CONST::SSID_PS_L];                // 14 Имя сети
    unsigned char password[G_CONST::SSID_PS_L];            // 15 Пароль
    // Добавляя новые уставки сюда, не забывайте обновлять defaultSettings ниже!!!
  };
//  Статические константные уставки по умолчанию (во Flash) ---
// 'static const inline' позволяет определить ее прямо здесь, в .h файле.
  static const inline WorkSettings defaultSettings{
    .checkSum = 0x0000,
    .SNboard_month = 1,
    .SNboard_year = 0,
    .SNboard_number = 999,
    .Language = 0,
    .shift_adc =   {2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047},
    .incline_adc = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    .shift_dac0 = 0,
    .shift_dac1_pwm = 0,
    .shift_dac2_pwm = 0,
    .din_Pi_invert = {0, 0, 0, 0},
    .dout_spi_invert = {0, 0, 0},
    .set_reg =
    {
      .KpCr = 1.0f,
      .KiCr = 0.001f,
      .Iset0 = 0,
      .Ifors = 0,
      .Idry = 0,
      .A0 = 6667,
      .KpCos = 1.0f,
      .KiCos = 0.001f,
      .Cos_set = 1.0,
      .KpQ = 1.0f,
      .KiQ = 0.001f,
      .Qset = 0
    },
    .set_sifu =
    {
      .power_shift = 0,
      .d_power_shift = 0,
    },   
    .ssid = "NetName",
    .password = "Password"
  };
  
  // Текущий набор уставок, хранящийся в RAM ---
  WorkSettings settings;

  // Механизмы Singleton ---
  CEEPSettings();                                         // Приватный конструктор
  CEEPSettings(const CEEPSettings&) = delete;             // Запрещаем копирование
  CEEPSettings& operator=(const CEEPSettings&) = delete;  // Запрещаем присваивание

  // Приватные методы для работы с EEP  ---
  StatusRet readFromEEPInternal(WorkSettings& outSettings);  // Чтение в WorkSettings
  void writeToEEPInternal(WorkSettings& inSettings);         // Запись из WorkSettings

  void EEP_init(void);
  void EEPr(uint16_t page_offset, uint16_t page_address, void* data, EEPROM_Mode_Type mode, uint32_t count);
  void EEPw(uint16_t page_offset, uint16_t page_address, void* data, EEPROM_Mode_Type mode, uint32_t count);

 public:
  // Публичный метод для получения единственного экземпляра ---
  static CEEPSettings& getInstance();

  // Загрузки/сохранения уставок ---
  StatusRet loadSettings();  // Загружает из EEP. Если ошибка CRC - остаются дефолтные.
  void saveSettings();       // Сохраняет текущие уставки в EEP.

  // Прочитать/Изменить уставку ---
  inline WorkSettings& getSettings() { return settings; }
};

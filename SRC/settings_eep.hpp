#pragma once

#include "bool_name.hpp"
#include "crc16.hpp"
#include "lpc_eeprom.h"
#include <stddef.h>



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
  /* Константные (compile-time) */
  constexpr float one     = 1.0f;
  constexpr float Alpha   = 180.0f / 10000;     // 180deg / 10000 tick timers

  /* Параметрические (runtime) */  
  // Коэффициенты передачи (дискрет на номинальное значение)
  constexpr unsigned short ADC_DISCR_ID = 1000;
  constexpr unsigned short ADC_DISCR_UD = 1500;
  constexpr unsigned short ADC_DISCR_IS = 1000;
  constexpr unsigned short ADC_DISCR_US = 2000;
  // Коэффициенты отображения
  struct DisplayCoeffs { 
    float Id; 
    float Ud;
    float IS; 
    float US;
    float P;
    float Q;
  };
  inline DisplayCoeffs cdr;
  
  // Вычисляются в фабрике при создании менеджера терминала
  inline float cd_r(float NomVal, unsigned short discr) { return NomVal / discr; } 
}

class CEEPSettings {
  
 private:
  // --- Структура уставок ---
  struct WorkSettings {
    unsigned short checkSum;                            // 0 Контрольная сумма
    unsigned short SNboard_month;                       // 1 Серийный номер платы - месяц
    unsigned short SNboard_year;                        // 2 Серийный номер платы - год
    unsigned short SNboard_number;                      // 3 Серийный номер платы - порядковый номер
    unsigned short Language;                            // 4 Номер языка
    signed short shift_adc[G_CONST::NUMBER_CHANNELS];   // 5 Смещения АЦП
    float incline_adc[G_CONST::NUMBER_CHANNELS];        // 6 Наклон
    signed short shift_dac0;                            // 7 Смещение DAC0
    signed short shift_dac1_pwm;                        // 8 Смещение DAC1_pwm
    signed short shift_dac2_pwm;                        // 9 Смещение DAC2_pwm
    unsigned char din_Pi_invert[G_CONST::BYTES_RW_MAX + 1];     // 10 Признак инвертирования дискретных входов (+1 - порт Pi0)        
    unsigned char dout_spi_invert[G_CONST::BYTES_RW_MAX];       // 11 Признак инвертирования SPI выходов    
    struct {                                                    // 12 Параметры регуляторов
      float KpCr;
      float KiCr;
      signed short A0;
      float KpCos;
      float KiCos;
      float KpQ;
      float KiQ;
    } set_reg;
    struct {                                               // 13 Уставки СИФУ
      signed short power_shift;                                 /* Точный сдвиг силового напряжения */
      unsigned char d_power_shift;                              /* Дискретный сдвиг силового напряжения 60гр */
    } set_sifu;
    struct {                                               // 14 Уставки Аварийные
      unsigned short IdMax;                                     /* IdMax */
      unsigned short IdMin;                                     /* IdMin */
    } set_faults;
    struct {                                               // 15 Параметры. Ном. значения
      unsigned short IdNom;                                     /* IdNom */
      unsigned short UdNom;                                     /* UdNom */
      unsigned short ISNom;                                     /* ISNom */
      unsigned short USNom;                                     /* USNom */ 
    } params;  
    struct {                                               // 16 Уставки пуска
      unsigned short IFors;                                     /* Величина форсировки */
      unsigned short TFors;                                     /* Длительность форсировки */
      unsigned short TPusk;                                     /* Максимальное время пуска */ 
      unsigned short ISPusk;                                    /* Пусковой ток статора */
      float SPusk;                                              /* Пусковое скольжение */      
    } set_pusk; 
    struct {                                               // 17 Рабочие начальные значения
      unsigned short Iset_0;                                     /* Ток после форсировки в режиме РТ */      
      unsigned short Idry_0;                                     /* Ток Сушки */  
      float Qset_0;                                              /* Задание Q при включении Q reg*/
      float Cos_0;                                               /* Задание Cos при включении Cos reg*/
    } work_set;     
    unsigned char ssid[G_CONST::SSID_PS_L];                // 18 Имя сети
    unsigned char password[G_CONST::SSID_PS_L];            // 19 Пароль
    // Добавляя новые уставки сюда, не забывайте обновлять defaultSettings ниже!!!
  };
  //  Статические константные уставки по умолчанию (во Flash) ---
  static const inline WorkSettings defaultSettings {
    .checkSum = 0x0000,
    .SNboard_month = 1,
    .SNboard_year = 0,
    .SNboard_number = 999,
    .Language = 1,
    .shift_adc =   {2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047, 2047},
    .incline_adc = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    .shift_dac0 = 0,
    .shift_dac1_pwm = 0,
    .shift_dac2_pwm = 0,
    .din_Pi_invert = {0, 0, 0, 0},
    .dout_spi_invert = {0, 0, 0},
    .set_reg = {
      .KpCr = 1.0f,
      .KiCr = 0.001f,
      .A0 = static_cast<signed short>(120.0f/cd::Alpha),
      .KpCos = 1.0f,
      .KiCos = 0.001f,
      .KpQ = 1.0f,
      .KiQ = 0.001f,
    },
    .set_sifu = {
      .power_shift = 0,
      .d_power_shift = 0,
    },
    .set_faults = {
      .IdMax = 0,
      .IdMin = 0,
    },
    .params = {
      .IdNom = 315,
      .UdNom = 75,
      .ISNom = 100,
      .USNom = 400,
    },
    .set_pusk = {
      .IFors = 470,
      .TFors = 3,
      .TPusk = 10,
      .ISPusk = 1000,
      .SPusk = 0,
    },
    .work_set = {
      .Iset_0 = 150,
      .Idry_0 = 10,
      .Qset_0 = 0,
      .Cos_0 = 1.0,
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

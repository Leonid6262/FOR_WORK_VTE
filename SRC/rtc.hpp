#pragma once

class CRTC
{ 

public:
  
  CRTC();
  
  struct SDateTime {
    unsigned char  year;
    unsigned char  month;
    unsigned char  day;
    unsigned char  hour;
    unsigned char  minute;
    unsigned char  second;
  };
   
  void update_now();                    // Обновление данных из RTC
  const SDateTime& get_now() const;     // Доступ к актуальным данным  
  void setDateTime(const SDateTime&);   // Установка даты и времени
  void setDefault();                    // Установка дефолтных значений
  
private: 
  
  static constexpr unsigned int RTC_OSCF = 1<<4; // RTC Oscillator Fail detect flag 
  static constexpr unsigned int CLKEN    = 1<<0; // Clock Enable.
  static constexpr unsigned int CCALDS   = 1<<4; // Calibration counter disable.
  
  bool isDateTimeValid();               // Проверка корректности данных при включеии питания
  
  SDateTime date_now;                   // Кэш актуального времени  
 
  
};


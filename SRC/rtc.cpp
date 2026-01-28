#include "rtc.hpp"
#include "LPC407x_8x_177x_8x.h"

CRTC::CRTC() {
  if ((LPC_RTC->RTC_AUX & RTC_OSCF) || !isDateTimeValid()) {
    setDefault();                  // Установка дефолтных значений если была остановка
    LPC_RTC->RTC_AUX |= RTC_OSCF;  // генератора или прочитанные значения не корректные
  }
  DateTimeForSet = {0,0,0,0,0,0};
}

bool CRTC::isDateTimeValid() {
  unsigned int t = LPC_RTC->CTIME0;
  unsigned int d = LPC_RTC->CTIME1;

  unsigned char month = static_cast<unsigned char>((d >> 8) & 0x0F);  // MONTH
  unsigned char day = static_cast<unsigned char>(d & 0x1F);           // DAY
  unsigned char hour = static_cast<unsigned char>((t >> 16) & 0x1F);  // HOUR
  unsigned char min = static_cast<unsigned char>((t >> 8) & 0x3F);    // MIN
  unsigned char sec = static_cast<unsigned char>(t & 0x3F);           // SEC
  
  return (sec <= 59) && (min <= 59) && (hour <= 23) && (day >= 1 && day <= 31) && (month >= 1 && month <= 12);
}

void CRTC::update_now() {
  unsigned int t = LPC_RTC->CTIME0;
  unsigned int d = LPC_RTC->CTIME1;

  date_now.year = static_cast<unsigned char>((d >> 16) & 0x7F);   // YEAR
  date_now.month = static_cast<unsigned char>((d >> 8) & 0x0F);   // MONTH
  date_now.day = static_cast<unsigned char>(d & 0x1F);            // DAY
  date_now.hour = static_cast<unsigned char>((t >> 16) & 0x1F);   // HOUR
  date_now.minute = static_cast<unsigned char>((t >> 8) & 0x3F);  // MIN
  date_now.second = static_cast<unsigned char>(t & 0x3F);         // SEC
  
  DateTimeForSet.year = date_now.year;
  DateTimeForSet.month = date_now.month;
  DateTimeForSet.day = date_now.day;
  DateTimeForSet.hour = date_now.hour;
  DateTimeForSet.minute = date_now.minute;
  DateTimeForSet.second = 0;  

}

const CRTC::SDateTime& CRTC::get_now() const { return date_now; }

void CRTC::setDateTime(const SDateTime& dt) {
  LPC_RTC->CCR &= ~CLKEN;  // Stop Clock

  LPC_RTC->YEAR = dt.year;
  LPC_RTC->MONTH = dt.month;
  LPC_RTC->DOM = dt.day;
  LPC_RTC->HOUR = dt.hour;
  LPC_RTC->MIN = dt.minute;
  LPC_RTC->SEC = dt.second;

  LPC_RTC->CCR |= CLKEN;  // Start Clock
}

void CRTC::setDefault() {
  LPC_RTC->CCR |= CCALDS;  // Отключение калибратора
  setDateTime({
      25,  // Default year
      6,   // Default month
      15,  // Default day
      0,   // Default hour
      0,   // Default minute
      0    // Default second
  });
}

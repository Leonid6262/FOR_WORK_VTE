#include "message_display.hpp"
#include "_SystemManager.hpp"
#include "string_utils.hpp"
#include "pause_us.hpp"

CMessageDisplay::CMessageDisplay(CTerminalUartDriver& uartDrv, CSystemManager& rSysMgr, CRTC& rRTC) : 
  uartDrv(uartDrv), rSysMgr(rSysMgr), rRTC(rRTC), l(CEEPSettings::getInstance().getSettings().Language - 1),
  COUNT_CATEGORIES(static_cast<unsigned char>(ECategory::COUNT)) {
  // Регистрация категорий в CategoryContext
  contexts[static_cast<unsigned char>(ECategory::NOT_READY)] = {
      SNotReady::active, 0,
      SNotReady::NAME, SNotReady::MSG,
      static_cast<unsigned char>(ENotReadyId::COUNT)};
  contexts[static_cast<unsigned char>(ECategory::READY)] = {
      SReady::active, 0,
      SReady::NAME, SReady::MSG,
      static_cast<unsigned char>(EReadyId::COUNT)};
  contexts[static_cast<unsigned char>(ECategory::WORK)] = {
      SWork::active, 0,
      SWork::NAME, SWork::MSG,
      static_cast<unsigned char>(EWorkId::COUNT)};
  contexts[static_cast<unsigned char>(ECategory::FAULT)] = {
      SFault::active, 0,
      SFault::NAME, SFault::MSG,
      static_cast<unsigned char>(EFaultId::COUNT)};
  contexts[static_cast<unsigned char>(ECategory::WARNING)] = {
      SWarning::active, 0,
      SWarning::NAME, SWarning::MSG,
      static_cast<unsigned char>(EWarningId::COUNT)};
}

void CMessageDisplay::set_pTerminal(CTerminalManager* pTerminal_manager){
  this->pTerminal_manager = pTerminal_manager;
}

// Отправки строки
void CMessageDisplay::sendLine(const std::string& s, bool newline) {
    std::string text = StringUtils::padTo16(s);
    text += newline ? "\r\n" : "\r";
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
}


// Вывод заголовка категории (если print_title == true), сообщения и даты
void CMessageDisplay::render_messages(signed char cat, bool print_title = false) {
  
  if (cat == data_time) {
    rRTC.update_now();                  // Обновление экземпляра структуы SDateTime данными из RTC
    auto now = rRTC.get_now();    
    char date_str[disp_l];
    snprintf(date_str, sizeof(date_str), "%02u.%02u.%02u", now.day, now.month, now.year);    
    char time_str[disp_l];
    snprintf(time_str, sizeof(time_str), "%02u:%02u:%02u", now.hour, now.minute, now.second);    
    sendLine(date_str, newline);
    sendLine(time_str);
    return;
  }

  const auto& ctx = contexts[static_cast<unsigned char>(cat)];
  
  if (print_title) {
    sendLine(StringUtils::utf8_to_cp1251(ctx.NAME[l]), newline);
  }  
  sendLine(StringUtils::utf8_to_cp1251(ctx.MSG[ctx.cursor][l]));
}

void CMessageDisplay::rotate_messages() {
  static unsigned char cur_cat = 0;     // Текущая категория
  static unsigned char emp_num = 0;     // Счётчик "пустых" категорий
  static bool print_title = true;       // Признак вывода заголовка
  static bool had_active = false;       // В текущей категории был актив
  
  while (true) {
    
    if (contexts[cur_cat].cursor >= contexts[cur_cat].count) {
      contexts[cur_cat].cursor = 0;
      cur_cat = (cur_cat + 1) % COUNT_CATEGORIES;
      print_title = true;
    }
    
    auto &ctx = contexts[cur_cat];
    
    if (ctx.active[ctx.cursor]) {
      if (first_call) {
        first_call = false;
        print_title = true;
      }
      render_messages(cur_cat, print_title);
      print_title = false;
      ctx.cursor++;
      emp_num = 0;
      had_active = true;
      return;      
    } else {
      ctx.cursor++;
      if (ctx.cursor >= ctx.count) {
        ctx.cursor = 0;
        cur_cat = (cur_cat + 1) % COUNT_CATEGORIES;
        print_title = true;      
        if (had_active) {
          emp_num = 0;
        } else {
          emp_num++;
          if (emp_num >= COUNT_CATEGORIES) {
            emp_num = 0;
            render_messages(data_time);
            return;
          }
        }
        had_active = false;        
        if (cur_cat == 0) {
          render_messages(data_time);
          return;
        }
      }
    }
    
  }
}


// "Опрос" клавиатуры
void CMessageDisplay::get_key() {
  unsigned char input_key;
  if (uartDrv.poll_rx(input_key)) {
    Key_Handler(static_cast<EKey_code>(input_key));
  } else {
    Key_Handler(EKey_code::NONE);
  }
}

void CMessageDisplay::Key_Handler(EKey_code key) {
  switch (key) {
  case EKey_code::START:
    rSysMgr.set_bsWorkTest(State::ON);
    break;
  case EKey_code::STOP:
    rSysMgr.set_bsWorkTest(State::OFF);
    break;
  case EKey_code::ESCAPE:
    first_call = true;
    pTerminal_manager->switchToMenu(); // переключаемся в меню
    break;
  case EKey_code::FnEsc:
    {
      unsigned char led_blue[] = {static_cast<unsigned char>(ELED::LED_BLUE), '\r'}; 
      uartDrv.sendBuffer(led_blue, sizeof(led_blue));
      CategoryUtils::clearMessages(ECategory::FAULT);
      CategoryUtils::clearMessages(ECategory::COUNT);
      Pause_us(200000);
      unsigned char led_off[] = {static_cast<unsigned char>(ELED::LED_OFF), '\r'};
      uartDrv.sendBuffer(led_off, sizeof(led_off));
      rSysMgr.set_bsFault(State::OFF);
      rSysMgr.set_bsReadyCheck(State::ON);
      rSysMgr.rFault_ctrl.rDinStr.Relay_FAULT(State::OFF);     
    }
    break;
  case EKey_code::NONE:
  default: {
    unsigned int dTrs = LPC_TIM0->TC - prev_TC0;
    if (dTrs >= MESSAGE_PERIOD_TICKS) { 
      prev_TC0 = LPC_TIM0->TC;
      if (first_call) {
        unsigned char led_off[] = {static_cast<unsigned char>(ELED::LED_OFF), '\r'};
        uartDrv.sendBuffer(led_off, sizeof(led_off));
      }     
      rotate_messages();
    }
  }
  }
}

#include "message_display.hpp"
#include "string_utils.hpp"

CMessageDisplay::CMessageDisplay(CTerminalUartDriver& uartDrv) : 
  uartDrv(uartDrv), l(CEEPSettings::getInstance().getSettings().Language),
  COUNT_CATEGORIES(static_cast<unsigned char>(ECategory::COUNT)) {
  // Регистрация категорий в CategoryContext
  contexts[static_cast<unsigned char>(ECategory::NOT_READY)] = {
      SNotReady::active, 0,
      SNotReady::NAME, SNotReady::MSG,
      static_cast<unsigned char>(ENotReadyId::COUNT)};
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

// Вывод сообщения на терминал
void CMessageDisplay::render_messages(signed char cat, bool print_title) {
  if (cat == not_mes) {
    sendLine("Not", newline);
    sendLine("Messages");
    return;
  }
  if (print_title) {
    sendLine(StringUtils::utf8_to_cp1251(contexts[cat].NAME[l]), newline);
  }
  sendLine(StringUtils::utf8_to_cp1251(contexts[cat].MSG[contexts[cat].cursor][l]));
}

// Отправки строки
void CMessageDisplay::sendLine(const std::string& s, bool newline) {
    std::string text = StringUtils::padTo16(s);
    text += newline ? "\r\n" : "\r";
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
}

/*
void CMessageManager::render_messages(signed char cat, bool print_title) {
  std::string text;
  if(cat == not_mes) {
    text = StringUtils::padTo16("Not") + "\r\n";   
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
    text = StringUtils::padTo16("Messages") + "\r";
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
    return;
  }
  if (print_title) {
    text = StringUtils::utf8_to_cp1251(contexts[cat].NAME[l]);
    text = StringUtils::padTo16(text) + "\r\n";
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
  }
  text = StringUtils::utf8_to_cp1251(contexts[cat].MSG[contexts[cat].cursor][l]);
  text = StringUtils::padTo16(text) + "\r";  
  uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
}
*/


void CMessageDisplay::rotate_messages() {
  static unsigned char cur_cat = 0;
  static unsigned char emp_num = 0;
  static bool print_title = true;
  
  while (true) {
    if (contexts[cur_cat].active[contexts[cur_cat].cursor]) {
      render_messages(cur_cat, print_title);
      print_title = false;
      contexts[cur_cat].cursor++;
      emp_num = 0;
      if (contexts[cur_cat].cursor >= contexts[cur_cat].count) {
        contexts[cur_cat].cursor = 0;
        cur_cat = (cur_cat + 1) % COUNT_CATEGORIES;
        print_title = true;
      }
      return;
    }
    else {
      contexts[cur_cat].cursor++;
      if (contexts[cur_cat].cursor >= contexts[cur_cat].count) {
        contexts[cur_cat].cursor = 0;
        cur_cat = (cur_cat + 1) % COUNT_CATEGORIES;
        print_title = true;
        emp_num++;
        if (emp_num >= COUNT_CATEGORIES) {
          emp_num = 0;
          render_messages(not_mes, false);
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
  case EKey_code::ESCAPE:
    pTerminal_manager->switchToMenu(); // переключаемся в меню
    break;
  case EKey_code::FnEsc:
    CategoryUtils::clearAllMessages();
    break;
  case EKey_code::NONE:
  default: {
    static unsigned int prev_TC0 = LPC_TIM0->TC;
    unsigned int dTrs = LPC_TIM0->TC - prev_TC0;
    if (dTrs >= 10000000) {  // 1sec
      prev_TC0 = LPC_TIM0->TC;
      rotate_messages();
    }
  }
  }
}

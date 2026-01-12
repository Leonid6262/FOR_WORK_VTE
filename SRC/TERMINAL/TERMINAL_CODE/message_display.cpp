#include "message_display.hpp"
#include "string_utils.hpp"

CMessageDisplay::CMessageDisplay(CTerminalUartDriver& uartDrv) : 
  uartDrv(uartDrv), l(CEEPSettings::getInstance().getSettings().Language - 1),
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

// Вывод сообщения на терминал
/*void CMessageDisplay::render_messages(signed char cat, bool print_title) {
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
*/
// Отправки строки
void CMessageDisplay::sendLine(const std::string& s, bool newline) {
    std::string text = StringUtils::padTo16(s);
    text += newline ? "\r\n" : "\r";
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
}

// Ищет следующий активный индекс начиная с позиции start.
// Возвращает индекс, либо -1 если активных нет.
int CMessageDisplay::find_next_active(SCategoryContext& ctx, int start) {
    const int n = ctx.count;
    for (int k = 0; k < n; ++k) {
        int i = (start + k) % n;
        if (ctx.active[i]) {
            return i; // нашли активное сообщение
        }
    }
    return -1; // нет активных
}

// Выводит заголовок категории (если print_title == true) и сообщение по текущему cursor
void CMessageDisplay::render_messages(signed char cat, bool print_title) {
    if (cat == not_mes) {
        sendLine("Not", newline);
        sendLine("Messages");
        return;
    }

    // ВАЖНО: приводим индекс к unsigned, чтобы избежать отрицательных значений
    const unsigned char ucat = static_cast<unsigned char>(cat);
    const auto& ctx = contexts[ucat];

    // Заголовок печатается строго по флагу
    if (print_title) {
        sendLine(StringUtils::utf8_to_cp1251(ctx.NAME[l]), newline);
    }

    // Сообщение печатается всегда
    sendLine(StringUtils::utf8_to_cp1251(ctx.MSG[ctx.cursor][l]), newline);
}

void CMessageDisplay::rotate_messages() {
    static unsigned char cur_cat = 0;   // текущая категория
    static unsigned char emp_num = 0;   // счётчик пустых категорий
    static bool print_title = true;     // печатать заголовок при входе в категорию

    if (COUNT_CATEGORIES == 0) {
        render_messages(not_mes, false);
        return;
    }

    SCategoryContext& ctx = contexts[cur_cat];
    int next_idx = find_next_active(ctx, ctx.cursor);

    if (next_idx >= 0) {
        // нашли активное сообщение
        ctx.cursor = static_cast<unsigned char>(next_idx);

        // выводим заголовок при первом сообщении категории
        render_messages(cur_cat, print_title);
        if (print_title) {
            print_title = false; // сбрасываем только после вывода заголовка
        }

        // готовим cursor к следующему поиску
        ctx.cursor = static_cast<unsigned char>((ctx.cursor + 1) % ctx.count);
        emp_num = 0;

        // если вернулись в начало — переключаем категорию
        if (ctx.cursor == 0) {
            cur_cat = (cur_cat + 1) % COUNT_CATEGORIES;
            print_title = true; // при входе в новую категорию снова печатаем заголовок
        }
        return;
    }

    // если в категории нет активных сообщений
    ctx.cursor = 0;

    // проверяем, есть ли вообще активные сообщения в этой категории
    bool has_active = false;
    for (int i = 0; i < ctx.count; ++i) {
        if (ctx.active[i]) { has_active = true; break; }
    }

    cur_cat = (cur_cat + 1) % COUNT_CATEGORIES;
    print_title = true;

    if (!has_active) {
        emp_num++;
        if (emp_num >= COUNT_CATEGORIES) {
            emp_num = 0;
            render_messages(not_mes, false);
            return;
        }
    } else {
        emp_num = 0; // категория не пустая, сбрасываем счётчик
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
    if (dTrs >= 20000000) {  // 2sec
      prev_TC0 = LPC_TIM0->TC;
      rotate_messages();
    }
  }
  }
}

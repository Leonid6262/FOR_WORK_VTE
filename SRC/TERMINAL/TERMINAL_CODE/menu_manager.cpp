#include "menu_manager.hpp"
#include <algorithm>
#include "string_utils.hpp"
#include "menu_factory.hpp"
#include "pause_us.hpp"
#include "settings_eep.hpp"

CMenuManager::CMenuManager(CTerminalUartDriver& uartDrv) : uartDrv(uartDrv) {
  // очистка экрана
  unsigned char clr_data[] = {"                \r\n"};
  uartDrv.sendBuffer(clr_data, sizeof(clr_data));
  uartDrv.sendBuffer(clr_data, sizeof(clr_data));
  uartDrv.sendBuffer(clr_data, sizeof(clr_data));

  // выключение светодиода
  unsigned char led_off[] = {static_cast<unsigned char>(ELED::LED_OFF), '\r'};
  uartDrv.sendBuffer(led_off, sizeof(led_off));

  listIndex = 0;
  screenPosition = 0;
  cursorLine = 0;
  start_e_data.edit_mode = false;
  d.i_delta_max = 10000.0f;
  d.f_delta_max = 10000;
  d.show_delta = false;
  d.delta_timer_ms = 0;
  
  // Создание дерева узлов меню
  MENU = MENU_Factory(CADC_STORAGE::getInstance(), CEEPSettings::getInstance());
  MENU.shrink_to_fit();
  currentList = &MENU;
  render_menu();        // первая отрисовка
}

// Конструктор узла универсальный (все поля из меню)
CMenuManager::MenuNode::MenuNode(const char* t,
                                 std::vector<MenuNode> c,
                                 void* v,
                                 const char* u,
                                 float s,
                                 unsigned char p,
                                 NProxyVar::EVarType vt,
                                 ENodeMode m,
                                 float min_value,
                                 float max_value)
: title(t),
children(std::move(c)),
pVariable(v),
unit(u),
scale(s),
precision(p),
varType(vt),
mode(m),
min_value(min_value),
max_value(max_value) {}

// Конструктор узла переменных реестра
CMenuManager::MenuNode::MenuNode(const char* t,
                                 std::vector<MenuNode> c,
                                 NProxyVar::ProxyVarID id,
                                 unsigned char p,
                                 ENodeMode m,
                                 float min_value,
                                 float max_value)
: title(t),
children(std::move(c)),
precision(p),
mode(m),
min_value(min_value),
max_value(max_value)
{
  CProxyPointerVar& ppv = CProxyPointerVar::getInstance();
  pVariable = ppv.gE(id).ptr;
  unit      = ppv.gE(id).unit;   
  scale     = ppv.gE(id).c_disp;
  varType   = ppv.gE(id).type;
}

// DualReg
CMenuManager::MenuNode
CMenuManager::MenuNode::DualReg(const char* title1,
                                NProxyVar::ProxyVarID id1,
                                unsigned char precision1,
                                const char* title2,
                                NProxyVar::ProxyVarID id2,
                                unsigned char precision2,
                                ENodeMode m,
                                float min_value,
                                float max_value) {
    MenuNode child1(title1, {}, id1, precision1, ENodeMode::NONE);
    MenuNode child2(title2, {}, id2, precision2, ENodeMode::NONE, min_value, max_value);
    return MenuNode("",
                    {std::move(child1), std::move(child2)},
                    nullptr, "",
                    1.0f, 0,
                    NProxyVar::EVarType::NONE,
                    m);
}

// DualRaw
CMenuManager::MenuNode
CMenuManager::MenuNode::DualRaw(const char* title1,
                                void* pVar1,
                                const char* unit1,
                                float scale1,
                                unsigned char precision1,
                                NProxyVar::EVarType type1,
                                const char* title2,
                                void* pVar2,
                                const char* unit2,
                                float scale2,
                                unsigned char precision2,
                                NProxyVar::EVarType type2,
                                ENodeMode m,
                                float min_value,
                                float max_value) {
    MenuNode child1(title1, {}, pVar1, unit1, scale1, precision1, type1, ENodeMode::NONE);
    MenuNode child2(title2, {}, pVar2, unit2, scale2, precision2, type2, ENodeMode::NONE, min_value, max_value);
    return MenuNode("",
                    {std::move(child1), std::move(child2)},
                    nullptr, "",
                    1.0f, 0,
                    NProxyVar::EVarType::NONE,
                    m);
}

void CMenuManager::set_pTerminal(CTerminalManager* pTerminal_manager){
  this->pTerminal_manager = pTerminal_manager;
}

// "Опрос" клавиатуры
void CMenuManager::get_key() {
  unsigned char input_key;
  if (uartDrv.poll_rx(input_key)) {
    Key_Handler(static_cast<EKey_code>(input_key));
  } else {
    Key_Handler(EKey_code::NONE);  // нет нажатий
  }
}

// Manager Renders
void CMenuManager::render_node(ETitleType title_type) {
  unsigned short listIndex = screenPosition + cursorLine;
  const auto& node = (*currentList)[listIndex];
  
  switch (node.mode) {
  case ENodeMode::In1V:
  case ENodeMode::Ed1V:
    render_1var(title_type);  // рендер окна с однрй переменной
    break;
  case ENodeMode::In2V:
  case ENodeMode::IE2V:
    render_2var(title_type);  // рендер окна с двумч переменными
    break;
  case ENodeMode::NONE:
    // дети Dual никогда напрямую не рендерятся
    break;
  default:
    break;
  }
}

// Отображение двух строк меню. Рендер Меню.
void CMenuManager::render_menu() {
  // выводим два элемента списка начиная с screenPosition
  for (int line = 0; line < 2; ++line) {
    int listIndex = screenPosition + line;
    std::string text;
    if (listIndex < currentList->size()) {
      text = StringUtils::utf8_to_cp1251((*currentList)[listIndex].title);
    } else {
      text = "";
    }
    // добавляем курсор, если это текущая строка
    if (line == cursorLine) {
      text = "-" + text;
    } else {
      text = " " + text;
    }
    text = StringUtils::padTo16(text);
    text += (line == 0 ? "\r\n" : "\r");
    
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
  }
}

// Отображение окна с одной переменной
void CMenuManager::render_1var(ETitleType title_type) {
  unsigned short listIndex = screenPosition + cursorLine;
  const auto& node = (*currentList)[listIndex];
  if (title_type == ETitleType::TitleName) {
    // верхняя строка — имя переменной
    std::string title_cp1251 = StringUtils::utf8_to_cp1251(node.title);  // UTF-8 -> CP1251
    //std::string title = StringUtils::padRight(title_cp1251, (disp_l - node.unit.size())) + node.unit + "\r\n";
    unsigned short  unitLen = node.unit ? strlen(node.unit) : 0; 
    std::string title = StringUtils::padRight(title_cp1251, (disp_l - unitLen));
    if (unitLen > 0) { title += node.unit; } 
    title += "\r\n";
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(title.c_str()), title.size());
  }
  // нижняя строка — значение переменной
  char buf[disp_l + 1];
  formatValue(node, buf, sizeof(buf));  // вспомогательная функция форматирования
  std::string value(buf);
  if (start_e_data.edit_mode) {
    value += "<";
    // Добавляем справа значение текущей дельты 
    char deltaBuf[disp_l >> 1]; 
    switch (start_e_data.var_type) { 
    case NProxyVar::EVarType::sshort: 
    case NProxyVar::EVarType::ushort: 
      snprintf(deltaBuf, sizeof(deltaBuf), "%d", d.i_delta_cur); 
      break; 
    case NProxyVar::EVarType::vfloat: 
      snprintf(deltaBuf, sizeof(deltaBuf), "%.4f", d.f_delta_cur); 
      break; 
    default: 
      deltaBuf[0] = '\0'; 
      break; 
    } // Выравнивание: значение слева, дельта справа 
    std::string delta(deltaBuf); 
    int totalLen = value.size() + delta.size(); 
    if (totalLen < disp_l) { 
      // добавляем пробелы между значением и дельтой 
      value += std::string(disp_l - totalLen, ' '); 
    } 
    value += delta;
  }
  value = StringUtils::padTo16(value) + "\r";
  uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(value.c_str()), value.size());
}

// Отображение окна с двумя переменными
void CMenuManager::render_2var(ETitleType title_type) {
  unsigned short listIndex = screenPosition + cursorLine;
  const auto& parent = (*currentList)[listIndex];
  
  if (parent.children.size() < 2) return;  // защита
  
  const auto& node1 = parent.children[0];
  const auto& node2 = parent.children[1];
  
  if (title_type != ETitleType::NoTitle) {
    std::string title1;
    std::string title2;
    switch (title_type) {
    case ETitleType::TitleName:
      title1 = node1.title ? StringUtils::utf8_to_cp1251(node1.title) : "";
      title2 = node2.title ? StringUtils::utf8_to_cp1251(node2.title) : "";
      break;
    case ETitleType::TitleUnit:
      title1 = node1.unit ? StringUtils::utf8_to_cp1251(node1.unit) : "";
      title2 = node2.unit ? StringUtils::utf8_to_cp1251(node2.unit) : "";
      break;
      
    default:
      break;
    }
    // ограничиваем до 7 символов
    if (title1.size() > 7) title1 = title1.substr(0, 7);
    if (title2.size() > 7) title2 = title2.substr(0, 7);
    // формируем строку: имя1 (позиция 1), имя2 (позиция 10)
    std::string line = StringUtils::padRight(title1, 9) + StringUtils::padRight(title2, 7) + "\r\n";
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(line.c_str()), line.size());
  }
  // нижняя строка — значения переменных
  char buf1[disp_l >> 1], buf2[disp_l >> 1];
  formatValue(node1, buf1, sizeof(buf1));  // вспомогательная функция форматирования
  formatValue(node2, buf2, sizeof(buf2));
  
  // ограничиваем до 6 символов
  std::string value1(buf1);
  std::string value2(buf2);
  if (value1.size() > 6) value1 = value1.substr(0, 6);
  if (value2.size() > 6) value2 = value2.substr(0, 6);
  if (start_e_data.edit_mode) { 
    if (d.show_delta) { 
      // формируем строку дельты 
      char deltaBuf[disp_l]; 
      switch (start_e_data.var_type) { 
      case NProxyVar::EVarType::sshort: 
      case NProxyVar::EVarType::ushort: 
        snprintf(deltaBuf, sizeof(deltaBuf), "%d", d.i_delta_cur); 
        break; 
      case NProxyVar::EVarType::vfloat: 
        snprintf(deltaBuf, sizeof(deltaBuf), "%.4f", d.f_delta_cur); 
        break; default: 
          deltaBuf[0] = '\0'; 
          break; 
      } 
      std::string delta(deltaBuf); // строка = value1 + пробелы + delta справа 
      std::string line = value1; int totalLen = value1.size() + delta.size(); 
      if (totalLen < disp_l) { 
        line += std::string(disp_l - totalLen, ' '); 
      } 
      line += delta; line += "\r"; 
      uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(line.c_str()), line.size()); return; 
      // выходим, чтобы не формировать обычную строку ниже 
    } else { 
      value2 += "<"; // обычный маркер редактирования 
    } 
  }
  // формируем строку: значение1 (позиция 1), значение2 (позиция 10)
  std::string line = StringUtils::padRight(value1, 9) + StringUtils::padRight(value2, 7) + "\r";
  uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(line.c_str()), line.size());
}

void CMenuManager::Key_Handler(EKey_code key) {
  unsigned short listIndex = screenPosition + cursorLine;
  const auto& node = (*currentList)[listIndex];  // ← берём текущий узел
  
  switch (key) {
  case EKey_code::DOWN:
    if (node.mode == ENodeMode::Menu)
      navigateDownMenu();
    else
      navigateDownVar();
    break;
    
  case EKey_code::UP:
    if (node.mode == ENodeMode::Menu)
      navigateUpMenu();
    else
      navigateUpVar();
    break;
    
  case EKey_code::ENTER:
    handleEnter();
    break;
    
  case EKey_code::ESCAPE:
    handleEscape();
    break;
    
  case EKey_code::FnENTER: 
    save_settings();
    break;
    
  case EKey_code::FnUP:
    edit_delta(10.0f);
    break;    
  case EKey_code::FnDOWN:
    edit_delta(0.1f);
    break;
  case EKey_code::FnEsc:
    edit_delta(0);
    break;   
    
  case EKey_code::NONE:
  default: {
    static unsigned int prev_TC0 = LPC_TIM0->TC;
    unsigned int dTrs = LPC_TIM0->TC - prev_TC0;
    
    static ETitleType currentTitle = ETitleType::TitleName;
    static unsigned int elapsed_ms = 0;
    
    if (dTrs >= 2000000) {  // 200 мс
      
      prev_TC0 = LPC_TIM0->TC;
      elapsed_ms += dTrs / 10000; // пересчёт в мс      
      d.delta_timer_ms += dTrs / 10000;
      
      if (d.show_delta && d.delta_timer_ms >= DELTA_DURATION_MS) { 
        d.show_delta = false;       
      }
      
      switch (node.mode) {
      case ENodeMode::In2V:
      case ENodeMode::IE2V:       
        if (currentTitle == ETitleType::TitleName) { 
          if (elapsed_ms >= DATA_DURATION_MS) { 
            currentTitle = ETitleType::TitleUnit; 
            render_node(currentTitle); 
            elapsed_ms = 0;
            break;
          } 
        } else { // TitleUnit 
          if (elapsed_ms >= UNIT_DURATION_MS) { 
            currentTitle = ETitleType::TitleName;
            render_node(currentTitle); 
            elapsed_ms = 0;
            break;
          } 
        } 
        render_node(ETitleType::NoTitle);
        break;        
      case ENodeMode::In1V:
      case ENodeMode::Ed1V:
        render_node(ETitleType::NoTitle);
        break;        
      case ENodeMode::Menu:
        // меню не перерисовываем
        break;       
      case ENodeMode::NONE:
        // дети Dual не рендерятся напрямую
        break;        
      default:
        break;
      }
    }
  }
  }
}

// Навигация вниз по меню
void CMenuManager::navigateDownMenu() {
  unsigned short listIndex = screenPosition + cursorLine;
  if (listIndex + 1 < currentList->size()) {
    if (cursorLine == FirstLine) {
      cursorLine = SecondLine;
    } else {
      screenPosition++;
    }
  } else {
    // циклический переход: с последнего на первый
    screenPosition = 0;
    cursorLine = FirstLine;
  }
  render_menu();
}
// Навигация вверх по меню
void CMenuManager::navigateUpMenu() {
  unsigned short listIndex = screenPosition + cursorLine;
  if (listIndex > 0) {
    if (cursorLine == SecondLine) {
      cursorLine = FirstLine;
    } else {
      screenPosition--;
    }
  } else {
    // циклический переход: с первого на последний
    if (currentList->size() > 1) {
      screenPosition = currentList->size() - 2;
      cursorLine = SecondLine;
    } else {
      screenPosition = 0;
      cursorLine = FirstLine;
    }
  }
  render_menu();
}
// Навигация вниз по узлам с переменными / редактированием
void CMenuManager::navigateDownVar() {
  unsigned short listIndex = screenPosition + cursorLine;

  if (start_e_data.edit_mode) {
    auto& node = (*currentList)[listIndex];
    if (node.mode == ENodeMode::IE2V && node.children.size() == 2) {
      auto& node2 = node.children[1];
      editValue(node2, -1);
      return;
    } else if (node.mode == ENodeMode::Ed1V) {
      editValue(node, -1);
      return;
    }
  }

  if (listIndex + 1 < currentList->size()) {
    if (cursorLine == FirstLine) {
      cursorLine = SecondLine;
    } else {
      screenPosition++;
    }
  } else {
    screenPosition = 0;
    cursorLine = FirstLine;
  }
  render_node(ETitleType::TitleName);
}
// Навигация вверх по узлам с переменными / редактирование
void CMenuManager::navigateUpVar() {
  
  unsigned short listIndex = screenPosition + cursorLine;

  if (start_e_data.edit_mode) {
    auto& node = (*currentList)[listIndex];
    if (node.mode == ENodeMode::IE2V) {
      auto& node2 = node.children[1];
      editValue(node2, +1);                     // Редактирование второй переменной дуал узла
      return;
    } else if (node.mode == ENodeMode::Ed1V) {
      editValue(node, +1);                      // Редактирование переменной одиночного узла
      return;
    }
  }

  if (listIndex > 0) {
    if (cursorLine == SecondLine) {
      cursorLine = FirstLine;
    } else {
      screenPosition--;
    }
  } else {
    if (currentList->size() > 1) {
      screenPosition = currentList->size() - 2;
      cursorLine = SecondLine;
    } else {
      screenPosition = 0;
      cursorLine = FirstLine;
    }
  }
  render_node(ETitleType::TitleName);
}
// Обработка ENTER
void CMenuManager::handleEnter() {
  unsigned short listIndex = screenPosition + cursorLine;
  auto& node = (*currentList)[listIndex];

  if (node.mode != ENodeMode::Menu) {
    // переход в режим редактирования и обратно
    if (!start_e_data.edit_mode && (node.mode == ENodeMode::IE2V || node.mode == ENodeMode::Ed1V)) {
      enterEditMode(node);
    } else {
      start_e_data.edit_mode = false;
    }
    return;
  }
  // 1. Нет детей — нечего открывать
  if (node.children.empty()) return;
  // 2. Сохраняем текущее состояние
  history.push({currentList, screenPosition, cursorLine, listIndex});
  // 3. Определяем тип уровня по первому ребёнку
  const bool firstIsLeaf = node.children.front().mode != ENodeMode::Menu;
  // 4. Если это уровень переменных (листы) — сразу открываем первый узел
  if (firstIsLeaf) {
    currentList = &node.children;
    screenPosition = 0;
    cursorLine = FirstLine;
    render_node(ETitleType::TitleName);
    return;
  }
  // 5. Иначе — обычный переход внутрь меню
  currentList = &node.children;
  screenPosition = 0;
  cursorLine = FirstLine;
  render_menu();
}

// Обработка ESCAPE
void CMenuManager::handleEscape() {
  if (start_e_data.edit_mode) {
    exitEditMode();
    return;
  }
  if (!history.empty()) {
    SFrame f = history.top();
    history.pop();

    currentList = f.currentList;
    screenPosition = f.screenPosition;
    cursorLine = f.cursorLine;

    render_menu();
  }
}

//  Обработка Fn+UP, Fn+DOWN и Fn+ESC (изменеие приращения)
void CMenuManager::edit_delta(float mul) {
  if(!start_e_data.edit_mode) return;

  switch (start_e_data.var_type) {
  case NProxyVar::EVarType::sshort:
  case NProxyVar::EVarType::ushort:
    {
      if(mul == 0){
        d.i_delta_cur = d.i_delta_min;
        d.show_delta = true; 
        d.delta_timer_ms = 0;
        break;
      }
      unsigned short candidate = d.i_delta_cur * mul;
      if((candidate > 0 ) && (candidate < d.i_delta_max)) {
        d.i_delta_cur = candidate;
      }
    }
    d.show_delta = true; 
    d.delta_timer_ms = 0;
    break;
  case NProxyVar::EVarType::vfloat:
    {
      if(mul == 0){
        d.f_delta_cur = d.f_delta_min;
        break;
      }
      float candidate = d.f_delta_cur * mul;
      if((candidate >= d.f_delta_min) && (candidate < d.f_delta_max)) {
        d.f_delta_cur = candidate;
      }
    }
    d.show_delta = true; 
    d.delta_timer_ms = 0;
    break;
  default:
    break;
  }
}

// Обработка Fn+ENTER (запись уставок в EEPROM)
void CMenuManager::save_settings() {
  unsigned char led_green[] = {static_cast<unsigned char>(ELED::LED_GREEN), '\r'};
  uartDrv.sendBuffer(led_green, sizeof(led_green));
  CEEPSettings::getInstance().saveSettings();
  Pause_us(200000);
  unsigned char led_off[] = {static_cast<unsigned char>(ELED::LED_OFF), '\r'};
  uartDrv.sendBuffer(led_off, sizeof(led_off));
}

// Вход в режим редактирования
void CMenuManager::enterEditMode(MenuNode& parent) { 
  start_e_data.edit_mode = true; 
  MenuNode* targetNode = nullptr; 
  
  if (parent.mode == ENodeMode::IE2V && parent.children.size() >= 2) { 
    targetNode = &parent.children[1]; 
  } else if (parent.mode == ENodeMode::Ed1V) { 
    targetNode = &parent;
  }
  
  if (!targetNode) return; 
  
  start_e_data.pVar = targetNode->pVariable; 
  start_e_data.var_type = targetNode->varType; 
  
  switch (targetNode->varType) { 
  case NProxyVar::EVarType::sshort: 
    start_e_data.original.sshort_val = *static_cast<short*>(targetNode->pVariable); 
    d.i_delta_min = static_cast<int>(std::ceil(std::pow(10.0, -targetNode->precision) / targetNode->scale)); 
    d.i_delta_cur = d.i_delta_min; 
    break; 
  case NProxyVar::EVarType::ushort: 
    start_e_data.original.ushort_val = *static_cast<unsigned short*>(targetNode->pVariable); 
    d.i_delta_min = static_cast<int>(std::ceil(std::pow(10.0, -targetNode->precision) / targetNode->scale)); 
    d.i_delta_cur = d.i_delta_min; 
    break; 
  case NProxyVar::EVarType::vfloat: 
    start_e_data.original.float_val = *static_cast<float*>(targetNode->pVariable); 
    d.f_delta_min = 1.0f / std::pow(10.0f, targetNode->precision); d.f_delta_cur = d.f_delta_min; 
    break; 
    
  default: 
    if (bitIndex(targetNode->varType) >= 0) { 
      start_e_data.original.bit_in_char = *static_cast<char*>(targetNode->pVariable); 
      d.f_delta_min = 1.0f; d.f_delta_cur = d.f_delta_min; 
    } break; 
    
  } 
  render_node(ETitleType::NoTitle);
}

void CMenuManager::exitEditMode() {
  if (!start_e_data.edit_mode) return;
  
  switch (start_e_data.var_type) {
  case NProxyVar::EVarType::sshort:
    *static_cast<short*>(start_e_data.pVar) = start_e_data.original.sshort_val;
    break;
  case NProxyVar::EVarType::ushort:
    *static_cast<unsigned short*>(start_e_data.pVar) = start_e_data.original.ushort_val;
    break;
  case NProxyVar::EVarType::vfloat:
    *static_cast<float*>(start_e_data.pVar) = start_e_data.original.float_val;
    break;
  default:
    // Если тип переменнрй бит байта 
    if (bitIndex(start_e_data.var_type) >= 0) { 
      *static_cast<unsigned char*>(start_e_data.pVar) = start_e_data.original.bit_in_char; 
    }
    break;
  }  
  start_e_data.edit_mode = false;
  render_node(ETitleType::NoTitle);
}

void CMenuManager::editValue(MenuNode& node, signed short dir) {
  switch (node.varType) {
  case NProxyVar::EVarType::sshort: {
    short* pRaw = static_cast<short*>(node.pVariable);
    int raw_copy = *pRaw;  
    int candidate = raw_copy + dir * d.i_delta_cur;
    float disp = candidate * node.scale;
    
    if (disp > node.max_value) {
      raw_copy = static_cast<int>(node.max_value / node.scale);
    } else if (disp < node.min_value) {
      raw_copy = static_cast<int>(node.min_value / node.scale);
    } else {
      raw_copy = candidate;
    }   
    *pRaw = static_cast<short>(raw_copy);
    break;
  }
  case NProxyVar::EVarType::ushort: {
    unsigned short* pRaw = static_cast<unsigned short*>(node.pVariable);
    int raw_copy = *pRaw;
    int candidate = raw_copy + dir * d.i_delta_cur;
    float disp = candidate * node.scale;
    
    if (disp > node.max_value) {
      raw_copy = static_cast<int>(node.max_value / node.scale);
    } else if (disp < node.min_value) {
      raw_copy = static_cast<int>(node.min_value / node.scale);
    } else {
      raw_copy = candidate;
    }    
    *pRaw = static_cast<unsigned short>(raw_copy);
    break;
  }
  case NProxyVar::EVarType::vfloat: {
    float* pRaw = static_cast<float*>(node.pVariable);
    float raw_copy = *pRaw;
    float candidate = raw_copy + dir * d.f_delta_cur;
    
    if (candidate > node.max_value) {
      raw_copy = node.max_value;
    } else if (candidate < node.min_value) {
      raw_copy = node.min_value;
    } else {
      raw_copy = candidate;
    }   
    *pRaw = raw_copy;
    break;
  }
  default:
    // Если тип переменнрй бит байта 
    if (bitIndex(node.varType) >= 0) { 
      unsigned char* pRaw = static_cast<unsigned char*>(node.pVariable); 
      unsigned char bit = bitIndex(node.varType); 
      if (dir > 0) { *pRaw |= (1 << bit); 
      } else if (dir < 0) { 
        *pRaw &= ~(1 << bit); 
      }
    }
    break;
  }
  render_node(ETitleType::NoTitle);
}

void CMenuManager::formatValue(const MenuNode& node, char* buf, unsigned short bufSize) {
  switch (node.varType) {
  case NProxyVar::EVarType::sshort: {
    signed short raw = *static_cast<signed short*>(node.pVariable);
    float val = raw * node.scale;
    snprintf(buf, bufSize, "%.*f", node.precision, val);
    break;
  }
  case NProxyVar::EVarType::ushort: {
    unsigned short raw = *static_cast<unsigned short*>(node.pVariable);
    float val = raw * node.scale;
    snprintf(buf, bufSize, "%.*f", node.precision, val);
    break;
  }
  case NProxyVar::EVarType::vfloat: {
    float raw = *static_cast<float*>(node.pVariable);
    float val = raw * node.scale;
    snprintf(buf, bufSize, "%.*f", node.precision, val);
    break;
  }
  case NProxyVar::EVarType::vbool: {
    bool raw = *static_cast<bool*>(node.pVariable);
    snprintf(buf, bufSize, "%s", raw ? "ON" : "OFF");
    break;
  }
  case NProxyVar::EVarType::char2b: {
    unsigned char raw = *static_cast<unsigned char*>(node.pVariable);
    std::string bits = StringUtils::charToBits(raw);
    snprintf(buf, bufSize, "%s", bits.c_str());
    break;
  }
  case NProxyVar::EVarType::short2b: {
    unsigned short raw = *static_cast<unsigned short*>(node.pVariable);
    std::string bits = StringUtils::shortToBits(raw);
    snprintf(buf, bufSize, "%s", bits.c_str());
    break;
  }
  case NProxyVar::EVarType::text: {
    const char* raw = static_cast<const char*>(node.pVariable);
    snprintf(buf, bufSize, "%s", raw ? raw : "");
    break;
  }
  default:
    if (bitIndex(node.varType) >= 0) {
      unsigned char raw = *static_cast<unsigned char*>(node.pVariable); 
      unsigned char bit = bitIndex(node.varType); 
      bool val = (raw & (1 << bit)) != 0; snprintf(buf, bufSize, "%s", val ? "ON" : "OFF"); 
      break; 
    }
    snprintf(buf, bufSize, "----");
    break;
  }
}

// Возвращает индекс бита или -1 
unsigned char CMenuManager::bitIndex(NProxyVar::EVarType t) {
  switch(t) { 
  case NProxyVar::EVarType::eb_0: return 0; 
  case NProxyVar::EVarType::eb_1: return 1; 
  case NProxyVar::EVarType::eb_2: return 2; 
  case NProxyVar::EVarType::eb_3: return 3; 
  case NProxyVar::EVarType::eb_4: return 4; 
  case NProxyVar::EVarType::eb_5: return 5; 
  case NProxyVar::EVarType::eb_6: return 6; 
  case NProxyVar::EVarType::eb_7: return 7; 
  default: return -1; // не битовый тип 
  } 
} 

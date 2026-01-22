#pragma once

#include <stack>
#include <string>
#include <vector>

#include "terminalUartDriver.hpp"
#include "terminal_manager.hpp"
#include "proxy_pointer_var.hpp"
#include "_SystemManager.hpp" 
#include "rtc.hpp"

class CMenuNavigation {
public:
  CMenuNavigation(CTerminalUartDriver&, CSystemManager&, CRTC&, CEEPSettings&);
  void get_key();
  void set_pTerminal(CTerminalManager*);
  void first_render();
  
  // Типы узлов
  enum class ENodeMode {
    NONE,
    Menu,  // Меню
    In1V,  // Индикация одной переменной
    In2V,  // Индикация двух переменных
    Ed1V,  // Редактирование одной переменной
    IE2V   // Индикация одной переменных и редактирование другой переменной
  };
  
  /* Рекурсивная структура узла меню */
  using et = NProxyVar::EVarType;
  
  struct MenuNode {
    const char* title;                  // Название меню/Имя переменной (указатель на литерал/таблицу)
    std::vector<MenuNode> children;     // Вложенный узел
    void* pVariable;                    // Указатель отображаемой/редактируемой переменной
    const char* unit;                   // Единицы измерения (указатель на литерал/таблицу)
    float scale;                        // Коэффициент отображения
    unsigned char precision;            // Количество знаков после запятой
    et varType;                         // Тип переменной
    ENodeMode mode;                     // Типы узлов
    float min_value;                    // Min
    float max_value;                    // Max
    
    // Конструктор
    MenuNode(const char* t,
             std::vector<MenuNode> c = {},
             void* v = nullptr,
             const char* u = "",
             float s = 1.0f,
             unsigned char p = 0,
             et vt = et::NONE,
             ENodeMode m = ENodeMode::Menu,
             float min = -99999.0f,
             float max = +99999.0f);       
    
    // Фабрика дуал-узла
    static MenuNode Dual(const char* title1,
                         void* pVar1,
                         const char* unit1,
                         float scale1,
                         unsigned char precision1,
                         et type1,
                         const char* title2,
                         void* pVar2,
                         const char* unit2,
                         float scale2,
                         unsigned char precision2,
                         et type2,
                         ENodeMode m,
                         float min = -99999.0f,
                         float max = +99999.0f);
  };
  
  
private:
  CTerminalUartDriver& uartDrv;
  CTerminalManager* pTerminal_manager;
  CEEPSettings& rSet;
  CRTC& rRTC;
  
  struct SFrame {
    std::vector<MenuNode>* currentList;  // указатель на список
    unsigned short screenPosition;       // позиция окна
    unsigned short cursorLine;           // строка курсора (0 или 1)
    unsigned short listIndex;            // фактический индекс выбранного узла
  };
  
  std::vector<MenuNode> MENU;          // всё меню
  std::vector<MenuNode>* currentList;  // указатель на текущий список
  std::stack<SFrame> history;          // стек истории
  
  enum class ETitleType { 
    TitleName,  // Вывод имён переменных и значений
    TitleUnit,  // Вывод размерностей и значений
    NoTitle     // Вывод только значений
  };
  
  // Приращения в режиме редактирования
  struct { 
    signed int i_delta_max; 
    signed int i_delta_cur;
    signed int i_delta_min;
    float f_delta_max;
    float f_delta_cur;
    float f_delta_min;
    bool show_delta;
    unsigned int delta_timer_ms;
  } d;
    
  // Структура для хранения исходного значения при входе в режим редактирования
  struct StartEditingData {
    void* pVar;                         // указатель на редактируемую переменную
    bool edit_mode;                     // режим редактирования
    NProxyVar::EVarType var_type;       // тип переменной    
    union {
      short sshort_val;
      unsigned short ushort_val;
      float float_val;
      char bit_in_char;
    } original;                         // исходные значения
  } start_e_data;
      
  void navigateDownVar();
  void navigateDownMenu();
  void navigateUpVar();
  void navigateUpMenu();
  void handleEnter();
  void handleEscape();
  void save_settings();
  void edit_delta(float);
  
  void Key_Handler(EKey_code);
  void render_node(ETitleType);
  
  void render_menu();
  void render_1var(ETitleType);
  void render_2var(ETitleType);
  
  void editValue(MenuNode&, signed short);
  void enterEditMode(MenuNode&);
  void exitEditMode();
  
  unsigned char bitIndex(NProxyVar::EVarType);
  void formatValue(const MenuNode&, char*, unsigned short); 
  
  unsigned char listIndex;       // текущий индекс
  unsigned char screenPosition;  // индекс первой строки окна
  unsigned char cursorLine;      // позиция курсора, первая строка / вторая строка
    
  // Константы длительности (в мс)  
  static constexpr unsigned int DATA_DURATION_MS = 2000; 
  static constexpr unsigned int UNIT_DURATION_MS = 500;
  static constexpr unsigned int DELTA_DURATION_MS = 1000;
  
  static constexpr unsigned char disp_l = 16;
  static constexpr unsigned char FirstLine = 0;
  static constexpr unsigned char SecondLine = 1;
};

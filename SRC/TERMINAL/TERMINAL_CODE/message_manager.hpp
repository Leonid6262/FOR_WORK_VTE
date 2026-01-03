#pragma once
#include "message_factory.hpp"
#include "terminalUartDriver.hpp"
#include "terminal_manager.hpp"
#include <string>

class CTerminalManager;

class CMessageManager { 
 
public: 
  CMessageManager(CTerminalUartDriver&);
  void get_key();
  void set_pTerminal(CTerminalManager*);
  
private:
  CTerminalUartDriver& uartDrv;  
  CTerminalManager* pTerminal_manager;
  
  unsigned short l;
  const unsigned char COUNT_CATEGORIES;
    
  // Структура Context описывает интерфейс доступа к сообщениям одной категории*.
  struct SCategoryContext
  {
    bool* active;                               // массив активостей
    unsigned char cursor;                       // текущий индекс массива
    const char* const* NAME;                    // название категории
    const char* const (*MSG)[G_CONST::Nlang];   // массив сообщений
    unsigned char count;                        // количество сообщений в категории
  };
  // *При добавлении новой категории требуется регистрация в contexts
  
  SCategoryContext contexts[static_cast<unsigned char>(ECategory::COUNT)];
  
  enum class EKey_code { 
    NONE = 0x00, 
    ESCAPE = 0x1B, 
    FnEsc = 0x79
  };
  
  static constexpr signed char not_mes = -1;
  static constexpr bool newline = true;
  
  void render_messages(signed char, bool); 
  void sendLine(const std::string&, bool newline = false);
  void Key_Handler(EKey_code);  
  void rotate_messages();
  
};


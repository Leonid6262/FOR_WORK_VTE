#pragma once
#include "message_factory.hpp"
#include "terminalUartDriver.hpp"
#include "terminal_manager.hpp"
#include "_SystemManager.hpp" 
#include "rtc.hpp"
#include <string>

class CTerminalManager;

class CMessageDisplay { 
 
public: 
  CMessageDisplay(CTerminalUartDriver&, CSystemManager&, CRTC&);
  void get_key();
  void set_pTerminal(CTerminalManager*);
  
  unsigned int prev_TC0;
  static constexpr unsigned int MESSAGE_PERIOD_TICKS = 20000000; // 2 сек
  
private:
  CTerminalUartDriver& uartDrv;  
  CSystemManager& rSysMgr;
  CTerminalManager* pTerminal_manager;
  CRTC& rRTC;
  
  unsigned short l;
  const unsigned char COUNT_CATEGORIES;
  bool first_call = true;               // Первый вызов режима
    
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
  
  static constexpr signed char data_time = -1;
  static constexpr bool newline = true;
  static constexpr unsigned char disp_l = 16;
  
  void render_messages(signed char, bool); 
  void sendLine(const std::string&, bool newline = false);
  void Key_Handler(EKey_code);  
  void rotate_messages();
  
};


#pragma once

class CMenuNavigation;
class CMessageDisplay;

class CTerminalManager {
public:
  CTerminalManager(CMenuNavigation&, CMessageDisplay&);
  void dispatch();
  void switchToMenu();
  void switchToMessages();
  
private:
  enum class ETerminalMode { Menu_mode, Mess_mode } mode;
  CMenuNavigation& menuNavigation;
  CMessageDisplay& messageDisplay;
};

enum class ELED { 
  LED_GREEN  = 0x02, 
  LED_BLUE   = 0x03,
  LED_YELLOW = 0x04,
  LED_OFF = 0x0B 
};

enum class EKey_code { 
  NONE = 0x00, 
  UP = 0x2B, 
  DOWN = 0x2D, 
  ENTER = 0x0D, 
  ESCAPE = 0x1B, 
  FnENTER = 0x78,
  FnUP = 0x3D,     
  FnDOWN = 0x5F,
  FnEsc = 0x79
};
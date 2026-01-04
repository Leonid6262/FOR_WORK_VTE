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

#pragma once

class CMenuManager;
class CMessageManager;

class CTerminalManager {
public:
  CTerminalManager(CMenuManager& menuManager, CMessageManager& messageManager);
  void dispatch();
  void switchToMenu();
  void switchToMessages();
  
private:
  enum class ETerminalMode { Menu_mode, Mess_mode } mode;
  CMenuManager& menuManager;
  CMessageManager& messageManager;
};

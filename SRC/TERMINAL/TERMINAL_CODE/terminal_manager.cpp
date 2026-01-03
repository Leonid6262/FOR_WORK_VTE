#include "terminal_manager.hpp"
#include "menu_manager.hpp"
#include "message_manager.hpp"

CTerminalManager::CTerminalManager(CMenuManager& menuManager, CMessageManager& messageManager)
: menuManager(menuManager), messageManager(messageManager), mode(ETerminalMode::Mess_mode) {}

void CTerminalManager::dispatch() {
  switch (mode) {
  case ETerminalMode::Menu_mode:
    menuManager.get_key();
    break;
  case ETerminalMode::Mess_mode:
    messageManager.get_key();
    break;
  }
}

void CTerminalManager::switchToMenu() { mode = ETerminalMode::Menu_mode; }
void CTerminalManager::switchToMessages() { mode = ETerminalMode::Mess_mode; }

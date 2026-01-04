#include "terminal_manager.hpp"
#include "menu_navigation.hpp"
#include "message_display.hpp"

CTerminalManager::CTerminalManager(CMenuNavigation& menuNavigation, CMessageDisplay& messageDisplay)
: menuNavigation(menuNavigation), messageDisplay(messageDisplay), mode(ETerminalMode::Mess_mode) {}

void CTerminalManager::dispatch() {
  switch (mode) {
  case ETerminalMode::Menu_mode:
    menuNavigation.get_key();
    break;
  case ETerminalMode::Mess_mode:
    messageDisplay.get_key();
    break;
  }
}

void CTerminalManager::switchToMenu() { mode = ETerminalMode::Menu_mode; }
void CTerminalManager::switchToMessages() { mode = ETerminalMode::Mess_mode; }

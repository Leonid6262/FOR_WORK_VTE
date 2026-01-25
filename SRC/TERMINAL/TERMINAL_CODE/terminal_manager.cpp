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

void CTerminalManager::switchToMenu() { 
  mode = ETerminalMode::Menu_mode; 
  menuNavigation.prev_TC0 = LPC_TIM0->TC - CMenuNavigation::DISPLAY_PERIOD_TICKS;
  menuNavigation.first_render();
}
void CTerminalManager::switchToMessages() {
  messageDisplay.prev_TC0 = LPC_TIM0->TC - CMessageDisplay::MESSAGE_PERIOD_TICKS;
  mode = ETerminalMode::Mess_mode; 
}

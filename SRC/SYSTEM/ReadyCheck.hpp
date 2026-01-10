#pragma once 
#include "bool_name.hpp"
#include "SystemManager.hpp"

class CSystemManager;

class CReadyCheck {
  
public:
  CReadyCheck();
  void getManager(CSystemManager*);
  void check(bool);
  
private:
  CSystemManager* pSys_manager; 
 
};
#pragma once 
#include <string>

class StringUtils { 
public: 
  static std::string utf8_to_cp1251(const std::string& utf8); 
  static std::string charToBits(unsigned char c); 
  static std::string shortToBits(unsigned short c); 
  static std::string padRight(const std::string& s, unsigned char width); 
  static std::string padTo16(const std::string& s); 
};
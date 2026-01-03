#include "string_utils.hpp" 
#include <limits>

// UTF-8 в Windows-1251
std::string StringUtils::utf8_to_cp1251(const std::string& utf8) {
  std::string cp1251;
  cp1251.reserve(utf8.size());

  for (size_t i = 0; i < utf8.size();) {
    unsigned char c = (unsigned char)utf8[i];
    if (c < 0x80) {
      cp1251.push_back(c);
      i++;
    } else if ((c == 0xD0 || c == 0xD1) && i + 1 < utf8.size()) {
      unsigned char c2 = (unsigned char)utf8[i + 1];
      unsigned char out;
      if (c == 0xD0) {
        if (c2 >= 0x90 && c2 <= 0xBF)
          out = c2 + 0x30;  // А..Я
        else if (c2 == 0x81)
          out = 0xA8;  // Ё
        else
          out = '?';
      } else {
        if (c2 >= 0x80 && c2 <= 0x8F)
          out = c2 + 0x70;  // а..п
        else if (c2 >= 0x90 && c2 <= 0x9F)
          out = c2 + 0x10;  // р..я
        else if (c2 == 0x91)
          out = 0xB8;  // ё
        else
          out = '?';
      }
      cp1251.push_back(out);
      i += 2;
    } else {
      cp1251.push_back('?');
      i++;
    }
  }
  return cp1251;
}
// Преобразование байта в строку бит
std::string StringUtils::charToBits(unsigned char c) {
  constexpr int width = std::numeric_limits<unsigned char>::digits;
  std::string out;
  out.reserve(width);
  for (int b = width - 1; b >= 0; --b) {
    out.push_back((c & (1u << b)) ? '1' : '0');
  }
  return out;
}
// Преобразование short в строку бит
std::string StringUtils::shortToBits(unsigned short c) {
  constexpr int width = std::numeric_limits<unsigned short>::digits;
  std::string out;
  out.reserve(width);
  for (int b = width - 1; b >= 0; --b) {
    out.push_back((c & (1u << b)) ? '1' : '0');
  }
  return out;
}
// Дополнение строки справа пробелами
std::string StringUtils::padRight(const std::string& s, unsigned char width) {
  if (s.size() >= width) return s.substr(0, width);
  std::string out = s;
  out.append(width - out.size(), ' ');
  return out;
}
// Дополнение строки до 16 символов
std::string StringUtils::padTo16(const std::string& s) {
  if (s.size() >= 16) return s.substr(0, 16);    // если длиннее — обрезаем
  return (s + std::string(16 - s.size(), ' '));  // дополняем пробелами
}

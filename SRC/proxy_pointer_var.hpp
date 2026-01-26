#pragma once

#include <string>

/* Prox-Singleton реестра указателей, типов и коэффициентов отображения

  Абстракция предназначена для доступа к переменным сетевых интерфейсов
  Для регистрации переменных в реестре служат методы registerVar(VarID, Pointer, c_display)
  id - ID переменной из ProxyVarID
  ptr - void* указатель
  c_disp - коэффициент отображения
  unit - единицы измерения
  Пример регистрации текущего угла управления:
  PPV.registerVar (NProxyVar::ProxyVarID::AlphaCur, &Alpha_current, cd::Alpha, NProxyVar::Unit::Deg);
  PPV - Instance CProxyPointerVar
*/

namespace NProxyVar {
  enum class EVarType {
    NONE,
    sshort,
    ushort,
    vfloat,
    vbool,
    char2b,
    short2b,
    text,
    eb_0, eb_1, eb_2,  eb_3,  eb_4,  eb_5,  eb_6,  eb_7,
    eb_8, eb_9, eb_10, eb_11, eb_12, eb_13, eb_14, eb_15
  }; 
  enum class ProxyVarID // ID переменных
  {
    Irotor,  Urotor,  Istat,    Ustat,   P,            // 0...4
    Q,       CosPhi,  AlphaCur, Rinsul,  pi0_cpu,      // 5...9
    pi0_spi,                                           // 10
    LENGTH
  };  
  struct Unit {
    static constexpr const char*   Amp = "A";
    static constexpr const char*    Hz = "Hz";
    static constexpr const char*  Volt = "V";
    static constexpr const char*    kW = "kW";
    static constexpr const char*  kVAR = "kVAR";
    static constexpr const char*   kVA = "kVA";
    static constexpr const char*   Deg = "deg";
    static constexpr const char*  bits = "bits";
    static constexpr const char*     d = "d";
    static constexpr const char*     b = "b";
    static constexpr const char*   sec = "sec";
    static constexpr const char*   ApS = "A/s";
  };
}

class CProxyPointerVar {
 public:

  struct ProxyEntry {
    void* ptr;                  // указатель
    NProxyVar::EVarType type;   // тип
    float c_disp;               // коэффициент отображения
    const char* unit;           // единицы измерения ("A", "V", "deg")
  };

  // Безопасная регистрация
  void registerVar(NProxyVar::ProxyVarID id, signed short* ptr, float c_disp = 1.0f, const char* unit = "") {
    table[(unsigned char)id] = {ptr, NProxyVar::EVarType::sshort, c_disp, unit};  // signed short*
  }

  void registerVar(NProxyVar::ProxyVarID id, unsigned short* ptr, float c_disp = 1.0f, const char* unit = "") {
    table[(unsigned char)id] = {ptr, NProxyVar::EVarType::ushort, c_disp, unit};  // unsigned short*
  }

  void registerVar(NProxyVar::ProxyVarID id, float* ptr, float c_disp = 1.0f, const char* unit = "") {
    table[(unsigned char)id] = {ptr, NProxyVar::EVarType::vfloat, c_disp, unit};  // float*
  }

  void registerVar(NProxyVar::ProxyVarID id, bool* ptr, float c_disp = 1.0f, const char* unit = "") {
    table[(unsigned char)id] = {ptr, NProxyVar::EVarType::vbool, c_disp, unit};  // bool*
  }
  
  void registerVar(NProxyVar::ProxyVarID id, unsigned char* ptr, float c_disp = 1.0f, const char* unit = "") {
    table[(unsigned char)id] = {ptr, NProxyVar::EVarType::char2b, c_disp, unit};  // char*
  }

  // Доступ к записи
  inline const ProxyEntry& gE(NProxyVar::ProxyVarID id) const { return table[(unsigned char)id]; }

  // Singleton instance
  static CProxyPointerVar& getInstance() {
    static CProxyPointerVar instance;
    return instance;
  }

private:

  ProxyEntry table[static_cast<unsigned char>(NProxyVar::ProxyVarID::LENGTH)] = {nullptr};

  CProxyPointerVar() = default;
  CProxyPointerVar(const CProxyPointerVar&) = delete;
  CProxyPointerVar& operator=(const CProxyPointerVar&) = delete;
};
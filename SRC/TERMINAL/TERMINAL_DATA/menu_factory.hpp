#pragma once

#include "AdcStorage.hpp"
#include "proxy_pointer_var.hpp"
#include "settings_eep.hpp"
#include "dIOStorage.hpp"
#include "din_cpu.hpp"
#include "version.hpp"
#include "SystemManager.hpp" 
#include "Adjustment.hpp" 

namespace menu_alias {
  using id   = NProxyVar::ProxyVarID;
  using vt   = NProxyVar::EVarType;
  using un   = NProxyVar::Unit;
  using x    = CMenuNavigation::MenuNode;
  using nm   = CMenuNavigation::ENodeMode;
  using sadc = CADC_STORAGE;
  using sbin = CDIN_STORAGE::EIBNumber;
  using sbon = CDIN_STORAGE::EOBNumber;
}

// таблица переводов (масштабируемая)
static const struct {
    const char* INDICATION[G_CONST::Nlang]     = {"ИНДИКАЦИЯ",       "INDICATION",       "ІНДИКАЦІЯ"};
    const char* CURRENT_DATA[G_CONST::Nlang]   = {"ТЕКУЩИЕ ДАННЫЕ",  "CURRENT DATA",     "ПОТОЧНІ ДАНІ"};
    const char* BIT_DATA[G_CONST::Nlang]       = {"БИТОВЫЕ ДАННЫЕ",  "BIT DATA",         "БІТОВІ ДАНІ"};
    const char* SETTINGS[G_CONST::Nlang]       = {"УСТАВКИ",         "SETTINGS",         "УСТАНОВКИ"};
    const char* REGULATORS[G_CONST::Nlang]     = {"РЕГУЛЯТОРОВ",     "REGULATORS",       "РЕГУЛЯТОРИ"};
    const char* CURRENT[G_CONST::Nlang]        = {"ТОКА",            "CURRENT",          "СТРУМУ"};
    const char* COS_PHI[G_CONST::Nlang]        = {"COS PHI",         "COS PHI",          "COS PHI"};
    const char* Q_POWER[G_CONST::Nlang]        = {"Q МОЩНОСТИ",      "Q POWER",          "Q ПОТУЖНОСТІ"};
    const char* LIMITS[G_CONST::Nlang]         = {"ОГРАНИЧЕНИЯ",     "LIMITS",           "ОБМЕЖЕННЯ"};
    const char* FAULTS[G_CONST::Nlang]         = {"АВАРИЙНЫЕ",       "FAULTS",            "АВАРІЙНІ"};
    const char* ADC_SHIFT[G_CONST::Nlang]      = {"СМЕЩЕНИЯ АЦП",    "ADC SHIFT",        "ЗСУВ АЦП"};
    const char* ADJ_MODE[G_CONST::Nlang]       = {"РЕЖИМ НАЛАДКИ",   "ADJUSTMENT MODE",  "РЕЖИМ НАЛАДКИ"};
    const char* CLOCK_SETUP[G_CONST::Nlang]    = {"УСТАНОВКА ЧАСОВ", "CLOCK SETUP",      "ГОДИННИК"};
    const char* INFO[G_CONST::Nlang]           = {"ИНФОРМАЦИЯ",      "INFO",             "ІНФОРМАЦІЯ"};
    const char* CONTROLLER_SN[G_CONST::Nlang]  = {"SN КОНТРОЛЛЕРА",  "CONTROLLER SN",    "SN КОНТРОЛЕРА"};
    const char* LANGUAGE[G_CONST::Nlang]       = {"ЯЗЫК",            "LANGUAGE",         "МОВА"};
} Mn;


// Фабрика дерева меню.
inline std::vector<menu_alias::x> MENU_Factory(CADC_STORAGE& pAdc, CEEPSettings& rSet, CSystemManager& rSysMgr) {
  auto& set = rSet.getSettings();
  //rSysMgr.rAdj_mode.set_adj_mode=9;
  using namespace menu_alias;
  
  unsigned short l = set.Language;                              // Установка языка отображения согласно уставке
  enum Precision : unsigned char { p0, p1, p2, p3, p4 };        // количество знаков после запятой p4->0.0001

  std::vector<x> MENU = {
    
  x(Mn.INDICATION[l],{
      x(Mn.CURRENT_DATA[l],{
          x::DualReg("I-Rotor", id::Irotor,   p0, "U-Rotor", id::Urotor,   p0, nm::In2V),
          x::DualReg("IStator", id::IstatRms, p0, "UStator", id::UstatRms, p0, nm::In2V)}),
      x(Mn.BIT_DATA[l],{
          x("dInCPU-D", {}, id::pi0_cpu, p0, nm::In1V),
          x("dInCPU-S", {}, id::pi0_spi, p0, nm::In1V),}),}),
  x(Mn.SETTINGS[l],{
      x(Mn.REGULATORS[l],{
          x(Mn.CURRENT[l],{
              x("KpCr",  {}, &set.set_reg.KpCr,    "", cd::one, p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
              x("CiCr",  {}, &set.set_reg.CiCr, "1/s", cd::one, p3, vt::vfloat, nm::Ed1V, 0, 1.0f)}),
          x(Mn.COS_PHI[l],{
              x("KpCos", {}, &set.set_reg.KpCos,    "", cd::one, p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
              x("CiCos", {}, &set.set_reg.CiCos, "1/s", cd::one, p3, vt::vfloat, nm::Ed1V, 0, 1.0f),}),
          x(Mn.Q_POWER[l],{
              x("KpQ",   {}, &set.set_reg.KpQ,      "", cd::one, p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
              x("CiQ",   {}, &set.set_reg.CiQ,   "1/s", cd::one, p3, vt::vfloat, nm::Ed1V, 0, 1.0f),}),}),
      x(Mn.LIMITS[l]),
      x(Mn.FAULTS[l]),
      x(Mn.ADC_SHIFT[l],{
          x::DualRaw("I-Rotor", pAdc.getEPointer(sadc::ROTOR_CURRENT),   un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::ROTOR_CURRENT],   un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          x::DualRaw("UStator", pAdc.getEPointer(sadc::STATOR_VOLTAGE),  un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::STATOR_VOLTAGE],  un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          x::DualRaw("U-Rotor", pAdc.getEPointer(sadc::ROTOR_VOLTAGE),   un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::ROTOR_VOLTAGE],   un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          x::DualRaw("I-Leak" , pAdc.getEPointer(sadc::LE_CURRENT),      un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::LE_CURRENT],      un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          x::DualRaw("IStator", pAdc.getEPointer(sadc::STATOR_CURRENT),  un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::STATOR_CURRENT],  un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          x::DualRaw("I-Node" , pAdc.getEPointer(sadc::NODE_CURRENT),    un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::NODE_CURRENT],    un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          x::DualRaw("Ex-Set" , pAdc.getEPointer(sadc::EXT_SETTINGS),    un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::EXT_SETTINGS],    un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),}),
      x("dinCPU-DIRECT~",{
          x("Bl Contact Q", {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_0, nm::Ed1V),
          x("Reg Auto",     {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_1, nm::Ed1V),
          x("Reg Drying",   {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_2, nm::Ed1V),  
          x("Setting More", {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_3, nm::Ed1V),
          x("Setting Less", {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_4, nm::Ed1V),
          x("CU_testing",   {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_5, nm::Ed1V),  
          x("Bl HVS NO",    {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_6, nm::Ed1V),
          x("Bl HVS NC",    {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_7, nm::Ed1V),}),  
      x("dinCPU-SPI~",{
          x("Heating",      {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_0, nm::Ed1V),
          x("Stator Key",   {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_1, nm::Ed1V),
          x("Not Used",     {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_2, nm::Ed1V),  
          x("Not Used",     {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_3, nm::Ed1V),
          x("IN1",          {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_4, nm::Ed1V),
          x("IN2",          {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_5, nm::Ed1V),  
          x("IN3",          {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_6, nm::Ed1V),
          x("Fuses BC",     {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_7, nm::Ed1V),}),
      x("doutCPU-SPI~",{
          x("Heating",      {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_0, nm::Ed1V),
          x("Stator Key",   {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_1, nm::Ed1V),
          x("Not Used",     {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_2, nm::Ed1V),  
          x("Not Used",     {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_3, nm::Ed1V),
          x("IN1",          {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_4, nm::Ed1V),
          x("IN2",          {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_5, nm::Ed1V),  
          x("IN3",          {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_6, nm::Ed1V),
          x("Fuses_BC",     {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_7, nm::Ed1V),}),
      x(Mn.CONTROLLER_SN[l],{
          x("Number:",      {}, &set.SNboard_number,"", cd::one, p0, vt::ushort, nm::Ed1V, 0, 1000),
          x("Month:",       {}, &set.SNboard_month, "", cd::one, p0, vt::ushort, nm::Ed1V, 1, 12),
          x("Year:",        {}, &set.SNboard_year,  "", cd::one, p0, vt::ushort, nm::Ed1V, 20, 99),}),
      x(Mn.LANGUAGE[l],{
          x("Language:",    {}, &set.Language,"", cd::one, p0, vt::ushort, nm::Ed1V, 0, (G_CONST::Nlang - 1)),}),}),
  x(Mn.ADJ_MODE[l]),
  x(Mn.CLOCK_SETUP[l]),
  x(Mn.INFO[l],{
      x("Description:", {}, static_cast<void*>(const_cast<char*>(BuildInfo::Description)), "", cd::one, p0,vt::text, nm::In1V),
      x("Git Version:", {}, static_cast<void*>(const_cast<char*>(BuildInfo::Version)),     "", cd::one, p0,vt::text, nm::In1V),
      x("Commit Date:", {}, static_cast<void*>(const_cast<char*>(BuildInfo::CommitDate)),  "", cd::one, p0,vt::text, nm::In1V),
      x("Build Date:",  {}, static_cast<void*>(const_cast<char*>(BuildInfo::BuildDate)),   "", cd::one, p0,vt::text, nm::In1V),
  })};
  
  return MENU;
}

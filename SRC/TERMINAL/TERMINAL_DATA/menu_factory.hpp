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
  using o    = CMenuNavigation::MenuNode;
  using nm   = CMenuNavigation::ENodeMode;
  using sadc = CADC_STORAGE;
  using sbin = CDIN_STORAGE::EIBNumber;
  using sbon = CDIN_STORAGE::EOBNumber;
}

// таблица переводов (масштабируемая)
static const struct {
    const char* INDICATION[G_CONST::Nlang]     = {"ИНДИКАЦИЯ",       "INDICATION",       "IНДИКАЦIЯ"};
    const char* CURRENT_DATA[G_CONST::Nlang]   = {"ТЕКУЩИЕ ДАННЫЕ",  "CURRENT DATA",     "ПОТОЧНI ДАНI"};
    const char* BIT_DATA[G_CONST::Nlang]       = {"БИТОВЫЕ ДАННЫЕ",  "BIT DATA",         "БIТОВІ ДАНI"};
    const char* SETTINGS[G_CONST::Nlang]       = {"УСТАВКИ",         "SETTINGS",         "УСТАНОВКИ"};
    const char* REGULATORS[G_CONST::Nlang]     = {"РЕГУЛЯТОРОВ",     "REGULATORS",       "РЕГУЛЯТОРИ"};
    const char* CURRENT[G_CONST::Nlang]        = {"ТОКА",            "CURRENT",          "СТРУМУ"};
    const char* COS_PHI[G_CONST::Nlang]        = {"COS PHI",         "COS PHI",          "COS PHI"};
    const char* Q_POWER[G_CONST::Nlang]        = {"Q МОЩНОСТИ",      "Q POWER",          "Q ПОТУЖНОСТI"};
    const char* LIMITS[G_CONST::Nlang]         = {"ОГРАНИЧЕНИЯ",     "LIMITS",           "ОБМЕЖЕННЯ"};
    const char* FAULTS[G_CONST::Nlang]         = {"АВАРИЙНЫЕ",       "FAULTS",           "АВАРIЙНI"};
    const char* ADC_SHIFT[G_CONST::Nlang]      = {"СМЕЩЕНИЯ АЦП",    "ADC SHIFT",        "ЗСУВ АЦП"};
    const char* ADJ_MODE[G_CONST::Nlang]       = {"РЕЖИМ НАЛАДКИ",   "ADJUSTMENT MODE",  "РЕЖИМ НАЛАДКИ"};
    const char* CLOCK_SETUP[G_CONST::Nlang]    = {"УСТАНОВКА ЧАСОВ", "CLOCK SETUP",      "ГОДИННИК"};
    const char* INFO[G_CONST::Nlang]           = {"ИНФОРМАЦИЯ",      "INFO",             "IНФОРМАЦIЯ"};
    const char* CONTROLLER_SN[G_CONST::Nlang]  = {"SN КОНТРОЛЛЕРА",  "CONTROLLER SN",    "SN КОНТРОЛЕРА"};
    const char* LANGUAGE[G_CONST::Nlang]       = {"ЯЗЫК",            "LANGUAGE",         "МОВА"};
} Mn;


// Фабрика дерева меню.
inline std::vector<menu_alias::o> MENU_Factory(CADC_STORAGE& pAdc, CEEPSettings& rSet, CSystemManager& rSysMgr) {
  auto& set = rSet.getSettings();
  using namespace menu_alias;
  auto& sfc = rSysMgr.rSIFU.s_const;
  
  unsigned short l = set.Language - 1;                          // Установка языка отображения согласно уставке
  enum Precision : unsigned char { p0, p1, p2, p3, p4 };        // количество знаков после запятой p4->0.0001
   
  std::vector<o> MENU = {
    
  o(Mn.INDICATION[l],{
      o(Mn.CURRENT_DATA[l],{
          o::DualReg("I-Rotor", id::Irotor,   p0, "U-Rotor", id::Urotor,   p0, nm::In2V),
          o::DualReg("IStator", id::IstatRms, p0, "UStator", id::UstatRms, p0, nm::In2V)}),
      o(Mn.BIT_DATA[l],{
          o("dInCPU-D", {}, id::pi0_cpu, p0, nm::In1V),
          o("dInCPU-S", {}, id::pi0_spi, p0, nm::In1V),}),}),
  o(Mn.ADJ_MODE[l],{
      o("On-Off ADJ MODE",{
           o("ADJ Mode",   {}, &rSysMgr.USMode_r.all, un::b, cd::one,  p0,vt::eb_0,  nm::Ed1V)}),
      o("PULSES",{
           o("Fors Bridge",{},   &rSysMgr.rAdj_mode.reqADJmode,         un::b,   cd::one,    p0, vt::eb_1,   nm::Ed1V),
           o("Main Bridge",{},   &rSysMgr.rAdj_mode.reqADJmode,         un::b,   cd::one,    p0, vt::eb_2,   nm::Ed1V),
           o::DualRaw("I-Rotor", pAdc.getEPointer(sadc::ROTOR_CURRENT), un::Amp, cd::IRotor, p0, vt::sshort,
                        "Alpha", &rSysMgr.rAdj_mode.AlphaAdj,           un::Deg, cd::Alpha,  p1, vt::sshort, nm::IE2V, sfc.AMin*0.018, sfc.AMax*0.018),}),
      o("I-REG",{
           o("I-Regulator",{},   &rSysMgr.rAdj_mode.reqADJmode,         un::b,   cd::one,    p0, vt::eb_3,   nm::Ed1V),
           o::DualRaw("I-Rotor", pAdc.getEPointer(sadc::ROTOR_CURRENT), un::Amp, cd::IRotor, p0, vt::sshort,
                        "I-set", &rSysMgr.rAdj_mode.IsetAdj,            un::Amp, cd::IRotor, p0, vt::sshort, nm::IE2V, 0, 200),}),
      o("I-CYCLES",{
           o("Iset cyc1",{},    &rSysMgr.rAdj_mode.IsetCyc_1,  un::Amp, cd::IRotor, p0, vt::ushort, nm::Ed1V, 0, 200),
           o("Iset cyc2",{},    &rSysMgr.rAdj_mode.IsetCyc_2,  un::Amp, cd::IRotor, p0, vt::ushort, nm::Ed1V, 0, 200),
           o("Npulses",{},      &rSysMgr.rAdj_mode.NpulsCyc,   "",      cd::one,    p0, vt::ushort, nm::Ed1V, 1, 100),
           o("I-Cycles",{},     &rSysMgr.rAdj_mode.reqADJmode, un::b,   cd::one,    p0, vt::eb_4,   nm::Ed1V),
           o("KpCr",   {},      &set.set_reg.KpCr,             "",      cd::one,    p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
           o("KiCr",   {},      &set.set_reg.KiCr,             "",      cd::one,    p3, vt::vfloat, nm::Ed1V, 0, 1.0f),}),
      o("PHASING",{
           o("Phasing mode", {}, &rSysMgr.rAdj_mode.reqADJmode,un::b,  cd::one,  p0,vt::eb_5,  nm::Ed1V),
           o("60deg shift",  {}, &set.set_sifu.d_power_shift,  "",     cd::one,  p0,vt::ushort,nm::Ed1V, 0, (sfc.N_PULSES-1)),
           o("Precise shift",{}, &set.set_sifu.power_shift,    un::Deg,cd::Alpha,p1,vt::sshort,nm::Ed1V, sfc.MinPshift*0.018, sfc.MaxPshift*0.018)}),}),
  o(Mn.SETTINGS[l],{
      o(Mn.REGULATORS[l],{
          o(Mn.CURRENT[l],{
              o("KpCr",   {}, &set.set_reg.KpCr, "",      cd::one,    p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
              o("KiCr",   {}, &set.set_reg.KiCr, "",      cd::one,    p3, vt::vfloat, nm::Ed1V, 0, 1.0f),
              o("I-set",  {}, &set.set_reg.Iset0,un::Amp, cd::IRotor, p0, vt::sshort, nm::Ed1V, 0, 4095),
              o("I-fors", {}, &set.set_reg.Ifors,un::Amp, cd::IRotor, p0, vt::sshort, nm::Ed1V, 0, 4095),
              o("I-dry",  {}, &set.set_reg.Idry, un::Amp, cd::IRotor, p0, vt::sshort, nm::Ed1V, 0, 4095),
              o("Alpha-0",{}, &set.set_reg.A0,   un::Deg, cd::Alpha,  p0, vt::sshort, nm::Ed1V, 90, 120)
          }),
          o(Mn.COS_PHI[l],{
              o("KpCos", {}, &set.set_reg.KpCos, "", cd::one, p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
              o("KiCos", {}, &set.set_reg.KiCos, "", cd::one, p3, vt::vfloat, nm::Ed1V, 0, 1.0f),}),
          o(Mn.Q_POWER[l],{
              o("KpQ",   {}, &set.set_reg.KpQ, "", cd::one, p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
              o("KiQ",   {}, &set.set_reg.KiQ, "", cd::one, p3, vt::vfloat, nm::Ed1V, 0, 1.0f),}),}),
      o(Mn.LIMITS[l]),
      o(Mn.FAULTS[l]),
      o(Mn.ADC_SHIFT[l],{
          o::DualRaw("I-Rotor", pAdc.getEPointer(sadc::ROTOR_CURRENT),   un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::ROTOR_CURRENT],   un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          o::DualRaw("UStator", pAdc.getEPointer(sadc::STATOR_VOLTAGE),  un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::STATOR_VOLTAGE],  un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          o::DualRaw("U-Rotor", pAdc.getEPointer(sadc::ROTOR_VOLTAGE),   un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::ROTOR_VOLTAGE],   un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          o::DualRaw("I-Leak" , pAdc.getEPointer(sadc::LE_CURRENT),      un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::LE_CURRENT],      un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          o::DualRaw("IStator", pAdc.getEPointer(sadc::STATOR_CURRENT),  un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::STATOR_CURRENT],  un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          o::DualRaw("I-Node" , pAdc.getEPointer(sadc::NODE_CURRENT),    un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::NODE_CURRENT],    un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          o::DualRaw("Ex-Set" , pAdc.getEPointer(sadc::EXT_SETTINGS),    un::d, cd::one, p0, vt::sshort,
                       "shift", &set.shift_adc[  sadc::EXT_SETTINGS],    un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),}),
      o("dinCPU-DIRECT~",{
          o("Bl Contact Q", {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_0, nm::Ed1V),
          o("Reg Auto",     {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_1, nm::Ed1V),
          o("Reg Drying",   {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_2, nm::Ed1V),  
          o("Setting More", {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_3, nm::Ed1V),
          o("Setting Less", {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_4, nm::Ed1V),
          o("CU_testing",   {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_5, nm::Ed1V),  
          o("Bl HVS NO",    {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_6, nm::Ed1V),
          o("Bl HVS NC",    {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_7, nm::Ed1V),}),  
      o("dinCPU-SPI~",{
          o("Heating",      {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_0, nm::Ed1V),
          o("Stator Key",   {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_1, nm::Ed1V),
          o("Not Used",     {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_2, nm::Ed1V),  
          o("Not Used",     {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_3, nm::Ed1V),
          o("IN1",          {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_4, nm::Ed1V),
          o("IN2",          {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_5, nm::Ed1V),  
          o("IN3",          {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_6, nm::Ed1V),
          o("Fuses BC",     {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_7, nm::Ed1V),}),
      o("doutCPU-SPI~",{
          o("Heating",      {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_0, nm::Ed1V),
          o("Stator Key",   {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_1, nm::Ed1V),
          o("Not Used",     {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_2, nm::Ed1V),  
          o("Not Used",     {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_3, nm::Ed1V),
          o("IN1",          {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_4, nm::Ed1V),
          o("IN2",          {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_5, nm::Ed1V),  
          o("IN3",          {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_6, nm::Ed1V),
          o("Fuses_BC",     {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], un::b, cd::one, p0, vt::eb_7, nm::Ed1V),}),
      o(Mn.CONTROLLER_SN[l],{
          o("Number:",      {}, &set.SNboard_number,"", cd::one, p0, vt::ushort, nm::Ed1V, 0, 1000),
          o("Month:",       {}, &set.SNboard_month, "", cd::one, p0, vt::ushort, nm::Ed1V, 1, 12),
          o("Year:",        {}, &set.SNboard_year,  "", cd::one, p0, vt::ushort, nm::Ed1V, 20, 99),}),
      o(Mn.LANGUAGE[l],{
          o("Language:",    {}, &set.Language,"", cd::one, p0, vt::ushort, nm::Ed1V, 1, G_CONST::Nlang),}),}),
  o(Mn.CLOCK_SETUP[l]),
  o(Mn.INFO[l],{
      o("Description:", {}, static_cast<void*>(const_cast<char*>(BuildInfo::Description)), "", cd::one, p0,vt::text, nm::In1V),
      o("Git Version:", {}, static_cast<void*>(const_cast<char*>(BuildInfo::Version)),     "", cd::one, p0,vt::text, nm::In1V),
      o("Commit Date:", {}, static_cast<void*>(const_cast<char*>(BuildInfo::CommitDate)),  "", cd::one, p0,vt::text, nm::In1V),
      o("Build Date:",  {}, static_cast<void*>(const_cast<char*>(BuildInfo::BuildDate)),   "", cd::one, p0,vt::text, nm::In1V),
  })};
  
  return MENU;
}

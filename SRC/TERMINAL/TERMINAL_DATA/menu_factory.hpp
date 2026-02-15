#pragma once

#include "AdcStorage.hpp"
#include "proxy_pointer_var.hpp"
#include "settings_eep.hpp"
#include "dIOStorage.hpp"
#include "din_cpu.hpp"
#include "version.hpp"
#include "_SystemManager.hpp" 
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
    const char* SERVICE[G_CONST::Nlang]        = {"СЛУЖЕБНАЯ",       "SERVICE DATA",     "СЛУЖБОВА"};
    const char* SETTINGS[G_CONST::Nlang]       = {"УСТАВКИ",         "SETTINGS",         "УСТАНОВКИ"};
    const char* REGULATORS[G_CONST::Nlang]     = {"РЕГУЛЯТОРОВ",     "REGULATORS",       "РЕГУЛЯТОРИ"};
    const char* CURRENT[G_CONST::Nlang]        = {"ТОКА",            "CURRENT",          "СТРУМУ"};
    const char* COS_PHI[G_CONST::Nlang]        = {"COS PHI",         "COS PHI",          "COS PHI"};
    const char* Q_POWER[G_CONST::Nlang]        = {"Q МОЩНОСТИ",      "Q POWER",          "Q ПОТУЖНОСТI"};
    const char* PUSK[G_CONST::Nlang]           = {"ПУСК",            "PUSK",             "ПУСК"};
    const char* WORK[G_CONST::Nlang]           = {"РАБОЧИЕ",         "WORK",             "РОБОЧI"};    
    const char* PARAMS[G_CONST::Nlang]         = {"ПАРАМЕТРЫ",       "PARAMETERS",       "ПАРАМЕТРИ"};    
    const char* FAULTS[G_CONST::Nlang]         = {"АВАРИЙНЫЕ",       "FAULTS",           "АВАРIЙНI"};
    const char* ADC_SHIFT[G_CONST::Nlang]      = {"СМЕЩЕНИЯ АЦП",    "ADC SHIFT",        "ЗСУВ АЦП"};
    const char* ADJ_MODE[G_CONST::Nlang]       = {"РЕЖИМ НАЛАДКИ",   "ADJUSTMENT MODE",  "РЕЖИМ НАЛАДКИ"};
    const char* CLOCK_SETUP[G_CONST::Nlang]    = {"УСТАНОВКА ЧАСОВ", "CLOCK SETUP",      "ГОДИННИК"};
    const char* INFO[G_CONST::Nlang]           = {"ИНФОРМАЦИЯ",      "INFO",             "IНФОРМАЦIЯ"};
    const char* CONTROLLER_SN[G_CONST::Nlang]  = {"SN КОНТРОЛЛЕРА",  "CONTROLLER SN",    "SN КОНТРОЛЕРА"};
    const char* LANGUAGE[G_CONST::Nlang]       = {"ЯЗЫК",            "LANGUAGE",         "МОВА"};
} Mn;


//--- Фабрика дерева меню ---
/*
  Структура узла:

  { "Name", {children}, &var, un, cd, p, vt, nm, min, max }

  un - размерност
  cd - коэффициент отображения
  p  - точность
  vt - тип переменной
  nm - тип узла
*/
inline std::vector<menu_alias::o> MENU_Factory(CADC_STORAGE& pAdc, CEEPSettings& rSet, 
                                               CSystemManager& rSysMgr, CRTC& rRTC) {
                                                   
  auto& set = rSet.getSettings();
  
  using namespace menu_alias;
  
  auto& sfc = rSysMgr.rSIFU.s_const;
  auto& sifu = rSysMgr.rSIFU;
  auto& str = CDIN_STORAGE::getInstance();
  unsigned short l = set.Language - 1;                          // Установка языка отображения согласно уставке
  enum Precision : unsigned char { p0, p1, p2, p3, p4 };        // количество знаков после запятой p4->0.0001
 
  std::vector<o> MENU = {
    
  o(Mn.INDICATION[l],{
      o(Mn.CURRENT_DATA[l],{          
          o::Dual("I-Rotor", pAdc.getEPointer(sadc::ROTOR_CURRENT),   un::Amp,  cd::cdr.Id, p0, vt::sshort,
                  "U-Rotor", pAdc.getEPointer(sadc::ROTOR_VOLTAGE),   un::Volt, cd::cdr.Ud, p0, vt::sshort, nm::In2V),
          o::Dual("I-Stat",  sifu.rPulsCalc.getPointer_istator_rms(), un::Amp,  cd::cdr.IS, p0, vt::vfloat,
                  "U-Stat",  sifu.rPulsCalc.getPointer_ustator_rms(), un::Volt, cd::cdr.US, p0, vt::vfloat, nm::In2V),
          o::Dual("Q-Power", sifu.rPulsCalc.getPointer_Q_Power(),     un::kVAR, cd::cdr.Q,  p1, vt::vfloat,
                  "Cos-Phi", sifu.rPulsCalc.getPointer_CosPhi(),      "",       cd::one,    p2, vt::vfloat, nm::In2V),
          o::Dual("P-Power", sifu.rPulsCalc.getPointer_P_Power(),     un::kW,   cd::cdr.P,  p1, vt::vfloat,
                  "S-Power", sifu.rPulsCalc.getPointer_S_Power(),     un::kVA,  cd::cdr.S,  p1, vt::vfloat, nm::In2V),           
          o::Dual("Sync",    sifu.get_pSyncStat(),                    "Sync",   cd::one,    p0, vt::vbool,
                  "Fsync",   sifu.get_Sync_Frequency(),               un::Hz,   cd::one,    p1, vt::vfloat, nm::In2V),
          o::Dual("P5",      pAdc.getIPointer(sadc::SUPPLY_P5),       un::Volt, cd::one,    p1, vt::vfloat,
                  "N5",      pAdc.getIPointer(sadc::SUPPLY_N5),       un::Volt, cd::one,    p1, vt::vfloat, nm::In2V),
          o::Dual("I-Rotor", pAdc.getEPointer(sadc::ROTOR_CURRENT),   un::Amp,  cd::cdr.Id, p0, vt::sshort,
                  "I-Set",   rSysMgr.rReg_manager.rCurrent_reg.getPointerIset(),   un::Amp,  cd::cdr.Id, p0, vt::ushort, nm::In2V),}),
      o(Mn.BIT_DATA[l],{
          o("dInCPU-D", {}, &str.UData_din_f[static_cast<unsigned char>(sbin::CPU_PORT)].all, un::d, cd::one, p0, vt::char2b, nm::In1V),
          o("dInCPU-S", {}, &str.UData_din_f[static_cast<unsigned char>(sbin::CPU_SPI)].all,  un::d, cd::one, p0, vt::char2b, nm::In1V),}),
      o(Mn.SERVICE[l],{          
          o::Dual("IStat-c", sifu.rPulsCalc.getPointer_istator_rms(), un::Amp,  cd::cdr.IS, p0, vt::vfloat,
                  "Slip-c",  sifu.rPulsCalc.getPointer_slip(),        "sl-c",   cd::one,    p2, vt::vfloat, nm::In2V),          
          o::Dual("slipON",  rSysMgr.rPusk_mode.getPointerSlE(),      "",       cd::one,    p0, vt::vbool,
                  "PKstat",  rSysMgr.rPusk_mode.getPointerSPK(),      "",       cd::one,    p0, vt::vbool, nm::In2V),
          o::Dual("IStat-p", rSysMgr.rPusk_mode.getPointerPis(),      un::Amp,  cd::cdr.IS, p0, vt::vfloat,
                  "Slip-p",  rSysMgr.rPusk_mode.getPointerPslip(),    "sl-p",   cd::one,    p2, vt::vfloat, nm::In2V),}),}),
  o(Mn.ADJ_MODE[l],{
      o("On-Off ADJ MODE",{
          o("ADJ Mode",   {}, &rSysMgr.USystemStatus.all,            "",      cd::one,    p0, vt::eb_3,   nm::Ed1V),
          o("Fors Bridge",{}, &rSysMgr.rAdj_mode.reqADJmode,         "",      cd::one,    p0, vt::eb_0,   nm::Ed1V),
          o("Main Bridge",{}, &rSysMgr.rAdj_mode.reqADJmode,         "",      cd::one,    p0, vt::eb_1,   nm::Ed1V),
          o::Dual("I-Rotor",  pAdc.getEPointer(sadc::ROTOR_CURRENT), un::Amp, cd::cdr.Id, p0, vt::sshort,
                    "Alpha",  &rSysMgr.rAdj_mode.AlphaAdj,           un::Deg, cd::Alpha,  p1, vt::sshort, nm::IE2V, sfc.AMin*0.018, sfc.AMax*0.018),}),
      o("I-REG",{
          o("I-Regulator",{}, &rSysMgr.rAdj_mode.reqADJmode,                 "",      cd::one,    p0, vt::eb_2,   nm::Ed1V),
          o::Dual("I-Rotor",  pAdc.getEPointer(sadc::ROTOR_CURRENT),         un::Amp, cd::cdr.Id, p0, vt::sshort,
                    "I-set",  &rSysMgr.rAdj_mode.IsetAdj,                    un::Amp, cd::cdr.Id, p0, vt::sshort, nm::IE2V, 0, 200),
          o("RCon",       {}, &rSysMgr.rReg_manager.rCurrent_reg.ResPusk,    "",      cd::one,    p0, vt::vbool,  nm::Ed1V, 0, 1),}),
      o("I-CYCLES",{
          o("Iset cyc1", {},  &rSysMgr.rAdj_mode.IsetCyc_1,  un::Amp, cd::cdr.Id, p0, vt::ushort, nm::Ed1V, 0, 1.5f*set.params.IdNom),
          o("Iset cyc2", {},  &rSysMgr.rAdj_mode.IsetCyc_2,  un::Amp, cd::cdr.Id, p0, vt::ushort, nm::Ed1V, 0, 1.5f*set.params.IdNom),
          o("Npulses",   {},  &rSysMgr.rAdj_mode.NpulsCyc,   "",      cd::one,    p0, vt::ushort, nm::Ed1V, 1, 1000),
          o("I-Cycles",  {},  &rSysMgr.rAdj_mode.reqADJmode, "",      cd::one,    p0, vt::eb_3,   nm::Ed1V),
          o("KpCr",      {},  &set.set_reg.KpCr,             "",      cd::one,    p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
          o("KiCr",      {},  &set.set_reg.KiCr,             "",      cd::one,    p3, vt::vfloat, nm::Ed1V, 0, 1.0f),
          o("KpCrR",     {},  &set.set_reg.KpCrR,            "",      cd::one,    p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
          o("KiCrR",     {},  &set.set_reg.KiCrR,            "",      cd::one,    p3, vt::vfloat, nm::Ed1V, 0, 1.0f),}),
      o("PHASING",{
          o("Phasing mode", {}, &rSysMgr.rAdj_mode.reqADJmode,"",     cd::one,  p0,vt::eb_4,  nm::Ed1V),
          o("60deg shift",  {}, &set.set_sifu.d_power_shift,  "",     cd::one,  p0,vt::ushort,nm::Ed1V, 0, (sfc.N_PULSES-1)),
          o("Precise shift",{}, &set.set_sifu.power_shift,    un::Deg,cd::Alpha,p1,vt::sshort,nm::Ed1V, sfc.MinPshift*0.018, sfc.MaxPshift*0.018)}),}),
  o(Mn.SETTINGS[l],{
      o(Mn.REGULATORS[l],{
          o(Mn.CURRENT[l],{
              o("KpCr",   {}, &set.set_reg.KpCr, "",      cd::one,    p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
              o("KiCr",   {}, &set.set_reg.KiCr, "",      cd::one,    p3, vt::vfloat, nm::Ed1V, 0, 1.0f),
              o("KpCrR",  {}, &set.set_reg.KpCrR,"",      cd::one,    p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
              o("KiCrR",  {}, &set.set_reg.KiCrR,"",      cd::one,    p3, vt::vfloat, nm::Ed1V, 0, 1.0f),
              o("Alpha-0",{}, &set.set_reg.A0,   un::Deg, cd::Alpha,  p0, vt::sshort, nm::Ed1V, 90, 120)}),
          o(Mn.COS_PHI[l],{
              o("KpCos", {}, &set.set_reg.KpCos, "", cd::one, p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
              o("KiCos", {}, &set.set_reg.KiCos, "", cd::one, p3, vt::vfloat, nm::Ed1V, 0, 1.0f),}),
          o(Mn.Q_POWER[l],{
              o("KpQ",   {}, &set.set_reg.KpQ, "", cd::one, p1, vt::vfloat, nm::Ed1V, 0, 10.0f),
              o("KiQ",   {}, &set.set_reg.KiQ, "", cd::one, p3, vt::vfloat, nm::Ed1V, 0, 1.0f),}),}),
      o(Mn.PUSK[l], {
          o("I fors",   {}, &set.set_pusk.IFors,               un::Amp, cd::cdr.Id, p0, vt::ushort, nm::Ed1V, 0, 2*set.params.IdNom),
          o("T fors",   {}, &set.set_pusk.TFors,               un::sec, cd::one,    p0, vt::ushort, nm::Ed1V, 1, 10),
          o("T pusk",   {}, &set.set_pusk.TPusk,               un::sec, cd::one,    p0, vt::ushort, nm::Ed1V, 1, 60),
          o("T s-sync", {}, &set.set_pusk.TSelfSync,           un::sec, cd::one,    p0, vt::ushort, nm::Ed1V, 5, 15),          
          o("IS start", {}, &set.set_pusk.ISPusk,              un::Amp, cd::cdr.IS, p0, vt::ushort, nm::Ed1V, 0, set.params.ISNom),
          o("Slip",     {}, &set.set_pusk.SlipPusk,            "",      cd::one,    p2, vt::vfloat, nm::Ed1V, 0, 0.1f),
          o("W Ex",     {}, rSysMgr.rPusk_mode.getPointerWex(),"NoS",   cd::one,    p0, vt::vbool,  nm::Ed1V, 0, 1),}),      
      o(Mn.PARAMS[l], {
          o("Id Nom", {}, &set.params.IdNom, un::Amp,  cd::one,    p0, vt::ushort, nm::Ed1V,  50,  400),
          o("Ud Nom", {}, &set.params.UdNom, un::Volt, cd::one,    p0, vt::ushort, nm::Ed1V,  48,  230),
          o("IS Nom", {}, &set.params.ISNom, un::Amp,  cd::one,    p0, vt::ushort, nm::Ed1V,   0, 1000),
          o("US Nom", {}, &set.params.USNom, un::Volt, cd::one,    p0, vt::ushort, nm::Ed1V, 380, 6300),}),
      o(Mn.WORK[l], {
          o("Iset-0",  {}, &set.work_set.Iset_0,   un::Amp, cd::cdr.Id, p0, vt::ushort, nm::Ed1V, 0, set.params.IdNom),
          o("Iset-Max",{}, &set.work_set.IsetMax,  un::Amp, cd::cdr.Id, p0, vt::ushort, nm::Ed1V, 0, set.params.IdNom),
          o("Iset-Min",{}, &set.work_set.IsetMin,  un::Amp, cd::cdr.Id, p0, vt::ushort, nm::Ed1V, 0, set.params.IdNom/8),
          o("Idry-0",  {}, &set.work_set.Idry_0,   un::Amp, cd::cdr.Id, p0, vt::ushort, nm::Ed1V, 0, set.params.IdNom/8),
          o("Iset'",   {}, &set.work_set.derivIset,un::ApS, cd::cdr.Id, p0, vt::ushort, nm::Ed1V, 0, set.params.IdNom/8),
          o("Qset-0",  {}, &set.work_set.Qset_0,   un::kVA, cd::cdr.Q,  p0, vt::ushort, nm::Ed1V, 0, set.params.ISNom*set.params.USNom*0.001f),      
          o("Cos-0",   {}, &set.work_set.Cos_0,    "",      cd::one,    p2, vt::vfloat, nm::Ed1V, 0, 1),}),      
      o(Mn.FAULTS[l], {
          o("Id Max", {}, &set.set_faults.IdMax, un::Amp, cd::cdr.Id, p0, vt::ushort, nm::Ed1V, 0, 2.5f*set.params.IdNom),
          o("Id Min", {}, &set.set_faults.IdMin, un::Amp, cd::cdr.Id, p0, vt::ushort, nm::Ed1V, 0, 2.0f*set.params.IdNom),}),
      o(Mn.ADC_SHIFT[l],{
          o::Dual("I-Rotor", pAdc.getEPointer(sadc::ROTOR_CURRENT),   un::d, cd::one, p0, vt::sshort,
                    "shift", &set.shift_adc[  sadc::ROTOR_CURRENT],   un::d, cd::one, p0, vt::sshort, nm::IE2V, -2047, 2047),
          o::Dual("UStator", pAdc.getEPointer(sadc::STATOR_VOLTAGE),  un::d, cd::one, p0, vt::sshort,
                    "shift", &set.shift_adc[  sadc::STATOR_VOLTAGE],  un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          o::Dual("U-Rotor", pAdc.getEPointer(sadc::ROTOR_VOLTAGE),   un::d, cd::one, p0, vt::sshort,
                    "shift", &set.shift_adc[  sadc::ROTOR_VOLTAGE],   un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          o::Dual("I-Leak" , pAdc.getEPointer(sadc::LE_CURRENT),      un::d, cd::one, p0, vt::sshort,
                   "shift",  &set.shift_adc[  sadc::LE_CURRENT],      un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          o::Dual("IStator", pAdc.getEPointer(sadc::STATOR_CURRENT),  un::d, cd::one, p0, vt::sshort,
                    "shift", &set.shift_adc[  sadc::STATOR_CURRENT],  un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          o::Dual("I-Node" , pAdc.getEPointer(sadc::NODE_CURRENT),    un::d, cd::one, p0, vt::sshort,
                   "shift",  &set.shift_adc[  sadc::NODE_CURRENT],    un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),
          o::Dual("Ex-Set" , pAdc.getEPointer(sadc::EXT_SETTINGS),    un::d, cd::one, p0, vt::sshort,
                    "shift", &set.shift_adc[  sadc::EXT_SETTINGS],    un::d, cd::one, p0, vt::sshort, nm::IE2V, 0, 3000),}),
      o("Invert DIN-0",{
          o("CU Testing",   {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_0, nm::Ed1V),
          o("Mode Auto",    {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_1, nm::Ed1V),
          o("Mode Drying",  {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_2, nm::Ed1V),  
          o("Setting More", {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_3, nm::Ed1V),
          o("Setting Less", {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_4, nm::Ed1V),
          o("BC breaker Q1",{}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_5, nm::Ed1V),  
          o("HVS NO",       {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_6, nm::Ed1V),
          o("HVS NC",       {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_PORT)], un::b, cd::one, p0, vt::eb_7, nm::Ed1V),}),  
      o("Invert DIN-1",{
          o("Heating",      {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_0, nm::Ed1V),
          o("Stator Key",   {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_1, nm::Ed1V),
          o("-",            {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_2, nm::Ed1V),  
          o("-",            {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_3, nm::Ed1V),
          o("IN1",          {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_4, nm::Ed1V),
          o("IN2",          {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_5, nm::Ed1V),  
          o("IN3",          {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_6, nm::Ed1V),
          o("Fuses BC",     {}, &set.din_Pi_invert[static_cast<char>(sbin::CPU_SPI)], un::b, cd::one, p0, vt::eb_7, nm::Ed1V),}),
      o("Invert DOUT-0",{
          o("Lamp REDY",       {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], "", cd::one, p0, vt::eb_0, nm::Ed1V),
          o("-",               {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], "", cd::one, p0, vt::eb_1, nm::Ed1V),
          o("-",               {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], "", cd::one, p0, vt::eb_2, nm::Ed1V),  
          o("-",               {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], "", cd::one, p0, vt::eb_3, nm::Ed1V),
          o("Relay FAULT",     {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], "", cd::one, p0, vt::eb_4, nm::Ed1V),
          o("Relay Ex.Applied",{}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], "", cd::one, p0, vt::eb_5, nm::Ed1V),  
          o("Relay Premission",{}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], "", cd::one, p0, vt::eb_6, nm::Ed1V),
          o("Relay Shunt HVS", {}, &set.dout_spi_invert[static_cast<char>(sbon::CPU_SPI)], "", cd::one, p0, vt::eb_7, nm::Ed1V),}),
      o(Mn.CONTROLLER_SN[l],{
          o("Number:",      {}, &set.SNboard_number,"", cd::one, p0, vt::ushort, nm::Ed1V, 0, 1000),
          o("Month:",       {}, &set.SNboard_month, "", cd::one, p0, vt::ushort, nm::Ed1V, 1, 12),
          o("Year:",        {}, &set.SNboard_year,  "", cd::one, p0, vt::ushort, nm::Ed1V, 20, 99),}),
      o(Mn.LANGUAGE[l],{
          o("Language:",    {}, &set.Language,"", cd::one, p0, vt::ushort, nm::Ed1V, 1, G_CONST::Nlang),}),}),  
  o(Mn.CLOCK_SETUP[l],{
      o("Year:",        {}, &rRTC.DateTimeForSet.year,  "", cd::one, p0, vt::ushort, nm::Ed1V, 26, 99),
      o("Month:",       {}, &rRTC.DateTimeForSet.month, "", cd::one, p0, vt::ushort, nm::Ed1V,  1, 12),
      o("Day:",         {}, &rRTC.DateTimeForSet.day,   "", cd::one, p0, vt::ushort, nm::Ed1V,  1, 31),
      o("Hour:",        {}, &rRTC.DateTimeForSet.hour,  "", cd::one, p0, vt::ushort, nm::Ed1V,  0, 23),
      o("Minute:",      {}, &rRTC.DateTimeForSet.minute,"", cd::one, p0, vt::ushort, nm::Ed1V,  0, 60),
      o("SET",          {}, &rRTC.set_date_time,        "", cd::one, p0, vt::vbool,  nm::Ed1V,  0, 1),}),
  o(Mn.INFO[l],{
      o("Description:", {}, static_cast<void*>(const_cast<char*>(BuildInfo::Description)), "", cd::one, p0,vt::text, nm::In1V),
      o("Git Version:", {}, static_cast<void*>(const_cast<char*>(BuildInfo::Version)),     "", cd::one, p0,vt::text, nm::In1V),
      o("Commit Date:", {}, static_cast<void*>(const_cast<char*>(BuildInfo::CommitDate)),  "", cd::one, p0,vt::text, nm::In1V),
      o("Build Date:",  {}, static_cast<void*>(const_cast<char*>(BuildInfo::BuildDate)),   "", cd::one, p0,vt::text, nm::In1V),
  })};
  
  return MENU;
}

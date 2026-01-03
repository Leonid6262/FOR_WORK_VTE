#pragma once

#include "bool_name.hpp"
#include "controllerDMA.hpp"
#include "puls_calc.hpp"
#include "AdcStorage.hpp"

class CREM_OSC {
 private:
  CDMAcontroller& rContDMA;

  unsigned char number_actual_tracks;

  void init_dma();
  unsigned char get_actual_number();
  void transfer_disp_c();
  void transfer_name();
  void transfer_mode();
  StatusRet transfer_SN_ID();
  StatusRet transfer_SSID();
  StatusRet transfer_Password();
  StatusRet echo_check();
  void pack_chars(unsigned char*);

  static constexpr unsigned short TRANSACTION_LENGTH = 11;                // Слово управления + 10 треков
  static constexpr unsigned char NUMBER_TRACKS = TRANSACTION_LENGTH - 1;  // Максимальное количество треков
  static constexpr unsigned char NAME_LENGTH = 5 + 1;                     // Максимальная длина имени трека 5 символов
  static constexpr unsigned char n_repeat = 5;                            // Количество повторов при контроле эха

  // Директивы передачи
  static constexpr unsigned short send_SSID = 0x8000;  // Передача SS_ID
  static constexpr unsigned short send_PASS = 0x8001;  // Передача Password
  static constexpr unsigned short send_SNID = 0x8002;  // Передача SN_ID
  static constexpr unsigned short send_CIND = 0x8003;  // Передача коэффициентов отображения

  static constexpr unsigned short NAME_CODES[NUMBER_TRACKS / 2] = {
      0x8004,  // Передача имён 0,1
      0x8005,  // Передача имён 2,3
      0x8006,  // Передача имён 4,5
      0x8007,  // Передача имён 6,7
      0x8008   // Передача имён 8,9
  };

  static constexpr unsigned short send_TRACKS = 0x8F00;  // Передача данных треков

  // CDMAcontroller::ChannelMap ChMap_rx;
  // CDMAcontroller::ChannelMap ChMap_tx;
  CDMAcontroller::EConnNumber ConnN_rx;
  CDMAcontroller::EConnNumber ConnN_tx;

 public:
  bool StatusESP32;
  unsigned short ip_ap_sta[4];  // Полученный в режиме Station ip адрес

  // Режим работы
  enum class Operating_mode : unsigned char { Access_point, Station };
  // Структура инициализирующих значений
  struct SSET_init {
    signed short* pData[NUMBER_TRACKS];
    char Names[NUMBER_TRACKS][NAME_LENGTH];
    unsigned short d_100p[NUMBER_TRACKS];
    Operating_mode mode;
    unsigned short SNboard_number;
    unsigned char* pSSID;
    unsigned char* pPassword;
  };

  SSET_init set_init;
  CREM_OSC(CDMAcontroller&, CPULSCALC&, CADC_STORAGE&);

  static signed short tx_dma_buffer[TRANSACTION_LENGTH];
  static signed short rx_dma_buffer[TRANSACTION_LENGTH];

  void start_dma_transfer();
  void send_data();
};

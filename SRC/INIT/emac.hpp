#pragma once

#include "LPC407x_8x_177x_8x.h"
#include "system_LPC177x.h"
#include <cstring>       
#include "LAN8720.h"
#include "pause_us.hpp"
#include "bool_name.hpp"
#include "emac_g.hpp"

class CEMAC_DRV {
    
public:
      
    CEMAC_DRV();
    
    static const unsigned char MAC_Controller[]; 
    unsigned int status_rx;
    bool link_up;
 
    StatusRet initEMAC();
    
    enum class ReceiveStatus 
    {
      EMPTY,  
      FRAME_RECIVED,
      ERROR
    };
    
    ReceiveStatus receiveFrame(unsigned char*);
    void sendFrame(const unsigned char*);
    
    bool EMAC_Ready;
        
    static constexpr unsigned int ETH_FRAG_SIZE = 64; 
    
private:
  
  bool ID_LAN8720;
  unsigned int idx_Cons;
  unsigned int idx_Prod;
  
  StatusRet initPHY(); 
  void initDescriptors(); 
  StatusRet writePHY(unsigned char, unsigned short);
  StatusRet readPHY(unsigned char, unsigned short&);
  
  static constexpr unsigned int FUNC_ENET   = 0x01;
  static constexpr unsigned int MODE_PULLUP = 0x02 << 3;  // Pull-up resistor
  
  static constexpr unsigned char PHY_IDR1_SHIFT = 16;
  static constexpr unsigned short PHY_IDR2_MASK = 0xFFF0;
 
  static constexpr unsigned int PHYADDR =  8;
  static constexpr unsigned int BUSY    = 1UL << 0;
  static constexpr unsigned int READ    = 1UL << 0;
    
  static constexpr unsigned int RX_DESC_BASE  = 0x20004000;
  
  static constexpr unsigned int NUM_RX_FRAG   =  4;
  static constexpr unsigned int NUM_TX_FRAG   =  2;  
  static constexpr unsigned int ETH_MAX_FLEN  = 64; 
  
  static constexpr unsigned int DESC_RX_SIZE  = 0x7FF;
  
  static constexpr unsigned int RX_STAT_BASE = (RX_DESC_BASE + NUM_RX_FRAG*8); 
  static constexpr unsigned int TX_DESC_BASE = (RX_STAT_BASE + NUM_RX_FRAG*8); 
  static constexpr unsigned int TX_STAT_BASE = (TX_DESC_BASE + NUM_TX_FRAG*8); 
  static constexpr unsigned int RX_BUF_BASE  = (TX_STAT_BASE + NUM_TX_FRAG*4); 
  static constexpr unsigned int TX_BUF_BASE  = (RX_BUF_BASE  + NUM_RX_FRAG*ETH_FRAG_SIZE); 
  
  constexpr unsigned int& rxDescPacket(int i) {return *reinterpret_cast<unsigned int*>(RX_DESC_BASE + 8 * i);}    
  constexpr unsigned int& rxDescCtrl(int i)   {return *reinterpret_cast<unsigned int*>(RX_DESC_BASE + 4 + 8 * i);}    
  constexpr unsigned int& rxStatInfo(int i)   {return *reinterpret_cast<unsigned int*>(RX_STAT_BASE + 8 * i);}   
  constexpr unsigned int& rxStatHashCrc(int i){return *reinterpret_cast<unsigned int*>(RX_STAT_BASE + 4 + 8 * i);}
  
  constexpr unsigned int& txDescPacket(int i) {return *reinterpret_cast<unsigned int*>(TX_DESC_BASE + 8 * i);}
  constexpr unsigned int& txDescCtrl(int i)   {return *reinterpret_cast<unsigned int*>(TX_DESC_BASE + 4 + 8 * i);}
  constexpr unsigned int& txStatInfo(int i)   {return *reinterpret_cast<unsigned int*>(TX_STAT_BASE + 4 * i);}
  
  constexpr uintptr_t rxBuf(int i) {return RX_BUF_BASE + ETH_FRAG_SIZE * i;}  
  constexpr uintptr_t txBuf(int i) {return TX_BUF_BASE + ETH_FRAG_SIZE * i;}  
  
  enum : unsigned int {
    TX_CTRL_OVERRIDE      = (1UL << 26),
    TX_CTRL_PAD_ENABLE    = (1UL << 28),
    TX_CTRL_APPEND_CRC    = (1UL << 29),
    TX_CTRL_LAST_FRAGMENT = (1UL << 30)
  };
  
  static constexpr unsigned int TX_CTRL_DEFAULT = 
    TX_CTRL_OVERRIDE | TX_CTRL_PAD_ENABLE |
    TX_CTRL_APPEND_CRC | TX_CTRL_LAST_FRAGMENT;
  
  enum : unsigned int {
    RX_STATUS_RANGE_ERROR = (1UL << 26), // Ошибкой не считается, EMAC не различает тип и длину кадра.
    RX_STATU_LAST_FLAG    = (1UL << 30), // Уведомляющее сообщение о последнем фрагменте
    RX_STATU_SUM_ERROR    = (1UL << 31)  // Исключается из за RX_STATUS_RANGE_ERROR
  };
  
  static constexpr unsigned int RX_CTRL_ERR_BITS = 
    RX_STATUS_RANGE_ERROR | RX_STATU_LAST_FLAG |
    RX_STATU_SUM_ERROR | DESC_RX_SIZE;
  
};








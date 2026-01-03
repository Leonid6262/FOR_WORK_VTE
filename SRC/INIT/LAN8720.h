#ifndef RTL8201_H
#define RTL8201_H

/* RTL8201 PHY Registers */
#define PHY_REG_BMCR        0x00        /* Basic Mode Control Register       */
#define PHY_REG_BMSR        0x01        /* Basic Mode Status Register        */
#define PHY_REG_IDR1        0x02        /* PHY Identifier 1                  */
#define PHY_REG_IDR2        0x03        /* PHY Identifier 2                  */
#define PHY_REG_ANAR        0x04        /* Auto-Negotiation Advertisement    */
#define PHY_REG_ANLPAR      0x05        /* Auto-Neg. Link Partner Abitily    */
#define PHY_REG_ANER        0x06        /* Auto-Neg. Expansion Register      */

/* PHY Extended Registers */
#define PHY_REG_NSR         16        /* NWAY Setup Register               */
#define PHY_REG_MICR        17        /* MII Interrupt Control Register    */
#define PHY_REG_MISR        18        /* MII Interrupt Status Register     */
#define PHY_REG_FCSCR       19        /* False Carrier Sense Counter       */
#define PHY_REG_RECR        20        /* Receive Error Counter             */
#define PHY_REG_PCSR        21        /* PCS Sublayer Config. and Status   */
#define PHY_REG_RBR         22        /* RMII and Bypass Register          */
#define PHY_REG_LEDCR       23        /* LED Direct Control Register       */
#define PHY_REG_PHYCR       24        /* PHY Control Register              */
#define PHY_REG_10BTSCR     25        /* 10Base-T Status/Control Register  */

//Basic Mode Control Register bits
#define PHY_BMCR_RESET      (1<<15)     //Reset
#define PHY_BMCR_LOOPBACK   (1<<14)     //Loopback TX3..0 то RX3..0
#define PHY_BMCR_SPEED      (1<<13)     //Set network speed 1=100M, 0=10Mb
#define PHY_BMCR_ANE        (1<<12)     //Set Auto Negotiation Enable
#define PHY_BMCR_PWRDN      (1<<11)     //Set Power Down Mode
#define PHY_BMCR_RANE       (1<<9)      //Set Restart Auto Negotiation
#define PHY_BMCR_DUPLEX     (1<<8)      //Set Duplex Mode 1=Full, 0=Half

#define PHY_FULLD_100M      0x2100      /* Full Duplex 100Mbit               */
#define PHY_HALFD_100M      0x2000      /* Half Duplex 100Mbit               */
#define PHY_FULLD_10M       0x0100      /* Full Duplex 10Mbit                */
#define PHY_HALFD_10M       0x0000      /* Half Duplex 10MBit                */

//Basic Mode Status Register (Read only)
#define PHY_BMSR_100B_T4    (1<<15)     //Enable 100Base-T4 support
#define PHY_BMSR_100B_TX_FD (1<<14)     //Enable 100Base-TX Full duplex support
#define PHY_BMSR_100B_TX_HD (1<<13)     //Enable 100Base-TX Half duplex support
#define PHY_BMSR_10B_T_FD   (1<<12)     //Enable 10Base-T Full duplex support
#define PHY_BMSR_10B_T_HD   (1<<11)     //Enable 10Base-T Half duplex support
#define PHY_BMSR_100B_T2_FD (1<<10)     //Enable 100BASE-T2 Full Duplex
#define PHY_BMSR_100B_T2_HD (1<<9)      //Enable 100BASE-T2 Halp Duplex
#define PHY_BMSR_EXT_STATUS (1<<8)      //extended status information in register 15
//                           (1<<6)      //Reserved
//                           (1<<7)      //Reserved
#define PHY_BMSR_ANEC       (1<<5)      //1=Auto-negotiation process completed
#define PHY_BMSR_RFAULT     (1<<4)      //1=Remote fault condition detected (cleared on read)
#define PHY_BMSR_ANE        (1<<3)      //1=able to perform auto-negotiation function
#define PHY_BMSR_LINK       (1<<2)      //1=Valid link established
#define PHY_BMSR_JABBER     (1<<1)      //1=Jabber condition detected
#define PHY_BMSR_EXT        (1<<0)      //1=Support Extended register capability

struct TBSRegster
{ 
  unsigned short ExtCapabilities        :1;
  unsigned short JabberDetect           :1;  
  unsigned short LinkStatus             :1;
  unsigned short AutoNegotiateAbility   :1;
  unsigned short RemoteFault            :1;
  unsigned short AutoNegotiateCoplete   :1;
  unsigned short __reserved0            :2;
  unsigned short ExtendedStatus         :1;
  unsigned short HD100BaseT2            :1;
  unsigned short FD100BaseT2            :1;
  unsigned short HD10BaseT              :1;
  unsigned short FD10BaseT              :1;
  unsigned short HD100BaseXT            :1;
  unsigned short FD100BaseXT            :1;
  unsigned short FD100BaseT4            :1;
};

//Auto-Neg. Link Partner Abitily register
#define PHY_LPA_SLCT                0x001f  /* Same as advertise selector  */
#define PHY_LPA_10HALF              (1<<5)  /* Can do 10mbps half-duplex   */
#define PHY_LPA_10FULL              (1<<6)  /* Can do 10mbps full-duplex   */
#define PHY_LPA_100HALF             (1<<7)  /* Can do 100mbps half-duplex  */
#define PHY_LPA_100FULL             (1<<8)  /* Can do 100mbps full-duplex  */
#define PHY_LPA_100BASE4            (1<<9)  /* Can do 100mbps 4k packets   */
#define PHY_LPA_RESV                0x1c00  /* Unused...                   */
#define PHY_LPA_RFAULT              0x2000  /* Link partner faulted        */
#define PHY_LPA_LPACK               0x4000  /* Link partner acked us       */
#define PHY_LPA_NPAGE               0x8000  /* Next page bit               */

#define PHY_LPA_DUPLEX		(LPA_10FULL | LPA_100FULL)
#define PHY_LPA_100		(LPA_100FULL | LPA_100HALF | LPA_100BASE4)


//NWAY Setup Register
#define PHY_NSR_FLAGLSC     (1<<0)      //1=autoneg. experienced link status check state


#endif //RTL8201_H
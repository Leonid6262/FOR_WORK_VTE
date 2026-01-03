#pragma once

#include "LPC407x_8x_177x_8x.h"
#include <cstring>       
#include "pause_us.hpp"

// Registers Bit definitions according to LPC (Emac) datasheet 
// MAC Configuration Register 1 
#define MAC1_RXENABLE       (1 << 0)      // RECEIVE ENABLE. Set this to allow receive frames to be received. Internally the MAC synchronizes this control bit to the incoming receive stream 
#define MAC1_PASS_ALL       0x00000002  /* Pass All Receive Frames           */
#define MAC1_RX_FLOWC       0x00000004  /* RX Flow Control                   */
#define MAC1_TX_FLOWC       0x00000008  /* TX Flow Control                   */
#define MAC1_LOOPB          0x00000010  /* Loop Back Mode                    */
#define MAC1_RES_TX         0x00000100  /* Reset TX Logic                    */
#define MAC1_RES_MCS_TX     0x00000200  /* Reset MAC TX Control Sublayer     */
#define MAC1_RES_RX         0x00000400  /* Reset RX Logic                    */
#define MAC1_RES_MCS_RX     0x00000800  /* Reset MAC RX Control Sublayer     */
#define MAC1_SIM_RES        0x00004000  /* Simulation Reset                  */
#define MAC1_SOFT_RES       0x00008000  /* Soft Reset MAC                    */

// MAC Configuration Register 2 
#define MAC2_FULL_DUP       0x00000001  /* Full Duplex Mode                  */
#define MAC2_FRM_LEN_CHK    0x00000002  /* Frame Length Checking             */
#define MAC2_HUGE_FRM_EN    0x00000004  /* Huge Frame Enable                 */
#define MAC2_DLY_CRC        0x00000008  /* Delayed CRC Mode                  */
#define MAC2_CRC_EN         0x00000010  /* Append CRC to every Frame         */
#define MAC2_PAD_EN         0x00000020  /* Pad all Short Frames              */
#define MAC2_VLAN_PAD_EN    0x00000040  /* VLAN Pad Enable                   */
#define MAC2_ADET_PAD_EN    0x00000080  /* Auto Detect Pad Enable            */
#define MAC2_PPREAM_ENF     0x00000100  /* Pure Preamble Enforcement         */
#define MAC2_LPREAM_ENF     0x00000200  /* Long Preamble Enforcement         */
#define MAC2_NO_BACKOFF     0x00001000  /* No Backoff Algorithm              */
#define MAC2_BACK_PRESSURE  0x00002000  /* Backoff Presurre / No Backoff     */
#define MAC2_EXCESS_DEF     0x00004000  /* Excess Defer                      */

// Command Register 
#define CR_RXENABLE         (1 << 0)    // Enable Receive                    
#define CR_TXENABLE         (1 << 1)    // Enable Transmit                   
#define CR_REGRESET         (1 << 3)    // Reset Host Registers. When a 1 is written, all datapaths and the host registers are reset. The MAC needs to be reset separately.              
#define CR_TXRESET          (1 << 4)    // Reset Transmit Datapath. When a 1 is written, the transmit datapath is reset.         
#define CR_RXRESET          (1 << 5)    // Reset Receive Datapath. When a 1 is written, the receive datapath is reset.
#define CR_PASSRUNTFRAME    (1 << 6)    // Pass Runt Frames. When set to 1 , passes runt frames s1maller than 64 bytes to memory unless they have a CRC error. If 0 runt frames are filtered out.
#define CR_PASSRXFILTER     (1 << 7)    // Pass RX Filter. When set to 1 , disables receive filtering i.e. all frames received are written to memory.
#define CR_TXFLOWCONTROL    (1 << 8)    // TX Flow Control. Enable IEEE 802.3 / clause 31 flow control sending pause frames in full duplex and continuous preamble in half duplex.
#define CR_RMII             (1 << 9)    // Reduced MII Interface. When set to 1 , RMII mode is selected; if 0, MII mode is selected.
#define CR_FULLDUPLEX       (1 << 10)   // Full Duplex. When set to 1 , indicates full duplex operation.

/* Receive Filter Control Register */
#define RFC_UCAST_EN        0x00000001  /* Accept Unicast Frames Enable      */
#define RFC_BCAST_EN        0x00000002  /* Accept Broadcast Frames Enable    */
#define RFC_MCAST_EN        0x00000004  /* Accept Multicast Frames Enable    */
#define RFC_UCAST_HASH_EN   0x00000008  /* Accept Unicast Hash Filter Frames */
#define RFC_MCAST_HASH_EN   0x00000010  /* Accept Multicast Hash Filter Fram.*/
#define RFC_PERFECT_EN      0x00000020  /* Accept Perfect Match Enable       */
#define RFC_MAGP_WOL_EN     0x00001000  /* Magic Packet Filter WoL Enable    */
#define RFC_PFILT_WOL_EN    0x00002000  /* Perfect Filter WoL Enable         */

/* Non Back-to-Back Inter-Packet-Gap Register */
#define IPGR_DEF            0x00000012  /* Recommended value                 */

/* Back-to-Back Inter-Packet-Gap Register */
#define IPGT_FULL_DUP       0x00000015  /* Recommended value for Full Duplex */
#define IPGT_HALF_DUP       0x00000012  /* Recommended value for Half Duplex */

/* Collision Window/Retry Register */
#define CLRT_DEF            0x0000370F  /* Default value                     */

/* MII Management Configuration Register */
#define MCFG_SCAN_INC       0x00000001  /* Scan Increment PHY Address        */
#define MCFG_SUPP_PREAM     0x00000002  /* Suppress Preamble                 */
#define MCFG_CLK_SEL        0x0000003C  /* Clock Select Mask                 */
#define MCFG_RES_MII        0x00008000  /* Reset MII Management Hardware     */

#define MCFG_CLK_DIV28      0x0000001C  /* MDC = hclk / 28 */
#define MCFG_CLK_DIV36      0x00000020
#define MCFG_CLK_DIV48      0x0000002C    
#define MCFG_CLK_DIV64      0x0000003C

#define LAN8720_ID          0x0007C0F0 
#define PHY_ADDR            0x00000001  // может быть 0x00 в зависимости от конфигурации

#define SUPP_SPEED          (1 << 8) 


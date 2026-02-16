/**
 * @author Matt Ricci
 * @addtogroup API
 * @{
 * @addtogroup CAN
 * @}
 */

// ALLOW FORMATTING
#ifndef CAN_H
#define CAN_H

#include "stm32f439xx.h"
#include "stdbool.h"
#include <stdint.h>

#define CAN_STID_MAX        2048

#define CAN_DATA_INDEX_LOW  0
#define CAN_DATA_INDEX_HIGH 1

#define CAN_MCR_CONFIG_MASK ( \
  CAN_MCR_DBF                 \
  | CAN_MCR_TTCM              \
  | CAN_MCR_ABOM              \
  | CAN_MCR_AWUM              \
  | CAN_MCR_NART              \
  | CAN_MCR_RFLM              \
  | CAN_MCR_TXFP              \
)

#define CAN_IER_CONFIG_MASK ( \
  CAN_IER_SLKIE               \
  | CAN_IER_WKUIE             \
  | CAN_IER_ERRIE             \
  | CAN_IER_LECIE             \
  | CAN_IER_BOFIE             \
  | CAN_IER_EPVIE             \
  | CAN_IER_EWGIE             \
  | CAN_IER_FOVIE1            \
  | CAN_IER_FFIE1             \
  | CAN_IER_FMPIE1            \
  | CAN_IER_FOVIE0            \
  | CAN_IER_FFIE0             \
  | CAN_IER_FMPIE0            \
  | CAN_IER_TMEIE             \
)

#define CAN_BTR_CONFIG_MASK ( \
  CAN_BTR_SILM                \
  | CAN_BTR_LBKM              \
  | CAN_BTR_SJW               \
  | CAN_BTR_TS2               \
  | CAN_BTR_TS1               \
  | CAN_BTR_BRP               \
)

#define CAN_FMR_CONFIG_MASK ( \
  CAN_FMR_CAN2SB              \
)

#define CAN_FM1R_CONFIG_MASK ( \
  CAN_FM1R_FBM                 \
)

#define CAN_FS1R_CONFIG_MASK ( \
  CAN_FS1R_FSC                 \
)

#define CAN_FFA1R_CONFIG_MASK ( \
  CAN_FFA1R_FFA                 \
)

#define CAN_FA1R_CONFIG_MASK ( \
  CAN_FA1R_FACT                \
)

/******
 * Automatic Bus-Off Management and Automatic Wake-Up Mode
 * CAN bit rate is 125kb/s [SJW, TS2, TS1, BRP]
 * Filtering is set to use a 32 bit mask, however filters are effectively disabled.
 * Interrupt enabled on FIFO pending.
 *****/
#define CAN_CONFIG_DEFAULT                                                                          \
  (CAN_Config) {                                                                                    \
    .DBF    = false,                                                                                \
    .TTCM   = false,                                                                                \
    .ABOM   = true,                                                                                 \
    .AWUM   = true,                                                                                 \
    .NART   = false,                                                                                \
    .RFLM   = false,                                                                                \
    .TXFP   = false,                                                                                \
    .SLKIE  = false,                                                                                \
    .WKUIE  = false,                                                                                \
    .ERRIE  = false,                                                                                \
    .LECIE  = false,                                                                                \
    .BOFIE  = false,                                                                                \
    .EPVIE  = false,                                                                                \
    .EWGIE  = false,                                                                                \
    .FOVIE1 = false,                                                                                \
    .FFIE1  = false,                                                                                \
    .FMPIE1 = false,                                                                                \
    .FOVIE0 = false,                                                                                \
    .FFIE0  = false,                                                                                \
    .FMPIE0 = true,                                                                                 \
    .TMEIE  = false,                                                                                \
    .SJW    = 0x00,                                                                                 \
    .TS2    = 0x03,                                                                                 \
    .TS1    = 0x0F,                                                                                 \
    .BRP    = 0x000F,                                                                               \
    .CAN2SB = 28,                                                                                   \
    .FBM    = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
    .FSC    = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
    .FFA    = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
    .FACT   = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
    .FIR    = {                                                                                     \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0},                                          \
      (CAN_FilterRegister_TypeDef){.FR1 = 0, .FR2 = 0}                                           \
    }                                                                                               \
  }

typedef struct {
  // CAN MCR (Master Control Register)
  bool DBF;  //!< Freeze RX/TX during debug.
  bool TTCM; //!< Time Triggered Communication
  bool ABOM; //!< Automatic Bus-Off Management
  bool AWUM; //!< Automatic Wake-Up Mode
  bool NART; //!< No Automatic Retransmission
  bool RFLM; //!< Lock FIFO Against Overrun
  bool TXFP; //!< Transmit FIFO Priority

  // CAN IER (Interrupt Enable Register)
  bool SLKIE;  //!< Interrupt on Sleep
  bool WKUIE;  //!< Interrupt on Wake-up
  bool ERRIE;  //!< Interrupt on Error Pending in CAN_ESR
  bool LECIE;  //!< Interrupt on Error Code (LEC[2:0] Set)
  bool BOFIE;  //!< Interrupt on Bus-off (BOFF Set)
  bool EPVIE;  //!< Interrupt on Error Passive (EPVF Set)
  bool EWGIE;  //!< Interrupt on Error Warning (EWGF Set)
  bool FOVIE1; //!< Interrupt on FIFO 1 Overrun
  bool FFIE1;  //!< Interrupt on FIFO 1 Full
  bool FMPIE1; //!< Interrupt on FIFO 1 Pending
  bool FOVIE0; //!< Interrupt on FIFO 0 Overrun
  bool FFIE0;  //!< Interrupt on FIFO 0 Full
  bool FMPIE0; //!< Interrupt on FIFO 0 Pending
  bool TMEIE;  //!< Interrupt on Transmit Mailbox Empty

  // CAN BTR (Bit Timing Register)
  bool SILM;    //!<  Silent Mode
  bool LBKM;    //!<  Loopback Mode
  uint8_t SJW;  //!<  Resync Jump Width
  uint8_t TS2;  //!<  Time Segment 2
  uint8_t TS1;  //!<  Time Segment 1
  uint16_t BRP; //!<  Baud Rate Prescaler

  // CAN FMR (Filter Master Register)
  uint8_t CAN2SB; //!< Define the start bank for CAN2 Interface

  // CAN FM1R (Filter Mode Register)
  bool FBM[28];

  // CAN FS1R (Filter Scale Register)
  bool FSC[28];

  // CAN FFA1R (Filter FIFO Assignment Register)
  bool FFA[28];

  // CAN FA1R (Filter Activation Register)
  bool FACT[28];

  // CAN FiRx
  CAN_FilterRegister_TypeDef FIR[28];

} CAN_Config;


typedef struct {
  uint8_t word[2];
  uint8_t length;
} CAN_Data;


typedef struct {
  uint32_t id;
  CAN_Data data;
} CAN_Packet;





typedef enum {
  Return_CAN_transmit__Success,
  Return_CAN_transmit__Mailbox_Full,
  Return_CAN_transmit__TX_Error,
  Return_CAN_transmit__Timeout
} Return_CAN_transmit_t;

typedef enum {
  Return_CAN_receive__Message_Received,
  Return_CAN_receive__Message_Pending_None
} Return_CAN_receive_t;


// TODO: Populate and complete documentation comments:

typedef struct CAN {
  CAN_TypeDef *interface;                                                 //!<
  CAN_Config config;                                                      //!<
  Return_CAN_transmit_t (*transmit)(struct CAN *can, CAN_Packet *txData); //!<
  Return_CAN_receive_t (*receive)(struct CAN *can, CAN_Packet *rxData);   //!<
  void (*updateConfig)(struct CAN *can, CAN_Config *config);              //!<
} CAN_t;


Return_CAN_transmit_t
CAN_transmit(CAN_t *can, CAN_Packet *txData);

Return_CAN_receive_t
CAN_receive(CAN_t *can, CAN_Packet *rxData);


CAN_t CAN_init(CAN_TypeDef *interface, CAN_Config *config);

void CAN_updateConfig(CAN_t *can, CAN_Config *config);

#endif

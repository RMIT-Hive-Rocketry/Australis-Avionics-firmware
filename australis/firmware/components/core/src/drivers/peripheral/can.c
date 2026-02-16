/***********************************************************************************
 * @file        can.c                                                              *
 * @author      Matt Ricci                                                         *
 * @brief       Brief description of the file's purpose.                           *
 *                                                                                 *
 * @todo Cleanup CAN interface header and implementation.                          *
 ***********************************************************************************/

#include "can.h"

#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"

#include "gpiopin.h"
#include "stm32f439xx.h"

static void _CAN_init(CAN_TypeDef *, CAN_Config *);

/* =============================================================================== */
/**
 * @brief  Initialiser for a CAN peripheral bus.
 *
 * @param  interface Pointer to the CAN_TypeDef struct representing the CAN interface.
 * @param  config    Pointer to CAN_Config struct for initial configuration.
 *                   This may be passed as \c NULL to initialise a default
 *                   configuration.
 *
 * @return can       Initialised CAN_t struct.
 **
 * =============================================================================== */
CAN_t CAN_init(CAN_TypeDef *interface, CAN_Config *config) {
  // Early return error struct if peripheral is NULL
  if (interface == NULL) {
    return (CAN_t){.interface = NULL};
  }

  // Initialise CAN struct with interface
  CAN_t can = {.interface = interface};

  // Update config and enable peripheral
  CAN_updateConfig(&can, config);

  // Initialise remaining parameters and methods
  can.transmit     = CAN_transmit;
  can.receive      = CAN_receive;
  can.updateConfig = CAN_updateConfig;

  return can;
}

// ALLOW FORMATTING
#ifndef __DOXYGEN__

/* =============================================================================== */
/**
 * @brief   Private initialiser for CAN registers.
 *
 * @param   interface Pointer to the CAN_TypeDef struct representing the CAN interface.
 * @param   config    Pointer to CAN_Config struct for initial configuration.
 *                    This may be passed as \c NULL to initialise a default
 *                    configuration. @see CAN_Config
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
static void _CAN_init(CAN_TypeDef *interface, CAN_Config *config) {

  // TODO: assert() is typically a compile-time macro, but all asserts here
  //  should be capturing erroneous data at runtime.

  // Reset CAN device, then wait for hardware to clear RESET bit.
  interface->MCR |= CAN_MCR_RESET;
  while (interface->MCR & CAN_MCR_RESET);

  // Exit sleep mode (caused by RESET) and
  // wait for non-sleep mode acknowledgement.
  interface->MCR &= ~CAN_MCR_SLEEP;
  while (interface->MSR & CAN_MSR_SLAK);

  // Initialization Begin
  //
  // Enter initialization mode and
  // wait for init mode acknowledgement.
  interface->MCR |= CAN_MCR_INRQ;
  while (!(interface->MSR & CAN_MSR_INAK));

  interface->MCR &= ~CAN_MCR_CONFIG_MASK;
  interface->MCR |=
    (config->DBF << CAN_MCR_DBF_Pos)
    | (config->TTCM << CAN_MCR_TTCM_Pos)
    | (config->ABOM << CAN_MCR_ABOM_Pos)
    | (config->AWUM << CAN_MCR_AWUM_Pos)
    | (config->NART << CAN_MCR_NART_Pos)
    | (config->RFLM << CAN_MCR_RFLM_Pos)
    | (config->TXFP << CAN_MCR_TXFP_Pos);

  interface->IER &= ~CAN_IER_CONFIG_MASK;
  interface->IER |=
    (config->SLKIE << CAN_IER_SLKIE_Pos)
    | (config->WKUIE << CAN_IER_WKUIE_Pos)
    | (config->ERRIE << CAN_IER_ERRIE_Pos)
    | (config->LECIE << CAN_IER_LECIE_Pos)
    | (config->BOFIE << CAN_IER_BOFIE_Pos)
    | (config->EPVIE << CAN_IER_EPVIE_Pos)
    | (config->EWGIE << CAN_IER_EWGIE_Pos)
    | (config->FOVIE1 << CAN_IER_FOVIE1_Pos)
    | (config->FFIE1 << CAN_IER_FFIE1_Pos)
    | (config->FMPIE1 << CAN_IER_FMPIE1_Pos)
    | (config->FOVIE0 << CAN_IER_FOVIE0_Pos)
    | (config->FFIE0 << CAN_IER_FFIE0_Pos)
    | (config->FMPIE0 << CAN_IER_FMPIE0_Pos)
    | (config->TMEIE << CAN_IER_TMEIE_Pos);

  // NOTE: Temporarily disabled (pending ASSERT from private build)
  //
  // ASSERT(config->SJW < (1U<<2));
  // ASSERT(config->TS2 < (1U<<3));
  // ASSERT(config->TS1 < (1U<<4));
  // ASSERT(config->BRP < (1U<<10));

  interface->BTR &= ~CAN_BTR_CONFIG_MASK;
  interface->BTR |=
    (config->SILM << CAN_BTR_SILM_Pos)
    | (config->LBKM << CAN_BTR_LBKM_Pos)
    | (config->SJW << CAN_BTR_SJW_Pos)
    | (config->TS2 << CAN_BTR_TS2_Pos)
    | (config->TS1 << CAN_BTR_TS1_Pos)
    | (config->BRP << CAN_BTR_BRP_Pos);

  // Filter Initialization Begin
  interface->FMR |= CAN_FMR_FINIT;

  // NOTE: Temporarily disabled (pending ASSERT from private build)
  //
  // ASSERT(config->CAN2SB < (1U<<6));

  interface->FMR &= ~CAN_FMR_CONFIG_MASK;
  interface->FMR |=
    (config->CAN2SB << CAN_FMR_CAN2SB_Pos);

  interface->FM1R  &= ~CAN_FM1R_CONFIG_MASK;
  interface->FS1R  &= ~CAN_FS1R_CONFIG_MASK;
  interface->FFA1R &= ~CAN_FFA1R_CONFIG_MASK;
  interface->FA1R  &= ~CAN_FA1R_CONFIG_MASK;

  for (int i = 0; i < 28; i++) {
    interface->FM1R               |= config->FBM[i] << i;
    interface->FS1R               |= config->FSC[i] << i;
    interface->FFA1R              |= config->FFA[i] << i;
    interface->FA1R               |= config->FACT[i] << i;
    interface->sFilterRegister[i]  = config->FIR[i];
  }

  interface->FMR &= ~CAN_FMR_FINIT;
  // Filter Initialization Complete

  interface->MCR &= ~CAN_MCR_INRQ;
  while (interface->MSR & CAN_MSR_INAK);
  // Initialization Complete
}

#endif

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 *
 **
 * =============================================================================== */
Return_CAN_receive_t
CAN_receive(CAN_t *can, CAN_Packet *rxData) {

  volatile uint32_t *fifo = (can->interface == CAN1) ? &CAN1->RF0R : &CAN2->RF1R;
  uint8_t nFifo           = (can->interface == CAN1) ? 0 : 1;

  // If there are messages pending, then receive it.
  if (*fifo & CAN_RF0R_FMP0) {
    // Read frame identifier and received data
    rxData->id =
      (can->interface->sFIFOMailBox[nFifo].RIR & CAN_RI0R_IDE)
        ? (can->interface->sFIFOMailBox[nFifo].RIR & CAN_RI0R_EXID) >> CAN_RI0R_EXID_Pos
        : (can->interface->sFIFOMailBox[nFifo].RIR & CAN_RI0R_STID) >> CAN_RI0R_STID_Pos;

    // Read data out
    rxData->data.word[CAN_DATA_INDEX_LOW]  = can->interface->sFIFOMailBox[nFifo].RDLR;
    rxData->data.word[CAN_DATA_INDEX_HIGH] = can->interface->sFIFOMailBox[nFifo].RDHR;

    // Update FIFO register
    *fifo |= CAN_RF0R_RFOM0;  // Release FIFO
    *fifo &= ~CAN_RF0R_FOVR0; // Clear overrun flag
    *fifo &= ~CAN_RF0R_FULL0; // Clear FIFO full flag
    return Return_CAN_receive__Message_Received;
  }

  return Return_CAN_receive__Message_Pending_None; // No frame to receive
}

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 **
 * =============================================================================== */
Return_CAN_transmit_t
CAN_transmit(CAN_t *can, CAN_Packet *txData) {

  // Exit if no mailboxes are free
  bool mailboxFree = (can->interface->TSR & CAN_TSR_TME) ? true : false;
  if (!mailboxFree) {
    return Return_CAN_transmit__Mailbox_Full;
  }

  // Retrieve index of the next free mailbox.
  uint8_t mailbox = (can->interface->TSR & CAN_TSR_CODE_Msk) >> CAN_TSR_CODE_Pos;

  // Set frame data
  can->interface->sTxMailBox[mailbox].TDHR = txData->data.word[CAN_DATA_INDEX_HIGH];
  can->interface->sTxMailBox[mailbox].TDLR = txData->data.word[CAN_DATA_INDEX_LOW];
  can->interface->sTxMailBox[mailbox].TDTR = txData->data.length;

  // Set frame identifier
  if (txData->id < CAN_STID_MAX) {
    // Use STID if within range of standard identifier length
    can->interface->sTxMailBox[mailbox].TIR = txData->id << CAN_TI0R_STID_Pos;
  } else {
    // Otherwise, set as EXID
    can->interface->sTxMailBox[mailbox].TIR  = txData->id << CAN_TI0R_EXID_Pos;
    can->interface->sTxMailBox[mailbox].TIR |= CAN_TI0R_IDE;
  }

  // Select the corresponding TXOK, TERR, ABRQ registers for the mailbox.
  uint32_t CAN_TSR_TXOKx, CAN_TSR_TERRx, CAN_TSR_ABRQx;
  switch (mailbox) {
    case 0:
      CAN_TSR_TXOKx = CAN_TSR_TXOK0;
      CAN_TSR_TERRx = CAN_TSR_TERR0;
      CAN_TSR_ABRQx = CAN_TSR_ABRQ0;
      break;
    case 1:
      CAN_TSR_TXOKx = CAN_TSR_TXOK1;
      CAN_TSR_TERRx = CAN_TSR_TERR1;
      CAN_TSR_ABRQx = CAN_TSR_ABRQ1;
      break;
    case 2:
      CAN_TSR_TXOKx = CAN_TSR_TXOK2;
      CAN_TSR_TERRx = CAN_TSR_TERR2;
      CAN_TSR_ABRQx = CAN_TSR_ABRQ2;
      break;
  }

  // Request transmission
  can->interface->sTxMailBox[mailbox].TIR |= CAN_TI0R_TXRQ;

  // TODO: Add timer for timeout detection
  while (1) {
    if (can->interface->TSR & CAN_TSR_TXOKx) {
      return Return_CAN_transmit__Success;
    } else if (can->interface->TSR & CAN_TSR_TERRx) {
      can->interface->TSR |= CAN_TSR_ABRQx;
      return Return_CAN_transmit__TX_Error;
    }
  }
  return Return_CAN_transmit__Timeout;
}

/* =============================================================================== */
/**
 * @brief   Update CAN peripheral configuration
 * @details Uses the provided configuration to update the CAN registers and resets the
 *          associated peripheral.
 *          As with initialisation, passing \c NULL will set the default config.
 *
 * @param   can Pointer to CAN_t struct.
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
void CAN_updateConfig(CAN_t *can, CAN_Config *config) {
  // Initialise config with default values if passed NULL.
  if (config == NULL) {
    config = &CAN_CONFIG_DEFAULT;
  }

  // TODO: perform config parameter checks/assertions
  // in this function, prior to any actual update.

  // Update peripheral with new config
  can->config = *config;

  // Initialise CAN registers and enable peripheral
  _CAN_init(can->interface, config);
}


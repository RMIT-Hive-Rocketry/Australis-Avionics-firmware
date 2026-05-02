/***********************************************************************************
 * @file                                                                           *
 * @brief                                                                          *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "stm32f439xx.h"
#include "cmsis_gcc.h"

#include "system.h"

#include "interrupts/interrupts.h"
#include "interrupts/interrupt_can.h"

static InterruptContext CAN1_TX_IRQContext  = {.callback = Interrupt_defaultIRQHandler};
static InterruptContext CAN1_RX0_IRQContext = {.callback = Interrupt_defaultIRQHandler};
static InterruptContext CAN1_RX1_IRQContext = {.callback = Interrupt_defaultIRQHandler};
static InterruptContext CAN1_SCE_IRQContext = {.callback = Interrupt_defaultIRQHandler};

static InterruptContext CAN2_TX_IRQContext  = {.callback = Interrupt_defaultIRQHandler};
static InterruptContext CAN2_RX0_IRQContext = {.callback = Interrupt_defaultIRQHandler};
static InterruptContext CAN2_RX1_IRQContext = {.callback = Interrupt_defaultIRQHandler};
static InterruptContext CAN2_SCE_IRQContext = {.callback = Interrupt_defaultIRQHandler};

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return @c NULL.
 **
 * =============================================================================== */
void CAN_RegisterIRQ(InterruptHandle handle) {

  ASSERT(handle.type == INTERRUPT_CAN);

  // clang-format off
  ASSERT(
     handle.identifier >= CAN1_TX_IRQn && handle.identifier <= CAN1_SCE_IRQn
  || handle.identifier >= CAN2_TX_IRQn && handle.identifier <= CAN2_SCE_IRQn
  );
  // clang-format on

  /* REGISTER INTERRUPT */

  // Assign context from interrupt handle
  // to associated context struct
  switch (handle.identifier) {
    /* CAN1 INTERRUPTS */
    case CAN1_TX_IRQn:
      CAN1_TX_IRQContext = handle.context;
      break;
    case CAN1_RX0_IRQn:
      CAN1_RX0_IRQContext = handle.context;
      break;
    case CAN1_RX1_IRQn:
      CAN1_RX1_IRQContext = handle.context;
      break;
    case CAN1_SCE_IRQn:
      CAN1_SCE_IRQContext = handle.context;
      break;
    /* CAN2 INTERRUPTS */
    case CAN2_TX_IRQn:
      CAN2_TX_IRQContext = handle.context;
      break;
    case CAN2_RX0_IRQn:
      CAN2_RX0_IRQContext = handle.context;
      break;
    case CAN2_RX1_IRQn:
      CAN2_RX1_IRQContext = handle.context;
      break;
    case CAN2_SCE_IRQn:
      CAN2_SCE_IRQContext = handle.context;
      break;
    default:
      break;
  }

  /* ENABLE INTERRUPT */

  __disable_irq();
  NVIC_SetPriority(handle.identifier, handle.priority);
  NVIC_EnableIRQ(handle.identifier);
  __enable_irq();
}

/* CAN1 */
void CAN1_TX_IRQHandler(void) { CAN1_TX_IRQContext.callback(CAN1_TX_IRQContext.context); }
// Compiling AV2-dual: multiple definition of `CAN1_RX0_IRQHandler';
// void CAN1_RX0_IRQHandler(void) { CAN1_RX0_IRQContext.callback(CAN1_RX0_IRQContext.context); }
void CAN1_RX1_IRQHandler(void) { CAN1_RX1_IRQContext.callback(CAN1_RX1_IRQContext.context); }
void CAN1_SCE_IRQHandler(void) { CAN1_SCE_IRQContext.callback(CAN1_SCE_IRQContext.context); }

/* CAN2 */
void CAN2_TX_IRQHandler(void) { CAN2_TX_IRQContext.callback(&CAN2_TX_IRQContext.context); }
void CAN2_RX0_IRQHandler(void) { CAN2_RX0_IRQContext.callback(&CAN2_RX0_IRQContext.context); }
void CAN2_RX1_IRQHandler(void) { CAN2_RX1_IRQContext.callback(&CAN2_RX1_IRQContext.context); }
void CAN2_SCE_IRQHandler(void) { CAN2_SCE_IRQContext.callback(&CAN2_SCE_IRQContext.context); }

/** @} */

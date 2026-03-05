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
#include "interrupts/interrupt_exti.h"

static InterruptContext EXTI1_IRQContext     = {.callback = Interrupt_defaultIRQHandler};
static InterruptContext EXTI2_IRQContext     = {.callback = Interrupt_defaultIRQHandler};
static InterruptContext EXTI3_IRQContext     = {.callback = Interrupt_defaultIRQHandler};
static InterruptContext EXTI4_IRQContext     = {.callback = Interrupt_defaultIRQHandler};
static InterruptContext EXTI9_5_IRQContext   = {.callback = Interrupt_defaultIRQHandler};
static InterruptContext EXTI15_10_IRQContext = {.callback = Interrupt_defaultIRQHandler};

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return @c NULL.
 **
 * =============================================================================== */
void EXTI_RegisterIRQ(InterruptHandle handle) {

  ASSERT(handle.type == INTERRUPT_EXTI);

  // clang-format off
  ASSERT(
     handle.identifier == EXTI1_IRQn
  || handle.identifier == EXTI2_IRQn
  || handle.identifier == EXTI3_IRQn
  || handle.identifier == EXTI4_IRQn
  || handle.identifier == EXTI9_5_IRQn
  || handle.identifier == EXTI15_10_IRQn
  );
  // clang-format on

  /* REGISTER INTERRUPT */

  // Assign context from interrupt handle
  // to associated context struct
  switch (handle.identifier) {
    case EXTI1_IRQn:
      EXTI1_IRQContext = handle.context;
      break;
    case EXTI2_IRQn:
      EXTI2_IRQContext = handle.context;
      break;
    case EXTI3_IRQn:
      EXTI3_IRQContext = handle.context;
      break;
    case EXTI4_IRQn:
      EXTI4_IRQContext = handle.context;
      break;
    case EXTI9_5_IRQn:
      EXTI9_5_IRQContext = handle.context;
      break;
    case EXTI15_10_IRQn:
      EXTI15_10_IRQContext = handle.context;
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

// Compiling AV2-dual: multiple definition of `EXTI1_IRQHandler';
// void EXTI1_IRQHandler(void) { EXTI1_IRQContext.callback(EXTI1_IRQContext.context); }
void EXTI2_IRQHandler(void) { EXTI2_IRQContext.callback(EXTI2_IRQContext.context); }
void EXTI3_IRQHandler(void) { EXTI3_IRQContext.callback(EXTI3_IRQContext.context); }
void EXTI4_IRQHandler(void) { EXTI4_IRQContext.callback(EXTI4_IRQContext.context); }
void EXTI9_5_IRQHandler(void) { EXTI9_5_IRQContext.callback(EXTI9_5_IRQContext.context); }
void EXTI15_10_IRQHandler(void) { EXTI15_10_IRQContext.callback(EXTI15_10_IRQContext.context); }

/** @} */

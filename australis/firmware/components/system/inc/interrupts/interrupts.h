/**
 * @author Matt Ricci
 * @addtogroup API API Reference
 * @{
 * @addtogroup
 * @}
 */

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "stdint.h"

#ifdef STM32F439xx
#include "stm32f439xx.h"
#define InterruptIdentifier IRQn_Type
#endif

/**
 * @brief
 * @details
 */
typedef void (*InterruptCallback)(void *context);

/**
 * @brief
 * @details
 */
typedef struct {
  InterruptCallback callback;
  void *context;
} InterruptContext;

/**
 * @brief
 * @details
 */
typedef enum {
  INTERRUPT_EXTI,
  INTERRUPT_SPI,
  INTERRUPT_CAN,
  INTERRUPT_INVALID
} InterruptType;

/**
 * @brief
 * @details
 */
typedef struct {
  //
  InterruptIdentifier identifier;
  uint32_t priority;

  //
  InterruptType type;
  InterruptContext context;
} InterruptHandle;

void Interrupt_registerHandle(InterruptHandle handle);
void Interrupt_defaultIRQHandler(void *context);

/** @} */
#endif

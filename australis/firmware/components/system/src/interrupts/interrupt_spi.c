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
#include "interrupts/interrupt_spi.h"
#include "_interrupts/_interrupt_spi.h"

InterruptContext SPI1_IRQContext = {.callback = Interrupt_defaultIRQHandler};
InterruptContext SPI2_IRQContext = {.callback = Interrupt_defaultIRQHandler};
InterruptContext SPI3_IRQContext = {.callback = Interrupt_defaultIRQHandler};

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return @c NULL.
 **
 * =============================================================================== */
void SPI_RegisterIRQ(InterruptHandle handle) {

  ASSERT(handle.type == INTERRUPT_SPI);

  // clang-format off
  ASSERT(
     handle.identifier == SPI1_IRQn
  || handle.identifier == SPI2_IRQn
  || handle.identifier == SPI3_IRQn
  || handle.identifier == SPI4_IRQn
  || handle.identifier == SPI5_IRQn
  || handle.identifier == SPI6_IRQn
  );
  // clang-format on

  /* REGISTER INTERRUPT */

  // Assign context from interrupt handle
  // to associated context struct
  switch (handle.identifier) {
    case SPI1_IRQn:
      SPI1_IRQContext = handle.context;
      break;
    case SPI2_IRQn:
      SPI2_IRQContext = handle.context;
      break;
    case SPI3_IRQn:
      SPI3_IRQContext = handle.context;
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

void SPI1_IRQHandler(void) { SPI1_IRQContext.callback(SPI1_IRQContext.context); }
void SPI2_IRQHandler(void) { SPI2_IRQContext.callback(SPI2_IRQContext.context); }
void SPI3_IRQHandler(void) { SPI3_IRQContext.callback(SPI3_IRQContext.context); }

/** @} */

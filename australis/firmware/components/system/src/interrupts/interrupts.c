/***********************************************************************************
 * @file                                                                           *
 * @brief                                                                          *
 *                                                                                 *
 * @{                                                                              *
 ***********************************************************************************/

#include "interrupts/interrupts.h"

// TODO:
// Later these should be provided by a portable layer
// to allow more flexible hardware abstraction.
#include "interrupts/interrupt_exti.h"
#include "interrupts/interrupt_spi.h"
#include "interrupts/interrupt_can.h"

void Interrupt_registerHandle(InterruptHandle handle) {

  switch (handle.type) {
    case INTERRUPT_EXTI:
      EXTI_RegisterIRQ(handle);
      break;
    case INTERRUPT_SPI:
      SPI_RegisterIRQ(handle);
      break;
    case INTERRUPT_CAN:
      CAN_RegisterIRQ(handle);
      break;
    default:
      break;
  }
}

void Interrupt_defaultIRQHandler(void *context) {
  (void)context;
}

/** @} */

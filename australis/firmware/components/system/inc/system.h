#ifndef SYSTEM_H
#define SYSTEM_H

#include "stm32f439xx.h"
#include "cmsis_gcc.h"

#ifndef ASSERT

/**
 * @brief
 *
 */
#define ASSERT(cond)               \
  if (!(cond))                     \
    do {                           \
      _assert(__FILE__, __LINE__); \
  } while (0)

/**
 * @brief
 *
 */
static inline void _assert(const char *filename, int line) {

  if (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) {
    __ASM("bkpt");
  }

  while (1);

  // TODO: Debug print here
}

#endif // ASSERT

#endif // SYSTEM_H

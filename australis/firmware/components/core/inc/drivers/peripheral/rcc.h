
/**
 * @author Matt Ricci
 * @addtogroup API API Reference
 * @addtogroup RCC
 * @{
 */

// ALLOW FORMATTING
#ifndef RCC_H
#define RCC_H

// Enable clock and reset peripheral
#define RCC_START_PERIPHERAL(bus, peripheral)             \
  RCC->bus##ENR  |= (RCC_##bus##ENR_##peripheral##EN);    \
  RCC->bus##RSTR |= (RCC_##bus##RSTR_##peripheral##RST);  \
  __ASM("NOP");                                           \
  __ASM("NOP");                                           \
  RCC->bus##RSTR &= ~(RCC_##bus##RSTR_##peripheral##RST);

// Reset peripheral and registers
#define RCC_ENABLE_PERIPHERAL(bus, peripheral)        \
  RCC->bus##ENR |= (RCC_##bus##ENR_##peripheral##EN);

// Reset peripheral and registers
#define RCC_RESET_PERIPHERAL(bus, peripheral)             \
  RCC->bus##RSTR |= (RCC_##bus##RSTR_##peripheral##RST);  \
  __ASM("NOP");                                           \
  __ASM("NOP");                                           \
  RCC->bus##RSTR &= ~(RCC_##bus##RSTR_##peripheral##RST);

#endif
/** @} */

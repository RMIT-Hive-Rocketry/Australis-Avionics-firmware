/***********************************************************************************
 * @file        timer.c                                                            *
 * @author      Matt Ricci                                                         *
 * @brief                                                                          *
 ***********************************************************************************/

#include "tim.h"

#include "system_stm32f4xx.h"
#include "config.h"

#include "math.h"
#include "stddef.h"
#include "string.h"

static void _TIM_init(TIM_TypeDef *, TIM_Config *);

/* =============================================================================== */
/**
 * @brief  Initialiser for an SPI device interface.
 *
 * @param  interface Pointer to the SPI_TypeDef struct representing the SPI interface.
 * @param  config    Pointer to SPI_Config struct for initial configuration.
 *                   This may be passed as \c NULL to initialise a default
 *                   configuration.
 *
 * @return spi       Initialised SPI_t struct.
 **
 * =============================================================================== */
TIM_t TIM_init(TIM_TypeDef *interface, TIM_Config *config) {
  // Early return error struct if peripheral is NULL
  if (interface == NULL) {
    return (TIM_t){.interface = NULL};
  }

  // Initialise TIM struct with interface
  TIM_t tim           = {.interface = interface};

  tim.startCounter    = TIM_startCounter;
  tim.isRunning       = TIM_isRunning;
  tim.pollUpdate      = TIM_pollUpdate;
  tim.setTimingPeriod = TIM_setTimingPeriod;
  tim.pollCompare     = TIM_pollCompare;
  tim.setTimingPWM    = TIM_setTimingPWM;
  tim.updateConfig    = TIM_updateConfig;

  // Update config and enable peripheral
  TIM_updateConfig(&tim, config);

  return tim;
}

// ALLOW FORMATTING
#ifndef __DOXYGEN__

/* =============================================================================== */
/**
 * @brief   Private initialiser for TIM registers.
 *
 * @param   interface Pointer to the TIM_TypeDef struct representing the TIM interface.
 * @param   config    Pointer to TIM_Config struct for initial configuration.
 *                    This may be passed as \c NULL to initialise a default
 *                    configuration. @see TIM_Config
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
static void _TIM_init(TIM_TypeDef *interface, TIM_Config *config) {

  // --- Configure CR1 ---
  //
  // clang-format off
  interface->CR1 &= ~TIM_CR1_CONFIG_MASK;
  interface->CR1 |= (
      (config->ARPE << TIM_CR1_ARPE_Pos)
    | (config->DIR  << TIM_CR1_DIR_Pos)
    | (config->OPM  << TIM_CR1_OPM_Pos)
    | (config->URS  << TIM_CR1_URS_Pos)
    | (config->UDIS << TIM_CR1_UDIS_Pos)
  );
  // clang-format on

  // --- Configure CR2 ---
  interface->CR2 &= ~TIM_CR2_CONFIG_MASK;
  interface->CR2 |= (config->CCDS << TIM_CR2_CCDS_Pos);

  // --- Configure DIER ---
  //
  // clang-format off
  interface->DIER &= ~TIM_DIER_CONFIG_MASK;
  interface->DIER |= (
      ((config->CCDE[0] & 0x01) << TIM_DIER_CC1DE_Pos)
    | ((config->CCDE[1] & 0x01) << TIM_DIER_CC2DE_Pos)
    | ((config->CCDE[2] & 0x01) << TIM_DIER_CC3DE_Pos)
    | ((config->CCDE[3] & 0x01) << TIM_DIER_CC4DE_Pos)
    | (config->UDE << TIM_DIER_UDE_Pos)
    | ((config->CCIE[0] & 0x01) << TIM_DIER_CC1IE_Pos)
    | ((config->CCIE[1] & 0x01) << TIM_DIER_CC2IE_Pos)
    | ((config->CCIE[2] & 0x01) << TIM_DIER_CC3IE_Pos)
    | ((config->CCIE[3] & 0x01) << TIM_DIER_CC4IE_Pos)
    | (config->UIE << TIM_DIER_UIE_Pos)
  );
  // clang-format on

  // --- Configure CCMR 1 & 2 ---
  for (TIM_Channel i = 0; i < TIM_CHANNEL4; i++) {
    volatile uint32_t *reg = (i > TIM_CHANNEL2) ? &interface->CCMR2 : &interface->CCMR1;
    // clang-format off
    uint8_t channelConfig = (
        config->OC[i].S  << 0
      | config->OC[i].FE << 2
      | config->OC[i].PE << 3
      | config->OC[i].M  << 4
      | config->OC[i].CE << 7
    );
    // clang-format on
    *reg |= channelConfig << (i % 2);
  }
}

/* =============================================================================== */
/**
 * @brief   Private
 *
 * @param   interface
 *
 * @return  .
 **
 * =============================================================================== */
uint32_t _TIM_getBaseCLock(TIM_TypeDef *interface) {
  // Calculate relevant APBx clock for peripheral
  uint32_t sysclk = HSE_USED ? SYSCLK_HSE : SYSCLK_HSI;
  uint8_t hpre    = AHBPresc[((RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos)];

  uint8_t ppre;
  // Set prescale value according to peripheral bus
  if ((unsigned long)interface >= APB2PERIPH_BASE) {
    ppre = APBPresc[((RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos)];
  } else {
    ppre = APBPresc[((RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos)];
  }

  // Calculate peripheral bus clock from prescalers
  uint32_t pclk = sysclk / hpre / ppre;

  // TIMCLK = pclk when APB prescale = 1,
  // otherwise TIMCLK = 2 * pclk
  return (ppre > 1) ? 2 * pclk : pclk;
}

#endif

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return  .
 **
 * =============================================================================== */
void TIM_startCounter(TIM_t *tim) {
  tim->interface->EGR  = TIM_EGR_UG;
  tim->interface->SR  &= ~TIM_SR_UIF;
  tim->interface->CR1 |= TIM_CR1_CEN;
}

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return  .
 **
 * =============================================================================== */
bool TIM_isRunning(TIM_t *tim) {
  return (tim->interface->CR1 & TIM_CR1_CEN);
}

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return  .
 **
 * =============================================================================== */
bool TIM_pollUpdate(TIM_t *tim) {
  if (tim->interface->SR & TIM_SR_UIF) {
    tim->interface->SR &= ~TIM_SR_UIF;
    return true;
  }
  return false;
}

/* =============================================================================== */
/**
 * @brief
 *
 * @param   time
 * @param   micros
 *
 * @return  .
 **
 * =============================================================================== */
bool TIM_setTimingPeriod(TIM_t *tim, float period) {

  // Determine active peripheral clock frequency and period
  uint32_t pclk   = _TIM_getBaseCLock(tim->interface);
  float clkPeriod = 1.0f / pclk;

  // Eearly exit false if attempting to set
  // a period less than the base clock period
  if (period < clkPeriod) {
    return false;
  }

  uint32_t maxCount;
  // Determine maximum value that can be stored by the counter
  if (tim->interface == TIM2 || tim->interface == TIM5) {
    maxCount = 0xFFFFFFFF; // 32-bit counters
  } else {
    maxCount = 0xFFFF;     // 16-bit counters
  }

  // Maximum time period that can be achieved by
  // the counter at base frequency.
  float maxPeriod = maxCount * clkPeriod;

  // Timer prescale is proportional to the max achievable period
  // by a factor of the total desired period
  tim->interface->PSC = (uint16_t)(period / maxPeriod);

  // New clock period = fck_psc / PSC[15:0] + 1
  float timClk = (float)pclk / (tim->interface->PSC + 1);

  // ARR value is the product of calculated frequency and desired period
  float arr = period * timClk;

  // Set ARR to nearest integer value
  tim->interface->ARR = (uint32_t)(arr + 0.5f);

  return true;
}

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return  .
 **
 * =============================================================================== */
bool TIM_pollCompare(TIM_t *tim, TIM_Channel channel) {
  if (tim->interface->SR & (TIM_SR_CC1IF << channel)) {
    tim->interface->SR &= ~(TIM_SR_CC1IF << channel);
    return true;
  }
  return false;
}

/* =============================================================================== */
/**
 * @brief
 *
 * @param   time
 * @param   micros
 *
 * @return  .
 **
 * =============================================================================== */
bool TIM_setTimingPWM(TIM_t *tim, TIM_Channel channel, float duty) {
  // Early exit false if duty cycle is
  // not in valid range
  if (duty > 100 || duty < 0) {
    return false;
  }

  volatile uint32_t *channelCCR = &(tim->interface->CCR1) + (4 * channel);
  *channelCCR                   = (((float)tim->interface->ARR / 100) * duty);
  return true;
}

/* =============================================================================== */
/**
 * @brief   Update TIM peripheral configuration
 * @details Uses the provided configuration to update the TIM registers and resets the
 *          associated peripheral.
 *          As with initialisation, passing \c NULL will set the default config.
 *
 * @param   spi Pointer to TIM_t struct.
 *
 * @return  @c NULL.
 **
 * =============================================================================== */
void TIM_updateConfig(TIM_t *tim, TIM_Config *config) {
  // Initialise config with default values if passed NULL.
  if (config == NULL) {
    config = &TIM_CONFIG_DEFAULT;
  }

  // Update peripheral with new config
  tim->config = *config;

  // Initialise TIM registers and enable peripheral
  _TIM_init(tim->interface, config);
}
